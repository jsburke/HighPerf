/*****************************************************************************/
// gcc -O1 -o test_combine2d test_combine2d.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define BASE  0
//#define ITERS 30
//#define DELTA 200

#define OPTIONS 2
#define IDENT 1
#define OP *

#define FILE_PREFIX ((const unsigned char*) "doubleMul_")

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/*****************************************************************************/
int main(int argc, char *argv[])
{
  int DELTA, ITERS;

    if(argc != 3)
  {
    printf("num args wrong\n");
    return 0;
  }
  
  DELTA = strtol(argv[1],NULL,10);
  ITERS = strtol(argv[2],NULL,10);

  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  vec_ptr new_vec(long int len);
  int set_vec_length(vec_ptr v, long int index);
  long int get_vec_length(vec_ptr v);
  int init_vector(vec_ptr v, long int len);
  data_t *data_holder;
  void combine2D(vec_ptr v, data_t *dest);
  void combine2D_rev(vec_ptr v, data_t *dest);

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;
  
  char filename [255] = {0};
  FILE *fp;

  sprintf(filename, "%sI%d_D%d.csv", FILE_PREFIX, ITERS, DELTA);
  printf("Current file: %s\n", filename);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  data_holder = (data_t *) malloc(sizeof(data_t));
  init_vector(v0, MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine2D(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine2D_rev(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  fp = fopen(filename,"w");
  for (i = 0; i < ITERS; i++) {
    fprintf(fp,"\n%ld, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp,", ");
      fprintf(fp,"%ld", (long int)((double)(CPG)*(double)
     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }
  fclose(fp);

  //printf("\n");
  
}/* end main */
/*********************************/

/* Create 2D vector of specified length per dimension */
vec_ptr new_vec(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
    free((void *) result);
    printf("\n COULDN'T ALLOCATE STORAGE (%ld BYTES)\n", result->len);
    return NULL;  /* Couldn't allocate storage */
  }
  result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
  v->len = index;
  return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
  return v->len;
}

/* initialize 2D vector */
int init_vector(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++) v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

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

/************************************/

/* Combine2D:  Accumulate result in local variable */
void combine2D(vec_ptr v, data_t *dest)
{
  long int i, j;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;

  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      acc = acc OP data[i*length+j];
  *dest = acc;
}

/* Combine2D_rev:  Accumulate result in local variable */
void combine2D_rev(vec_ptr v, data_t *dest)
{
  long int i, j;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  data_t acc = IDENT;

  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      acc = acc OP data[j*length+i];
  *dest = acc;
}
