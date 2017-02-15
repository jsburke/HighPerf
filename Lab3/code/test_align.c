/**************************************************************/
// gcc -o test_align test_align.c -lrt

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <xmmintrin.h>
#include <mm_malloc.h>
#include <time.h>		// get time of day

#define OPTIONS 2

#define GIG 1000000000
#define CPG 2.0                 // Cycles per GHz -- Adjust to your computer

#define ARR_SIZE 8*1024*1024      // 1M -- this x8 = L3 cache size
#define BOUNDARY_ALIGNMENT 64   // cache block size

/**************************************************************/
main(int argc, char *argv[])
{
  int OPTION = 0;

  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][2*BOUNDARY_ALIGNMENT];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  long long int time_sec, time_ns;

  int i,j,k;   	   /* Local variables. */
  int ok;
  char *temp;
  double *x1,*x2,*x3;

  // posix memalign array declaration -- x1 not used, demo only
  ok = posix_memalign((void**)&x1, BOUNDARY_ALIGNMENT, ARR_SIZE * sizeof(double));

  // a malloc() with an alignment wrapper works too
  temp = (char*)_mm_malloc((ARR_SIZE+BOUNDARY_ALIGNMENT) * sizeof(double),
			   BOUNDARY_ALIGNMENT);

  x2 = (double*) (temp);  // save original starting point for another test

  printf("\n Hello World!  temp address = %ld\n", (unsigned long)temp);

  // Try different starting positions of x3, incrementing by 1 byte each iter.
  // Use this to show what happens when you fetch doubles on addresses not
  // aligned to doubles.
  for (k = 0; k < BOUNDARY_ALIGNMENT; k++) {
    x3 = (double*)(temp+k);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    for (i = 0; i < 100; i++)
      x3[i] += 1.3 + (double)(i);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][k] = diff(time1,time2);
  }

  /* output times */
  for (j = 0; j < BOUNDARY_ALIGNMENT; j++) {
    printf("\n %d, %ld", j, (long int)((double)(CPG)*(double)
	       (GIG * time_stamp[OPTION][j].tv_sec + time_stamp[OPTION][j].tv_nsec)));
  }

  // Again, try different starting positions of x3.  But this time, code in way
  // that has the following property:  In most cases it will cause one cache
  // miss per fetch.  But for some values of k, each fetch causes two cache misses!
  OPTION++;
  temp = (char*)(x2); // reset temp,
  for (k = 0; k < BOUNDARY_ALIGNMENT; k++) { 
    x3 = (double*)(temp+k);                  
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    /* NEW CODE GOES HERE */

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][k] = diff(time1,time2);
  }

  /* output times */
  for (k = 0; k < BOUNDARY_ALIGNMENT; k++)
    printf("\n %d, %ld", k, (long int)((double)(CPG)*(double)
      	 (GIG * time_stamp[OPTION][k].tv_sec + time_stamp[OPTION][k].tv_nsec)));

  printf("\n\nGood Bye World!\n\n");

}/* end main */


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

