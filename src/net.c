// net.c

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#include "net.h"

#define MAX_QUEUED_CLIENTS 16

Client clients[MAX_CLIENT] = {};
u32 client_count = 0;

struct sockaddr_in address = {};
socklen_t address_length = 0;
connect_cb on_handle_new = NULL;
connect_cb on_handle_connection = NULL;
fd_set current_sockets;
fd_set ready_sockets;
sigset_t mask;
sigset_t original_mask;
volatile u8 net_running = 0;

static Client* client_find_by_fd(i32 fd);
static i32 client_remove(Client* client);

// Slow linear search, but oh well
Client* client_find_by_fd(i32 fd) {
  for (i32 i = 0; i < client_count; ++i) {
    Client* client = &clients[i];
    if (client->fd == fd) {
      return client;
    }
  }

  return NULL;
}

i32 client_remove(Client* client) {
  *client = clients[--client_count];
  printf("Remove client: %i\n", client->fd);
  return NoError;
}

void net_init() {
  client_count = 0;
  net_running = 1;
}

void net_print_info(FILE* fp) {
  fprintf(fp, "Clients connected: %i/%i\n", client_count, MAX_CLIENT);
}

// Initialize fd set
void net_host_init(i32 fd) {
  FD_ZERO(&current_sockets);
  FD_SET(fd, &current_sockets);
  i32 flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | SOCK_NONBLOCK);
}

i32 net_connect(i32* fd, const char* host_address, i32 port) {
  struct sockaddr_in server_address = {};
  i32 sock_fd = 0;

  if ((sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    errprintf("Failed to create client socket\n");
    return Error;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, host_address, &server_address.sin_addr) <= 0) {
    errprintf("Invalid address\n");
    return Error;
  }

  for (;;) {
    i32 connect_status = connect(sock_fd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connect_status < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }
    }
    else {
      break;
    }
  }

  *fd = sock_fd;
  return NoError;
}

i32 net_create_host(i32* fd, i32 port) {
  i32 options = 0;
  address_length = sizeof(address);

  i32 host_fd = 0;

  if ((host_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    errprintf("Failed to create host socket\n");
    return Error;
  }
  if (setsockopt(host_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options)) != 0) {
    errprintf("Failed to set socket options\n");
    return Error;
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(host_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    errprintf("Failed to bind socket\n");
    return Error;
  }

  if (listen(host_fd, MAX_QUEUED_CLIENTS) < 0) {
    errprintf("Failed to setup server listener");
    return Error;
  }

  sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
 
	if (sigprocmask(SIG_BLOCK, &mask, &original_mask) < 0) {
    errprintf("Failed to bind sigprocmask\n");
		return 1;
	}

  *fd = host_fd;
  return NoError;
}

// Remove pointer to fd, it is not needed like that
i32 net_host_accept(i32* fd, i32* new_socket) {
  i32 sock = accept(*fd, (struct sockaddr*)&address, (socklen_t*)&address_length);
  if (sock < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    return 0;
  }
  *new_socket = sock;
  return 1;
}

void* net_host_select(void* data) {
  i32 fd = *(i32*)data;
  while (net_running) {
    ready_sockets = current_sockets;
    if (pselect(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL, &original_mask) < 0) {
      errprintf("Host select failed\n");
      return NULL;
    }
    for (i32 i = 0; i < FD_SETSIZE; ++i) {
      if (FD_ISSET(i, &ready_sockets)) {
        if (fd == i) {  // New connection
          i32 new_socket = -1;
          if (net_host_accept(&fd, &new_socket)) {
            if (on_handle_new) {
              on_handle_new(new_socket);
            }
            FD_SET(new_socket, &current_sockets);
          }
        }
        else {
          if (on_handle_connection) {
            i32 handle_result = on_handle_connection(i);
            if (handle_result != NoError) {
              FD_CLR(i, &current_sockets);
              net_client_remove(i);
            }
            else {
              // OK
            }
          }
        }
      }
    }
    sleep(0);
  }
  return NULL;
}

i32 net_read_all(i32 fd, void* buffer, i32 num_bytes) {
  assert(num_bytes <= NET_BUFFER_SIZE);
  i32 read_bytes = 0;
  i32 total_read_bytes = 0;
  while ((read_bytes = read(fd, buffer, num_bytes - total_read_bytes)) > 0) {
    total_read_bytes += read_bytes;
    buffer += read_bytes;
    if (total_read_bytes >= num_bytes) {
      printf("Filled the read buffer completely (fd: %i)\n", fd);
      break;
    }
  }
  return total_read_bytes;
}

i32 net_read(i32 fd, void* buffer, i32 num_bytes) {
  assert(num_bytes <= NET_BUFFER_SIZE);
  return read(fd, buffer, num_bytes);
}

i32 net_write(i32 fd, void* buffer, i32 num_bytes) {
  return (write(fd, buffer, num_bytes)) == num_bytes;
}

i32 net_broadcast(void* buffer, i32 num_bytes) {
  for (i32 client_index = 0; client_index < client_count; ++client_index) {
    Client* client = &clients[client_index];
    if (net_write(client->fd, buffer, num_bytes) == 0) {
      errprintf("Failed to send message to client %i\n", client->fd);
      // Handle
    }
  }
  return NoError;
}

i32 net_client_add(i32 fd, u32 id, double time_stamp) {
  if (client_count < MAX_CLIENT) {
    clients[client_count++] = (Client) {
      .fd = fd,
      .id = id,
      .time_stamp = time_stamp,
    };
    return NoError;
  }
  return Error;
}

i32 net_client_remove(i32 fd) {
  Client* client = client_find_by_fd(fd);
  if (client) {
    *client = clients[--client_count];
    return NoError;
  }
  return Error;
}

i32 net_client_keep_alive(i32 fd, double time_stamp) {
  Client* client = client_find_by_fd(fd);
  if (client) {
    client->time_stamp = time_stamp;
    return NoError;
  }
  return Error;
}

void net_clients_keep_alive(double time_stamp, double max_delta) {
  for (i32 client_index = 0; client_index < client_count; ++client_index) {
    Client* client = &clients[client_index];
    float delta = time_stamp - client->time_stamp;
    if (delta >= max_delta) {
      u8 byte = CMD_DISCONNECT;
      net_write(client->fd, &byte, 1);
      client_remove(client);
    }
  }
}

void net_set_host_handles(connect_cb handle_new, connect_cb handle_connection) {
  on_handle_new = handle_new;
  on_handle_connection = handle_connection;
}

void net_exit() {
  net_running = 0;
}

void net_close(i32* fd) {
  close(*fd);
  *fd = -1;
}
