#include "constants.h"
#include "gfx.h"
#include "rtree.h"

#include <stdio.h>

#define COUNT 1000
#define MOVE_PROB 0.9
#define DT 1/60 * 50

#define POINT(x, y)                                                            \
  (Rect) {                                                                     \
    {x, y}, { x, y }                                                           \
  }

typedef struct Vehicle {
  int id;
  double x;
  double y;
  double vx;
  double vy;
} Vehicle;

Graphics gfx;

Vehicle vehicles[COUNT];
Item items[COUNT];
Rtree* rtree;

void draw_rtree(Node* node) {
  if (node->kind == BRANCH) {
    for (int i = 0; i < node->count; i++)
      draw_rtree(node->children[i]);
  } else {
      gfx_draw_rect(&gfx, &node->mbr, (SDL_Color){0, 0, 255, 255});
  }
}

void update(void) {
  gfx_clear(&gfx, (SDL_Color){255, 255, 255, 255});

  for(int i = 0; i < COUNT; i++) {
    if(((float)rand() / (float)RAND_MAX) > MOVE_PROB) {
      bool valid = rtree_delete(rtree, items[i]);

      if(!valid) {
        printf("Couldn't delete node!\n");
      }

      vehicles[i].x += vehicles[i].vx * DT;
      vehicles[i].y += vehicles[i].vy * DT;

      if(vehicles[i].x > WIDTH || vehicles[i].x < 0 || vehicles[i].y > HEIGHT || vehicles[i].y < 0) {
        double vx = vehicles[i].vx;
        vehicles[i].vx = -vehicles[i].vy;
        vehicles[i].vy = vx;
      }

      items[i] = (Item){i, POINT(vehicles[i].x, vehicles[i].y) };

      rtree_insert(rtree, items[i]);
    }

    gfx_draw_circle(&gfx, vehicles[i].x,
              vehicles[i].y, 1, (SDL_Color){0, 0, 0, 255});
  }

  draw_rtree(rtree->root);
  gfx_present(&gfx);
}

int main(void) {
  srand(136399);
  for(int i = 0; i < COUNT; i++) {
    vehicles[i].id = i;
    vehicles[i].x = rand() % (WIDTH - 50) + 30;
    vehicles[i].y = rand() % (HEIGHT - 30) + 10;
    vehicles[i].vx = 2.0 * ((float)rand() / (float)RAND_MAX) - 1.0;
    vehicles[i].vy = 2.0 * ((float)rand() / (float)RAND_MAX) - 1.0;
    double norm = sqrt(vehicles[i].vx*vehicles[i].vx + vehicles[i].vy*vehicles[i].vy);
    if(norm > 0) {
      vehicles[i].vx /= norm;
      vehicles[i].vy /= norm;
    }

    items[i] = (Item){i, POINT(vehicles[i].x, vehicles[i].y)};
  }

  gfx_init(&gfx, "TIPE - Dynamic Vehicles", WIDTH, HEIGHT);
  rtree = rtree_new();
  rtree_bulk_insert(rtree, items, COUNT, HILBERT);

  bool running = true;
  int iter = 0;
  SDL_Event event;
  while (running) {
    printf("iter %i\n", iter);
    iter++;
    if(iter == 3 ) {
      save_screenshot(&gfx, "capture/police1.png");
    }

        if(iter == 200) {
      save_screenshot(&gfx, "capture/police2.png");
    }

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = false;
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          running = false;
          break;
        }
      }
    }
    update();
    SDL_Delay(20);
  }

  rtree_free(rtree);
  gfx_free(&gfx);
}