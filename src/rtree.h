#ifndef TIPE_RTREE
#define TIPE_RTREE

#include "rect.h"

enum { LEAF, BRANCH } typedef Kind;

typedef struct Item {
  int id;
  Rect r;
} Item;

typedef struct Node {
  Kind kind;
  int count;
  Rect mbr;

  union {
    struct Node* children[M + 1];
    Item data[M + 1];
  };
} Node;

typedef struct Rtree {
  Node* root;
} Rtree;

typedef struct ItemListNode {
  int id;
  struct ItemListNode* next;
} ItemListNode;
typedef ItemListNode* ItemList;

void split_pick_seeds(Node* node, int* seed1, int* seed2);
SPLIT_MASK split_greene(Node* node);
SPLIT_MASK split_exponential(Node* node);
SPLIT_MASK split_quadratic(Node* node);

Node* node_new(Kind kind);
void node_free(Node* node);
void node_fit_mbr(Node* node);
Rect node_ith_mbr(Node* node);
int node_choose_best(Node* node, Rect* r);
void node_split(Node* node, Node** sibling_out);
void node_insert(Node* node, Rect r, int id, bool* split);
void node_delete(Rtree* rtree, Node* node, Rect r, int id, bool* shrink);

Rtree* rtree_new(void);
void rtree_free(Rtree* rtree);
void rtree_insert(Rtree* rtree, Rect r, int id);
void rtree_delete(Rtree* rtree, Rect r, int id);
void rtree_debug(Rtree* rtree);
ItemList rtree_search(Rtree* rtree, Rect window);

void itemlist_free(ItemList list);

#endif
