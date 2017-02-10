/*****************************************************************************/

// gcc -o test_combine1-7 test_combine1-7.c -lrt

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

#define OPTIONS 7
#define IDENT 1.0
#define OP *

#define FILE_PREFIX ((const unsigned char*) "doubleMulC7_")

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

//struct timespec {
//  time_t tv_sec; /* seconds */
//  long tv_nsec;  /* nanoseconds */
//};

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
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  vec_ptr new_vec(long int len);
  int get_vec_element(vec_ptr v, long int index, data_t *dest);
  long int get_vec_length(vec_ptr v);
  int set_vec_length(vec_ptr v, long int index);
  int init_vector(vec_ptr v, long int len);
  data_t *data_holder;
  void combine1(vec_ptr v, data_t *dest);
  void combine2(vec_ptr v, data_t *dest);
  void combine3(vec_ptr v, data_t *dest);
  void combine4(vec_ptr v, data_t *dest);
  void combine5(vec_ptr v, data_t *dest);
  void combine6(vec_ptr v, data_t *dest);
  void combine7(vec_ptr v, data_t *dest);
  
  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  //printf("\n Hello World -- psum examples\n");

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  data_holder = (data_t *) malloc(sizeof(data_t));
  init_vector(v0, MAXSIZE);

  // execute and time all 7 options from B&O 
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine1(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine2(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine3(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine4(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine5(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine6(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine7(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
  
  /* output times */
  fp = fopen(filename,"w");
  for (i = 0; i < ITERS; i++) {
    fprintf(fp, "\n%ld,  ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp, ", ");
       fprintf(fp, "%ld", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }
  fclose(fp);

  printf("\n");
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
/* Combine1:  Implementation with maximum use of data abstraction */
void combine1(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);

  *dest = IDENT;
  for (i = 0; i < get_vec_length(v); i++) {
    data_t val;
    get_vec_element(v, i, &val);
    *dest = *dest OP val;
  }
}

/* Combine2:  Move call to vec_length out of loop 
 * Example of --> Code motion) */
void combine2(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  long int length = get_vec_length(v);

  *dest = IDENT;
  for (i = 0; i < length; i++) {
    data_t val;
    get_vec_element(v, i, &val);
    *dest = *dest OP val;
  }
}

/* Combine3:  Direct access to vector
 * Example of --> Reduce procedure calls */
void combine3(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);

  *dest = IDENT;
  for (i = 0; i < length; i++) {
    *dest = *dest OP data[i];
  }
}

/* Combine4:  Accumulate result in local variable
 * Example of --> Eliminate unneeded memory references */
void combine4(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;

  for (i = 0; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}

/* Combine5:  Unroll loop by 2
 * Example of --> Loop unrolling */
void combine5(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=2) {
    acc = (acc OP data[i]) OP data[i+1];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc = acc OP data[i];
  }
  *dest = acc;
}

/* Combine6:  Unroll loop by 2, 2 accumulators
 * Example of --> parallelism */
void combine6(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;

  /* Combine two elements at a time w/ 2 acculators */
  for (i = 0; i < limit; i+=2) {
    acc0 = acc0 OP data[i];
    acc1 = acc1 OP data[i+1];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 OP acc1;
}

/* Combine7:  Unroll loop by 2, change associativity
 * Example of --> parallelism */
void combine7(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
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
