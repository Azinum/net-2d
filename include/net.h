// net.h

#ifndef _NET_H
#define _NET_H

#include "common.h"

#define PORT 7501
#define NET_BUFFER_SIZE 512

#define MAX_CLIENT 32

enum Net_commands {
  CMD_BAD = 0,

  CMD_PING,
  CMD_DISCONNECT,
  CMD_MESSAGE,
};

typedef struct Client {
  i32 fd;
  u32 id;
  double time_stamp;
} Client;

extern Client clients[];
extern u32 client_count;

typedef i32 (*connect_cb)(i32 fd);

void net_init();

void net_print_info(FILE* fp);

void net_host_init(i32 fd);

i32 net_connect(i32* fd, const char* host_address, i32 port);

i32 net_create_host(i32* fd, i32 port);

i32 net_host_accept(i32* fd, i32* new_socket);

void* net_host_select(void* data);

i32 net_read_all(i32 fd, void* buffer, i32 num_bytes);

i32 net_read(i32 fd, void* buffer, i32 num_bytes);

i32 net_write(i32 fd, void* buffer, i32 num_bytes);

i32 net_broadcast(void* buffer, i32 num_bytes);

i32 net_client_add(i32 fd, u32 id, double time_stamp);

i32 net_client_remove(i32 fd);

i32 net_client_keep_alive(i32 fd, double time_stamp);

void net_clients_keep_alive(double time_stamp, double max_delta);

void net_set_host_handles(connect_cb handle_new, connect_cb handle_connection);

void net_exit();

void net_close(i32* fd);

#endif
