#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9

#define OPTIONS 2
#define IDENT 0

#define FILE_PREFIX ((const unsigned char*) "doubleTranspose_")

typedef double data_t;
typedef long int lint;

typedef struct{
	lint    len;
	data_t *data;
}matrix_rec, *matrix_ptr;

matrix_ptr matirx_new(lint len);
int matrix_set_length(matrix_ptr src, lint index);
lint matrix_get_length(matrix_ptr src);
int matrix_init(matrix_ptr src, lint len);
int matrix_zero(matrix_ptr src, lint len);
data_t* matrix_get_start(matrix_ptr src);
void matrix_transpose_linsrc(matrix_ptr dst, matrix_ptr src);
void matrix_transpose_lindst(matrix_ptr dst, matrix_ptr src);

int clock_gettime(clockid_t clk_id, struct timespec *tp);
struct timespec diff(struct timespec start, struct timespec end);



main(int argc, char* argv[])
{
  int BASE, DELTA, ITERS;

  if(argc != 4)
  {
    printf("num args wrong\n");
    return 0;
  }

  BASE  = strtol(argv[1],NULL,10);
  DELTA = strtol(argv[2],NULL,10);
  ITERS = strtol(argv[3],NULL,10);

  int OPTION;

  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];

  lint i, j, k;
  lint CURR_SIZE;
  lint time_sec, time_ns;
  lint MAXSIZE = BASE+(ITERS+1)*DELTA;

  char filename[255] = {0};
  FILE *fp;

  sprintf(filename, "%sB%d_I%d_D%d.csv", FILE_PREFIX, BASE, ITERS, DELTA);
  printf("Current file: %s\n", filename);

  matrix_ptr src = matirx_new(MAXSIZE);
  matrix_init(src, MAXSIZE);
  matrix_ptr dst = matirx_new(MAXSIZE);
  matrix_zero(dst, MAXSIZE);

  OPTION = 0;
  for(i = 0; i < ITERS; i++)
  {
    CURR_SIZE = BASE+(i+1)*DELTA;
    matrix_set_length(src,CURR_SIZE);
    matrix_set_length(dst,CURR_SIZE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    matrix_transpose_linsrc(dst, src);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for(i = 0; i < ITERS; i++)
  {
    CURR_SIZE = BASE+(i+1)*DELTA;
    matrix_set_length(src,CURR_SIZE);
    matrix_set_length(dst,CURR_SIZE);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    matrix_transpose_lindst(dst, src);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }  

  fp = fopen(filename,"w");
  for(i = 0; i < ITERS; i++)
  {
    fprintf(fp, "\n%ld, ", BASE+(i+1)*DELTA);
    for(j = 0; j < OPTIONS; j++)
    {
      if(j != 0) fprintf(fp, ", ");
      fprintf(fp, "%ld", (long int)((double)(CPG)*(double)
     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }
  fclose(fp);

}


//-----------------------------------------------------------------------
//
// Begin matrix functions
//
//-----------------------------------------------------------------------

matrix_ptr matirx_new(lint len)
{
  long int i;

  /* Allocate and declare header structure */
  matrix_ptr result = (matrix_ptr) malloc(sizeof(matrix_rec));
  if (!result) return NULL;
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
    free((void *) result);
    printf("\n COULDN'T ALLOCATE STORAGE (%ld BYTES)\n", result->len);
    return NULL;  /* Couldn't allocate storage */
  }
  result->data = data;
  }
  else result->data = NULL;

  return result;	
}

int  matrix_set_length(matrix_ptr src, lint index)
{
	src->len = index;
	return 1;
}

lint matrix_get_length(matrix_ptr src)
{
	return src->len;
}

int matrix_init(matrix_ptr src, lint len)
{
	lint i;

	if(len > 0)
	{
		src->len = len;
		for(i = 0; i< len*len; i++) src->data[i] = (data_t)i;
		return 1;
	}
	else return 0;
}

int matrix_zero(matrix_ptr src, lint len)
{
	lint i,j;

	if(len > 0)
	{
		src->len = len;
		for(i = 0; i < len*len; i++) src->data[i] = (data_t)IDENT;
		return 1;
	}
	else return 0;
}

data_t* matrix_get_start(matrix_ptr src)
{
	return src->data;
}

void matrix_transpose_linsrc(matrix_ptr dst, matrix_ptr src)
{
  int i,j;

  lint len = matrix_get_length(src);
  data_t* dst_d = matrix_get_start(dst);
  data_t* src_d = matrix_get_start(src);

  for(i = 0; i < len; i++)
  {
    for(j = 0; j < len; j++)
      dst_d[j*len + i] = src_d[i*len + j];
  }
}

void matrix_transpose_lindst(matrix_ptr dst, matrix_ptr src)
{
  int i,j;

  lint len = matrix_get_length(src);
  data_t* dst_d = matrix_get_start(dst);
  data_t* src_d = matrix_get_start(src);

  for(i = 0; i < len; i++)
  {
    for(j = 0; j < len; j++)
      dst_d[i*len + j] = src_d[j*len + i];
  }
}

//---------------------------------------------------------------------
//
// timespec function
//
//----------------------------------------------------------------------

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