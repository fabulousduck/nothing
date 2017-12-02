#ifndef PLAYER_H_
#define PLAYER_H_

#include "./camera.h"

struct player_t;
struct platforms_t;
struct SDL_Renderer;

struct player_t *create_player(float x, float y);
void destroy_player(struct player_t * player);

int render_player(const struct player_t * player,
                  SDL_Renderer *renderer,
                  const camera_t *camera);
void update_player(struct player_t * player,
                   const struct platforms_t *platforms,
                   Uint32 delta_time);

void player_move_left(struct player_t *player);
void player_move_right(struct player_t *player);
void player_stop(struct player_t *player);
void player_jump(struct player_t *player);

void player_focus_camera(struct player_t *player,
                         camera_t *camera);

#endif  // PLAYER_H_
