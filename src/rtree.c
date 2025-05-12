#include "rtree.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_GET_ITH_CHILD_MBR(node, i)                                        \
  node->kind == LEAF ? node->data[i].r : node->children[i]->mbr
#define NODE_GET_ITH_CHILD_MBR_PTR(node, i)                                    \
  node->kind == LEAF ? &node->data[i].r : &node->children[i]->mbr

NUM_TYPE min(NUM_TYPE a, NUM_TYPE b) { return a < b ? a : b; }

NUM_TYPE max(NUM_TYPE a, NUM_TYPE b) { return a < b ? b : a; }

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

Node* node_new(Kind kind) {
  Node* node = malloc(sizeof(Node));
  node->count = 0;
  node->kind = kind;
  for (int i = 0; i < N; i++) {
    node->mbr.min[i] = 0;
    node->mbr.max[i] = 0;
  }
  return node;
}

void node_free(Node* node) {
  if (node->kind == BRANCH) {
    for (int i = 0; i < node->count; i++)
      node_free(node->children[i]);
  }

  free(node);
}

void node_fit_mbr(Node* node) {
  if (node->count >= 1) {
    node->mbr = NODE_GET_ITH_CHILD_MBR(node, 0);
    for (int i = 1; i < node->count; i++) {
      node->mbr = rect_expand(&node->mbr, NODE_GET_ITH_CHILD_MBR_PTR(node, i));
    }
  }
}

int node_choose_best(Node* node, Rect* r) {
  int j = 0;
  NUM_TYPE jenlarge = INFINITY;
  for (int i = 0; i < node->count; i++) {
    NUM_TYPE uarea = rect_unioned_area(&node->children[i]->mbr, r);
    NUM_TYPE area = rect_area(&node->children[i]->mbr);
    NUM_TYPE enlarge = uarea - area;
    if (enlarge < jenlarge) {
      j = i;
      jenlarge = enlarge;
    }
  }
  return j;
}

SPLIT_MASK best_split_exponential(Node* node) {
  uint64_t best_mask = 0;
  NUM_TYPE best_deadspace = INFINITY;

  for (uint64_t mask = 0; mask < 1ULL << (M + 1); mask++) {
    Rect mbrA, mbrB;
    bool initA = false, initB = false;
    int cA = 0, cB = 0;
    for (int i = 0; i < M + 1; i++) {
      Rect mbr = NODE_GET_ITH_CHILD_MBR(node, i);

      if (mask >> i & 1) {
        mbrA = rect_expand(initA ? &mbrA : &mbr, &mbr);
        cA++;
        initA = true;
      } else {
        mbrB = rect_expand(initB ? &mbrB : &mbr, &mbr);
        cB++;
        initB = true;
      }
    }

    if (cA > M || cB > M || cA < m || cB < m)
      continue;

    NUM_TYPE ds =
        rect_unioned_area(&mbrA, &mbrB) - rect_area(&mbrA) - rect_area(&mbrB);
    if (ds < best_deadspace) {
      best_mask = mask;
      best_deadspace = ds;
    }
  }

  return best_mask;
}

SPLIT_MASK best_split_bad(void) {
  uint64_t full_mask = (1ULL << (M + 1)) - 1;
  uint64_t clear_mask = (1ULL << ((M + 1) / 2)) - 1;
  return full_mask & ~clear_mask;
}

void node_split(Node* node, Node** sibling_out) {
  Node* sibling = node_new(node->kind);

  SPLIT_MASK split_mask = best_split_exponential(node);

  int j = 0;
  int k = 0;
  for (int i = 0; i < M + 1; i++) {
    bool keep = (split_mask >> i) & 1;
    Node* receiptor = keep ? node : sibling;
    int* index = keep ? &j : &k;

    if (node->kind == LEAF) {
      receiptor->data[*index] = node->data[i];
    } else {
      receiptor->children[*index] = node->children[i];
    }

    (*index)++;
  }

  node->count = j;
  sibling->count = k;

  node_fit_mbr(node);
  node_fit_mbr(sibling);

  *sibling_out = sibling;
}

void node_insert(Node* node, Rect r, int id, bool* split) {
  if (node->kind == LEAF) {
    node->mbr = rect_expand(&node->mbr, &r);
    node->data[node->count] = (Item){id, r};
    node->count++;
  } else {
    int i = node_choose_best(node, &r);
    node_insert(node->children[i], r, id, split);

    if (*split) {
      Node* sibling;
      node_split(node->children[i], &sibling);

      node->children[node->count] = sibling;
      node->count++;
    }

    node_fit_mbr(node);
  }
  *split = node->count > M;
}

Rtree* rtree_new(void) {
  Rtree* rtree = malloc(sizeof(Rtree));
  rtree->root = node_new(LEAF);

  return rtree;
}

void rtree_free(Rtree* rtree) {
  node_free(rtree->root);
  free(rtree);
}

void rtree_insert(Rtree* rtree, Rect r, int id) {
  bool split = false;
  node_insert(rtree->root, r, id, &split);

  // Special case: split the root
  if (split) {
    Node* root = node_new(BRANCH);
    root->count = 2;

    Node* sibling;
    node_split(rtree->root, &sibling);

    root->children[0] = rtree->root;
    root->children[1] = sibling;

    node_fit_mbr(root);

    rtree->root = root;
  }
}

void node_delete_rec(Node* node, Rect r, int id, bool* shrunk) {
  if (node->kind == LEAF) {
    for (int i = 0; i < node->count; i++) {
      if (node->data[i].id == id) {
        node->data[i] = node->data[node->count - 1];
        node->count--;
        Rect old_mbr = node->mbr;
        node_fit_mbr(node);

        *shrunk = !rect_equal(&old_mbr, &node->mbr);
      }
    }
  } else {
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->children[i]->mbr, &r)) {
        node_delete_rec(node->children[i], r, id, shrunk);

        if (*shrunk) {
          node_fit_mbr(node);
        }

        if (node->children[i]->count == 0) {
          // TODO le faire pour m au lieu de 0?
          free(node->children[i]);
          node->children[i] = node->children[node->count - 1];
          node->count--;

          Rect old_mbr = node->mbr;
          node_fit_mbr(node);

          *shrunk = !rect_equal(&old_mbr, &node->mbr);
        }
      }
    }
  }
}

void rtree_delete(Rtree* rtree, Rect r, int id) {
  bool shrinked = false;
  node_delete_rec(rtree->root, r, id, &shrinked);

  if (rtree->root->count == 0)
    rtree->root->kind = LEAF;

  if (shrinked)
    node_fit_mbr(rtree->root);
}

void rtree_search_rec(Node* node, Rect* window, ItemList* list) {
  if (node->kind == LEAF) {
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->data[i].r, window)) {
        ItemListNode* head = malloc(sizeof(ItemListNode));
        head->id = node->data[i].id;
        head->next = *list;
        *list = head;
      }
    }
  } else {
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->children[i]->mbr, window)) {
        rtree_search_rec(node->children[i], window, list);
      }
    }
  }
}

ItemList rtree_search(Rtree* rtree, Rect window) {
  ItemList list = NULL;

  rtree_search_rec(rtree->root, &window, &list);

  return list;
}

void itemlist_free(ItemList list) {
  if (list->next != NULL)
    itemlist_free(list->next);

  free(list);
}
