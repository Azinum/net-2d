// game.c

#include <time.h>

#include "common.h"
#include "platform.h"
#include "image.h"
#include "entity.h"
#include "game.h"

#define MAX_DT 1

Game_state game_state = {};

static i32 game_init(Game_state* game);
static i32 game_run(Game_state* game);
static i32 game_run_client(Game_state* game);

i32 game_init(Game_state* game) {
  game->dt = 0;
  game->running = 1;
  game->client = 0;
  game->entity_count = 0;
  game->id = 0;

  {
    Entity* e = game_add_entity();
    entity_init(e, game->id, V2(rand() % 300, rand() % 300), V2(32, 32), FLAG_MOVER, ENTITY_NONE);
  }

  {
    Entity* e = game_add_entity();
    entity_init(e, game->id, V2(rand() % 300, rand() % 300), V2(32, 32), FLAG_MOVER, ENTITY_NONE);
  }

  return NoError;
}

i32 game_run(Game_state* game) {
  struct timeval now = {};
  struct timeval prev = {};

  while (game->running) {
    prev = now;
    gettimeofday(&now, NULL);
    game->dt = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
    if (game->dt > MAX_DT) {
      game->dt = MAX_DT;
    }
    game->time_stamp += game->dt;

    for (i32 i = 0; i < game->entity_count; ++i) {
      Entity* e = &game->entities[i];
      entity_update(e, game);
    }
  }

  return NoError;
}

// Might not want to seperate the server and client like this... Something to think about.
i32 game_run_client(Game_state* game) {
  struct timeval now = {};
  struct timeval prev = {};
  Render_state* renderer = &render_state;

  // game->entity_count = 0;

  while (game->running && platform_process_events() == 0) {
    prev = now;
    gettimeofday(&now, NULL);
    game->dt = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
    if (game->dt > MAX_DT) {
      game->dt = MAX_DT;
    }
    game->time_stamp += game->dt;

    for (i32 i = 0; i < game->entity_count; ++i) {
      Entity* e = &game->entities[i];
      entity_update(e, game);
      entity_render(e, game);
    }

    renderer_swap_buffers(&render_state);
  }

  return NoError;
}

i32 game_start(i32 argc, char** argv) {
  srand(time(NULL));
  Game_state* game = &game_state;
  Render_state* renderer = &render_state;

  game_init(game);
  game->client = 1;

  if (game->client) {
    platform_open_window(WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    renderer_init(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    game_run_client(game);
    renderer_free(renderer);
    platform_close_window();
  }
  else {
    game_run(game);
  }

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
