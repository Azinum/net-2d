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

// TODO(lucas): Clean this up when things are working properly
i32 socket_fd = -1;
double prev_ping = 0;
double prev_net_time = 0;
double prev_net_update = 0;
pthread_t host_select_thread = {0};
Buffer net_buffer = {0};

static void sigint_handle(i32 sig);
static i32 game_init(Game_state* game);
static i32 host_handle_new(i32 fd);
static i32 host_handle_connection(i32 fd);
static i32 host_send_entity(i32 fd, i32 entity_index, u8 broadcast);
static i32 host_send_all_entities(Game_state* game);
static i32 client_parse_commands(Game_state* game);
static i32 game_run_host(Game_state* game);
static i32 game_run_client(Game_state* game);

void sigint_handle(i32 sig) {
  game_state.running = 0;
  net_exit();
}

i32 game_init(Game_state* game) {
  game->dt = 1;
  game->running = 1;
  game->client = 1;
  game->entity_count = 0;
  game->id = 0;

#if 1
  for (i32 i = 0; i < 10; ++i) {
    Entity* e = game_add_entity();
    entity_init(e, game->id, V2(rand() % 300, rand() % 300), V2(16 + rand() % 64, 16 + rand() % 64), FLAG_MOVER, ENTITY_NONE);
    e->dir = V2(
      ((rand() % 100) - (rand() % 100)) / 100.0f,
      ((rand() % 100) - (rand() % 100)) / 100.0f
    );
    e->sprite_id = rand() % 2;
  }
#endif
  return NoError;
}

i32 host_handle_new(i32 fd) {
  Game_state* game = &game_state;

  u8 write_buffer[NET_BUFFER_SIZE] = {};

  if (net_client_add(fd, game->id, game->time_stamp) == NoError) {
    log_printf("New connection (fd: %i)\n", fd);
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
          log_printf("Disconnecting client %i due to having too high latency (has %g, expected to be less than %g)\n", fd, latency, NET_MAX_LATENCY);
          u8 command = CMD_DISCONNECT;
          net_write(fd, &command, 1);
          net_client_remove(fd);
        }
        break;
      }
      case CMD_DISCONNECT: {
        log_printf("CMD_DISCONNECT from client %i\n", fd);
        net_client_remove(fd);
        break;
      }
      default: {
        log_printf("Got undefined command from client(%i): %i\n", fd, byte);
        net_client_remove(fd);
        break;
      }
    }
  }
  else {
    log_printf("Invalid connection, disconnecting... (%i)\n", fd);
    return Error;
  }
  return NoError;
}

i32 host_send_entity(i32 fd, i32 entity_index, u8 broadcast) {
  Game_state* game = &game_state;

  if (game->entity_count < entity_index) {
    errprintf("Cannot send an entity which do not exist (tried to send: %i)\n", entity_index);
    return Error;
  }

  u8 write_buffer[NET_BUFFER_SIZE] = {0};
  u8* buffer = &write_buffer[0];

  u8 command = CMD_UPDATE_ENTITY;
  u8 index = (u8)entity_index;
  u8 entity_count = (u8)game->entity_count;
  Entity entity = game->entities[index];

  // TODO(lucas): Implement a common buffer which is easier to read and write to
  buffer = write_byte(buffer, command);
  buffer = write_byte(buffer, index);
  buffer = write_byte(buffer, entity_count);
  buffer = write_data(buffer, NET_BUFFER_SIZE, &entity, sizeof(Entity));
  i32 size = buffer - write_buffer;

  if (broadcast) {
    if (net_broadcast(write_buffer, size)) {
      return NoError;
    }
  }
  else {
    if (net_write(fd, write_buffer, size)) {
      return NoError;
    }
  }

  return Error;
}

i32 host_send_all_entities(Game_state* game) {
  i32 result = NoError;
  for (i32 i = 0; i < game->entity_count; ++i) {
    result = host_send_entity(-1, i, 1 /* broadcast */);
  }
  return result;
}

i32 client_parse_commands(Game_state* game) {
  u8 byte = 0;
  u8* buffer = &net_buffer.data[0];
  u8 partial_packet = 0;
  i32 command_size = 0;

  for (i32 i = 0; i < net_buffer.count; ++i) {
    command_size = 0;
    partial_packet = 0;
    buffer = &net_buffer.data[i];
    byte = *buffer++;
    switch (byte) {
      case CMD_PING: {
        u8 command = CMD_PING;
        net_write(socket_fd, &command, 1);
        break;
      }
      case CMD_DISCONNECT: {
        printf("Got CMD_DISCONNECT\n");
        break;
      }
      case CMD_MESSAGE: {
        printf("Got CMD_MESSAGE\n");
        assert(0 && "not implemented");
        break;
      }
      case CMD_UPDATE_ENTITY: {
        command_size = 2 + sizeof(Entity);

        // This packet is within the buffer area
        if (i + command_size < net_buffer.count) {
          u8 entity_index = *buffer++;
          u8 entity_count = *buffer++;
          Entity entity = *(Entity*)buffer;
          buffer += sizeof(Entity);

          Entity old_entity = game->entities[entity_index];
          entity.pos = old_entity.pos;

          game->entities[entity_index] = entity;
          game->entity_count = entity_count;
        }
        // This packet has not fully been recieved yet
        else {
          partial_packet = 1;
        }
        break;
      }
      default: {
        printf("Got undefined command from host: %i\n", byte);
        break;
      }
    }
    if (partial_packet) {
      i32 bytes_to_copy = net_buffer.count - i;
      net_buffer.count = 0;
      buffer_write_data(&net_buffer, buffer - 1, bytes_to_copy);
      net_buffer.count = bytes_to_copy;
      goto parsing_done;
    }
    i += command_size;
  }

  net_buffer.count = 0; // Only reset the buffer when we have a successful non-partially parsed buffer
parsing_done:
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

  double prev_game_time_stamp = 0;
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
    }

    double net_time_stamp = prev_net_update + NET_INTERVAL;
    if (game->time_stamp >= net_time_stamp) {
      double delta = game->time_stamp - net_time_stamp;
      prev_net_update = game->time_stamp - delta;
      host_send_all_entities(game);
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

    i32 bytes_read = 0;
    if ((bytes_read = net_read(socket_fd, (void*)&read_buffer[0], NET_BUFFER_SIZE)) > 0) {
      buffer_write_data(&net_buffer, (void*)&read_buffer[0], bytes_read);
    }
    client_parse_commands(game);

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
  FILE* fp = fopen(LOG_PATH, "w");
  if (fp) {
    log_init(fp);
  }
  else {
    fprintf(stderr, "Failed to create log file '%s'\n", LOG_PATH);
  }

  srand(time(NULL));
  signal(SIGINT, sigint_handle);
  Game_state* game = &game_state;
  Render_state* renderer = &render_state;

  net_init();
  game_init(game);

  buffer_init(&net_buffer);

  if (argc > 1) {
    game->client = 0;
  }

  if (game->client) {
    const char* address = "127.0.0.1";
    printf("Connecting to server... (%s:%i)\n", address, PORT);
    net_connect(&socket_fd, address, PORT);
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

  buffer_free(&net_buffer);

  net_exit(&socket_fd);
  fclose(fp);
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
