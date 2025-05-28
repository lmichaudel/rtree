#ifndef TIPE_MAIN
#define TIPE_MAIN

#include "competitors/quadtree.h"
#include "constants.h"
#include "gfx.h"
#include "rtree.h"

#define DATASET_SIZE 50000

#define POINT(x, y)                                                            \
  (Rect) {                                                                     \
    {x, y}, { x, y }                                                           \
  }

const SDL_Color BACKGROUND_COLOR = {255, 255, 255, 255};
const SDL_Color QUERY_COLOR = {255, 0, 255, 122};
const SDL_Color NODE_COLOR = {0, 0, 100, 100};
const SDL_Color FOUND_COLOR = {255, 0, 0, 255};
const SDL_Color LEAF_COLOR = {0, 0, 0, 100};

Rect search_window = {{800, 500}, {1200, 750}};

Item dataset[DATASET_SIZE];

Rtree* rtree;
Quadtree* qtree;
Graphics gfx;

bool is_rtree_constructed = true;
bool draw_leaves = true;
bool draw_branches = false;

int main(void);

void init(void);
void draw(void);
void shutdown(void);
void loop(void);

void draw_rtree(Node* node, int d);
void draw_qtree(Quadtree* qtree);

void construct_rtree(void);
void clear_rtree(void);

void construct_qtree(void);

void map_to_screenspace(NUM_TYPE lon, NUM_TYPE lat, NUM_TYPE* x, NUM_TYPE* y);
void read_dataset_from_file(char* filename);
#endif
