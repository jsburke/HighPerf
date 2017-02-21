/* -*- C++ -*- *************************************************/
// gcc -mavx -O1 -o intrin test_intrinsics.c -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>  

#define FALSE 0
#define TRUE 1

#define GIG 1000000000
#define CPG 2.0           // Cycles per GHz -- Adjust to your computer
double CPS = 2.9e9;       // Cycles/sec     -- adjusts

#define ARRAY_SIZE 100000000 // 100,000,000

#define SIZE 10000000
// #define ITERS 30
// #define DELTA 32
// #define BASE 0

#define OPTIONS 11
#define IDENT 1.0
#define OP *

#define FILE_PREFIX ((const unsigned char*) "floatMul_intrin_")

typedef float data_t;

float sum_256(__m256 x);

void  InitArray(data_t* pA, long int nSize);
void  InitArray_rand(data_t* pA, long int nSize);
void  ZeroArray(data_t* pA, long int nSize);

///////////////  Tests ///////////////////////////////

void  ArrayTest1(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
void  ArrayTest2(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
void  ArrayTest3(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);

void  Test_Add_128(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize); // benchmark add
void  Test_Add_256(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize); // benchmark add
void  Test_Mul_128(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize); // benchmark multiply
void  Test_Mul_256(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize); // benchmark multiply

float Test_Dot_128(data_t* pArray1, data_t* pArray2, long int nSize);  // first dot prod with intrin
float Test_Dot_256(data_t* pArray1, data_t* pArray2, long int nSize);  // second dot prod with intrin

float Test_Dot_128_single(data_t* pArray1, data_t* pArray2, long int nSize); // uses intrinsic dp_ps
float Test_Dot_256_single(data_t* pArray1, data_t* pArray2, long int nSize);

///////////// Timing related  /////////////////////////////////////////

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

/**************************************************************/
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
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  int i,j,k;       
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  double* var;
  int     ok;
  data_t*  pArray1;
  data_t*  pArray2;
  data_t*  pResult;
  long int nSize;

 
  ok = posix_memalign((void**)&pArray1, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pArray2, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pResult, 64, ARRAY_SIZE*sizeof(data_t));

  // initialize pArray1, pArray2
  InitArray_rand(pArray1,MAXSIZE);
  InitArray_rand(pArray2,MAXSIZE);
  ZeroArray(pResult,MAXSIZE);

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_D%d_I%d.csv", FILE_PREFIX, BASE, DELTA, ITERS);
  printf("Current File: %s\n",filename);

  measure_cps();

  ///////////////////////////////////////////////
  //
  //  Begin tests
  //
  ///////////////////////////////////////////////

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest1(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest2(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest3(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    Test_Add_128(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    Test_Add_256(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }  

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    Test_Mul_128(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    Test_Mul_256(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }    
  


    OPTION++;
    float dot_res = 0;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot_res = Test_Dot_128(pArray1, pArray2, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }   

  printf("%f\n", dot_res);

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot_res = Test_Dot_256(pArray1, pArray2, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }

  printf("%f\n", dot_res);

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot_res = Test_Dot_128_single(pArray1, pArray2, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }   

  printf("%f\n", dot_res);  

    OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot_res = Test_Dot_256_single(pArray1, pArray2, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = ts_diff(time1,time2);
  }   

  printf("%f\n", dot_res);  
  
  ///////////////////////////////////////////////
  //
  //  Write to file
  //
  ///////////////////////////////////////////////

  fp = fopen(filename,"w");
  fprintf(fp,"size, test1, test2, tes3, add 128, add 256, mul 128, mul 256 dot 128, dot 256, dp intrin 128, dp intrin 256\n");  

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

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/* initialize array to index */
void InitArray(data_t* v, long int len)
{
  long int i;

  for (i = 0; i < len; i++) v[i] = (data_t)(i);
}

/* initialize vector with another */
void InitArray_rand(data_t* v, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  for (i = 0; i < len; i++)
    v[i] = (data_t)(fRand((double)(0.0),(double)(10.0)));
}

/* initialize vector with 0s */
void ZeroArray(data_t* v, long int len)
{
  long int i;

  for (i = 0; i < len; i++)
    v[i] = (data_t)(0);
}

//******************************************************************

/* Simple distance calc */
void ArrayTest1(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int i;

  data_t* pSource1 = pArray1;
  data_t* pSource2 = pArray2;
  data_t* pDest = pResult;
  float sqrtf(float x);

  for (i = 0; i < nSize; i++){
    *pDest = sqrtf((*pSource1) * (*pSource1) +
      (*pSource2) * (*pSource2)) + 0.5f;
    pSource1++;
    pSource2++;
    pDest++;
  }
}

/* Simple distance calc w/ SSE */
void ArrayTest2(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/4;

  __m128   m1, m2, m3, m4;
  __m128   m0_5 = _mm_set_ps1(0.5f);

  __m128*  pSrc1 = (__m128*) pArray1;
  __m128*  pSrc2 = (__m128*) pArray2;
  __m128*  pDest = (__m128*) pResult;

  for (i = 0; i < nLoop; i++){
    m1 = _mm_mul_ps(*pSrc1, *pSrc1);
    m2 = _mm_mul_ps(*pSrc2, *pSrc2);
    m3 = _mm_add_ps(m1,m2);
    m4 = _mm_sqrt_ps(m3);
    *pDest = _mm_add_ps(m4,m0_5);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

/* Simple distance calc w/ SSE */
void ArrayTest3(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/8;

  __m256   m1, m2, m3, m4;
  __m256   m0_5 = _mm256_set1_ps(0.5f);   //changed

  __m256*  pSrc1 = (__m256*) pArray1;
  __m256*  pSrc2 = (__m256*) pArray2;
  __m256*  pDest = (__m256*) pResult;

  for (i = 0; i < nLoop; i++){
    m1 = _mm256_mul_ps(*pSrc1, *pSrc1);
    m2 = _mm256_mul_ps(*pSrc2, *pSrc2);
    m3 = _mm256_add_ps(m1,m2);
    m4 = _mm256_sqrt_ps(m3);
    *pDest = _mm256_add_ps(m4,m0_5);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void Test_Add_128(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize)
{
  int i;
  int nLoop = nSize/4;

  __m128* pSrc1 = (__m128*) pArray1;
  __m128* pSrc2 = (__m128*) pArray2;
  __m128* pDest = (__m128*) pResult;

  for(i = 0; i < nLoop; i++)
  {
    *pDest = _mm_add_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void Test_Add_256(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize)
{
  int i;
  int nLoop = nSize/8;

  __m256* pSrc1 = (__m256*) pArray1;
  __m256* pSrc2 = (__m256*) pArray2;
  __m256* pDest = (__m256*) pResult;

  for(i = 0; i < nLoop; i++)
  {
    *pDest = _mm256_add_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void Test_Mul_128(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize)
{
  int i;
  int nLoop = nSize/4;

  __m128* pSrc1 = (__m128*) pArray1;
  __m128* pSrc2 = (__m128*) pArray2;
  __m128* pDest = (__m128*) pResult;

  for(i = 0; i < nLoop; i++)
  {
    *pDest = _mm_mul_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void Test_Mul_256(data_t* pArray1, data_t* pArray2, data_t* pResult, long int nSize)
{
  int i;
  int nLoop = nSize/8;

  __m256* pSrc1 = (__m256*) pArray1;
  __m256* pSrc2 = (__m256*) pArray2;
  __m256* pDest = (__m256*) pResult;

  for(i = 0; i < nLoop; i++)
  {
    *pDest = _mm256_mul_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

float Test_Dot_128(data_t* pArray1, data_t* pArray2, long int nSize)
{ //Sum of products of aligned multiplies
  int i;
  int nLoop = nSize/4;
  float result;

  __m128* pSrc1 = (__m128*) pArray1;
  __m128* pSrc2 = (__m128*) pArray2;
  __m128  m_mul;
  __m128  m_accum = _mm_set_ps1(0.0f); // set an accumulator to zero

  for(i = 0; i < nLoop; i++)
  {
    m_mul   = _mm_mul_ps(*pSrc1,*pSrc2);
    m_accum = _mm_add_ps(m_mul, m_accum);

    pSrc1++;
    pSrc2++;  //  This code very incomplete
  }

  m_accum = _mm_hadd_ps(m_accum, m_accum); //sum the values
  m_accum = _mm_hadd_ps(m_accum, m_accum);
  _mm_store_ss(&result, m_accum);          // store the result

  return result;
}

// commenting out for further intrisic tests
// will evaluate later
//
float Test_Dot_256(data_t* pArray1, data_t* pArray2, long int nSize)
{ //Sum of products of aligned multiplies, floats
  int i;
  int nLoop = nSize/8;

  __m256* pSrc1 = (__m256*) pArray1;
  __m256* pSrc2 = (__m256*) pArray2;
  __m256  m_mul;
  __m256  m_accum = _mm256_set1_ps(0.0f);//, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

  for(i = 0; i < nLoop; i++)
  {
    m_mul   = _mm256_mul_ps(*pSrc1,*pSrc2);
    m_accum = _mm256_add_ps(m_mul, m_accum);

    pSrc1++;
    pSrc2++;  //  This code very incomplete
  }

  return sum_256(m_accum);
}

float sum_256(__m256 x)
{

  //should cover all corner cases
  //tldr: line and add up each of the 8 in the __MM256
  const __m128 hiQuad  = _mm256_extractf128_ps(x, 1);
  const __m128 lowQuad = _mm256_castps256_ps128(x);
  const __m128 sumQuad = _mm_add_ps(lowQuad, hiQuad);

  const __m128 lowDual = sumQuad;
  const __m128 hiDual  = _mm_movehl_ps(sumQuad, sumQuad);
  const __m128 sumDual = _mm_add_ps(lowDual, hiDual);

  const __m128 low = sumDual;
  const __m128 hi  = _mm_shuffle_ps(sumDual, sumDual, 0x1);
  const __m128 sum = _mm_add_ss(low, hi);

  return _mm_cvtss_f32(sum);
}

float Test_Dot_128_single(data_t* pArray1, data_t* pArray2, long int nSize)
{

  int i;
  int nLoop = nSize/4;
  float result;

  __m128* pSrc1 = (__m128*) pArray1;
  __m128* pSrc2 = (__m128*) pArray2;
  __m128  m_accum = _mm_set_ps1(0.0f);
  __m128  m_dotpart;

  for(i = 0; i < nLoop; i++)
  {
    m_dotpart = _mm_dp_ps(*pSrc1, *pSrc2, 0xff);
    m_accum   = _mm_add_ps(m_dotpart, m_accum);    

    pSrc1++;
    pSrc2++;
  }

  _mm_store_ss(&result, m_accum);// store the result, don't need hadd because dp covers it

  return result;
}

float Test_Dot_256_single(data_t* pArray1, data_t* pArray2, long int nSize)
{

  int i;
  int nLoop = nSize/8;
  float result;

  __m256* pSrc1   = (__m256*) pArray1;
  __m256* pSrc2   = (__m256*) pArray2;
  __m256  m_accum = _mm256_set1_ps(0.0f);//, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  __m256  m_dotpart;

  for(i = 0; i < nLoop; i++)
  {
    m_dotpart = _mm256_dp_ps(*pSrc1, *pSrc2, 0xff);
    m_accum   = _mm256_add_ps(m_dotpart, m_accum);    

    pSrc1++;
    pSrc2++;
  }

  return sum_256(m_accum)/4;  // always off by a factor of four and can't intrinsic it
}
