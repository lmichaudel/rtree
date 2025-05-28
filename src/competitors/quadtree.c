#include "quadtree.h"

#include <assert.h>

Quadtree* quadtree_new(Rect bounds) {
  Quadtree* qtree = malloc(sizeof(Quadtree));
  qtree->NW = NULL;
  qtree->NE = NULL;
  qtree->SW = NULL;
  qtree->SE = NULL;
  qtree->mbr = bounds;
  qtree->item.id = -1;

  return qtree;
}

void quadtree_free(Quadtree* quadtree) {
  if (quadtree != NULL) {
    quadtree_free(quadtree->NW);
    quadtree_free(quadtree->NE);
    quadtree_free(quadtree->SW);
    quadtree_free(quadtree->SE);

    free(quadtree);
  }
}

bool is_leaf(Quadtree* quadtree) { return quadtree->NE == NULL; }

Quadtree* get_correct_quadrant(Quadtree* qtree, Item i) {
  if (rect_intersect(&i.mbr, &qtree->NW->mbr)) {
    return qtree->NW;
  }
  if (rect_intersect(&i.mbr, &qtree->NE->mbr)) {
    return qtree->NE;
  }
  if (rect_intersect(&i.mbr, &qtree->SW->mbr)) {
    return qtree->SW;
  }
  if (rect_intersect(&i.mbr, &qtree->SE->mbr)) {
    return qtree->SE;
  }

  return NULL;
}

void quadtree_insert(Quadtree* root, Item i) {
  assert(root != NULL);

  Quadtree* current = root;

  // i had to limit the depth because there are some duplicates
  // in the dataset and it crashed the construction
  int j = 0;
  while (true && j < 200) {
    j++;

    if (is_leaf(current)) {
      if (current->item.id == -1) {
        // Empty leaf - insert here
        current->item = i;
        return;
      }

      double width = current->mbr.max[0] - current->mbr.min[0];
      double height = current->mbr.max[1] - current->mbr.min[1];
      double x = current->mbr.min[0];
      double y = current->mbr.min[1];

      current->NW = quadtree_new(
          (Rect){{x, y + height / 2}, {x + width / 2, y + height}});
      current->NE = quadtree_new(
          (Rect){{x + width / 2, y + height / 2}, {x + width, y + height}});
      current->SW =
          quadtree_new((Rect){{x, y}, {x + width / 2, y + height / 2}});
      current->SE =
          quadtree_new((Rect){{x + width / 2, y}, {x + width, y + height / 2}});

      get_correct_quadrant(current, current->item)->item = current->item;
      current->item.id = -1;
    }

    Quadtree* next = get_correct_quadrant(current, i);
    current = next;
  }
}

void quadtree_search_rec(Quadtree* qtree, Rect window, ItemList* list) {
  if (is_leaf(qtree)) {
    if (rect_intersect(&qtree->item.mbr, &window)) {

      ItemListNode* head = malloc(sizeof(ItemListNode));
      head->id = qtree->item.id;
      head->next = *list;
      *list = head;
    }
  } else {
    if (rect_intersect(&window, &qtree->NW->mbr)) {
      quadtree_search_rec(qtree->NW, window, list);
    }
    if (rect_intersect(&window, &qtree->NE->mbr)) {
      quadtree_search_rec(qtree->NE, window, list);
    }
    if (rect_intersect(&window, &qtree->SW->mbr)) {
      quadtree_search_rec(qtree->SW, window, list);
    }
    if (rect_intersect(&window, &qtree->SE->mbr)) {
      quadtree_search_rec(qtree->SE, window, list);
    }
  }
}

ItemList quadtree_search(Quadtree* quadtree, Rect window) {
  ItemList list = NULL;
  quadtree_search_rec(quadtree, window, &list);
  return list;
}
