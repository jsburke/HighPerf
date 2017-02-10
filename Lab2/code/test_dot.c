// -*- C++ -*-
//
// gcc test_dot.c -lrt -o tdt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define SIZE 10000000
#define ITERS 20
#define DELTA 1000
#define BASE 0

double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/* System prototypes */
int clock_gettime(clockid_t clk_id, struct timespec *tp);

/* Prototypes */
long int get_vec_length(vec_ptr v);
vec_ptr new_vec(long int len);
int set_vec_length(vec_ptr v, long int index);
int init_vector(vec_ptr v, long int len);
double dotprod4(vec_ptr a, vec_ptr b);
data_t *get_vec_start(vec_ptr v);
struct timespec ts_diff(struct timespec start, struct timespec end);
double ts_sec(struct timespec ts);


typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} tsc_counter;

#define RDTSC(cpu_c) __asm__ __volatile__ ("rdtsc" : \
                     "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))

/*****************************************************************************/
int main(int argc, char *argv[])
{
  struct timespec cal_start, cal_end;
  tsc_counter tsc_start, tsc_end;
  struct timespec time1, time2;
  struct timespec time_stamp[ITERS+1];
  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cal_start);
  RDTSC(tsc_start);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  vec_ptr v1 = new_vec(MAXSIZE);
  init_vector(v0, MAXSIZE);
  init_vector(v1, MAXSIZE);

  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0, BASE+(i+1)*DELTA);
    set_vec_length(v1, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dotprod4(v0, v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[i] = ts_diff(time1,time2);
  }

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cal_end);
  RDTSC(tsc_end);

  CPS = ((double)(tsc_end.int64-tsc_start.int64))
                                        / ts_sec(ts_diff(cal_start, cal_end));
  printf("CPS == %g\n", CPS);

  /* output times */
  for (i = 0; i < ITERS; i++) {
    printf("%ld,  ", ((long int) (BASE+(i+1)*DELTA)));
    printf("%ld\n", ((long int) (CPS * ts_sec(time_stamp[i]))));
  }

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

double ts_sec(struct timespec ts)
{
  return ((double)(ts.tv_sec)) + ((double)(ts.tv_nsec))/1.0e9;
}

/* Dot product, Accumulating result in a local variable
 * (This is analogous to the 'Combine4' version of the vector reduction
 * functions in test_combine1-7.c) */
double dotprod4(vec_ptr a, vec_ptr b)
{
  long int i;
  long int alen = get_vec_length(a);
  long int blen = get_vec_length(b);
  data_t *adata = get_vec_start(a);
  data_t *bdata = get_vec_start(b);

  data_t acc = 0;
  if (alen == blen) {
    for (i = 0; i < alen; i++) {
      acc = acc + adata[i] + bdata[i];
    }
  }
  return acc;
}
