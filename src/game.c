// game.c

#include "common.h"
#include "platform.h"
#include "image.h"
#include "game.h"

Game_state game_state = {};

static i32 game_init(Game_state* game);
static i32 game_run(Game_state* game);

i32 game_init(Game_state* game) {
  game->dt = 0;
  game->is_running = 1;
  return NoError;
}

i32 game_run(Game_state* game) {
  Render_state* renderer = &render_state;
  while (game->is_running && platform_process_events() == 0) {
    renderer_swap_buffers(&render_state);
  }
  return NoError;
}

i32 game_start(i32 argc, char** argv) {
  Game_state* game = &game_state;
  Render_state* renderer = &render_state;
  game_init(game);
  platform_open_window(WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
  renderer_init(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
  game_run(game);
  renderer_free(renderer);
  platform_close_window();
  assert("memory leak!" && memory_total() == 0);
  return NoError;
}
