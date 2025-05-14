#ifndef TIPE_CONSTANTS
#define TIPE_CONSTANTS

#define N 2

#define NUM_TYPE long double
#define NUM_TYPE_ONE 1.0
#define NUM_TYPE_ZERO 0.0

#define SPLIT_MASK uint64_t
#define SPLIT_HEURISTIC 2
// 0 exponential; 1 quadratic; 2 Greene's split; 3 R* split

// MAX VALUE FOR M IS 63!!
#define M 30
#define m 15

#define WIDTH 1440
#define HEIGHT 810

static double MIN_LON = -125.0, MAX_LON = -66.0;
static double MIN_LAT = 24.0, MAX_LAT = 50.0;

#endif
