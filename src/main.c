#include "main.h"
#include "dataset.h"

Rect search_window = {{WIDTH / 4, HEIGHT / 4}, {3 * WIDTH / 4, 3 * HEIGHT / 4}};

#define POINT(x, y)                                                            \
  (Rect) {                                                                     \
    {x, y}, { x, y }                                                           \
  }

#define DATASET_SIZE 100000
Rect dataset[DATASET_SIZE];

Rtree* rtree;
GraphicContext gfx;

void draw_node(Node* node) {
  if (node->kind == BRANCH) {
    for (int i = 0; i < node->count; i++)
      draw_node(node->children[i]);
    sdl2_draw_rect(&gfx, node->mbr.min[0], node->mbr.min[1], node->mbr.max[0],
                   node->mbr.max[1], (SDL_Color){128, 0, 0, 255});
  } else {
    sdl2_draw_rect(&gfx, node->mbr.min[0], node->mbr.min[1], node->mbr.max[0],
                   node->mbr.max[1], (SDL_Color){255, 255, 255, 255});
  }
}

void draw(void) {
  sdl2_clear(&gfx, (SDL_Color){30, 30, 30, 255});

  for (int i = 0; i < DATASET_SIZE; i++) {
    sdl2_draw_circle(&gfx, dataset[i].min[0], dataset[i].min[1], 0.5f,
                     (SDL_Color){0, 255, 0, 255});
  }

  // draw_node(rtree->root);
  sdl2_draw_rect(&gfx, search_window.min[0], search_window.min[1],
                 search_window.max[0], search_window.max[1],
                 (SDL_Color){0, 0, 255, 255});

  ItemList query = rtree_search(rtree, search_window);
  ItemList current = query;
  while (current != NULL) {
    sdl2_draw_circle(&gfx, dataset[current->id].min[0],
                     dataset[current->id].min[1], 0.5f,
                     (SDL_Color){255, 0, 0, 255});
    current = current->next;
  }

  itemlist_free(query);

  sdl2_present(&gfx);
}

void start_gui(void) {
  sdl2_init(&gfx, "TIPE - RTree", WIDTH, HEIGHT);

  draw();

  bool running = true;
  SDL_Event event;

  bool s = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = false;
      if (event.type == SDL_KEYDOWN) {
        if (s) {
          for (int i = 0; i < DATASET_SIZE; i++) {
            rtree_delete(rtree, dataset[i], i);
          }
        } else {
          rtree->root->mbr = dataset[0];
          for (int i = 0; i < DATASET_SIZE; i++) {
            rtree_insert(rtree, dataset[i], i);
          }
        }
        draw();
        s = !s;
      }
    }

    SDL_Delay(50);
  }
}

int main(void) {
  Pair raw_data[DATASET_SIZE];
  load_dataset("us_restaurants", &raw_data[0], DATASET_SIZE);
  for (int i = 0; i < DATASET_SIZE; i++) {
    Pair screen_coords = map_to_screenspace(raw_data[i]);
    dataset[i] = POINT(screen_coords.a, screen_coords.b);
  }

  rtree = rtree_new();
  rtree->root->mbr = dataset[0];
  for (int i = 0; i < DATASET_SIZE; i++) {
    rtree_insert(rtree, dataset[i], i);
  }

  start_gui();

  sdl2_shutdown(&gfx);
  rtree_free(rtree);

  return 0;
}
