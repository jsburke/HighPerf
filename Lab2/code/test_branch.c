/* -*- C++ -*- **************************************************************/

// gcc test_branch.c -lrt -o tbr

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9   

#define BASE 0
#define DELTA 5000
#define ITERS 20

#define OPTIONS 10

#define ITER_VECLEN(i) (BASE+((i)+1)*DELTA)

#define FILE_PREFIX ((const unsigned char*) "branch_")

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

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
double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime
int clock_gettime(clockid_t clk_id, struct timespec *tp);

typedef union {
  unsigned long long int64;
  struct {unsigned int lo, hi;} int32;
} mcps_tctr;

#define MCPS_RDTSC(cpu_c) __asm__ __volatile__ ("rdtsc" : \
                     "=a" ((cpu_c).int32.lo), "=d"((cpu_c).int32.hi))

double ts_sec(struct timespec ts);
struct timespec ts_diff(struct timespec start, struct timespec end);
double measure_cps(void);

double ts_sec(struct timespec ts)
{
  return ((double)(ts.tv_sec)) + ((double)(ts.tv_nsec))/1.0e9;
}

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

int clock_gettime(clockid_t clk_id, struct timespec *tp);
vec_ptr new_vec(long int len);
int get_vec_element(vec_ptr v, long int index, data_t *dest);
long int get_vec_length(vec_ptr v);
int set_vec_length(vec_ptr v, long int index);
int init_vec_zero(vec_ptr v, long int len);
int init_vec_low(vec_ptr v, long int len);
int init_vec_mid(vec_ptr v, long int len);
int init_vec_hi_lo(vec_ptr v, long int len);
int init_vec_rand(vec_ptr v, long int len);
int init_vec_linear(vec_ptr v, long int len);
void branch1(vec_ptr v0, vec_ptr v1, vec_ptr v2);
void branch2(vec_ptr v0, vec_ptr v1, vec_ptr v2);

/*****************************************************************************/
int main(int argc, char *argv[])
{
  int OPTION;
  long int elements[ITERS+1];
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  long int i, j, k;
  long long int time_sec, time_ns;
  long int MAXSIZE = ITER_VECLEN(ITERS);

  measure_cps();

  // declare vector structure         // init the vectors
  vec_ptr v0 = new_vec(MAXSIZE);      init_vec_zero(v0, MAXSIZE);
  vec_ptr vlow = new_vec(MAXSIZE);    init_vec_low(v0, MAXSIZE);
  vec_ptr vmid = new_vec(MAXSIZE);    init_vec_mid(vmid, MAXSIZE);
  vec_ptr vhilo = new_vec(MAXSIZE);   init_vec_hi_lo(vhilo, MAXSIZE);
  vec_ptr vrand = new_vec(MAXSIZE);   init_vec_rand(vrand, MAXSIZE); 
  vec_ptr vlin = new_vec(MAXSIZE);    init_vec_linear(vlin, MAXSIZE);

  vec_ptr v_out = new_vec(MAXSIZE);   init_vec_zero(v_out, MAXSIZE);

  // execute and time all options 
  for (i = 0; i < ITERS; i++) {
    set_vec_length(v0,   ITER_VECLEN(i));
    set_vec_length(vlow, ITER_VECLEN(i));
    set_vec_length(vmid, ITER_VECLEN(i));
    set_vec_length(vhilo,ITER_VECLEN(i));
    set_vec_length(vrand,ITER_VECLEN(i));
    set_vec_length(vlin, ITER_VECLEN(i));
    set_vec_length(v_out,ITER_VECLEN(i));
    elements[i] = ITER_VECLEN(i);

    OPTION = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(v0, vmid, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(v0, vmid, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(vlow, vmid, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(vlow, vmid, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(vmid, vlin, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(vmid, vlin, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(vmid, vhilo, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(vmid, vhilo, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch1(vmid, vrand, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    branch2(vmid, vrand, v_out);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    OPTION++;
  }

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n", filename);
  fp = fopen(filename, "w");

  /* output times */
  fprintf(fp, "Vector Size, ");
  fprintf(fp, "0-mid ifelse, 0-mid ?:, ");
  fprintf(fp, "low-mid ifelse, low-mid ?:, ");
  fprintf(fp, "linear ifelse, linear ?:, ");
  fprintf(fp, "hilo ifelse, hilo ?:, ");
  fprintf(fp, "rand ifelse, rand ?:, ");
  fprintf(fp, "\n");
  for (i = 0; i < ITERS; i++) {
    double cycles;
    int opt;
    fprintf(fp, "%ld", ((long int) ITER_VECLEN(i)));
    for(opt=0; opt<OPTIONS; opt++) {
      cycles = CPS * ts_sec(time_stamp[opt][i]);
      fprintf(fp, ", %f", cycles / elements[opt]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
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

/* initialize vector with all zeros */
int init_vec_zero(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = 0.0;
    }
    return 1;
  }
  else return 0;
}

/* initialize vector with all low values */
int init_vec_low(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = 1.0;
    }
    return 1;
  }
  else return 0;
}

/* initialize vector with all-"middle" values */
int init_vec_mid(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = 50.0;
    }
    return 1;
  }
  else return 0;
}

/* initialize vector with alternating high and low values */
int init_vec_hi_lo(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = (i % 2) ? 1.0 : 99.0;
    }
    return 1;
  }
  else return 0;
}

/* initialize vector with random values */
int init_vec_rand(vec_ptr v, long int len)
{
  long int i;
  double ratio = 100.0 / ((double)RAND_MAX);

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) {
      v->data[i] = ((double)rand()) * ratio;
    }
    return 1;
  }
  else return 0;
}

/* initialize vector with steadily increasing values, going from 0 to 100 */
int init_vec_linear(vec_ptr v, long int len)
{
  long int i;
  double ratio;

  if (len > 0) {
    v->len = len;
    ratio = 100.0 / ((double)len);
    for (i = 0; i < len; i++) {
      v->data[i] = ((double)i) * ratio;
    }
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
