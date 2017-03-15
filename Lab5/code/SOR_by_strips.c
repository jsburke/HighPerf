/* -*- C++ -*- **************************************************************/
// gcc -O2 SOR_by_strips.c -lrt -lm -o sbs

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

double CPS = 2.9e9;       // Cycles/sec - adjusted by measure_cps()

long int ASIZE = 16;

#define OPTIONS 4
int BLOCK_SIZE = 8;
int NTHREADS = 4;

#define MINVAL   0.0
#define MAXVAL  10.0

#define TOL 0.00001

double OMEGA = 1.5;

#define FILE_PREFIX ((const unsigned char*) "p2_SOR")

typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

vec_ptr new_vec(long int len);
int set_vec_length(vec_ptr v, long int index);
long int get_vec_length(vec_ptr v);
int init_vector(vec_ptr v, long int len);
int init_vector_rand(vec_ptr v, long int len);
int print_vector(vec_ptr v);

double fRand(double fMin, double fMax);

//////////////////  SOR Functions  ///////////////////////////

void SOR(vec_ptr v, long int itermax);
void SOR_ji(vec_ptr v, long int itermax);
void SOR_blocked(vec_ptr v, long int itermax);
void threaded_SOR(vec_ptr v, long int itermax);

///////////////// vector stuff  //////////////////////////////

/* Number of bytes in a vector (SSE sense) */
#define VBYTES 16

/* Number of elements in a vector (SSE sense) */
#define VSIZE VBYTES/sizeof(data_t)

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
  vec_t v;
  data_t d[VSIZE];
} pack_t;

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

/*****************************************************************************/
int main(int argc, char *argv[])
{
  int MAX_ITERS = 100;

  if (argc != 5) {
    printf("must have 4 arguments\n");
    return 0;
  }
  ASIZE = strtol(argv[1], NULL, 10);
  NTHREADS = strtol(argv[2], NULL, 10);
  BLOCK_SIZE = strtol(argv[3], NULL, 10);
  MAX_ITERS = strtol(argv[4], NULL, 10);

  if (NTHREADS < 1) {
    printf("NTHREADS must be at least one\n");
    return 0;
  }

  if (ASIZE < NTHREADS) {
    printf("ASIZE must be at least as large as NTHREADS\n");
    return 0;
  }

  if (BLOCK_SIZE <= 0) {
    printf("BLOCK_SIZE must be at least one\n");
    return 0;
  }

  if (MAX_ITERS < 1) {
    printf("MAX_ITERS must be at least one\n");
    return 0;
  }

  int OPTION;
  
  struct timespec t_beg, t_end;
  double time_stamp[OPTIONS];

  long int i, j, k;
  long int time_sec, time_ns;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename,"%s_S%ld_T%d_BS%d_I%d.csv",
                          FILE_PREFIX, ASIZE, NTHREADS, BLOCK_SIZE, MAX_ITERS);
  printf("Current File: %s\n", filename);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(ASIZE);
  set_vec_length(v0, ASIZE);

  measure_cps();

  //////////////////////////////////////////
  //
  //  Begin Tests
  //
  //////////////////////////////////////////

  OPTION = 0;
  //printf("\niter = %ld length = %ld OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
  init_vector_rand(v0, ASIZE);
  //print_vector(v0);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_beg);
    SOR(v0, MAX_ITERS);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
  time_stamp[OPTION] = ts_sec(diff(t_beg, t_end));
  //print_vector(v0);

  OPTION++;
  //printf("\niter = %ld length = %ld", i, BASE+(i+1)*DELTA);
  init_vector_rand(v0, ASIZE);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_beg);
    SOR_ji(v0, MAX_ITERS);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
  time_stamp[OPTION] = ts_sec(diff(t_beg, t_end));

  OPTION++;
  //printf("\niter = %ld length = %ld", i, BASE+(i+1)*DELTA);
  init_vector_rand(v0, ASIZE);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_beg);
    SOR_blocked(v0, MAX_ITERS);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
  time_stamp[OPTION] = ts_sec(diff(t_beg, t_end));

  OPTION++;
  //printf("\niter = %ld length = %ld", i, BASE+(i+1)*DELTA);
  init_vector_rand(v0, ASIZE);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_beg);
    threaded_SOR(v0, MAX_ITERS);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end);
  time_stamp[OPTION] = ts_sec(diff(t_beg, t_end));

  ///////////////////////////////////////////
  //
  //   Write Results
  //
  ///////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp, "Elems, ij, ji, blocked, strips\n");

  long int elements;
  elements = ASIZE * ASIZE * MAX_ITERS;
  fprintf(fp, "%ld", elements);
  printf("%ld", elements);
  for (j = 0; j < OPTIONS; j++) {
    double seconds = time_stamp[j];
    fprintf(fp, ", %lf", CPS * seconds / ((double)elements));
    printf(", %lf", CPS * seconds / ((double)elements));
  }
  fprintf(fp, "\n");
  printf("\n");

  fclose(fp);
  
}/* end main */
/*********************************/

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
    data_t *data = (data_t *) calloc(len*(len+2*NTHREADS), sizeof(data_t));
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
    for (i = 0; i < len*(len+2*NTHREADS); i++) v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

/* initialize vector with another */
int init_vector_rand(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*(len+2*NTHREADS); i++)
      v->data[i] = (data_t)(fRand((double)(MINVAL),(double)(MAXVAL)));
    return 1;
  }
  else return 0;
}

