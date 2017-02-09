/*****************************************************************************/

// gcc -o test_branch test_branch.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9   

#define ITERS 50
#define DELTA 10
#define BASE 0

#define OPTIONS 4
#define IDENT 1.0
#define OP *

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/*****************************************************************************/
main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  vec_ptr new_vec(long int len);
  int get_vec_element(vec_ptr v, long int index, data_t *dest);
  long int get_vec_length(vec_ptr v);
  int set_vec_length(vec_ptr v, long int index);
  int init_vector1(vec_ptr v, long int len);
  int init_vector2(vec_ptr v, long int len);
  void branch1(vec_ptr v0, vec_ptr v1, vec_ptr v2);
  void branch2(vec_ptr v0, vec_ptr v1, vec_ptr v2);

  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  printf("\n Hello World -- psum examples\n");

  // declare vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  vec_ptr v1 = new_vec(MAXSIZE);
  vec_ptr v2 = new_vec(MAXSIZE);

  // init the vectors
  init_vector1(v0, MAXSIZE);
  init_vector1(v1, MAXSIZE);

  // execute and time all options 
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    set_vec_length(v2,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(v0, v1, v2);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  // init the vectors
  init_vector2(v0, MAXSIZE);
  init_vector2(v1, MAXSIZE);

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    set_vec_length(v2,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(v0, v1, v2);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  // init the vectors
  init_vector1(v0, MAXSIZE);
  init_vector1(v1, MAXSIZE);

  // execute and time all options 
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    set_vec_length(v2,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(v0, v1, v2);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  // init the vectors
  init_vector2(v0, MAXSIZE);
  init_vector2(v1, MAXSIZE);

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    set_vec_length(v2,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(v0, v1, v2);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  /* output times */
  for (i = 0; i < ITERS; i++) {
    printf("\n%d,  ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
       printf("%ld", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }

  printf("\n");
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

/* initialize vector with first pattern */
int init_vector1(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) v->data[i] = ;
    return 1;
  }
  else return 0;
}

/* initialize vector with another */
int init_vector2(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) v->data[i] = ;
    return 1;
  }
  else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/*************************************************/
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

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/*************************************************/
/* branch1:  test branch, based on example in B&O 5.11
 * For each element i in vectors v0 and v1, write the
 * larger into element i of v2. */
void branch1(vec_ptr v0, vec_ptr v1, vec_ptr v2)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t *data2 = get_vec_start(v2);
  data_t acc = (data_t)(0);

  for (i = 0; i < length; i++) {
    if (data0[i] > data1[i]) data2[i] = data0[i];
    else data2[i] = data1[1];
  }
}

/*************************************************/
/* branch2:  test branch, based on example in B&O 5.11
 * For each element i in vectors v0 and v1, write the
 * larger into element i of v2. */
void branch2(vec_ptr v0, vec_ptr v1, vec_ptr v2)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t *data2 = get_vec_start(v2);
  data_t acc = (data_t)(0);

  for (i = 0; i < length; i++) 
    data2[i] = data0[i] > data1[i] ? data0[i] : data1[i];  
}
