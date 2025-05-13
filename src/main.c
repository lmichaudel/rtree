#include "main.h"
#include "dataset.h"

Rect search_window = {{WIDTH / 4, HEIGHT / 4}, {3 * WIDTH / 4, 3 * HEIGHT / 4}};

#define POINT(x, y)                                                            \
  (Rect) {                                                                     \
    {x, y}, { x, y }                                                           \
  }

#define DATASET_SIZE 1000
Rect dataset[DATASET_SIZE];

Rtree* rtree;
Graphics gfx;

bool is_rtree_constructed = true;
bool draw_leaves = true;
bool draw_branches = true;

SDL_Color colors[5] = {
    (SDL_Color){75, 0, 0, 255}, (SDL_Color){128, 128, 0, 255},
    (SDL_Color){200, 0, 255, 255}, (SDL_Color){255, 100, 100, 255}};

void draw_node(Node* node, int d) {
  if (node->kind == BRANCH) {
    for (int i = 0; i < node->count; i++)
      draw_node(node->children[i], d + 1);
    if (draw_branches)
      graphics_draw_rect(&gfx, &node->mbr, colors[d]);
  } else {
    if (draw_leaves) {
      graphics_draw_rect(&gfx, &node->mbr, (SDL_Color){255, 255, 255, 120});
      for (int i = 0; i < node->count; i++) {
        graphics_draw_circle(&gfx, node->data[i].r.min[0],
                             node->data[i].r.min[1], 1,
                             (SDL_Color){0, 255, 0, 255});
      }
    }
  }
}

void draw(void) {
  graphics_clear(&gfx, (SDL_Color){30, 30, 30, 255});

  draw_node(rtree->root, 0);

  graphics_draw_rect(&gfx, &search_window, (SDL_Color){0, 0, 255, 255});

  ItemList query = rtree_search(rtree, search_window);
  ItemList current = query;
  while (current != NULL) {
    // graphics_draw_circle(&gfx, dataset[current->id].min[0],
    //  dataset[current->id].min[1], 1,
    //  (SDL_Color){255, 0, 0, 255});
    current = current->next;
  }

  itemlist_free(query);

  graphics_present(&gfx);
}

void start_gui(void) {
  graphics_init(&gfx, "TIPE - RTree", WIDTH, HEIGHT);

  draw();

  int i = 0;
  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = false;
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          running = false;
          break;
        case SDLK_e:
          while (
              !rect_intersect(&dataset[i], &(Rect){{0, 0}, {WIDTH, HEIGHT}})) {
            i = (i + 1) % DATASET_SIZE;
          }
          rtree_delete(rtree, dataset[i], i);
          i = (i + 1) % DATASET_SIZE;
          rtree_debug(rtree);
          break;
        case SDLK_SPACE:
          if (is_rtree_constructed) {
            clear_rtree();
          } else {
            construct_rtree();
          }

          is_rtree_constructed = !is_rtree_constructed;
          break;
        case SDLK_a:
          draw_leaves = !draw_leaves;
          break;
        case SDLK_z:
          draw_branches = !draw_branches;
          break;
        default:
          break;
        }
        draw();
      }
    }

    SDL_Delay(50);
  }
}

void load_dataset(void) {
  Pair raw_data[DATASET_SIZE];
  load_dataset_from_file("us_restaurants", &raw_data[0], DATASET_SIZE);
  for (int i = 0; i < DATASET_SIZE; i++) {
    Pair screen_coords = map_to_screenspace(raw_data[i]);
    dataset[i] = POINT(screen_coords.a, screen_coords.b);
  }
}

void construct_rtree(void) {
  rtree->root->mbr = dataset[0];
  for (int i = 0; i < DATASET_SIZE; i++) {
    if (!rect_intersect(&dataset[i], &(Rect){{0, 0}, {WIDTH, HEIGHT}})) {
      continue;
    }
    rtree_insert(rtree, dataset[i], i);
  }

  rtree_debug(rtree);
}

void clear_rtree(void) {
  for (int i = 0; i < DATASET_SIZE; i++) {
    if (!rect_intersect(&dataset[i], &(Rect){{0, 0}, {WIDTH, HEIGHT}})) {
      continue;
    }
    rtree_delete(rtree, dataset[i], i);
  }

  rtree_debug(rtree);
}

int main(void) {
  load_dataset();
  rtree = rtree_new();
  construct_rtree();

  start_gui();

  graphics_shutdown(&gfx);
  rtree_free(rtree);

  return 0;
}
