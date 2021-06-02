// game.c

#include "common.h"
#include "platform.h"
#include "image.h"
#include "game.h"

#define MAX_DT 1

Game_state game_state = {};

static i32 game_init(Game_state* game);
static i32 game_run(Game_state* game);

i32 game_init(Game_state* game) {
  game->dt = 0;
  game->is_running = 1;
  return NoError;
}

i32 game_run(Game_state* game) {
  struct timeval now = {};
  struct timeval prev = {};
  Render_state* renderer = &render_state;

  Image image = {};
  image_load("resource/sprite/shroom.bmp", &image);

  float x = 0;

  while (game->is_running && platform_process_events() == 0) {
    prev = now;
    gettimeofday(&now, NULL);
    game->dt = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
    if (game->dt > MAX_DT)
      game->dt = MAX_DT;

    x += 50.0f * game->dt;

    render_quad(V2(x, 25), V2(80, 25), ColorRGB(255, 50, 50));
    render_image(&image, V2(x, 0), V2(0, 0), ColorRGB(255, 255, 255));

    renderer_swap_buffers(&render_state);
  }

  image_free(&image);
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
