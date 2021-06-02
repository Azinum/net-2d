// entity.c

#include "game.h"
#include "entity.h"

u32 you = 0;

void entity_init(Entity* e, u32 id, v2 pos, v2 size, u32 flags, u32 type) {
  memset(e, 0, sizeof(Entity));
  e->id = id;
  e->pos = e->target_pos = pos;
  e->size = size;
  e->flags = flags;
  e->type = type;
}

void entity_update(Entity* e, Game_state* game) {

}

void entity_render(Entity* e, Game_state* game) {
  render_quad(e->pos, e->size, ColorRGB(255, 50, 50));
}
