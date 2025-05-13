#ifndef TIPE_CONSTANTS
#define TIPE_CONSTANTS

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define N 2

#define NUM_TYPE double
#define NUM_TYPE_ONE 1.0
#define NUM_TYPE_ZERO 0.0

#define SPLIT_MASK uint64_t
#define SPLIT_HEURISTIC 1

// MAX VALUE FOR M IS 63!!
#define M 10
#define m 5

#define WIDTH 1440
#define HEIGHT 810

static double MIN_LON = -125.0, MAX_LON = -66.0;
static double MIN_LAT = 24.0, MAX_LAT = 50.0;

#endif
