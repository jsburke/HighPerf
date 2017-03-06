/*************************************************************************/
// gcc -pthread -o test_pt test_pt.c -lm -lrt

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.0           // Cycles per GHz -- Adjust to your computer

#define BASE  0
#define ITERS 20
#define DELTA 10

#define OPTIONS 3        // Current setting, vary as you wish!
#define IDENT 0

#define INIT_LOW -10.0
#define INIT_HIGH 10.0

typedef double data_t;

/* Create abstract data type for matrix */
typedef struct {
  long int len;
  data_t *data;
} matrix_rec, *matrix_ptr;

int NUM_THREADS = 4;

/* used to pass parameters to worker threads */
struct thread_data{
  int thread_id;
  matrix_ptr a;
  matrix_ptr b;
  matrix_ptr c;
  matrix_ptr d;
};

/*************************************************************************/
int main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  matrix_ptr new_matrix(long int len);
  int set_matrix_length(matrix_ptr m, long int index);
  long int get_matrix_length(matrix_ptr m);
  int init_matrix(matrix_ptr m, long int len);
  int zero_matrix(matrix_ptr m, long int len);
  void pt_cb_bl(matrix_ptr a, matrix_ptr b, matrix_ptr c);
  void pt_cb(matrix_ptr a, matrix_ptr b, matrix_ptr c);
  void pt_mb(matrix_ptr a, matrix_ptr b, matrix_ptr c, matrix_ptr d);
  void pt_ob(matrix_ptr a, matrix_ptr b, matrix_ptr c);

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS)*DELTA;

  printf("\n Hello World -- Test OMP \n");

  // declare and initialize the matrix structure
  matrix_ptr a0 = new_matrix(MAXSIZE);
  init_matrix_rand(a0, MAXSIZE);
  matrix_ptr b0 = new_matrix(MAXSIZE);
  init_matrix_rand(b0, MAXSIZE);
  matrix_ptr c0 = new_matrix(MAXSIZE);
  zero_matrix(c0, MAXSIZE);
  matrix_ptr d0 = new_matrix(MAXSIZE);
  init_matrix_rand_ptr(d0, MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    init_matrix_rand(a0,BASE+(i+1)*DELTA);
    set_matrix_length(a0,BASE+(i+1)*DELTA);
    set_matrix_length(b0,BASE+(i+1)*DELTA);
    set_matrix_length(c0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb_bl(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    printf("\niter = %d", i);
  }

  NUM_THREADS = 2;
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    init_matrix_rand(a0,BASE+(i+1)*DELTA);
    set_matrix_length(a0,BASE+(i+1)*DELTA);
    set_matrix_length(b0,BASE+(i+1)*DELTA);
    set_matrix_length(c0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    printf("\niter = %d", i);
  }

  NUM_THREADS = 4;
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    init_matrix_rand(a0,BASE+(i+1)*DELTA);
    set_matrix_length(a0,BASE+(i+1)*DELTA);
    set_matrix_length(b0,BASE+(i+1)*DELTA);
    set_matrix_length(c0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    printf("\niter = %d", i);
  }
  /*
  NUM_THREADS = 8;
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    init_matrix_rand(a0,BASE+(i+1)*DELTA);
    set_matrix_length(a0,BASE+(i+1)*DELTA);
    set_matrix_length(b0,BASE+(i+1)*DELTA);
    set_matrix_length(c0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    printf("\niter = %d", i);
  }
  */
  printf("\nlength, other data");
  for (i = 0; i < ITERS; i++) {
    printf("\n%d, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPG)*(double)
      	 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }

  printf("\n Good Bye World!\n");

}/* end main */

/**********************************************/
/* Create matrix of specified length */
matrix_ptr new_matrix(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  matrix_ptr result = (matrix_ptr) malloc(sizeof(matrix_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
	  free((void *) result);
	  printf("\n COULDN'T ALLOCATE STORAGE \n", result->len);
	  return NULL;  /* Couldn't allocate storage */
	}
	result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Set length of matrix */
int set_matrix_length(matrix_ptr m, long int index)
{
  m->len = index;
  return 1;
}

/* Return length of matrix */
long int get_matrix_length(matrix_ptr m)
{
  return m->len;
}

/* initialize matrix */
int init_matrix(matrix_ptr m, long int len)
{
  long int i;

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

/* initialize matrix to rand */
int init_matrix_rand(matrix_ptr m, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(fRand((double)(INIT_LOW),(double)(INIT_HIGH)));
    return 1;
  }
  else return 0;
}

int init_matrix_rand_ptr(matrix_ptr m, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(fRand((double)(0.0),(double)(i)));
    return 1;
  }
  else return 0;
}

/* initialize matrix */
int zero_matrix(matrix_ptr m, long int len)
{
  long int i,j;

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(IDENT);
    return 1;
  }
  else return 0;
}

data_t *get_matrix_start(matrix_ptr m)
{
  return m->data;
}

/* print matrix */
int print_matrix(matrix_ptr v)
{
  long int i, j, len;

  len = v->len;
  for (i = 0; i < len; i++) {
    printf("\n");
    for (j = 0; j < len; j++)
      printf("%.4f ", (data_t)(v->data[i*len+j]));
  }
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
  double f = (double)random() / (double)(RAND_MAX);
  return fMin + f * (fMax - fMin);
}

/*************************************************/
/* CPU bound baseline */
void pt_cb_bl(matrix_ptr a, matrix_ptr b, matrix_ptr c)
{
  long int i, j, k;
  long int length = get_matrix_length(a);
  data_t *a0 = get_matrix_start(a);
  data_t *b0 = get_matrix_start(b);
  data_t *c0 = get_matrix_start(c);

  for (i = 0; i < length*length; i++) 
    c0[i] = (data_t)(cosh(tan(sqrt(cos(exp((double)(a0[i])))))));
  //c0[i] = a0[i];
}

/********************/
/* CPU bound multithreaded code*/
/* first, the actual thread function */
void *cb_work(void *threadarg)
{
  long int i, j, k, low, high;
  struct thread_data *my_data;
  my_data = (struct thread_data *) threadarg;
  int taskid = my_data->thread_id;
  matrix_ptr a0 = my_data->a;
  matrix_ptr b0 = my_data->b;
  matrix_ptr c0 = my_data->c;
  long int length = get_matrix_length(a0);
  data_t *aM = get_matrix_start(a0);
  data_t *bM = get_matrix_start(b0);
  data_t *cM = get_matrix_start(c0);

  low = (taskid * length * length)/NUM_THREADS;
  high = ((taskid+1)* length * length)/NUM_THREADS;

  for (i = low; i < high; i++)
    cM[i] = (data_t)(cosh(tan(sqrt(cos(exp((double)(aM[i])))))));
  //cM[i] = aM[i];

  pthread_exit(NULL);
}

/* Now, the pthread calling function */
void pt_cb(matrix_ptr a, matrix_ptr b, matrix_ptr c)
{
  long int i, j, k;
  pthread_t threads[NUM_THREADS];
  struct thread_data thread_data_array[NUM_THREADS];
  int rc;
  long t;

  for (t = 0; t < NUM_THREADS; t++) {
    thread_data_array[t].thread_id = t;
    thread_data_array[t].a = a;
    thread_data_array[t].b = b;
    thread_data_array[t].c = c;
    thread_data_array[t].d = 0;
    rc = pthread_create(&threads[t], NULL, cb_work,
			(void*) &thread_data_array[t]);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (t = 0; t < NUM_THREADS; t++) {
    if (pthread_join(threads[t],NULL)){ 
      printf("ERROR; code on return from join is %d\n", rc);
      exit(-1);
    }
  }
}





