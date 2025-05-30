#include "rtree.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_GET_ITH_CHILD_MBR(node, i)                                        \
  (node->kind == LEAF ? node->data[i].mbr : node->children[i]->mbr)
#define NODE_GET_ITH_CHILD_MBR_PTR(node, i)                                    \
  (node->kind == LEAF ? &node->data[i].mbr : &node->children[i]->mbr)

void split_pick_seeds(Node* node, int* seed1, int* seed2) {
  NUM_TYPE worst_d = -1;
  for (int i = 0; i < M + 1; i++) {
    Rect ri = NODE_GET_ITH_CHILD_MBR(node, i);
    for (int j = i + 1; j < M + 1; j++) {
      Rect rj = NODE_GET_ITH_CHILD_MBR(node, j);
      NUM_TYPE d = rect_dead_space(&ri, &rj);
      if (d > worst_d) {
        worst_d = d;
        *seed1 = i;
        *seed2 = j;
      }
    }
  }
}

int compare_item_on_axis(void* _k, const void* _a, const void* _b) {
  int k = *(int*)_k;
  Item a = *(Item*)_a;
  Item b = *(Item*)_b;
  return a.mbr.min[k] - b.mbr.min[k];
}

int compare_nodes_on_axis(void* _k, const void* _a, const void* _b) {
  int k = *(int*)_k;
  Node* a = *(Node**)_a;
  Node* b = *(Node**)_b;

  return a->mbr.min[k] - b->mbr.min[k];
}

SPLIT_MASK split_greene(Node* node) {
  int seed1, seed2;
  split_pick_seeds(node, &seed1, &seed2);

  int greatest_separation_axis = 0;
  NUM_TYPE greatest_separation = -1.0f;

  for (int k = 0; k < N; k++) {
    NUM_TYPE separation = fabsl(NODE_GET_ITH_CHILD_MBR(node, seed1).min[k] -
                                NODE_GET_ITH_CHILD_MBR(node, seed2).min[k]);
    NUM_TYPE relative_separation =
        fabsl(separation / (node->mbr.max[k] - node->mbr.min[k]));

    if (relative_separation > greatest_separation) {
      greatest_separation = relative_separation;
      greatest_separation_axis = k;
    }
  }

  if (node->kind == BRANCH) {
    qsort_r(&node->children[0], M + 1, sizeof(Node*), &greatest_separation_axis,
            compare_nodes_on_axis);
  } else {
    qsort_r(&node->data[0], M + 1, sizeof(Item), &greatest_separation_axis,
            compare_item_on_axis);
  }

  uint64_t full_mask = (1ULL << (M + 1)) - 1;
  uint64_t clear_mask = (1ULL << ((M + 1) / 2)) - 1;
  return full_mask & ~clear_mask;
}

SPLIT_MASK split_exponential(Node* node) {
  uint64_t best_mask = 0;
  NUM_TYPE best_deadspace = INFINITY;

  for (uint64_t mask = 0; mask < 1ULL << (M + 1); mask++) {
    Rect mbrA = uninitialized_rect(), mbrB = uninitialized_rect();
    int cA = 0, cB = 0;
    for (int i = 0; i < M + 1; i++) {
      Rect mbr = NODE_GET_ITH_CHILD_MBR(node, i);

      if (mask >> i & 1) {
        mbrA = rect_union(&mbrA, &mbr);
        cA++;
      } else {
        mbrB = rect_union(&mbrB, &mbr);
        cB++;
      }
    }

    if (cA > M || cB > M || cA < m || cB < m)
      continue;

    NUM_TYPE ds = rect_dead_space(&mbrA, &mbrB);
    if (ds < best_deadspace) {
      best_mask = mask;
      best_deadspace = ds;
    }
  }

  return best_mask;
}

