// game.h

#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "config.h"
#include "memory.h"
#include "math_util.h"
#include "renderer.h"

typedef struct Game_state {
  float dt;
  u8 is_running;
} Game_state;

extern Game_state game_state;

i32 game_start(i32 argc, char** argv);

#endif
