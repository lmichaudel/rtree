#pragma once

#include "constants.h"

#include <stdbool.h>
#include <stdint.h>

enum { LEAF, BRANCH } typedef Kind;

struct {
  NUM_TYPE min[N];
  NUM_TYPE max[N];
} typedef Rect;

struct {
  int id;
  Rect r;
} typedef Item;

struct Node {
  Kind kind;
  int count;
  Rect mbr;

  union {
    struct Node* children[M + 1];
    Item data[M + 1];
  };
} typedef Node;

struct {
  Node* root;
} typedef Rtree;

struct ItemListNode {
  int id;
  struct ItemListNode* next;
} typedef ItemListNode;
typedef ItemListNode* ItemList;

NUM_TYPE min(NUM_TYPE, NUM_TYPE);
NUM_TYPE max(NUM_TYPE, NUM_TYPE);

NUM_TYPE rect_area(Rect* r);
NUM_TYPE rect_perimeter(Rect* r);
NUM_TYPE rect_unioned_area(Rect* a, Rect* b);
Rect rect_expand(Rect* a, Rect* b);
bool rect_intersect(Rect* a, Rect* b);
bool rect_equal(Rect* a, Rect* b);

SPLIT_MASK best_split_exponential(Node* node);
SPLIT_MASK best_split_bad(void);

Node* node_new(Kind kind);
void node_free(Node* node);
void node_fit_mbr(Node* node);
Rect node_ith_mbr(Node* node);
int node_choose_best(Node* node, Rect* r);
void node_split(Node* node, Node** sibling_out);
void node_insert(Node* node, Rect r, int id, bool* split);

Rtree* rtree_new(void);
void rtree_free(Rtree* rtree);
void rtree_insert(Rtree* rtree, Rect r, int id);
void rtree_delete(Rtree* rtree, Rect r, int id);
ItemList rtree_search(Rtree* rtree, Rect window);

void itemlist_free(ItemList list);
