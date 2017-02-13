/* -*- C++ -*- ***************************************************************/

// gcc test_combine1-7.c -lrt -o tcmb

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define SIZE 10000000
// #define ITERS 20
// #define DELTA 10
// #define BASE 0

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer
double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime

#define OPTIONS 6
#define IDENT 1.0
#define OP *

#define FILE_PREFIX ((const unsigned char*) "parallel_")

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

//rdtsc related
typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} mcps_tctr;

#define MCPS_RDTSC(cpu_c) __asm__ __volatile__ ("rdtsc" : \
                     "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))

vec_ptr new_vec(long int len);
int get_vec_element(vec_ptr v, long int index, data_t *dest);
long int get_vec_length(vec_ptr v);
int set_vec_length(vec_ptr v, long int index);
int init_vector(vec_ptr v, long int len);
data_t* get_vec_start(vec_ptr v);

int clock_gettime(clockid_t clk_id, struct timespec *tp);
struct timespec diff(struct timespec start, struct timespec end);
double ts_sec(struct timespec ts);
struct timespec ts_diff(struct timespec start, struct timespec end);
double measure_cps(void);

void unroll_2_mult_acc(vec_ptr v, data_t *dest);
void unroll_2_reassoc(vec_ptr v, data_t *dest);
// two because I can't think of a way to combine both for unroll 2
void unroll_3(vec_ptr v, data_t *dest);
void unroll_6(vec_ptr v, data_t *dest);
void unroll_8(vec_ptr v, data_t *dest);
void unroll_10(vec_ptr v, data_t *dest);

/*****************************************************************************/
int main(int argc, char *argv[])
{
  
  int BASE, DELTA, ITERS;

  if(argc != 4)
  {
    printf("num args wrong\n");
    return 0;
  }

  BASE  = strtol(argv[1], NULL, 10);
  DELTA = strtol(argv[2], NULL, 10);
  ITERS = strtol(argv[3], NULL, 10);

  if(DELTA == 0)
  {
    printf("DELTA must be greater than zero\n");
    return 0;
  }

  if(ITERS == 0)
  {
    printf("ITERS must be at least one\n");
    return 0;
  }

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n", filename);

  int OPTION;

  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  data_t *data_holder;
  
  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  measure_cps();

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  data_holder = (data_t *) malloc(sizeof(data_t));
  init_vector(v0, MAXSIZE);

  ///////////////////////////////////////////////
  //
  //  Begin unroll
  //
  ///////////////////////////////////////////////
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_2_mult_acc(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_2_reassoc(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_3(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_6(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_8(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    unroll_10(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
  ////////////////////////////////////////////////////
  //  Write to file
  ////////////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp, "x-axis, unroll_2m, unroll_2r, unroll_03, unroll_06, unroll_08, unroll_10\n"); // kludge line one

  long int elements;

  for (i = 0; i < ITERS; i++) {
    elements = BASE+(i+1)*DELTA;
    fprintf(fp, "%ld,  ", elements);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp, ", ");
       fprintf(fp, "%lf", ((double)(CPG)*(double)
     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)/elements));
    }
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

/* Retrieve vector element and store at dest.
   Return 0 (out of bounds) or 1 (successful)
*/
int get_vec_element(vec_ptr v, long int index, data_t *dest)
{
  if (index < 0 || index >= v->len) return 0;
  *dest = v->data[index];
  return 1;
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
    for (i = 0; i < len; i++) v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/*************************************************/
double ts_sec(struct timespec ts)
{
  return ((double)(ts.tv_sec)) + ((double)(ts.tv_nsec))/1.0e9;
}

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
    ilim = 100*1000*1000;
    for (i=0; i<ilim; i++)
      z = z * z + chaosC;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cal_end);
    MCPS_RDTSC(tsc_end);
  }

  total_time = ts_sec(ts_diff(cal_start, cal_end));
  total_cycles = (double)(tsc_end.int64-tsc_start.int64);
  CPS = total_cycles / total_time;
  //printf("z == %f, CPS == %g\n", z, CPS);

  return CPS;
}
/* ---------------------------------------------------------------------------
| End of measure_cps code
--------------------------------------------------------------------------- */

struct timespec diff(struct timespec start, struct timespec end)
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

/*************************************************/



/* Example of --> Loop unrolling */
void unroll_2_mult_acc(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=2) {
    acc0 = acc0 OP data[i];
    acc1 = acc1 OP data[i+1];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 * acc1;
}

void unroll_2_reassoc(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=2) {
    acc = acc OP (data[i] OP data[i+1]);
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}

void unroll_3(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=3) {
    acc0 = acc0 OP (data[i] OP data[i+1]);
    acc1 = acc1 OP data[i+2];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 * acc1;
}

void unroll_6(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;
  data_t acc2 = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=6) {
    acc0 = acc0 OP (data[i] OP data[i+1]);
    acc1 = acc1 OP (data[i+2] OP data[i+3]);
    acc2 = acc2 OP (data[i+4] OP data[i+5]);
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 OP acc1 OP acc2;
}

void unroll_8(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;
  data_t acc2 = IDENT;
  data_t acc3 = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=6) {
    acc0 = acc0 OP (data[i] OP data[i+1]);
    acc1 = acc1 OP (data[i+2] OP data[i+3]);
    acc2 = acc2 OP (data[i+4] OP data[i+5]);
    acc3 = acc3 OP (data[i+6] OP data[i+7]);
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 OP acc1 OP acc2 OP acc3;
}

void unroll_10(vec_ptr v, data_t *dest)
{
  long int i;
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;
  data_t acc2 = IDENT;
  data_t acc3 = IDENT;
  data_t acc4 = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=6) {
    acc0 = acc0 OP (data[i] OP data[i+1]);
    acc1 = acc1 OP (data[i+2] OP data[i+3]);
    acc2 = acc2 OP (data[i+4] OP data[i+5]);
    acc3 = acc3 OP (data[i+6] OP data[i+7]);
    acc4 = acc4 OP (data[i+8] OP data[i+9]);
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 OP acc1 OP acc2 OP acc3 OP acc4;
}