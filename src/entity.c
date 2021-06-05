// entity.c

#include "game.h"
#include "platform.h"
#include "entity.h"

u32 you = 0;

#define ENTITY_SPEED 150.0f
#define INTERPOLATION_SPEED 20.0f

void entity_init(Entity* e, u32 id, v2 pos, v2 size, u32 flags, u32 type) {
  memset(e, 0, sizeof(Entity));
  e->id = id;
  e->pos = e->target_pos = pos;
  e->size = size;
  e->dir = V2(0, 0);
  e->flags = flags;
  e->type = type;
  e->sprite_id = 0;
}

void entity_print_info(Entity* e, FILE* fp) {
  fprintf(fp,
    "Entity info:\n"
    "  id: %u\n"
    "  pos: (%g, %g)\n"
    "  target_pos: (%g, %g)\n"
    "  size: (%g, %g)\n"
    "  flags: 0x%x\n"
    "  type: 0x%x\n"
    ,
    e->id,
    e->pos.x, e->pos.y,
    e->target_pos.x, e->target_pos.y,
    e->size.w, e->size.h,
    e->flags,
    e->type
  );
}

void entity_update(Entity* e, Game_state* game) {
  if (e->flags & FLAG_MOVER) {
    if (e->target_pos.x > 800) {
      e->target_pos.x = 800;
      e->dir.x = -e->dir.x;
    }
    if (e->target_pos.x < 0) {
      e->target_pos.x = 0;
      e->dir.x = -e->dir.x;
    }
    if (e->target_pos.y > 600) {
      e->target_pos.y = 600;
      e->dir.y = -e->dir.y;
    }
    if (e->target_pos.y < 0) {
      e->target_pos.y = 0;
      e->dir.y = -e->dir.y;
    }
    e->target_pos.x += e->dir.x * ENTITY_SPEED * game->dt;
    e->target_pos.y += e->dir.y * ENTITY_SPEED * game->dt;
    e->pos.x = lerp(e->pos.x, e->target_pos.x, INTERPOLATION_SPEED * game->dt);
    e->pos.y = lerp(e->pos.y, e->target_pos.y, INTERPOLATION_SPEED * game->dt);
  }
}

void entity_render(Entity* e, Game_state* game) {
  render_sprite(0, e->sprite_id, e->pos, e->size, ColorRGB(255, 255, 255));
  render_quad_border(e->pos, e->size, ColorRGB(100, 100, 255));
}
