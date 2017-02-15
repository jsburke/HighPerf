/**************************************************************/
// gcc -msse4.1 -O1 -o test_intrinsics test_intrinsics.c -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#define FALSE 0
#define TRUE 1

#define GIG 1000000000
#define CPG 2.0           // Cycles per GHz -- Adjust to your computer

#define ARRAY_SIZE 100000000 // 100,000,000

#define SIZE 10000000
#define ITERS 30
#define DELTA 32
#define BASE 0

#define OPTIONS 2
#define IDENT 1.0
#define OP *

typedef float data_t;

/**************************************************************/
main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);

  int i,j,k;   	   /* Local variables. */
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  double* var;
  int     ok;
  data_t*  pArray1;
  data_t*  pArray2;
  data_t*  pResult;
  long int nSize;
  void  InitArray(data_t* pA, long int nSize);
  void  InitArray_rand(data_t* pA, long int nSize);
  void  ZeroArray(data_t* pA, long int nSize);
  void  ArrayTest1(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
  void  ArrayTest2(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
 
  printf("\nHello World!  SSE Test");

  ok = posix_memalign((void**)&pArray1, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pArray2, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pResult, 64, ARRAY_SIZE*sizeof(data_t));

  // initialize pArray1, pArray2
  InitArray_rand(pArray1,MAXSIZE);
  InitArray_rand(pArray2,MAXSIZE);
  ZeroArray(pResult,MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest1(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest2(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  /* output times */
  for (i = 0; i < ITERS; i++) {
    printf("\n%d, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }

  printf("\n Goodbye World!\n");

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
  __m256   m0_5 = _mm256_set_ps1(0.5f);

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
