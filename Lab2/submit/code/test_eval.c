/* -*- C++ -*- ***************************************************************/

// gcc test_combine1-7.c -lrt -o tcmb

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer
double CPS = 2.9e9;    // Cycles per second -- Will be recomputed at runtime

#define OPTIONS 6
#define IDENT 1.0
#define OP *

#define FILE_PREFIX ((const unsigned char*) "polynomial_")

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

double basic_poly(double a[], double x, int degree);
double horner_poly(double a[], double x, int degree);
double estrin_poly(double a[], double x, int degree);
double estrin_aggro(double a[], double x, int degree);
double horner_inter(double a[], double x, int degree);
double horner_reassoc(double a[], double x, int degree);

/*****************************************************************************/
int main(int argc, char *argv[])
{
  
  int DEGREE, ITERS, X_VAL;

  if(argc != 4)
  {
  	printf("num args wrong\n");
  	return 0;
  }

  DEGREE  = strtol(argv[1], NULL, 10);
  ITERS   = strtol(argv[2], NULL, 10);
  X_VAL   = strtol(argv[3], NULL, 10);

  if(DEGREE == 0)
  {
  	printf("DEGREE must be greater than zero\n");
  	return 0;
  }

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sD%d_I%d_X%d.csv", FILE_PREFIX, DEGREE, ITERS, X_VAL);
  printf("Current File: %s\n", filename);

  int OPTION;

  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS];
  
  long int i, j, k;
  long long int time_sec, time_ns;

  double a[(DEGREE*ITERS)+1];
  int elements = 0;
  // initialize a with something
  for (i = 0; i < (DEGREE*ITERS)+1 ; i++) a[i] = i;

  measure_cps();

  ///////////////////////////////////////////////
  //
  //  Begin unroll
  //
  ///////////////////////////////////////////////
  double calc = 0 ;

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    elements += (i+1) * DEGREE;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += basic_poly(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += horner_poly(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += estrin_poly(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += horner_inter(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += horner_reassoc(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    calc += estrin_aggro(a, X_VAL, (i+1)*DEGREE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    //printf("%lf\n", calc);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  printf("%lf\n", calc);

  ////////////////////////////////////////////////////
  //  Write to file
  ////////////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp, "degree, basic, horner, estrin, horner Interleaved, horner reassoc, estrin aggro\n"); // kludge line one

  for (i = 0; i < ITERS; i++) {
    fprintf(fp, "%ld,  ", (i+1)*DEGREE); // i covers the degree
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) fprintf(fp, ", ");
       fprintf(fp, "%lf", ((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)/elements));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);

  return 0;
  
}/* end main */


/*************************************************/
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

/*************************************************/

double basic_poly(double a[], double x, int degree)
{
  long int i;
  double result = a[0];
  double xpwr   = x;

  for(i = 1; i <= degree; i++)
  {
    result += a[i] * xpwr;
    xpwr = x * xpwr;
  }

  return result;
}


double horner_poly(double a[], double x, int degree)
{
  long int i;
  double result = a[degree];

  for(i = degree - 1; i >= 0; i--)
  {
    result = a[i] + x*result;
  }

  return result;
}

// Esstrin's scheme
// (C0 + C1 x) + (C2 + C3 x)x^2 ...
// might have good potential for expanding
double estrin_poly(double a[], double x, int degree)
{
  long int i;
  double acc  = a[0] + a[1] * x;
  double x_sq  = x * x;
  double x_acc = x * x;

  for(i = 2; i < degree; i += 2)
  {
    acc  += (a[i] + a[i+1] * x) * x_acc;
    x_acc *= x_sq;
  }

  for (; i <= degree; i++) // clean up
  {
    acc += a[i] * x_acc;
  }

  return acc;
}

//aggressive version of estrin
// turned out to be just about as good
double estrin_aggro(double a[], double x, int degree)
{
  long int i;
  double x_sq  = x * x;
  double acc  = a[0] + a[1] * x + a[2] * x_sq; 
  double x_acc = x_sq * x;
  double x_tr  = x_acc;

  for(i = 3; i < degree; i += 3)
  {
    acc  += (a[i] + a[i+1] * x + a [i+2] * x_sq) * x_acc;
    x_acc *= x_tr;
  }

  for (; i <= degree; i++) // clean up
  {
    acc += a[i] * x_acc;
  }

  return acc;
}

//try interleaving horner with unroll since it seems the most straightforward for it
// seems to have sligh boost
double horner_inter(double a[], double x, int degree)
{
  long int i;
  double result = a[degree];

  for(i = degree - 1; i > 0; i-=2)
  {
    result = a[i-1] + x * (a[i] + x * result);
  }

  for(; i >= 0; i--) // clean up my mess
  {
    result = a[i] + x*result;
  }

  return result;
}

// try manipulating multiplier
//not powerful...
double horner_reassoc(double a[], double x, int degree)
{
  long int i;
  double x_sq   = x * x;
  double result = a[degree];
  double t1, t2; //temps to try and break muls

  for(i = degree - 1; i > 0; i-=2)
  {
    t1 = x_sq * result;
    t2 = x * a[i];
    result = a[i-1] + t1 + t2;
  }

  for(; i >= 0; i--) // clean up my mess
  {
    result = a[i] + x*result;
  }

  return result;
}