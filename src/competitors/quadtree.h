#ifndef QUADTREE
#define QUADTREE

#include "item.h"
#include "rect.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Quadtree {
  Rect mbr;
  Item item;

  struct Quadtree* NW;
  struct Quadtree* NE;
  struct Quadtree* SW;
  struct Quadtree* SE;
} Quadtree;

Quadtree* quadtree_new(Rect bounds);
void quadtree_free(Quadtree* quadtree);

void quadtree_insert(Quadtree* root, Item i);
ItemList quadtree_search(Quadtree* quadtree, Rect window);

#endif
