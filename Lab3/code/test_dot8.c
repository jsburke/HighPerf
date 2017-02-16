/*****************************************************************************/
// gcc -o test_dot test_dot.c -lrt -lm
//
// dot4    -- baseline scalar
// dot5    -- scalar unrolled by 2
// dot6_2  -- scalar unrolled by 2 w/ 2 accumulators
// dot6_5  -- scalar unrolled by 5 w/ 5 accumulators
// dot8    -- vector
// dot8_2  -- vector w/ 2 accumulators
// dot8_4  -- vector w/ 4 accumulators          TO BE WRITTEN


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer
double CPS = 2.9e9;       // Cycles/sec     -- adjusts

// #define ITERS 30
// #define DELTA 16
// #define BASE 0

#define OPTIONS 8                          
#define IDENT 1.0

#define FILE_PREFIX ((const unsigned char*) "int_dot8_")

typedef int data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/* Number of bytes in a vector */
#define VBYTES 32

/* Number of elements in a vector */
#define VSIZE VBYTES/sizeof(data_t)

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
  vec_t v;
  data_t d[VSIZE];
} pack_t;

vec_ptr new_vec(long int len);
int get_vec_element(vec_ptr v, long int index, data_t *dest);
long int get_vec_length(vec_ptr v);
int set_vec_length(vec_ptr v, long int index);
int init_vector(vec_ptr v, long int len);
int init_vector_rand(vec_ptr v, long int len);

/////////////// Timing related  ///////////////////////////////////////////////

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

