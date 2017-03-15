//////////////////////////////////////////////
//
//	Code to compare different methods of SOR
//
//  clear; gcc -O1 -pthread -o SOR SOR_comp.c -lm -lrt
//
//////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
double CPS = 2.9e9;		// Cycles/sec - adjusted by measure_cps()

#define OPTIONS 2
double  OMEGA;

typedef double data_t;

#define FILE_PREFIX ((const unsigned char*) "compSOR_")

//////////  Threads  ////////////////////////

#define RB_THREADS 2  // one red one black
int SR_BREAK = 0;     // exit from rb test

typedef struct{
	int      t_id;
	long int len;
	data_t   *data;
	double   acc_change;
}rb_data, *prb_data;

//  defines for bounding values in array
#define MINVAL 0.00
#define MAXVAL 10.0

#define TOL    0.00001

/////////////////  Vector Related /////////////////////////////

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

////////////////  SOR Functions ///////////////////////////////////
double omega_calc(int elements);

void SOR_basic(vec_ptr vec, int *iterations);
void SOR_red_black(vec_ptr vec, int *iterations);
//void SOR_strip(vec_ptr vec);

int main(int argc, char *argv[])
{
	// handle command line
	int BASE, DELTA, ITERS;

	if(argc != 4)
  {
    printf("must have four arguments\n");
    return 0;
  }

  BASE   = strtol(argv[1], NULL, 10);
  DELTA  = strtol(argv[2], NULL, 10);
  ITERS  = strtol(argv[3], NULL, 10);
  
  if(BASE <= 0)
  {
    printf("BASE must be greater than zero\n");
    return 0;
  }

  if(DELTA <= 0)
  {
    printf("DELTA must be at least one\n");
    return 0;
  }

    if(ITERS <= 0)
  {
    printf("ITERS must be at least one\n");
    return 0;
  }
 

  //  set up for measurement
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int convergence[OPTIONS][ITERS+1];
  int *iterations;

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  //set up file output
  char filename[255] = {0};
  FILE *fp;

  sprintf(filename,"%sB%d_D%d_I%d_O%lf.csv", FILE_PREFIX, BASE, DELTA, ITERS, OMEGA);
  printf("Current File: %s\n", filename);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  iterations = (int *) malloc(sizeof(int));

  //////////////////////////////////////
  //
  // Begin Tests
  //
  //////////////////////////////////////

  measure_cps();
  int OPTION = 0;
  int elements = 0;
  for(i = 0; i < ITERS; i++)
  {
  	elements = BASE+(i+1)*DELTA;
  	set_vec_length(v0, elements);
    init_vector_rand(v0, elements);
    OMEGA = omega_calc(elements * elements);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    SOR_basic(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1, time2);
  } 

  printf("\nBASIC DONE\n"); 

  OPTION++;
  elements = 0;
  for(i = 0; i < ITERS; i++)
  {
  	elements = BASE+(i+1)*DELTA;
  	set_vec_length(v0, elements);
    init_vector_rand(v0, elements);
    OMEGA = omega_calc(elements * elements);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    SOR_red_black(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1, time2);
  }  

  printf("\nRED BLACK DONE\n");

  /////////////////////////////////////
  //
  // Write results to file
  //
  /////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp, "Length, Basic, Red-Black");

  for(i = 0; i < ITERS; i++)
  {
  	elements = BASE+(i+1)*DELTA;
  	fprintf(fp, "\n%d,", elements);
  	for(j = 0; j < OPTIONS; j++)
  	{
  		if(j != 0) fprintf(fp, ", ");
  		fprintf(fp, "%ld", (long int)((CPS)*(double)(GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
  	}
  }
  fprintf(fp, "\n");
  fclose(fp);
  return 0;
}

////////////////////  Time related  //////////////////////////////

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

/////////////////////////////  vector related /////////////////////////////

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

/////////////////////////////////////////////////

double omega_calc(int elements)
{
	double calc = (1.8169 + 0.09 * log(19.6578 * elements) ) / 1.5521;
	return (calc >= 2.00) ? 1.99 : calc;
}

void SOR_basic(vec_ptr v, int *iterations)
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

/////////////////////  Parallel Work  ///////////////////////////

void* SOR_rb_calc(void* t_arg)  // split the SOR on red-black
{
	//  thread struct
	prb_data thread_arg;
	int      t_id;
	long int len;
	data_t   *data;
	double	 acc_change;

	thread_arg = (prb_data) t_arg;
	t_id	   = thread_arg -> t_id;
	len 	   = thread_arg -> len;
	data 	   = thread_arg -> data;
	acc_change = thread_arg -> acc_change;

	// do one iteration of SOR
	// t_id == 1 gets odds, == 0 even

	int i, j, row_offset;
	int odd_change = len % 2;
	int j_start    = t_id + 1;
	double change;

	//printf("\nTHREAD %d Check in", t_id);

	for(i = 1; i < len - 1; i++)
	{
		//  For non-even length array, flop the start
		if(odd_change == 0) j_start = (j_start == 1) ? 2 : 1;
		
		row_offset = i * len;

		for(j = j_start; j < len - 1; j++)
		{
			change = data[row_offset+j] - .25 * (data[row_offset-len+j] +
					  data[row_offset+len+j] +
					  data[row_offset+j+1] +
					  data[row_offset+j-1]);

			data[row_offset+j] -= change * OMEGA;

			change = (change < 0) ? -change : change;
			acc_change += change;
		}
	}

	//divergence check
	if (abs(data[(len-2)*(len-2)]) > 10.0*(MAXVAL - MINVAL)) {
      //printf("\n PROBABLY DIVERGENCE");
      SR_BREAK = 1;
    }

    // send accumulated change back
    thread_arg -> acc_change = acc_change;
    //printf("\nID: %d end", t_id);
    pthread_exit(NULL);
}

void SOR_red_black(vec_ptr v, int *iterations)
{
	pthread_t threads[RB_THREADS];
	rb_data t_rb_args[RB_THREADS];
	double  mean_change, accum;
	int     pt_ret, i, j, k;
	int 	iters = 0;

	long int len   = get_vec_length(v);
	data_t   *data = get_vec_start(v);

	//printf("\nRB start");

	do{  // mean_change > TOL

		iters++;
		// set up and launch threads
		for(i = 0; i < RB_THREADS; i++){
			t_rb_args[i].t_id       = i;
			t_rb_args[i].len        = len;
			t_rb_args[i].data       = data;
			t_rb_args[i].acc_change = 0;
			pt_ret = pthread_create(&threads[i], NULL, SOR_rb_calc, (void*) &t_rb_args[i]);
			if(pt_ret){
				printf("\n\n\tERROR: return code from pthread_create(): %d\n\n",pt_ret);
				exit(-1);
			}
			//printf("\nLaunch thread %d", i);
		}

		//  join on threads to wait for them
		for(j = 0; j < RB_THREADS; j++){
			//printf("\nJoin on thread %d", j);
			if(pthread_join(threads[j], NULL)){
				printf("\n\n\tERROR on join\n\n");
				exit(19);
			}
		}

		// calculate mean_change for exit condition
		accum = 0;
		for(k = 0; k < RB_THREADS; k++) accum += t_rb_args[k].acc_change;
		mean_change = accum / ((double) len * len);

	  }while((mean_change > (double) TOL) || (SR_BREAK != 0));

	  *iterations = iters;
}