SPLIT_MASK split_quadratic(Node* node) {
  int seed1, seed2;
  split_pick_seeds(node, &seed1, &seed2);

  // Step 2: Distribute entries
  uint64_t mask = 0;
  int groupA_count = 1;
  int groupB_count = 1;
  Rect mbrA = NODE_GET_ITH_CHILD_MBR(node, seed1);
  Rect mbrB = NODE_GET_ITH_CHILD_MBR(node, seed2);

  mask |= (1ULL << seed1); // assign to A
  // assign to B by default, so we don't set bit for seed2

  for (int i = 0; i < M + 1; i++) {
    if (i == seed1 || i == seed2)
      continue;

    Rect r = NODE_GET_ITH_CHILD_MBR(node, i);
    NUM_TYPE enlargeA = rect_unioned_area(&mbrA, &r) - rect_area(&mbrA);
    NUM_TYPE enlargeB = rect_unioned_area(&mbrB, &r) - rect_area(&mbrB);

    if ((groupA_count + (M + 1 - i)) == m) {
      mask |= (1ULL << i);
      mbrA = rect_union(&mbrA, &r);
      groupA_count++;
      continue;
    }
    if ((groupB_count + (M + 1 - i)) == m) {
      // default to group B
      mbrB = rect_union(&mbrB, &r);
      groupB_count++;
      continue;
    }

    if (enlargeA < enlargeB ||
        (enlargeA == enlargeB && rect_area(&mbrA) < rect_area(&mbrB))) {
      mask |= (1ULL << i);
      mbrA = rect_union(&mbrA, &r);
      groupA_count++;
    } else {
      mbrB = rect_union(&mbrB, &r);
      groupB_count++;
    }
  }

  return mask;
}

Node* node_new(Kind kind) {
  Node* node = malloc(sizeof(Node));
  if (node == NULL) {
    printf("(node_new) malloc failed.\n");
    abort();
  }
  node->count = 0;
  node->kind = kind;

  node->mbr = uninitialized_rect();

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
    for (int i = 0; i < node->count; i++) {
      node->mbr = rect_union(&node->mbr, NODE_GET_ITH_CHILD_MBR_PTR(node, i));
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

void node_split(Node* node, Node** sibling_out) {
  Node* sibling = node_new(node->kind);

  SPLIT_MASK split_mask =
#if SPLIT_HEURISTIC == 0
      split_exponential(node);
#elif SPLIT_HEURISTIC == 1
      split_quadratic(node);
#elif SPLIT_HEURISTIC == 2
      split_greene(node);
#endif

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
    node->mbr = rect_union(&node->mbr, &r);
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

bool node_delete(Rtree* rtree, Node* node, Rect r, int id, bool* shrink) {
  if (node->kind == LEAF) {
    for (int i = 0; i < node->count; i++) {
      if (node->data[i].id == id) {
        node->data[i] = node->data[node->count - 1];
        node->count--;
        Rect old_mbr = node->mbr;
        node_fit_mbr(node);

        *shrink = !rect_equal(&old_mbr, &node->mbr);
        return true;
      }
    }
    return false;
  } else {
    bool v = false;
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->children[i]->mbr, &r)) {
        bool t = node_delete(rtree, node->children[i], r, id, shrink);

        Rect old_mbr = node->mbr;

        if (*shrink) {
          node_fit_mbr(node);
        }

        if (node->children[i]->count == 0) {
          free(node->children[i]);
          node->children[i] = node->children[node->count - 1];
          node->count--;

          node_fit_mbr(node);
        }

        *shrink = !rect_equal(&old_mbr, &node->mbr);

        v = v || t;
      }
    }
    return v;
  }
}

Rtree* rtree_new(void) {
  Rtree* rtree = malloc(sizeof(Rtree));
  if (rtree == NULL) {
    printf("(rtree_new) malloc failed.\n");
    abort();
  }
  rtree->root = node_new(LEAF);

  return rtree;
}

void rtree_free(Rtree* rtree) {
  node_free(rtree->root);
  free(rtree);
}

void rtree_insert(Rtree* rtree, Item i) {
  bool split = false;
  node_insert(rtree->root, i.mbr, i.id, &split);

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

bool rtree_delete(Rtree* rtree, Item i) {
  bool shrink = false;
  Rect r = i.mbr;

  if (N == 2) {
    r.min[0] -= 1;
    r.min[1] -= 1;
    r.max[0] += 1;
    r.max[1] += 1;
  }
  bool t = node_delete(rtree, rtree->root, i.mbr, i.id, &shrink);

  if (shrink)
    node_fit_mbr(rtree->root);

  if (rtree->root->kind == BRANCH && rtree->root->count == 1) {
    Node* old_root = rtree->root;
    rtree->root = rtree->root->children[0];
    free(old_root);
  } else if (rtree->root->count == 0) {
    free(rtree->root);
    rtree->root = node_new(LEAF);
  }

  return t;
}

void node_search(Node* node, Rect* window, ItemList* list) {
  if (node->kind == LEAF) {
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->data[i].mbr, window)) {
        ItemListNode* head = malloc(sizeof(ItemListNode));
        if (head == NULL) {
          printf("(rtree_search) malloc failed.\n");
          abort();
        }
        head->id = node->data[i].id;
        head->next = *list;
        *list = head;
      }
    }
  } else {
    for (int i = 0; i < node->count; i++) {
      if (rect_intersect(&node->children[i]->mbr, window)) {
        node_search(node->children[i], window, list);
      }
    }
  }
}