//////////////  dot product prototypes  ///////////////////////////////////////

  void dot4(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot5(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot6_2(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot6_5(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot8(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot8_2(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot8_4(vec_ptr v0, vec_ptr v1, data_t *dest);
  void dot8_8(vec_ptr v0, vec_ptr v1, data_t *dest);  

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

  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  data_t *data_holder;

  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n",filename);

  measure_cps();

  /////////////////////////////////////////////////
  //
  //  Begin Dot Products
  //
  /////////////////////////////////////////////////
  vec_ptr v0 = new_vec(MAXSIZE);
  init_vector_rand(v0, MAXSIZE);
  vec_ptr v1 = new_vec(MAXSIZE);
  init_vector_rand(v1, MAXSIZE);
  data_holder = (data_t *) malloc(sizeof(data_t));

  // execute and time all options 
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot4(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot5(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot6_2(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot6_5(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot8(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot8_2(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
   //printf("\n");

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot8_4(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    set_vec_length(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot8_8(v0, v1, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  ///////////////////////////////////////////////////////////////
  //
  //  Write to file
  //
  ////////////////////////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp,"size, dot 4, dot 5, dot 6 by 2, dot 6 by 5, dot 8, dot 8 by 2, dot 8 by 4, dot 8 by 8\n");  

  int elements;

  for (i = 0; i < ITERS; i++) {
  	elements = BASE+(i+1)*DELTA;
    fprintf(fp,"%d, ", elements);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp,", ");
      fprintf(fp, "%lf", ((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)/elements));
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
  return 0;
}/* end main */

//////////////////////////////  TIming related  ////////////////////////////////

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

//////////////////////////////  End Timing Related //////////////////////////////

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

/* initialize vector with another */
int init_vector_rand(vec_ptr v, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++)
      v->data[i] = (data_t)(fRand((double)(0.0),(double)(10.0)));
    return 1;
  }
  else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/*************************************************/

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/*************************************************/
/* dot4:  Accumulate result in local variable
 * Example of --> Eliminate unneeded memory references */
void dot4(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t acc = (data_t)(0);

  for (i = 0; i < length; i++) {
    acc += data0[i] * data1[i];
  }
  *dest = acc;
}

/* dot5:  Unroll loop by 2
 * Example of --> Loop unrolling */
void dot5(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  long int limit = length - 1;
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t acc = (data_t)(0);

  /* Combine two elements at a time */
  for (i = 0; i < limit; i+=2) {
    acc += data0[i] * data1[i] + data0[i+1] * data1[i+1];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc += data0[i] * data1[i];
  }
  *dest = acc;
}

/* dot6_2:  Unroll loop by 2, 2 accumulators
 * Example of --> parallelism */
void dot6_2(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  long int limit = length - 1;
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t acc0 = (data_t)(0);
  data_t acc1 = (data_t)(0);

  /* Combine two elements at a time w/ 2 acculators */
  for (i = 0; i < limit; i+=2) {
    acc0 += data0[i] * data1[i];
    acc1 += data0[i+1] * data1[i+1];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 += data0[i] * data1[i];
  }
  *dest = acc0 + acc1;
}

/* dot6_5:  Unroll loop by 5, 5 accumulators
 * Example of --> parallelism */
void dot6_5(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v0);
  long int limit = length - 1;
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  data_t acc0 = (data_t)(0);
  data_t acc1 = (data_t)(0);
  data_t acc2 = (data_t)(0);
  data_t acc3 = (data_t)(0);
  data_t acc4 = (data_t)(0);

  /* Combine two elements at a time w/ 2 acculators */
  for (i = 0; i < limit; i+=5) {
    acc0 += data0[i] * data1[i];
    acc1 += data0[i+1] * data1[i+1];
    acc2 += data0[i+2] * data1[i+2];
    acc3 += data0[i+3] * data1[i+3];
    acc4 += data0[i+4] * data1[i+4];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 += data0[i] * data1[i];
  }
  *dest = acc0 + acc1 + acc2 + acc3 + acc4;
}

/* dot8:  Vector */
void dot8(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int cnt = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  vec_t accum;
  data_t result = (data_t)(0);
  pack_t xfer;

  /* initialize accum entries to 0 */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = (data_t)(0);
  accum = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) v0) % VBYTES && cnt) {
    result = *data0++ * *data1++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= VSIZE) {
    vec_t v0chunk = *((vec_t *) data0);
    vec_t v1chunk = *((vec_t *) data1);
    accum = accum + (v0chunk * v1chunk);
    data0 += VSIZE;
    data1 += VSIZE;
    cnt -= VSIZE;
  }

  /* Single step through remaining elements */
  while (cnt) {
    result += *data0++ * *data1++;
    cnt--;
  }

  /* Combine elements of accumulator vector */
  xfer.v = accum;
  for (i = 0; i < VSIZE; i++) result += xfer.d[i];

  /* store result */
  *dest = result;
}

/* dot8_2:  Vector */
void dot8_2(vec_ptr v0, vec_ptr v1, data_t *dest)
{
  long int i;
  long int cnt = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  vec_t accum0;
  vec_t accum1;
  data_t result = (data_t)(0); // initialize and cast as data_t zero
  pack_t xfer;

  /* initialize accum entries to 0 */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = (data_t)(0);
  accum0 = xfer.v;
  accum1 = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) v0) % VBYTES && cnt) {
    result = *data0++ * *data1++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= VSIZE) {
    vec_t v0chunk0 = *((vec_t *) data0);
    vec_t v1chunk0 = *((vec_t *) data1);
    vec_t v0chunk1 = *((vec_t *) data0+VSIZE);
    vec_t v1chunk1 = *((vec_t *) data1+VSIZE);
    accum0 = accum0 + (v0chunk0 * v1chunk0);
    accum1 = accum1 + (v0chunk1 * v1chunk1);
    data0 += 2*VSIZE;
    data1 += 2*VSIZE;
    cnt -= 2*VSIZE;
  }

  /* Single step through remaining elements */
  while (cnt) {
    result += *data0++ * *data1++;
    cnt--;
  }

  /* Combine elements of accumulator vector */
  xfer.v = accum0 + accum1;
  for (i = 0; i < VSIZE; i++) result += xfer.d[i];

  /* store result */
  //printf("Eight by two: %f\n", result);  	
  *dest = result;
}

void dot8_4(vec_ptr v0, vec_ptr v1, data_t *dest) //assumes they are same length like others
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int cnt = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  vec_t accum0;
  vec_t accum1;
  vec_t accum2;
  vec_t accum3;
  data_t result = (data_t)(0);
  pack_t xfer;

  /* initialize accum entries to 0 */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = (data_t)(0);
  accum0 = xfer.v;
  accum1 = xfer.v;
  accum2 = xfer.v;
  accum3 = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) v0) % (4*VBYTES) && cnt) {
    result = *data0++ * *data1++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= 4*VSIZE) {
    vec_t v0chunk0 = *((vec_t *) data0);
    vec_t v0chunk1 = *((vec_t *) data0+VSIZE);
    vec_t v0chunk2 = *((vec_t *) data0+2*VSIZE);
    vec_t v0chunk3 = *((vec_t *) data0+3*VSIZE);

    vec_t v1chunk0 = *((vec_t *) data1);
    vec_t v1chunk1 = *((vec_t *) data1+VSIZE);
    vec_t v1chunk2 = *((vec_t *) data1+2*VSIZE);
    vec_t v1chunk3 = *((vec_t *) data1+3*VSIZE);

    accum0 = accum0 + (v0chunk0 * v1chunk0);
    accum1 = accum1 + (v0chunk1 * v1chunk1);
    accum2 = accum2 + (v0chunk2 * v1chunk2);
    accum3 = accum3 + (v0chunk3 * v1chunk3);

    data0 += 4*VSIZE;
    data1 += 4*VSIZE;
    cnt -= 4*VSIZE;
  }

  /* Single step through remaining elements */
  while (cnt) {
    result += *data0++ * *data1++;
    cnt--;
  }

  /* Combine elements of accumulator vector */
  xfer.v = accum0 + accum1 + accum2 + accum3;
  for (i = 0; i < VSIZE; i++) result += xfer.d[i];

  /* store result */
  //printf("Eight by four: %f\n", result);
  *dest = result;
}

void dot8_8(vec_ptr v0, vec_ptr v1, data_t *dest) //assumes they are same length like others
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int cnt = get_vec_length(v0);
  data_t *data0 = get_vec_start(v0);
  data_t *data1 = get_vec_start(v1);
  vec_t accum0;
  vec_t accum1;
  vec_t accum2;
  vec_t accum3;
  vec_t accum4;
  vec_t accum5;
  vec_t accum6;
  vec_t accum7;
  data_t result = (data_t)(0);
  pack_t xfer;

  /* initialize accum entries to 0 */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = (data_t)(0);
  accum0 = xfer.v;
  accum1 = xfer.v;
  accum2 = xfer.v;
  accum3 = xfer.v;
  accum4 = xfer.v;
  accum5 = xfer.v;
  accum6 = xfer.v;
  accum7 = xfer.v;  

  /* Single step until we have memory alignment */
  while (((long) v0) % (8*VBYTES) && cnt) {
    result = *data0++ * *data1++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= 8*VSIZE) {
    vec_t v0chunk0 = *((vec_t *) data0);
    vec_t v0chunk1 = *((vec_t *) data0+VSIZE);
    vec_t v0chunk2 = *((vec_t *) data0+2*VSIZE);
    vec_t v0chunk3 = *((vec_t *) data0+3*VSIZE);
    vec_t v0chunk4 = *((vec_t *) data0+4*VSIZE);
    vec_t v0chunk5 = *((vec_t *) data0+5*VSIZE);
    vec_t v0chunk6 = *((vec_t *) data0+6*VSIZE);
    vec_t v0chunk7 = *((vec_t *) data0+7*VSIZE);    

    vec_t v1chunk0 = *((vec_t *) data1);
    vec_t v1chunk1 = *((vec_t *) data1+VSIZE);
    vec_t v1chunk2 = *((vec_t *) data1+2*VSIZE);
    vec_t v1chunk3 = *((vec_t *) data1+3*VSIZE);
    vec_t v1chunk4 = *((vec_t *) data1+4*VSIZE);
    vec_t v1chunk5 = *((vec_t *) data1+5*VSIZE);
    vec_t v1chunk6 = *((vec_t *) data1+6*VSIZE);
    vec_t v1chunk7 = *((vec_t *) data1+7*VSIZE);    

    accum0 = accum0 + (v0chunk0 * v1chunk0);
    accum1 = accum1 + (v0chunk1 * v1chunk1);
    accum2 = accum2 + (v0chunk2 * v1chunk2);
    accum3 = accum3 + (v0chunk3 * v1chunk3);
    accum4 = accum4 + (v0chunk4 * v1chunk4);
    accum5 = accum5 + (v0chunk5 * v1chunk5);
    accum6 = accum6 + (v0chunk6 * v1chunk6);
    accum7 = accum7 + (v0chunk7 * v1chunk7);    

    data0 += 8*VSIZE;
    data1 += 8*VSIZE;
    cnt -= 8*VSIZE;
  }

  /* Single step through remaining elements */
  while (cnt) {
    result += *data0++ * *data1++;
    cnt--;
  }

  /* Combine elements of accumulator vector */
  xfer.v = accum0 + accum1 + accum2 + accum3 + accum4 + accum5 + accum6 + accum7;
  for (i = 0; i < VSIZE; i++) result += xfer.d[i];

  /* store result */
  //printf("Eight by eight: %f\n", result);
  *dest = result;
}
