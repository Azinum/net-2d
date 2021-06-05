// game.h

#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "config.h"
#include "memory.h"
#include "math_util.h"
#include "entity.h"
#include "log.h"
#include "renderer.h"

#define MAX_ENTITY ((u8)128)

typedef struct Game_state {
  double dt;
  double time_stamp;
  volatile u8 running;
  u8 client;
  Entity entities[MAX_ENTITY];
  u32 entity_count;
  u32 id;
} Game_state;

extern Game_state game_state;
extern i32 socket_fd;

i32 game_start(i32 argc, char** argv);

Entity* game_add_entity();

#endif
