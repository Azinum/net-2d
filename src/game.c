// game.c

#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "common.h"
#include "platform.h"
#include "image.h"
#include "entity.h"
#include "net.h"
#include "game.h"

#define MAX_TITLE_LENGTH 256

Game_state game_state = {};
i32 socket_fd = -1;
double prev_ping = 0;
double prev_net_time = 0;
pthread_t host_select_thread = {0};

static void sigint_handle(i32 sig);
static i32 game_init(Game_state* game);
static i32 host_handle_new(i32 fd);
static i32 host_handle_connection(i32 fd);
static i32 game_run_host(Game_state* game);
static i32 game_run_client(Game_state* game);

void sigint_handle(i32 sig) {
  game_state.running = 0;
  net_exit();
}

i32 game_init(Game_state* game) {
  game->dt = 0;
  game->running = 1;
  game->client = 1;
  game->entity_count = 0;
  game->id = 0;

  for (i32 i = 0; i < 5; ++i) {
    Entity* e = game_add_entity();
    entity_init(e, game->id, V2(rand() % 300, rand() % 300), V2(64, 64), FLAG_MOVER, ENTITY_NONE);
  }
  return NoError;
}

i32 host_handle_new(i32 fd) {
  Game_state* game = &game_state;
  printf("[server]: New connection: %i\n", fd);

  u8 write_buffer[NET_BUFFER_SIZE] = {};

  if (net_client_add(fd, game->id, game->time_stamp) == NoError) {
    Entity* e = game_add_entity();
    entity_init(e, game->id, V2(rand() % 300, rand() % 300), V2(32, 32), FLAG_MOVER, ENTITY_NONE);

    u8* buffer = &write_buffer[0];
    buffer = write_byte(buffer, CMD_MESSAGE);
    buffer = write_string(buffer, NET_BUFFER_SIZE, "New connection: %i", fd);
    i32 size = buffer - write_buffer;
    net_broadcast(write_buffer, size);
  }
  return NoError;
}

// TODO(lucas): Make all of the networking thread safe!!!
i32 host_handle_connection(i32 fd) {
  Game_state* game = &game_state;

  u8 read_buffer[NET_BUFFER_SIZE] = {};
  u8* buffer = &read_buffer[0];
  i32 read_bytes = net_read(fd, read_buffer, NET_BUFFER_SIZE);
  if (read_bytes > 0) {
    u8 byte = *buffer++;
    switch (byte) {
      case CMD_PING: {
        net_client_keep_alive(fd, game->time_stamp);
        double latency = 1000.0f * (game->time_stamp - prev_ping);
        if (latency > NET_MAX_LATENCY) {
          printf("Disconnecting client %i due to having too high latency (has %g, expected to be less than %g)\n", fd, latency, NET_MAX_LATENCY);
          u8 command = CMD_DISCONNECT;
          net_write(fd, &command, 1);
          net_client_remove(fd);
        }
        break;
      }
      case CMD_DISCONNECT: {
        printf("DISCONNECT from client: %i\n", fd);
        net_client_remove(fd);
        break;
      }
      default: {
        printf("Got undefined command from client(%i): %i\n", fd, byte);
        net_client_remove(fd);
        break;
      }
    }
  }
  else {
    return Error;
  }
  return NoError;
}

