#ifndef RIGID_RECT_H_
#define RIGID_RECT_H_

#include "color.h"
#include "game/camera.h"
#include "game/level/lava.h"
#include "game/level/platforms.h"
#include "game/level/solid.h"
#include "math/mat3x3.h"
#include "math/rect.h"

// TODO(#215): replace Rigid_rect with Rigid_bodies

typedef struct Rigid_rect Rigid_rect;
typedef struct Boxes Boxes;

Rigid_rect *create_rigid_rect(Rect rect, Color color, const char *id);
Rigid_rect *create_rigid_rect_from_stream(FILE *stream);
void destroy_rigid_rect(Rigid_rect *rigid_rect);

Solid_ref rigid_rect_as_solid(Rigid_rect *rigid_rect);

int rigid_rect_render(const Rigid_rect *rigid_rect,
                      Camera *camera);
int rigid_rect_update(Rigid_rect * rigid_rect,
                      float delta_time);

void rigid_rect_touches_rect_sides(Rigid_rect *rigid_rect,
                                   Rect object,
                                   int sides[RECT_SIDE_N]);

void rigid_rect_collide_with_solid(Rigid_rect * rigid_rect,
                                   Solid_ref solid);

Rect rigid_rect_hitbox(const Rigid_rect *rigid_rect);

void rigid_rect_move(Rigid_rect *rigid_rect,
                     Vec movement);

int rigid_rect_touches_ground(const Rigid_rect *rigid_rect);

/** \brief implements solid_apply_force
 */
void rigid_rect_apply_force(Rigid_rect * rigid_rect,
                            Vec force);

void rigid_rect_transform_velocity(Rigid_rect *rigid_rect,
                                   mat3x3 trans_mat);

void rigid_rect_teleport_to(Rigid_rect *rigid_rect,
                            Vec position);

void rigid_rect_damper(Rigid_rect *rigid_rect,
                       Vec v);

bool rigid_rect_has_id(Rigid_rect *rigid_rect,
                       const char *id);

#endif  // RIGID_RECT_H_
