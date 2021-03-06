#ifndef LABELS_H_
#define LABELS_H_

#include "math/point.h"
#include "color.h"

typedef struct Labels Labels;
typedef struct Camera Camera;

Labels *create_labels_from_stream(FILE *stream);
void destroy_labels(Labels *label);

int labels_render(const Labels *label,
                  Camera *camera);
void labels_update(Labels *label,
                   float delta_time);
void labels_enter_camera_event(Labels *label,
                               const Camera *camera);

#endif  // LABELS_H_
