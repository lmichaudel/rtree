#include "rect.h"

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
  return area;
}

NUM_TYPE rect_perimeter(Rect* r) {
  NUM_TYPE perimeter = NUM_TYPE_ZERO;
  for (int i = 0; i < N; i++) {
    perimeter += r->max[i] - r->min[i];
  }
  return perimeter;
}

NUM_TYPE rect_unioned_area(Rect* a, Rect* b) {
  NUM_TYPE result = NUM_TYPE_ONE;
  for (int i = 0; i < N; i++) {
    result *= (max(a->max[i], b->max[i]) - min(a->min[i], b->min[i]));
  }
  return result;
}

Rect rect_expand(Rect* a, Rect* b) {
  if (!rect_is_initialized(a))
    return *b;

  Rect c;
  for (int i = 0; i < N; i++) {
    c.min[i] = min(a->min[i], b->min[i]);
    c.max[i] = max(a->max[i], b->max[i]);
  }
  return c;
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
