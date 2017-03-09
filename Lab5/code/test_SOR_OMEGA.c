/*****************************************************************************/
// gcc -O1 -o test_SOR_OMEGA test_SOR_OMEGA.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
#define CPG 2.0           // Cycles per GHz -- Adjust to your computer

//  #define BASE  2
#define ITERS 1
//  #define DELTA 8        // Right now only set up to do one size

#define MINVAL   0.0
#define MAXVAL  100.0

#define TOL 0.00001

#define O_ITERS 150        // # of OMEGA values to be tested
//  #define PER_O_ITERS 10    // trials per OMEGA value
double OMEGA;     // OMEGA base - first OMEGA tested
#define OMEGA_INC 0.01   // OMEGA increment for each O_ITERS

#define FILE_PREFIX ((const unsigned char*) "SOR_OMEGA_")

typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

vec_ptr new_vec(long int len);
int set_vec_length(vec_ptr v, long int index);
long int get_vec_length(vec_ptr v);
int init_vector(vec_ptr v, long int len);
int init_vector_rand(vec_ptr v, long int len);
int print_vector(vec_ptr v);

void SOR(vec_ptr v, int *iterations);
void SOR_ji(vec_ptr v, int *iterations);
void SOR_blocked(vec_ptr v, int *iterations);

/*****************************************************************************/

int main(int argc, char *argv[])
{
  int BASE, DELTA, PER_O_ITERS;

  if(argc != 5)
  {
    printf("must have four command line parameters!\n");
    return 0;
  }

  BASE  = strtol(argv[1], NULL, 10);
  DELTA = strtol(argv[2], NULL, 10);
  OMEGA  = strtod(argv[3], NULL);
  PER_O_ITERS = strtol(argv[4], NULL, 10);

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

  if(PER_O_ITERS == 0)
  {
    printf("PER_O_ITERS must be at least one\n");
    return 0;
  }    

  double convergence[O_ITERS][2];  
  int *iterations;

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS)*DELTA;

  char accum[255] = {0};
  char conv [255] = {0};
  FILE *fp;  //  pardon the fact that this gets reused sequentially please

  sprintf(accum, "%s_ACCUM_B%d_D%d_O%lf_PO%d.csv", FILE_PREFIX, BASE, DELTA, OMEGA, PER_O_ITERS);
  sprintf(conv,   "%s_CONV_B%d_D%d_O%lf_PO%d.csv", FILE_PREFIX, BASE, DELTA, OMEGA, PER_O_ITERS);
  printf("Current File: %s\n", accum);

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  iterations = (int *) malloc(sizeof(int));

  //////////////////////////////////////////////////
  //
  // Begin testing
  //
  //////////////////////////////////////////////////

  fp = fopen(accum,"w");
  //might want header line here

  for (i = 0; i < O_ITERS; i++) {
    fprintf(fp, "\n%0.2f", OMEGA);
    double acc = 0.0;
    for (j = 0; j < PER_O_ITERS; j++) {
      set_vec_length(v0, MAXSIZE);
      init_vector_rand(v0, MAXSIZE);
      SOR(v0,iterations);
      acc += (double)(*iterations);
      fprintf(fp,", %d", *iterations);
    }
    convergence[i][0] = OMEGA;
    convergence[i][1] = acc/(double)(PER_O_ITERS);
    OMEGA += OMEGA_INC;
  }

  fclose(fp);

  fp = fopen(conv,"w");

  for (i = 0; i < O_ITERS; i++)
    fprintf(fp,"\n%0.2f %0.1f", convergence[i][0], convergence[i][1]);

  fclose(fp);

  return 0;  
}/* end main */
/*********************************/

/* Create 2D vector of specified length per dimension */
vec_ptr new_vec(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
      free((void *) result);
      printf("\n COULDN'T ALLOCATE STORAGE \n");
      return NULL;  /* Couldn't allocate storage */
    }
    result->data = data;
  }
  else result->data = NULL;
  
  return result;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
  v->len = index;
  return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
  return v->len;
}

/* initialize 2D vector */
int init_vector(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++) v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

/* initialize vector with random number in a range */
int init_vector_rand(vec_ptr v, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++)
      v->data[i] = (data_t)(fRand((double)(MINVAL),(double)(MAXVAL)));
    return 1;
  }
  else return 0;
}

/* print vector for test */
int print_vector(vec_ptr v)
{
  long int i, j, len;

  len = v->len;
  printf("\n length = %ld", len);
  for (i = 0; i < len; i++) {
    printf("\n");
    for (j = 0; j < len; j++)
      printf("%.4f ", (data_t)(v->data[i*len+j]));
  }
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/************************************/

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/************************************/

/* SOR */
void SOR(vec_ptr v, int *iterations)
{
  long int i, j;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;   // start w/ something big
  int iters = 0;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (i = 1; i < length-1; i++) 
      for (j = 1; j < length-1; j++) {
	change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					  data[(i+1)*length+j] +
					  data[i*length+j+1] +
					  data[i*length+j-1]);
	data[i*length+j] -= change * OMEGA;
	if (change < 0){
	  change = -change;
	}
	mean_change += change;
      }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
   *iterations = iters;
}
