#include "main.h"

#include <competitors/quadtree.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define DATASET_SIZE 500000
Item dataset[DATASET_SIZE];

void benchmark_construction(int steps) {
  int step = DATASET_SIZE / steps;
  clock_t times[step];

  for (int i = 0; i < steps; i++) {
    clock_t t1 = clock();

    Rtree* rtree = rtree_new();
    rtree_bulk_insert(rtree, dataset, step * i, HILBERT);
    clock_t t2 = clock();

    times[i] = t2 - t1;
    rtree_free(rtree);
  }

  FILE* fp = fopen("result/benchmark_construction.csv", "w");
  if (fp == NULL) {
    perror("Could not open file for writing");
    return;
  }
  fprintf(fp, "N;BUILDTIME\n");
  for (int i = 0; i < steps; i++) {
    fprintf(fp, "%i;%llu\n", i * step, (long long)times[i]);
  }
}

void benchmark_search(int steps, Rect window) {

  int step = DATASET_SIZE / steps;
  clock_t times_rtree[step];
  clock_t times_naive[step];

  for (int i = 0; i < steps; i++) {
    Rtree* rtree = rtree_new();
    rtree_bulk_insert(rtree, dataset, i * step, HILBERT);

    clock_t t1 = clock();
    ItemList list = rtree_search(rtree, window);
    clock_t t2 = clock();

    times_rtree[i] = t2 - t1;
    rtree_free(rtree);
    itemlist_free(list);
  }

  for (int i = 0; i < steps; i++) {
    clock_t t1 = clock();
    ItemList l = NULL;
    for (int j = 0; j < i * step; j++) {
      if (rect_intersect(&dataset[j].mbr, &window)) {
        ItemList node = malloc(sizeof(ItemListNode));
        node->next = l;
        node->id = dataset[j].id;
        l = node;
      }
    }
    clock_t t2 = clock();
    times_naive[i] = t2 - t1;
    itemlist_free(l);
  }

  // Write to CSV
  FILE* fp = fopen("result/benchmark_search.csv", "w");
  if (fp == NULL) {
    perror("Could not open file for writing");
    return;
  }

  fprintf(fp, "N;NAIVE;RTREE\n");
  for (int i = 0; i < steps; i++) {
    double naive_time = times_naive[i];
    double rtree_time = times_rtree[i];
    fprintf(fp, "%i;%llu;%llu\n", i * step, (long long)naive_time,
            (long long)rtree_time);
  }
}

void benchmark_n_squared(int steps) {
  int step = DATASET_SIZE / steps;
  clock_t times_rtree[steps];
  clock_t times_naive[steps];
  clock_t times_qtree[steps];

  for (volatile int i = 0; i < steps; i++) {
    Rtree* rtree = rtree_new();
    rtree_bulk_insert(rtree, dataset, i * step, HILBERT);

    volatile clock_t t1 = clock();

    volatile double best = INFINITY;
    for (volatile int j = 0; j < i * step; j++) {
      ItemList list = rtree_search(
          rtree,
          (Rect){{dataset[j].mbr.min[0] - 5, dataset[j].mbr.min[1] - 5},
                 {dataset[j].mbr.min[0] + 5, dataset[j].mbr.min[1] + 5}});
      ItemList next = list;
      while (next != NULL) {
        volatile double euclidean_distance =
            sqrt((dataset[j].mbr.min[0] - dataset[next->id].mbr.min[0]) *
                 (dataset[j].mbr.min[0] - dataset[next->id].mbr.min[0]));
        if (euclidean_distance < best)
          best = euclidean_distance;

        next = next->next;
      }

      itemlist_free(list);
    }
    volatile clock_t t2 = clock();

    times_rtree[i] = t2 - t1;
    rtree_free(rtree);
  }

  printf("finished rtree\n");
  for (volatile int i = 0; i < steps; i++) {

    Quadtree* qtree = quadtree_new((Rect){{0, 0}, {WIDTH, HEIGHT}});

    for (int j = 0; j < i * step; j++) {
      quadtree_insert(qtree, dataset[j]);
    }

    volatile clock_t t1 = clock();

    volatile double best = INFINITY;
    for (volatile int j = 0; j < i * step; j++) {
      ItemList list = quadtree_search(
          qtree,
          (Rect){{dataset[j].mbr.min[0] - 5, dataset[j].mbr.min[1] - 5},
                 {dataset[j].mbr.min[0] + 5, dataset[j].mbr.min[1] + 5}});
      ItemList next = list;
      while (next != NULL) {
        volatile double euclidean_distance =
            sqrt((dataset[j].mbr.min[0] - dataset[next->id].mbr.min[0]) *
                 (dataset[j].mbr.min[0] - dataset[next->id].mbr.min[0]));
        if (euclidean_distance < best)
          best = euclidean_distance;

        next = next->next;
      }

      itemlist_free(list);
    }
    volatile clock_t t2 = clock();
    times_qtree[i] = t2 - t1;
    quadtree_free(qtree);
  }
  printf("finished qtree\n");

  for (volatile int i = 0; i < min(steps, 20); i++) {
    clock_t t1 = clock();

    volatile double best = INFINITY;
    for (volatile int j = 0; j < i * step; j++) {
      for (volatile int k = 0; k < i * step; k++) {
        volatile double euclidean_distance =
            sqrt((dataset[j].mbr.min[0] - dataset[k].mbr.min[0]) *
                 (dataset[j].mbr.min[0] - dataset[k].mbr.min[0]));
        if (euclidean_distance < best)
          best = euclidean_distance;
      }
    }

    clock_t t2 = clock();
    times_naive[i] = t2 - t1;
  }
  printf("finished naive\n");

  // Write to CSV
  FILE* fp = fopen("result/benchmark_n2.csv", "w");
  if (fp == NULL) {
    perror("Could not open file for writing");
    return;
  }

  fprintf(fp, "N;NAIVE;QTREE;RTREE\n");
  for (int i = 0; i < steps; i++) {
    double naive_time = times_naive[i];
    double qtree_time = times_qtree[i];
    double rtree_time = times_rtree[i];

    fprintf(fp, "%i;%llu;%llu;%llu\n", i * step, (long long)naive_time,
            (long long)qtree_time, (long long)rtree_time);
  }
}

int main(void) {
  read_dataset_from_file("us_places");
  Rect search_window = {{800, 500}, {900, 750}};

  // benchmark_construction(20);
  // benchmark_search(20, search_window);
  benchmark_n_squared(20);
  return 0;
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
