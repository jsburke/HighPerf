/*****************************************************************************/
// gcc -O1 test_mmm_block.c -lrt -o tmb

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define CPS 2.9e9           // Cycles per second -- Adjust to your computer

#define MAX_SUBTEST_TIME 60.0

#define BASE  0
#define ITERS 20
#define DELTA 113

#define BSZS 6
#define INIT_BSIZE 2
#define BSIZE_MUL 2

#define IDENT 0

typedef double data_t;

/* Create abstract data type for matrix */
typedef struct {
  long int len;
  data_t *data;
} matrix_rec, *matrix_ptr;

double ts_interval(struct timespec ts);
void mmm_bijk(matrix_ptr a, matrix_ptr b, matrix_ptr c, long int bsize);

/*****************************************************************************/
int main(int argc, char *argv[])
{
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  double time_stamp[BSZS+1][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);
  matrix_ptr new_matrix(long int len);
  int set_matrix_length(matrix_ptr m, long int index);
  long int get_matrix_length(matrix_ptr m);
  int init_matrix(matrix_ptr m, long int len);
  int zero_matrix(matrix_ptr m, long int len);
  void mmm_bijk(matrix_ptr a, matrix_ptr b, matrix_ptr c, long int bsize);

  long int i, j, k;
  long int time_sec, time_ns;
  double latest_time;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;
  long int bsize, bsz_index;
  int max_bsize;

  printf("test_mmm_block\n");

  // declare and initialize the matrix structure
  matrix_ptr a0 = new_matrix(MAXSIZE);
  init_matrix(a0, MAXSIZE);
  matrix_ptr b0 = new_matrix(MAXSIZE);
  init_matrix(b0, MAXSIZE);
  matrix_ptr c0 = new_matrix(MAXSIZE);
  zero_matrix(c0, MAXSIZE);

  for(i=0; i<=BSZS; i++) {
    for(j=0; j<=ITERS; j++) {
      time_stamp[i][j] = 0;
    }
  }

  for (bsize = INIT_BSIZE, bsz_index = 0; 
       bsz_index < BSZS;
       bsize *= BSIZE_MUL, bsz_index++)
  {
    printf("bsize = %ld\n", bsize);
    max_bsize = bsize; /* For generating output filename */
    latest_time = 0;
    for (i = 0; (i < ITERS) && (latest_time < MAX_SUBTEST_TIME); i++) {
      set_matrix_length(a0,BASE+(i+1)*DELTA);
      set_matrix_length(b0,BASE+(i+1)*DELTA);
      set_matrix_length(c0,BASE+(i+1)*DELTA);
      printf("bsize = %ld, iter = %ld, Mx size = %ld",
                                             bsize, i, BASE+(i+1)*DELTA);
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
      mmm_bijk(a0, b0, c0, bsize);
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
      latest_time = ts_interval(diff(time1,time2));
      time_stamp[bsz_index][i] = latest_time;
      printf("; time: %5.3f\n", latest_time);
    }
  }

  char filename [255] = {0};
  FILE *fp_cyc;
  FILE *fp_cpe;
  double elem_count;
  sprintf(filename, "mmmBlockCyc_MB%d.csv", max_bsize);
  fp_cyc = fopen(filename,"w");
  sprintf(filename, "mmmBlockCPE_MB%d.csv", max_bsize);
  fp_cpe = fopen(filename,"w");
  printf("Writing collected stats to: %s\n", filename);
  printf("length,      bsize 2,      bsize 4, ...\n");
  for (i = 0; i < ITERS; i++) {
    printf("%5ld", BASE+(i+1)*DELTA);
    fprintf(fp_cyc,"%ld", BASE+(i+1)*DELTA);
    fprintf(fp_cpe,"%ld", BASE+(i+1)*DELTA);
    elem_count = ((double)(BASE+(i+1)*DELTA));
    elem_count = elem_count*elem_count*elem_count;
    for (bsize = INIT_BSIZE, bsz_index = 0; 
         bsz_index < BSZS;
         bsize *= BSIZE_MUL, bsz_index++)
    {
      printf(", %12ld", (long)(CPS * time_stamp[bsz_index][i]));
      fprintf(fp_cyc, ", %ld", (long)(CPS * time_stamp[bsz_index][i]));
      fprintf(fp_cpe, ", %g", CPS * time_stamp[bsz_index][i] / elem_count);
    }
    fprintf(fp_cyc,"\n");
    fprintf(fp_cpe,"\n");
    printf("\n");
  }
  fclose(fp_cyc);
  fclose(fp_cpe);

}/* end main */

/**********************************************/

/* Create matrix of specified length */
matrix_ptr new_matrix(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  matrix_ptr result = (matrix_ptr) malloc(sizeof(matrix_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
      free((void *) result);
      printf("\n COULDN'T ALLOCATE STORAGE (%ld bytes wanted)\n", result->len);
      return NULL;  /* Couldn't allocate storage */
    }
    result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Set length of matrix */
int set_matrix_length(matrix_ptr m, long int index)
{
  m->len = index;
  return 1;
}

/* Return length of matrix */
long int get_matrix_length(matrix_ptr m)
{
  return m->len;
}

/* initialize matrix */
int init_matrix(matrix_ptr m, long int len)
{
  long int i;

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

/* initialize matrix */
int zero_matrix(matrix_ptr m, long int len)
{
  long int i,j;

  if (len > 0) {
    m->len = len;
    for (i = 0; i < len*len; i++)
      m->data[i] = (data_t)(IDENT);
    return 1;
  }
  else return 0;
}

data_t *get_matrix_start(matrix_ptr m)
{
  return m->data;
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

double ts_interval(struct timespec ts)
{
  return ((double) ts.tv_sec) + ((double) ts.tv_nsec)/1.0e9;
}

/*************************************************/

/* mmm */
void mmm_bijk(matrix_ptr a, matrix_ptr b, matrix_ptr c, long int bsize)
{
  long int i, j, k, kk, jj;
  long int get_matrix_length(matrix_ptr m);
  data_t *get_matrix_start(matrix_ptr m);
  long int length = get_matrix_length(a);
  data_t *a0 = get_matrix_start(a);
  data_t *b0 = get_matrix_start(b);
  data_t *c0 = get_matrix_start(c);
  data_t sum;
  long int en = bsize * (length-bsize);

  for(kk = 0; kk < en; kk += bsize) {
    for(jj = 0; jj < en; jj += bsize) {
      for (i = 0; i < length; i++) {
        for (j = jj; (j < jj + bsize) && (j < length); j++) {
          sum = IDENT;
          for (k = kk; (k < kk + bsize) && (k < length); k++) {
            sum += a0[i*length+k] * b0[k*length+j];
	  }
          c0[i*length+j] += sum;
        }
      }
    }
  }
}
