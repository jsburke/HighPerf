/* -*- C++ -*- ****************************************************************/
//
// gcc -O1 -mavx2 test_combine8.c -DDTYPE=float -lrt -lm -o test_combine8
//
// combine4   -- base scalar code
// combine6_5 -- unrolled 5 times with 5 accumulators -- best scalar code
// combine8   -- base vector code
// combine8_2 -- unrolled 2 times with 2 accumulators
// combine8_4 -- unrolled 4 times with 4 accumulators
// combine8_8 -- unrolled 8 times with 8 accumulators

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer
double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime

#define SIZE 10000000
// #define ITERS 30
// #define DELTA 32
// #define BASE 0

#define OPTIONS 6      // NEED TO MODIFY
#define IDENT 1.0
#define OP + 

// "stringize" macro turns a type into a string. For example STRINGIZE(int)
// gives "int". Thie is from stackoverflow.com/questions/5256313
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
#define FILE_PREFIX ((const unsigned char*) (STRINGIZE(DTYPE)"_add_comb8_"))

typedef DTYPE data_t;

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

///////////  Timing Related   /////////////////////////////////////////////////

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

//////////////  functions for this file ///////////////////////////////////////

void combine4(vec_ptr v, data_t *dest);
void combine6_5(vec_ptr v, data_t *dest);
void combine8(vec_ptr v, data_t *dest);
void combine8_2(vec_ptr v, data_t *dest);
void combine8_4(vec_ptr v, data_t *dest);
void combine8_8(vec_ptr v, data_t *dest);

/*****************************************************************************/
int main(int argc, char *argv[])
{
  int BASE, DELTA, ITERS;

  if(argc != 4)
  {
  	printf("Must give exactly 3 arguments (BASE, DELTA, ITERS)\n");
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

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  data_holder = (data_t *) malloc(sizeof(data_t));
  init_vector(v0, MAXSIZE);

  ////////////////////////////////////////////////
  //
  //  Begin combine calls
  //
  ////////////////////////////////////////////////
  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine4(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine6_5(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine8(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine8_2(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine8_4(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    combine8_8(v0, data_holder);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }  

  ///////////////////////////////////////////////////////
  //
  //  Write to file
  //
  ///////////////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp,"size, c4, c6_5,  c8,  c8_2, c8_4, c8_8\n");

  int elements;
  //printf("CPS calculated at: %lf",CPS);
  for (i = 0; i < ITERS; i++) {
  	elements = BASE+(i+1)*DELTA;
    fprintf(fp, "%d, ", elements);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp, ", ");
      fprintf(fp, "%lf", ((double)(CPG)*(double)(GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)/elements));
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
    int err;
    data_t *data;
    err = posix_memalign((void **)(&data), VBYTES, ((size_t)len)*sizeof(data_t));
    if (err) {
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

/* Combine6_5:  Unroll loop by 5, 5 accumulators
 * Example of --> parallelism */
void combine6_5(vec_ptr v, data_t *dest)
{
  long int i;
  long int get_vec_length(vec_ptr v);
  data_t *get_vec_start(vec_ptr v);
  long int length = get_vec_length(v);
  long int limit = length - 1;
  data_t *data = get_vec_start(v);
  data_t acc0 = IDENT;
  data_t acc1 = IDENT;
  data_t acc2 = IDENT;
  data_t acc3 = IDENT;
  data_t acc4 = IDENT;

  /* Combine 5 elements at a time w/ 5 acculators */
  for (i = 0; i < limit; i+=5) {
    acc0 = acc0 OP data[i];
    acc1 = acc1 OP data[i+1];
    acc2 = acc2 OP data[i+2];
    acc3 = acc3 OP data[i+3];
    acc4 = acc4 OP data[i+4];
  }

  /* Finish remaining elements */
  for (; i < length; i++) {
    acc0 = acc0 OP data[i];
  }
  *dest = acc0 OP acc1 OP acc2 OP acc3 OP acc4;
}

/* Combine8:  Vector version */
void combine8(vec_ptr v, data_t *dest)
{
  long int i;
  pack_t xfer;
  vec_t accum;
  data_t *data = get_vec_start(v);
  long int cnt = get_vec_length(v);
  data_t result = IDENT;

  /* Initialize accum entries to IDENT */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = IDENT;
  accum = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) data) % VBYTES && cnt) {
    result = result OP *data++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= VSIZE) {
    vec_t chunk = *((vec_t *) data);
    accum = accum OP chunk;
    data += VSIZE;
    cnt -= VSIZE;
  }

  /* Single-step through the remaining elements */
  while (cnt > 0) {
    result = result OP *data++;
    cnt--;
  }

  /* Combine elements of accumulator vector */
  xfer.v = accum;
  for (i = 0; i < VSIZE; i++)
    result = result OP xfer.d[i];

  /* store result */
  *dest = result;
}

/* Combine8_4:  Vector 4x unrolled version */
void combine8_4(vec_ptr v, data_t *dest)
{
  long int i;
  pack_t xfer;
  vec_t accum0;
  vec_t accum1;
  vec_t accum2;
  vec_t accum3;
  data_t *data = get_vec_start(v);
  long int cnt = get_vec_length(v);
  data_t result = IDENT;

  /* Initialize accum entries to IDENT */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = IDENT;
  accum0 = xfer.v;
  accum1 = xfer.v;
  accum2 = xfer.v;
  accum3 = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) data) % (4*VBYTES) && cnt) {
    result = result OP *data++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= 4*VSIZE) {
    vec_t chunk0 = *((vec_t *) data);
    vec_t chunk1 = *((vec_t *) data+VSIZE);
    vec_t chunk2 = *((vec_t *) data+2*VSIZE);
    vec_t chunk3 = *((vec_t *) data+3*VSIZE);
    accum0 = accum0 OP chunk0;
    accum1 = accum1 OP chunk1;
    accum2 = accum2 OP chunk2;
    accum3 = accum3 OP chunk3;
    data += 4*VSIZE;
    cnt -= 4*VSIZE;
  }

  /* Single-step through the remaining elements */
  while (cnt > 0) {
    result = result OP *data++;
    cnt--;
  }

  /* Combine elements of accumulator vectors */
  xfer.v = (accum0 OP accum1) OP (accum2 OP accum3);

  for (i = 0; i < VSIZE; i++)
    result = result OP xfer.d[i];

  /* store result */
  *dest = result;
}


void combine8_2(vec_ptr v, data_t *dest)
{
  long int i;
  pack_t xfer;
  vec_t accum0;
  vec_t accum1;
  // vec_t accum2;
  // vec_t accum3;
  data_t *data = get_vec_start(v);
  long int cnt = get_vec_length(v);
  data_t result = IDENT;

  /* Initialize accum entries to IDENT */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = IDENT;
  accum0 = xfer.v;
  accum1 = xfer.v;
  // accum2 = xfer.v;
  // accum3 = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) data) % (2*VBYTES) && cnt) {
    result = result OP *data++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= 2*VSIZE) {
    vec_t chunk0 = *((vec_t *) data);
    vec_t chunk1 = *((vec_t *) data+VSIZE);
    // vec_t chunk2 = *((vec_t *) data+2*VSIZE);
    // vec_t chunk3 = *((vec_t *) data+3*VSIZE);
    accum0 = accum0 OP chunk0;
    accum1 = accum1 OP chunk1;
    // accum2 = accum2 OP chunk2;
    // accum3 = accum3 OP chunk3;
    data += 2*VSIZE;
    cnt -= 2*VSIZE;
  }

  /* Single-step through the remaining elements */
  while (cnt > 0) {
    result = result OP *data++;
    cnt--;
  }

  /* Combine elements of accumulator vectors */
  xfer.v = (accum0 OP accum1);// OP (accum2 OP accum3);

  for (i = 0; i < VSIZE; i++)
    result = result OP xfer.d[i];

  /* store result */
  *dest = result;
}