ItemList rtree_search(Rtree* rtree, Rect window) {
  ItemList list = NULL;

  node_search(rtree->root, &window, &list);

  return list;
}

int rtree_depth(Node* node) {
  if (node->kind == BRANCH) {
    int maxd = 0;
    for (int i = 0; i < node->count; i++) {
      int d = rtree_depth(node->children[i]);
      if (d != maxd && i > 0)
        printf("Unbalanced!\n");
      maxd = maxd > d ? maxd : d;
    }
    return 1 + maxd;
  }

  return 0;
}

int rtree_count(Node* node) {
  if (node->kind == BRANCH) {
    int r = 0;
    for (int i = 0; i < node->count; i++) {
      r += rtree_count(node->children[i]);
    }
    return r;
  }

  return node->count;
}

int rtree_node_count(Node* node) {
  if (node->kind == BRANCH) {
    int r = 0;
    for (int i = 0; i < node->count; i++) {
      r += rtree_node_count(node->children[i]);
    }
    return r + node->count;
  }

  return 1;
}

int rtree_leaf_count(Node* node) {
  if (node->kind == BRANCH) {
    int r = 0;
    for (int i = 0; i < node->count; i++) {
      r += rtree_leaf_count(node->children[i]);
    }
    return r;
  }

  return 1;
}

void rtree_debug(Rtree* rtree) {
  printf("-- RTree checkhealth --\n");
  printf("Depth %i\n", rtree_depth(rtree->root));
  printf("Entries %i\n", rtree_count(rtree->root));
  int node_count = rtree_node_count(rtree->root);
  int leaf_count = rtree_leaf_count(rtree->root);
  printf("Node %i\n", node_count);
  printf("Leaf %i\n", leaf_count);
  printf("Memory usage %f mb.\n",
         (node_count + leaf_count) * sizeof(Node) / (1024.0 * 1024.0));
  printf("%f\n", 1000000 * sizeof(Item) / (1024.0 * 1024.0));
  printf("-----------------------\n\n");
}

// BULK LOADING
void rot(int n, int* x, int* y, int rx, int ry) {
  if (ry == 0) {
    if (rx == 1) {
      *x = n - 1 - *x;
      *y = n - 1 - *y;
    }
    // Swap x and y
    int t = *x;
    *x = *y;
    *y = t;
  }
}

uint64_t hilbert_index(int n, int x, int y) {
  uint64_t d = 0;
  for (int s = n / 2; s > 0; s /= 2) {
    int rx = (x & s) > 0;
    int ry = (y & s) > 0;
    d += s * s * ((3 * rx) ^ ry);
    rot(s, &x, &y, rx, ry);
  }
  return d;
}

int compare_hilbert(const void* a, const void* b) {
  Item p1 = *(Item*)a;
  Item p2 = *(Item*)b;
  int n = 1 << 16;
  uint64_t h1 = hilbert_index(n, p1.mbr.min[0], p1.mbr.min[1]);
  uint64_t h2 = hilbert_index(n, p2.mbr.min[0], p2.mbr.min[1]);
  return (h1 > h2) - (h1 < h2);
}

int compare_x(const void* _a, const void* _b) {
  Item a = *(Item*)_a;
  Item b = *(Item*)_b;
  return a.mbr.min[0] - b.mbr.min[0];
}

void rtree_bulk_insert(Rtree* rtree, Item* data, int count, BulkMode mode) {
  Item* data_copy = malloc(sizeof(Item) * count);
  if (data_copy == NULL) {
    printf("(rtree_bulk_insert) malloc failed.\n");
    abort();
  }
  for (int i = 0; i < count; i++) {
    data_copy[i] = data[i];
  }

  int (*cmp)(const void*, const void*) = compare_hilbert;

  switch (mode) {
  case X_SORT:
    cmp = compare_x;
    break;
  case HILBERT:
    cmp = compare_hilbert;
    break;
  default:;
  }

  if (mode != NAIVE)
    qsort(data_copy, count, sizeof(Item), cmp);

  for (int i = 0; i < count; i++) {
    rtree_insert(rtree, data_copy[i]);
  }
}
