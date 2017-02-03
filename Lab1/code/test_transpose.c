#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define GIG 1000000000
#define CPG 2.9

#define OPTIONS 1
#define IDENT 0

#define FILE_PREFIX ((const unsigned char*) "doubleTranspose_")

typedef double data_t;
typedef long int lint;

typedef struct{
	lint    len;
	data_t *data;
}matrix, *matrix_ptr;

matrix_ptr matirx_new(lint len);
int matrix_set_length(matrix_ptr src, lint index);
lint matrix_get_length(matrix_ptr src);
int matrix_init(matrix_ptr src, lint len);
int matrix_zero(matrix_ptr src, lint len);
data_t* matrix_get_start(matrix_ptr src);
void matrix_transpose(matrix_ptr dst, matrix_ptr src, lint len);

struct timespec diff(struct timespec start, struct timespec end);



main(int argc, char* argv[])
{}


//-----------------------------------------------------------------------
//
// Begin generic matrix functions
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
    printf("\n COULDN'T ALLOCATE STORAGE \n", result->len);
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