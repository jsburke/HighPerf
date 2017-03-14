/*************************************************************************/
// gcc -pthread -o test_pt test_pt.c -lm -lrt

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer
double  CPS  = 2.9e9;

// #define BASE  0
// #define ITERS 20
// #define DELTA 10

#define OPTIONS 3        // Current setting, vary as you wish!
#define IDENT 0

#define INIT_LOW -10.0
#define INIT_HIGH 10.0

#define FILE_PREFIX ((const unsigned char*) "pthread_hard_")

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

/////////////////  Time related  //////////////////////////////

//rdtsc related
typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} mcps_tctr;

#define MCPS_RDTSC(cpu_c) __asm__ __volatile__ ("rdtsc" : \
                     "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))

int clock_gettime(clockid_t clk_id, struct timespec *tp);
struct timespec diff(struct timespec start, struct timespec end);
double ts_sec(struct timespec ts);
struct timespec ts_diff(struct timespec start, struct timespec end);
double measure_cps(void);

////////////////////////////////////////////////////////////////
matrix_ptr new_matrix(long int len);
int set_matrix_length(matrix_ptr m, long int index);
long int get_matrix_length(matrix_ptr m);
int init_matrix(matrix_ptr m, long int len);
int zero_matrix(matrix_ptr m, long int len);
void pt_cb_bl(matrix_ptr a, matrix_ptr b, matrix_ptr c);
void pt_cb(matrix_ptr a, matrix_ptr b, matrix_ptr c);
void pt_mb(matrix_ptr a, matrix_ptr b, matrix_ptr c, matrix_ptr d);
void pt_ob(matrix_ptr a, matrix_ptr b, matrix_ptr c);

/*************************************************************************/
int main(int argc, char *argv[])
{
  int BASE, DELTA, ITERS;

  if(argc != 4)
  {
    printf("must have three arguments\n");
    return 0;
  }

  BASE  = strtol(argv[1], NULL, 10);
  DELTA = strtol(argv[2], NULL, 10);
  ITERS = strtol(argv[3], NULL, 10);  

  if (DELTA <= 0)
  {
    printf("DELTA must be more than zero\n");
    return 0;
  }

  if (BASE <= 0)
  {
    printf("BASE must be more than zero\n");
    return 0;
  }

  if (ITERS <= 0)
  {
    printf("ITERS must be more than zero\n");
    return 0;
  }

  int OPTION;
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS)*DELTA;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n", filename);

  // declare and initialize the matrix structure
  matrix_ptr a0 = new_matrix(MAXSIZE);
  init_matrix_rand(a0, MAXSIZE);
  matrix_ptr b0 = new_matrix(MAXSIZE);
  init_matrix_rand(b0, MAXSIZE);
  matrix_ptr c0 = new_matrix(MAXSIZE);
  zero_matrix(c0, MAXSIZE);
  matrix_ptr d0 = new_matrix(MAXSIZE);
  init_matrix_rand_ptr(d0, MAXSIZE);

  //////////////////////////////////////////////////////
  //
  // Begin tests
  //
  //////////////////////////////////////////////////////


  int elements;
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    elements = BASE+(i+1)*DELTA;
    init_matrix_rand(a0,elements);
    set_matrix_length(a0,elements);
    set_matrix_length(b0,elements);
    set_matrix_length(c0,elements);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb_bl(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
    //printf("\niter = %d", i);
  }

  NUM_THREADS = 2;
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    elements = BASE+(i+1)*DELTA;
    init_matrix_rand(a0,elements);
    set_matrix_length(a0,elements);
    set_matrix_length(b0,elements);
    set_matrix_length(c0,elements);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
    //printf("\niter = %d", i);
  }

  NUM_THREADS = 4;
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    elements = BASE+(i+1)*DELTA;
    init_matrix_rand(a0,elements);
    set_matrix_length(a0,elements);
    set_matrix_length(b0,elements);
    set_matrix_length(c0,elements);
    clock_gettime(CLOCK_REALTIME, &time1);
    pt_cb(a0,b0,c0);
    clock_gettime(CLOCK_REALTIME, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
    //printf("\niter = %d", i);
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
    time_stamp[OPTION][i] = ts_diff(time1,time2);
    printf("\niter = %d", i);
  }
  */

  /////////////////////////////////////////////////
  //
  // Write Results
  //
  /////////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp,"length, serial, threads = 2, threads = 4");

  for (i = 0; i < ITERS; i++) {
    fprintf(fp, "\n%ld, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp, ", ");
      fprintf(fp, "%ld", (long int)((double)(CPG)*(double)
      	 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }
  fprintf(fp, "\n");
  fclose(fp);

  //printf("\n Good Bye World!\n");

}/* end main */

/////////////////////////////  Timing related  ///////////////////////////////

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

/////////////////////////////  End Timing Related /////////////////////////////  

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
	  printf("\n COULDN'T ALLOCATE STORAGE \n");
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

double fRand(double fMin, double fMax)
{
  double f = (double)random() / (double)(RAND_MAX);
  return fMin + f * (fMax - fMin);
}

/*************************************************/

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





