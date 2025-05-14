#include "main.h"

int main(void) {
  init();
  loop();
  shutdown();

  return 0;
}

void init(void) {
  read_dataset_from_file("us_places");
  gfx_init(&gfx, "TIPE - RTree", WIDTH, HEIGHT);

  rtree = rtree_new();
  construct_rtree();
}

void shutdown(void) {
  rtree_free(rtree);
  gfx_free(&gfx);
}

void draw_node(Node* node, int d) {
  if (node->kind == BRANCH) {
    for (int i = 0; i < node->count; i++)
      draw_node(node->children[i], d + 1);
  } else {
    if (draw_leaves) {
      gfx_draw_rect(&gfx, &node->mbr, LEAF_COLOR);
      for (int i = 0; i < node->count; i++) {
        gfx_draw_circle(&gfx, node->data[i].mbr.min[0],
                        node->data[i].mbr.min[1], 1, NODE_COLOR);
      }
    }
  }
}

void draw(void) {
  gfx_clear(&gfx, BACKGROUND_COLOR);

  draw_node(rtree->root, 0);

  gfx_draw_rect(&gfx, &search_window, FOUND_COLOR);

  ItemList query = rtree_search(rtree, search_window);
  ItemList current = query;
  while (current != NULL) {
    gfx_draw_circle(&gfx, dataset[current->id].mbr.min[0],
                    dataset[current->id].mbr.min[1], 1, FOUND_COLOR);
    current = current->next;
  }

  itemlist_free(query);

  gfx_present(&gfx);
}

void loop(void) {

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
          while (!rect_intersect(&dataset[i].mbr,
                                 &(Rect){{0, 0}, {WIDTH, HEIGHT}})) {
            i = (i + 1) % DATASET_SIZE;
          }
          rtree_delete(rtree, dataset[i]);
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

void construct_rtree(void) {
  rtree_bulk_insert(rtree, &dataset[0], DATASET_SIZE, HILBERT);
  rtree_debug(rtree);
}

void clear_rtree(void) {
  for (int i = 0; i < DATASET_SIZE; i++) {
    rtree_delete(rtree, dataset[i]);
  }

  rtree_debug(rtree);
}

void map_to_screenspace(NUM_TYPE lon, NUM_TYPE lat, NUM_TYPE* x, NUM_TYPE* y) {
  double xNorm = (lon - MIN_LON) / (MAX_LON - MIN_LON);
  double yNorm = 1.0 - (lat - MIN_LAT) / (MAX_LAT - MIN_LAT);

  *x = (xNorm * WIDTH);
  *y = (yNorm * HEIGHT);
}

void read_dataset_from_file(char* filename) {
  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    perror("Error opening binary file");
    return;
  }

  size_t count;
  if (fread(&count, sizeof(size_t), 1, file) != 1) {
    fprintf(stderr, "Failed to read coordinate count.\n");
    fclose(file);
    return;
  }

  double* coords = malloc(2 * count * sizeof(double));
  if (coords == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    fclose(file);
    return;
  }

  if (fread(coords, sizeof(double), 2 * count, file) != 2 * count) {
    fprintf(stderr, "Failed to read all coordinates.\n");
    free(coords);
    fclose(file);
    return;
  }

  fclose(file);

  for (size_t i = 0; i < DATASET_SIZE; ++i) {
    double lon = coords[2 * i];
    double lat = coords[2 * i + 1];
    NUM_TYPE x, y;
    map_to_screenspace(lon, lat, &x, &y);
    dataset[i] = (Item){i, POINT(x, y)};
  }

  printf("Loaded %zu entries.\n", count);

  free(coords);
}
