/* -*- C++ -*- ****************************************************************/
//
// gcc -O1 -mavx test_transpose.c -lrt -lm -o tran

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>  

double CPS = 2.9e9;       // Cycles/sec - adjusted by measure_cps()

// #define BASE  0
// #define ITERS 31
// #define DELTA 100

#define BSIZE 10
#define BBASE 16
#define BITERS 5

#define OPTIONS 4       // ij and ji -- need to add other methods

#define FILE_PREFIX ((const unsigned char*) "double_transpose_")

typedef double data_t;

/* Create abstract data type for vector */
typedef struct {
  long int size;
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

vec_ptr new_array(long int size);
int set_array_size(vec_ptr v, long int index);
long int get_array_size(vec_ptr v);
int init_array(vec_ptr v, long int size);
data_t *get_array_buffer(vec_ptr v);
data_t *data_holder;

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

//////////////  transpose prototypes  /////////////////////////////////////////

void array_print(vec_ptr v0);
void transpose(vec_ptr v0, vec_ptr v1);
void transpose_rev(vec_ptr v0, vec_ptr v1);
void xpose_blocked(vec_ptr v0, vec_ptr v1);
void xpose_intrin(vec_ptr src, vec_ptr dst);

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

  if (BASE < 0) {
  	printf("BASE must be non-negative\n");
  	return 0;
  }

  if (DELTA <= 0) {
  	printf("DELTA must be greater than zero\n");
  	return 0;
  }

  if (ITERS <= 0) {
  	printf("ITERS must be at least one\n");
  	return 0;
  }

  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  long int i, j, k, bsize = BSIZE;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n",filename);

  measure_cps();

  // declare and initialize the vector structures
  vec_ptr v0 = new_array(MAXSIZE);
  vec_ptr v1 = new_array(MAXSIZE);
  init_array(v0, MAXSIZE);
  init_array(v1, MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    set_array_size(v0,BASE+(i+1)*DELTA);
    set_array_size(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    // array_print(v0);
    transpose(v0, v1);
    // array_print(v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_array_size(v0,BASE+(i+1)*DELTA);
    set_array_size(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    // array_print(v0);
    transpose_rev(v0, v1);
    // array_print(v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_array_size(v0,BASE+(i+1)*DELTA);
    set_array_size(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    // array_print(v0);
    xpose_blocked(v0, v1);
    // array_print(v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    set_array_size(v0,BASE+(i+1)*DELTA);
    set_array_size(v1,BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
     array_print(v0);
    xpose_intrin(v0, v1);
     array_print(v1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }


  /* output times */
  long int size, elements;
  fp = fopen(filename,"w");
  fprintf(fp, "size,   ij,   ji,  ij_blk\n");
  for (i = 0; i < ITERS; i++) {
  	size = BASE+(i+1)*DELTA;
    elements = size * size;
    fprintf(fp, "%ld", size);
    for (j = 0; j < OPTIONS; j++) {
      double seconds = ((double) time_stamp[j][i].tv_sec)
                     + ((double) time_stamp[j][i].tv_nsec)/1.0e9;
      fprintf(fp, ", %lf", CPS * seconds / ((double) elements));

      // printf("%ld", (long int)((double)(CPG)*(double)
	  // (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  
}/* end main */
/*********************************/

/* Create 2D vector of specified length per dimension */
vec_ptr new_array(long int size)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->size = size;

  /* Allocate and declare array */
  if (size > 0) {
    int err;
    data_t *data;
    err = posix_memalign((void **)(&data), VBYTES,
                                      ((size_t)(size*size))*sizeof(data_t));
    if (err) {
	  free((void *) result);
	  return NULL;  /* Couldn't allocate storage */
	}
	result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Set length of vector */
int set_array_size(vec_ptr v, long int index)
{
  v->size = index;
  return 1;
}

/* Return length of vector */
long int get_array_size(vec_ptr v)
{
  return v->size;
}

/* initialize 2D vector */
int init_array(vec_ptr v, long int size)
{
  long int i;

  if (size > 0) {
    v->size = size;
    for (i = 0; i < size*size; i++)
      v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

data_t *get_array_buffer(vec_ptr v)
{
  return v->data;
}

/************************************/

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

/************************************/

void array_print(vec_ptr v0)
{
  long int row, col;
  long int size = get_array_size(v0);
  data_t *data0 = get_array_buffer(v0);

  for (row = 0; row < size; row++) {
    for (col = 0; col < size; col++) {
      printf("%4.0f ", data0[row*size+col]);
    }
    printf("\n");
  }
}

/* transpose */
void transpose(vec_ptr src, vec_ptr dst)
{
  long int i, j;
  long int length = get_array_size(src);
  data_t *src_dat = get_array_buffer(src);
  data_t *dst_dat = get_array_buffer(dst);

  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      dst_dat[j*length+i] = src_dat[i*length+j];
}

/* transpose */
void transpose_rev(vec_ptr src, vec_ptr dst)
{
  long int i, j;
  long int length = get_array_size(src);
  data_t *src_dat = get_array_buffer(src);
  data_t *dst_dat = get_array_buffer(dst);

  for (i = 0; i < length; i++)
    for (j = 0; j < length; j++)
      dst_dat[i*length+j] = src_dat[j*length+i];
}

void xpose_blocked(vec_ptr src, vec_ptr dst)
{
  long int i, j;
  long int ib, jb;
  long int length = get_array_size(src);
  data_t *src_dat = get_array_buffer(src);
  data_t *dst_dat = get_array_buffer(dst);

  for (ib = 0; ib < length; ib+=4) {
    for (jb = 0; jb < length; jb+=4) {
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          dst_dat[(jb+j)*length+(ib+i)] = src_dat[(ib+i)*length+(jb+j)];
        }
      }
    }
  }
}

// Sources:
//   software.intel.com/sites/landingpage/IntrinsicsGuide
//   stackoverflow.com/questions/16941098
//     (shows an 8x8 transpose for floats)
//   www.randombit.net/bitbashing/2009/10/08/integer_matrix_transpose_in_sse2.html
inline void transpose4_pd(__m256d *r0, __m256d *r1, __m256d *r2, __m256d *r3)
{
  __m256d _t0, _t1, _t2, _t3;
  _t0 = _mm256_unpacklo_pd(*r0, *r1);
  _t1 = _mm256_unpacklo_pd(*r2, *r3);
  _t2 = _mm256_unpackhi_pd(*r0, *r1);
  _t3 = _mm256_unpackhi_pd(*r2, *r3);
  *r0 = _mm256_shuffle_pd(_t0, _t1, _MM_SHUFFLE(1,0,1,0));
  *r1 = _mm256_shuffle_pd(_t0, _t1, _MM_SHUFFLE(3,2,3,2));
  *r2 = _mm256_shuffle_pd(_t2, _t3, _MM_SHUFFLE(1,0,1,0));
  *r3 = _mm256_shuffle_pd(_t2, _t3, _MM_SHUFFLE(3,2,3,2));
}

void xpose_intrin(vec_ptr src, vec_ptr dst)
{
  long int rblk, cblk;
  long int ri, ci;
  long int length = get_array_size(src);
  long int l4 = length / 4;
  data_t *src_dat = get_array_buffer(src);
  data_t *dst_dat = get_array_buffer(dst);
  __m256d* pSrc   = (__m256d*) src_dat;
  __m256d* pDst   = (__m256d*) dst_dat;
  __m256d r0, r1, r2, r3;

  for (rblk = 0; rblk < length; rblk+=4) {
    for (cblk = 0; cblk < length; cblk+=4) {
      r0 = pSrc[(rblk+0)*l4+(cblk/4)];
      r1 = pSrc[(rblk+1)*l4+(cblk/4)];
      r2 = pSrc[(rblk+2)*l4+(cblk/4)];
      r3 = pSrc[(rblk+3)*l4+(cblk/4)];
      transpose4_pd(&r0, &r1, &r2, &r3);
      pDst[(cblk+0)*l4+(rblk/4)] = r0;
      pDst[(cblk+1)*l4+(rblk/4)] = r1;
      pDst[(cblk+2)*l4+(rblk/4)] = r2;
      pDst[(cblk+3)*l4+(rblk/4)] = r3;
    }
  }
}
