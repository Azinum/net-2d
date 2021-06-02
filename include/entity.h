// entity.h

#ifndef _ENTITY_H
#define _ENTITY_H

#include "common.h"

enum Entity_type {
  ENTITY_NONE = 0,
};

enum Entity_flag {
  FLAG_NONE     = 0,
  FLAG_MOVER    = 1 << 0,
};

typedef struct Entity {
  u32 id;
  v2 pos;
  v2 target_pos;
  v2 size;
  u32 flags;
  u32 type;
} Entity;

struct Game_state;

void entity_init(Entity* e, u32 id, v2 pos, v2 size, u32 flags, u32 type);

void entity_update(Entity* e, struct Game_state* game);

void entity_render(Entity* e, struct Game_state* game);

#endif
