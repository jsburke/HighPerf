// -*- C++ -*-
//
// gcc test_dot.c -lrt -o tdt && ./tdt
//
// NOTE:
//   This program runs so quickly that the SpeedStep control (in kernel and/or
// on-chip) doesn't catch up in time for the first test to be measured. A simple
// say to get around this is to always compile the source first (as shown above)
// but a better solution would be to run some kind of busy loop first.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define BASE 0
#define DELTA 5000
#define ITERS 20

#define ITER_VECLEN(i) (BASE+((i)+1)*DELTA)

#define FILE_PREFIX ((const unsigned char*) "dotProd_")

typedef double data_t;

/* ---------------------------------------------------------------------------
| Make the CPU busy, and measure CPS (cycles per second).
|
| Explanation:
| If tests are very fast, they can run so quickly that the SpeedStep control
| (in kernel and/or on-chip) doesn't notice in time, and the first few tests
| might finish while the CPU is still in its sleep state (about 800 MHz,
| judging from my measurements)
|   A simple way to get around this is to run some kind of busy-loop that
| forces the OS and/or CPU to notice it needs to go to full clock speed.
| We print out the results of the computation so the loop won't get optimised
| away.
|
| Copy this code into other programs as desired. It provides three entry
| points:
|
| double ts_sec(ts): converts a timespec into seconds
| timespec ts_diff(ts1, ts2): computes interval between two timespecs
| measure_cps(): Does the busy loop and prints out measured CPS (cycles/sec)
--------------------------------------------------------------------------- */
double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime
int clock_gettime(clockid_t clk_id, struct timespec *tp);

typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} mcps_tctr;

#define MCPS_RDTSC(cpu_c) __asm__ __volatile__ ("rdtsc" : \
                     "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))

double ts_sec(struct timespec ts);
struct timespec ts_diff(struct timespec start, struct timespec end);
double measure_cps(void);

double ts_sec(struct timespec ts)
{
  return ((double)(ts.tv_sec)) + ((double)(ts.tv_nsec))/1.0e9;
}

struct timespec ts_diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

double measure_cps()
{
  struct timespec cal_start, cal_end;
  mcps_tctr tsc_start, tsc_end;
  double total_time;
  double total_cycles;
  /* We perform a chaotic iteration and print the result, to defeat
     compiler optimisation */
  double chaosC = -1.8464323952913974; double z = 0.0;
  long int i, ilim, j;

  /* Do it twice and throw away results from the first time; this ensures the
   * OS and CPU will notice it's busy and set the clock speed. */
  for(j=0; j<2; j++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cal_start);
    MCPS_RDTSC(tsc_start);
    ilim = 50*1000*1000;
    for (i=0; i<ilim; i++)
      z = z * z + chaosC;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cal_end);
    MCPS_RDTSC(tsc_end);
  }

  total_time = ts_sec(ts_diff(cal_start, cal_end));
  total_cycles = (double)(tsc_end.int64-tsc_start.int64);
  CPS = total_cycles / total_time;
  printf("z == %f, CPS == %g\n", z, CPS);

  return CPS;
}
/* ---------------------------------------------------------------------------
| End of measure_cps code
--------------------------------------------------------------------------- */


/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;


/* Prototypes */
long int get_vec_length(vec_ptr v);
vec_ptr new_vec(long int len);
int set_vec_length(vec_ptr v, long int index);
int init_vector(vec_ptr v, long int len);
data_t *get_vec_start(vec_ptr v);



/* Each of variation of the task being benchmarked */
double dotprod1(vec_ptr a, vec_ptr b);
double dotprod2(vec_ptr a, vec_ptr b);
double dotprod3(vec_ptr a, vec_ptr b);
double dotprod4(vec_ptr a, vec_ptr b);
#define METHODS 4


