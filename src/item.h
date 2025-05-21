#ifndef ITEM_H
#define ITEM_H

#include "rect.h"

#include <stdlib.h>

typedef struct Item {
  int id;
  Rect mbr;
} Item;

typedef struct ItemListNode {
  int id;
  struct ItemListNode* next;
} ItemListNode;
typedef ItemListNode* ItemList;

static inline void itemlist_free(ItemList list) {
  if (list != NULL) {
    ItemList next = list->next;
    free(list);
    itemlist_free(next);
  }
}

#endif