i32 game_run_host(Game_state* game) {
  struct timeval now = {};
  struct timeval prev = {};
  i32 new_socket = 0;

  u8 read_buffer[NET_BUFFER_SIZE] = {};
  i32 bytes_read = 0;
  u8 write_buffer[NET_BUFFER_SIZE] = {};
  i32 net_tick = 0;

  net_set_host_handles(host_handle_new, host_handle_connection);
  net_host_init(socket_fd);

  pthread_create(&host_select_thread, NULL, net_host_select, (void*)&socket_fd);

  while (game->running) {
    prev = now;
    gettimeofday(&now, NULL);
    game->dt = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
    if (game->dt > MAX_DT) {
      game->dt = MAX_DT;
    }
    game->time_stamp += game->dt;

    double ping_time_stamp = prev_ping + NET_PING_INTERVAL;
    if (game->time_stamp >= ping_time_stamp) {
      double delta = game->time_stamp - ping_time_stamp;
      prev_ping = game->time_stamp - delta;
      u8 command = CMD_PING;
      net_broadcast(&command, 1);
      net_tick++;
    }

    for (i32 i = 0; i < game->entity_count; ++i) {
      Entity* e = &game->entities[i];
      entity_update(e, game);
    }
    net_clients_keep_alive(game->time_stamp, NET_MAX_DELTA);
  }
  net_close(&socket_fd);
  pthread_join(host_select_thread, NULL);
  return NoError;
}

i32 game_run_client(Game_state* game) {
  struct timeval now = {};
  struct timeval prev = {};

  char window_title[MAX_TITLE_LENGTH] = {0};

  u8 read_buffer[NET_BUFFER_SIZE] = {};
  i32 bytes_read = 0;
  u8 write_buffer[NET_BUFFER_SIZE] = {};
  i32 net_tick = 0;

  while (game->running && platform_process_events() == 0) {
    prev = now;
    gettimeofday(&now, NULL);
    game->dt = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
    if (game->dt > MAX_DT) {
      game->dt = MAX_DT;
    }
    game->time_stamp += game->dt;

    if ((bytes_read = net_read(socket_fd, (void*)&read_buffer[0], NET_BUFFER_SIZE)) > 0) {
      u8* read_iter = read_buffer;
      u8 byte = *read_iter++;
      switch (byte) {
        case CMD_BAD: {
          printf("Got BAD command\n");
          break;
        }
        // Got ping from server, ping back to let the server know that we are alive
        case CMD_PING: {
          u8 command = CMD_PING;
          net_write(socket_fd, &command, 1);
          net_tick++;
          printf("Got PING command from host. Client net tick: %i\n", net_tick);
          break;
        }
        case CMD_DISCONNECT: {
          printf("Got DISCONNECT command from host\n");
          break;
        }
        case CMD_MESSAGE: {
          printf("Got MESSAGE command from host: %s\n", read_iter);
          break;
        }
        default: {
          printf("Got undefined command from host: %i\n", byte);
          break;
        }
      }
    }

    for (i32 i = 0; i < game->entity_count; ++i) {
      Entity* e = &game->entities[i];
      entity_update(e, game);
      entity_render(e, game);
    }
    snprintf(window_title, MAX_TITLE_LENGTH, "%s | %i fps | %g delta", TITLE, (i32)(1.0f / game->dt), game->dt);
    platform_set_window_title(window_title);
    renderer_swap_buffers(&render_state);
  }

  u8 command = CMD_DISCONNECT;
  net_write(socket_fd, &command, 1);

  return NoError;
}

i32 game_start(i32 argc, char** argv) {
  srand(time(NULL));
  signal(SIGINT, sigint_handle);
  Game_state* game = &game_state;
  Render_state* renderer = &render_state;

  net_init();
  game_init(game);

  if (argc > 1) {
    game->client = 0;
  }

  if (game->client) {
    const char* address = "127.0.0.1";
    printf("Connecting to server... (%s:%i)\n", address, PORT);
    net_connect(&socket_fd, "127.0.0.1", PORT);
    printf("Successfully connected to server!\n");

    platform_open_window(WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    renderer_init(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    game_run_client(game);
    renderer_free(renderer);
    platform_close_window();
  }
  else {
    net_create_host(&socket_fd, PORT);
    game_run_host(game);
  }

  net_exit(&socket_fd);

  assert("memory leak!" && memory_total() == 0);
  return NoError;
}

Entity* game_add_entity() {
  Game_state* game = &game_state;
  if (game->entity_count < MAX_ENTITY) {
    game->id++;
    return &game->entities[game->entity_count++];
  }
  return NULL;
}
