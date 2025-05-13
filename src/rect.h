#ifndef TIPE_RECT
#define TIPE_RECT

#include "constants.h"

struct {
  NUM_TYPE min[N];
  NUM_TYPE max[N];
} typedef Rect;

NUM_TYPE min(NUM_TYPE, NUM_TYPE);
NUM_TYPE max(NUM_TYPE, NUM_TYPE);

Rect uninitialized_rect(void);
NUM_TYPE rect_area(Rect* r);
NUM_TYPE rect_perimeter(Rect* r);
NUM_TYPE rect_unioned_area(Rect* a, Rect* b);
Rect rect_expand(Rect* a, Rect* b);
bool rect_intersect(Rect* a, Rect* b);
bool rect_equal(Rect* a, Rect* b);
bool rect_is_initialized(Rect* r);

#endif