/*****************************************************************************/
int main(int argc, char *argv[])
{
  struct timespec time1, time2;
  struct timespec time_stamp[ITERS+1][METHODS];
  long int elements[ITERS+1];
  long int i, j, k;
  long int maxsize = ITER_VECLEN(ITERS);
  double grand_total;

  measure_cps();
  grand_total = 0.0;

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(maxsize);
  vec_ptr v1 = new_vec(maxsize);
  init_vector(v0, maxsize);
  init_vector(v1, maxsize);

  for (i = 0; i < ITERS; i++) {
    elements[i] = ITER_VECLEN(i);
    set_vec_length(v0, elements[i]);
    set_vec_length(v1, elements[i]);

    /* Measure each different type of optimisation */

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    grand_total += dotprod1(v0, v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[i][0] = ts_diff(time1,time2);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    grand_total += dotprod2(v0, v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[i][1] = ts_diff(time1,time2);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    grand_total += dotprod3(v0, v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[i][2] = ts_diff(time1,time2);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    grand_total += dotprod4(v0, v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[i][3] = ts_diff(time1,time2);
  }
  printf("All tests done; grand total = %g\n", grand_total);

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n", filename);
  fp = fopen(filename, "w");

  /* output times */
  fprintf(fp, "Vector Size, ");
  fprintf(fp, "Simple Loop, Unrolled 2x, ");
  fprintf(fp, "Two Accumulators, 2x Associativity, ");
  fprintf(fp, "\n");
  for (i = 0; i < ITERS; i++) {
    double cycles;
    fprintf(fp, "%ld", elements[i]);

    cycles = CPS * ts_sec(time_stamp[i][0]);
    fprintf(fp, ", %f", cycles / elements[i]);

    cycles = CPS * ts_sec(time_stamp[i][1]);
    fprintf(fp, ", %f", cycles / elements[i]);

    cycles = CPS * ts_sec(time_stamp[i][2]);
    fprintf(fp, ", %f", cycles / elements[i]);

    cycles = CPS * ts_sec(time_stamp[i][3]);
    fprintf(fp, ", %f", cycles / elements[i]);

    fprintf(fp, "\n");
  }
  fclose(fp);

  return 0;
}/* end main */

/**********************************************/
/* Create vector of specified length */
vec_ptr new_vec(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len, sizeof(data_t));
    if (!data) {
	  free((void *) result);
	  return NULL;  /* Couldn't allocate storage */
	}
	result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
  return v->len;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
  v->len = index;
  return 1;
}

/* initialize vector */
int init_vector(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = (data_t)(i);
    }
    return 1;
  }
  // else
  return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/*************************************************/

/* Dot product, Accumulating result in a local variable
 * (This is analogous to the 'Combine4' version of the vector reduction
 * functions in test_combine1-7.c) */
double dotprod1(vec_ptr a, vec_ptr b)
{
  long int i;
  long int alen = get_vec_length(a);
  long int blen = get_vec_length(b);
  data_t *adata = get_vec_start(a);
  data_t *bdata = get_vec_start(b);

  data_t acc = 0;
  if (alen == blen) {
    for (i = 0; i < alen; i++) {
      acc = acc + (adata[i] * bdata[i]);
    }
  }
  return acc;
}

/* This version unrolls the loop a bit. */
double dotprod2(vec_ptr a, vec_ptr b)
{
  long int i;
  long int alen = get_vec_length(a);
  long int blen = get_vec_length(b);
  data_t *adata = get_vec_start(a);
  data_t *bdata = get_vec_start(b);

  data_t acc = 0;
  if (alen == blen) {
    for (i = 0; i < alen; i+=2) {
      acc = acc + (adata[i] * bdata[i]);
      acc = acc + (adata[i+1] * bdata[i+1]);
    }
    if (i < alen) {
      acc = acc + (adata[i] * bdata[i]);
    }
  }
  return acc;
}

/* Unrolling didn't help; let's try interleaving nondependent computations. */
double dotprod3(vec_ptr a, vec_ptr b)
{
  long int i;
  long int alen = get_vec_length(a);
  long int blen = get_vec_length(b);
  data_t *adata = get_vec_start(a);
  data_t *bdata = get_vec_start(b);

  data_t acc0 = 0;
  data_t acc1 = 0;
  if (alen == blen) {
    for (i = 0; i < alen; i+=2) {
      acc0 = acc0 + (adata[i] * bdata[i]);
      acc1 = acc1 + (adata[i+1] * bdata[i+1]);
    }
    if (i < alen) {
      acc0 = acc0 + (adata[i] * bdata[i]);
    }
  }
  return acc0 + acc1;
}

/* Interleaving but with only one accumulator */
double dotprod4(vec_ptr a, vec_ptr b)
{
  long int i;
  long int alen = get_vec_length(a);
  long int blen = get_vec_length(b);
  data_t *adata = get_vec_start(a);
  data_t *bdata = get_vec_start(b);

  data_t acc = 0;
  if (alen == blen) {
    for (i = 0; i < alen; i+=2) {
      acc = acc + ((adata[i] * bdata[i]) + (adata[i+1] * bdata[i+1]));
    }
    if (i < alen) {
      acc = acc + (adata[i] * bdata[i]);
    }
  }
  return acc;
}