/* Combine8_8:  Vector 8x unrolled version */
void combine8_8(vec_ptr v, data_t *dest)
{
  long int i;
  pack_t xfer;
  vec_t accum0;
  vec_t accum1;
  vec_t accum2;
  vec_t accum3;
  vec_t accum4;
  vec_t accum5;
  vec_t accum6;
  vec_t accum7;
  data_t *data = get_vec_start(v);
  long int cnt = get_vec_length(v);
  data_t result = IDENT;

  /* Initialize accum entries to IDENT */
  for (i = 0; i < VSIZE; i++) xfer.d[i] = IDENT;
  accum0 = xfer.v;
  accum1 = xfer.v;
  accum2 = xfer.v;
  accum3 = xfer.v;
  accum4 = xfer.v;
  accum5 = xfer.v;
  accum6 = xfer.v;
  accum7 = xfer.v;

  /* Single step until we have memory alignment */
  while (((long) data) % (8*VBYTES) && cnt) {
    result = result OP *data++;
    cnt--;
  }

  /* Step through data with VSIZE-way parallelism */
  while (cnt >= 8*VSIZE) {
    vec_t chunk0 = *((vec_t *) data);
    vec_t chunk1 = *((vec_t *) data+VSIZE);
    vec_t chunk2 = *((vec_t *) data+2*VSIZE);
    vec_t chunk3 = *((vec_t *) data+3*VSIZE);
    vec_t chunk4 = *((vec_t *) data+4*VSIZE);
    vec_t chunk5 = *((vec_t *) data+5*VSIZE);
    vec_t chunk6 = *((vec_t *) data+6*VSIZE);
    vec_t chunk7 = *((vec_t *) data+7*VSIZE);

    accum0 = accum0 OP chunk0;
    accum1 = accum1 OP chunk1;
    accum2 = accum2 OP chunk2;
    accum3 = accum3 OP chunk3;
    accum4 = accum4 OP chunk4;
    accum5 = accum5 OP chunk5;
    accum6 = accum6 OP chunk6;
    accum7 = accum7 OP chunk7;
    data += 8*VSIZE;
    cnt -= 8*VSIZE;
  }

  /* Single-step through the remaining elements */
  while (cnt > 0) {
    result = result OP *data++;
    cnt--;
  }

  /* Combine elements of accumulator vectors */
  xfer.v = (accum0 OP accum1) OP (accum2 OP accum3) OP (accum4 OP accum5) OP (accum6 OP accum7);

  for (i = 0; i < VSIZE; i++)
    result = result OP xfer.d[i];

  /* store result */
  *dest = result;
}