/* print vector */
int print_vector(vec_ptr v)
{
  long int i, j, len;

  len = v->len;
  printf("\n length = %ld", len);
  for (i = 0; i < len+2*NTHREADS; i++) {
    printf("\n");
    for (j = 0; j < len; j++)
      printf("%.4f ", (data_t)(v->data[i*len+j]));
  }
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/************************************/

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/************************************/

/* SOR */
void SOR(vec_ptr v, long int itermax)
{
  long int i, j;
  long int im1, ip1, jm1, jp1;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change;
  int iters;

  for(iters=0; iters<itermax; iters++) {
    for (i = 0; i < length; i++) {
      for (j = 0; j < length; j++) {
        im1 = (i+length-1)%length; ip1 = (i+1)%length;
        jm1 = (j+length-1)%length; jp1 = (j+1)%length;
        change = data[i*length+j]
             - .25 * (data[im1*length+j] + data[ip1*length+j] +
                      data[i*length+jp1] + data[i*length+jm1]);
        data[i*length+j] -= change * OMEGA;
      }
    }
  }
  //printf("\n iters = %d", iters);
}

/* SOR with reversed indices */
void SOR_ji(vec_ptr v, long int itermax)
{
  long int i, j;
  long int im1, ip1, jm1, jp1;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change;
  int iters;

  for(iters=0; iters<itermax; iters++) {
    for (j = 0; j < length; j++) {
      for (i = 0; i < length; i++) {
        im1 = (i+length-1)%length; ip1 = (i+1)%length;
        jm1 = (j+length-1)%length; jp1 = (j+1)%length;
        change = data[i*length+j]
                 - .25 * (data[im1*length+j] + data[ip1*length+j] +
                          data[i*length+jp1] + data[i*length+jm1]);
        data[i*length+j] -= change * OMEGA;
      }
    }
  }
  //printf("\n iters = %d", iters);
}

/* SOR w/ blocking */
void SOR_blocked(vec_ptr v, long int itermax)
{
  long int i, j, ii, jj;
  long int im1, ip1, jm1, jp1;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change;
  int iters = 0;
  int k;

  for(iters=0; iters<itermax; iters++) {
    for (ii = 0; ii < length; ii+=BLOCK_SIZE) {
      for (jj = 0; jj < length; jj+=BLOCK_SIZE) {
        for (i = ii; i < ii+BLOCK_SIZE; i++) {
          for (j = jj; j < jj+BLOCK_SIZE; j++) {
            im1 = (i+length-1)%length; ip1 = (i+1)%length;
            jm1 = (j+length-1)%length; jp1 = (j+1)%length;
            change = data[i*length+j]
                       - .25 * (data[im1*length+j] + data[ip1*length+j] +
                                data[i*length+jp1] + data[i*length+jm1]);
            data[i*length+j] -= change * OMEGA;
          }
        }
      }
    }
  }
  //printf("\n iters = %d", iters);
}


typedef struct {
  int      t_id;
  long int len;
  data_t   *data;
  long int imax;
} sor_data;

pthread_barrier_t sor_barrier;

void* thr_SOR_wrkr(void* t_arg)  // split the SOR on red-black
{
  //  thread struct
  sor_data * thread_arg = (sor_data *) t_arg;
  int t_id = thread_arg -> t_id;
  long int len = thread_arg -> len;
  data_t *data = thread_arg -> data;
  long int itermax = thread_arg -> imax;

  long lfull = len+2*NTHREADS;

  long int i, j;
  long int im1, ip1, jm1, jp1;
  double change;
  int iters;
  int rv;

  long int i1, i2, minrow, maxrow;

  // NTHREADS=4, len=100
  //  0  1      25 26 27 28        52 53 54 55       79 80 81 82       106 107
  minrow = ((lfull * t_id) / NTHREADS) + 1;
  maxrow = ((lfull * (t_id+1)) / NTHREADS) - 2;
  // printf("t%d %ld..%ld of %ld\n", t_id, minrow, maxrow, lfull);
  for(iters=0; iters<itermax; iters++) {

    for (i = minrow; i < maxrow; i++) {
      for (j = 0; j < len; j++) {
        im1 = i-1; ip1 = i+1;
        jm1 = (j+len-1)%len; jp1 = (j+1)%len;
        change = data[i*len+j]
             - .25 * (data[im1*len+j] + data[ip1*len+j] +
                      data[i*len+jp1] + data[i*len+jm1]);
        data[i*len+j] -= change * OMEGA;
      }
    }

    rv = pthread_barrier_wait(&sor_barrier);

    i1 = minrow-1;
    im1 = (minrow+lfull-2) % lfull;
    i2 = maxrow;
    ip1 = (maxrow+1) % lfull;
    for (j = 0; j < len; j++) {
      data[i1*len+j] = data[im1*len+j];
      data[i2*len+j] = data[ip1*len+j];
    }  

    rv = pthread_barrier_wait(&sor_barrier);
  }
}

void threaded_SOR(vec_ptr v, long int itermax)
{
  int t;
  pthread_t threads[NTHREADS];
  sor_data thr_args[NTHREADS];
  int     rv;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);

  pthread_barrier_init(&sor_barrier, NULL, NTHREADS);
  for (t=0; t<NTHREADS; t++) {
    thr_args[t].t_id = t;
    thr_args[t].len = length;
    thr_args[t].data = data;
    thr_args[t].imax = itermax;

    rv = pthread_create(&threads[t], NULL, thr_SOR_wrkr, (void*) &thr_args[t]);
  }
  for (t=0; t<NTHREADS;t++) {
    rv = pthread_join(threads[t], NULL);
  }
}
