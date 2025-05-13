#include "rect.h"

#include <math.h>
#include <stdio.h>

NUM_TYPE min(NUM_TYPE a, NUM_TYPE b) { return a < b ? a : b; }

NUM_TYPE max(NUM_TYPE a, NUM_TYPE b) { return a < b ? b : a; }

Rect uninitialized_rect(void) {
  Rect r;
  r.min[0] = -NUM_TYPE_ONE;
  return r;
}

NUM_TYPE rect_area(Rect* r) {
  NUM_TYPE area = NUM_TYPE_ONE;
  for (int i = 0; i < N; i++) {
    area *= r->max[i] - r->min[i];
  }
  return area <= 0 ? -area : area;
}

NUM_TYPE rect_perimeter(Rect* r) {
  NUM_TYPE perimeter = NUM_TYPE_ZERO;
  for (int i = 0; i < N; i++) {
    perimeter += r->max[i] - r->min[i];
  }
  return perimeter;
}

Rect rect_union(Rect* a, Rect* b) {
  if (!rect_is_initialized(a))
    return *b;

  Rect c;
  for (int i = 0; i < N; i++) {
    c.min[i] = min(a->min[i], b->min[i]);
    c.max[i] = max(a->max[i], b->max[i]);
  }
  return c;
}

Rect rect_inter(Rect* a, Rect* b) {
  Rect result;

  for (int i = 0; i < N; i++) {
    result.min[i] = max(a->min[i], b->min[i]);
    result.max[i] = min(a->max[i], b->max[i]);
  }

  return result;
}

NUM_TYPE rect_unioned_area(Rect* a, Rect* b) {
  Rect c = rect_union(a, b);
  NUM_TYPE area = rect_area(&c) - rect_area(a) - rect_area(b);
  if (area < NUM_TYPE_ZERO) {
    printf("Error, c:{%f, %f, %f, %f} a: {%f, %f, %f, %f} b:{%f, %f, %f, %f}\n",
           c.min[0], c.min[1], c.max[0], c.max[1], a->min[0], a->min[1],
           a->max[0], a->max[1], b->min[0], b->min[1], b->max[0], b->max[1]);
    printf("area(c) %f area(a) %f area(b) %f r=%f\n", rect_area(&c),
           rect_area(a), rect_area(b), area);
  }
  return rect_area(&c);
}

NUM_TYPE rect_intersected_area(Rect* a, Rect* b) {
  Rect c = rect_inter(a, b);
  return rect_area(&c);
}

NUM_TYPE rect_dead_space(Rect* a, Rect* b) {
  return rect_unioned_area(a, b) - rect_area(a) - rect_area(b) +
         rect_intersected_area(a, b);
}

bool rect_intersect(Rect* a, Rect* b) {
  int bits = 0;
  for (int i = 0; i < N; i++) {
    bits |= b->min[i] > a->max[i];
    bits |= b->max[i] < a->min[i];
  }
  return bits == 0;
}

bool rect_equal(Rect* a, Rect* b) {
  for (int i = 0; i < N; i++) {
    if (a->max[i] != b->max[i] || a->min[i] != b->min[i])
      return false;
  }
  return true;
}

bool rect_is_initialized(Rect* r) { return r->min[0] >= 0; }
