#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

typedef struct Pair {
  double a;
  double b;
} Pair;

static bool load_dataset(char* filename, Pair* data, size_t dataset_size) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Error: Could not open input file.\n");
    return 0;
  }

  size_t sizeInFile;
  if (fread(&sizeInFile, sizeof(sizeInFile), 1, file) != 1) {
    fprintf(stderr, "Error: Could not read size from file.\n");
    fclose(file);
    return 0;
  }

  if (sizeInFile > dataset_size) {
    sizeInFile = dataset_size;
  }

  if (fread(data, sizeof(Pair), sizeInFile, file) != sizeInFile) {
    fprintf(stderr, "Error: Failed to read coordinate data.\n");
    fclose(file);
    return 0;
  }

  fclose(file);
  return sizeInFile;
}

static Pair map_to_screenspace(Pair world_coordinates) {
  double xNorm = (world_coordinates.b - MIN_LON) / (MAX_LON - MIN_LON);
  double yNorm = 1.0 - (world_coordinates.a - MIN_LAT) / (MAX_LAT - MIN_LAT);

  double x = (xNorm * WIDTH);
  double y = (yNorm * HEIGHT);

  return (Pair){x, y};
}
