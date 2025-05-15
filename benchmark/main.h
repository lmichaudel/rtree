#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "constants.h"

void map_to_screenspace(NUM_TYPE lon, NUM_TYPE lat, NUM_TYPE* x, NUM_TYPE* y);
void read_dataset_from_file(char* filename);

#endif
