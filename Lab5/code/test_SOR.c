/* -*- C++ -*- **************************************************************/
// gcc -O1 -o test_SOR test_SOR.c -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

double CPS = 2.9e9;       // Cycles/sec - adjusted by measure_cps()

//#define BASE  2
#define ITERS 1
//#define DELTA 3000

#define OPTIONS 3
//#define BLOCK_SIZE 8     // TO BE DETERMINED
int BLOCK_SIZE;

#define MINVAL   0.0
#define MAXVAL  10.0

#define TOL 0.00001
//#define OMEGA 1.60       // TO BE DETERMINED
double OMEGA;

#define FILE_PREFIX ((const unsigned char*) "p2_SOR_")

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

void SOR(vec_ptr v, int *iterations);
void SOR_ji(vec_ptr v, int *iterations);
void SOR_blocked(vec_ptr v, int *iterations);

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
  int BASE, DELTA;

  if(argc != 5)
  {
    printf("must have four arguments\n");
    return 0;
  }

  BASE  = strtol(argv[1], NULL, 10);
  DELTA = strtol(argv[2], NULL, 10);
  OMEGA  = strtod(argv[3], NULL);
  BLOCK_SIZE = strtol(argv[4], NULL, 10);  


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

  if(BLOCK_SIZE == 0)
  {
    printf("BLOCK_SIZE must be at least one\n");
    return 0;
  }    

  int OPTION;
  
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int convergence[OPTIONS][ITERS+1];
  int *iterations;

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename,"%sO%lf_B%d_D%d_BS%d.csv", FILE_PREFIX, OMEGA, BASE, DELTA, BLOCK_SIZE);
  printf("Current File: %s\n", filename);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  iterations = (int *) malloc(sizeof(int));

  measure_cps();

  //////////////////////////////////////////
  //
  //  Begin Tests
  //
  //////////////////////////////////////////

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    //printf("\niter = %ld length = %ld OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
    set_vec_length(v0, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    //print_vector(v0);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    SOR(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
    //print_vector(v0);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    //printf("\niter = %ld length = %ld", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_ji(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    //printf("\niter = %ld length = %ld", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

  ///////////////////////////////////////////
  //
  //   Write Results
  //
  ///////////////////////////////////////////

  fp = fopen(filename,"w");
  //  header line??

  for (i = 0; i < ITERS; i++) {
    long int elements;
    elements = BASE+(i+1)*DELTA;
    fprintf(fp, "\n%ld, ", elements);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      double seconds = ((double) time_stamp[j][i].tv_sec)
                     + ((double) time_stamp[j][i].tv_nsec)/1.0e9;
      fprintf(fp, "%lf sec", CPS * seconds / ((double)elements));
      //fprintf(fp, ", %d", convergence[j][i]);
    }
  }

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

/* initialize vector with another */
int init_vector_rand(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++)
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
  for (i = 0; i < len; i++) {
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
void SOR(vec_ptr v, int *iterations)
{
  long int i, j;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;   // start w/ something big
  int iters = 0;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (i = 1; i < length-1; i++) 
      for (j = 1; j < length-1; j++) {
	change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					  data[(i+1)*length+j] +
					  data[i*length+j+1] +
					  data[i*length+j-1]);
	data[i*length+j] -= change * OMEGA;
	if (change < 0){
	  change = -change;
	}
	mean_change += change;
      }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
   *iterations = iters;
  //printf("\n iters = %d", iters);
}

/* SOR with reversed indices */
void SOR_ji(vec_ptr v, int *iterations)
{
  long int i, j;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;   // start w/ something big
  int iters = 0;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (j = 1; j < length-1; j++) 
      for (i = 1; i < length-1; i++) {
	change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					  data[(i+1)*length+j] +
					  data[i*length+j+1] +
					  data[i*length+j-1]);
	data[i*length+j] -= change * OMEGA;
	if (change < 0){
	  change = -change;
	}
	mean_change += change;
      }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  //printf("\n iters = %d", iters);
}

/* SOR w/ blocking */
void SOR_blocked(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE)
	for (i = ii; i < ii+BLOCK_SIZE; i++)
	  for (j = jj; j < jj+BLOCK_SIZE; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  //printf("\n iters = %d", iters);
}
