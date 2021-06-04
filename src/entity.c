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
  if (e->flags & FLAG_MOVER) {
    e->pos.x += 10 * game->dt;
  }
}

void entity_render(Entity* e, Game_state* game) {
#if 0
  render_quad(e->pos, e->size, ColorRGB(255, 50, 50));
#else
  render_texture(0, e->pos, e->size, ColorRGB(255, 255, 255));
#endif
}
