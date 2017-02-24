/*************************************************************************/
// gcc -pthread -o test_sync test_sync.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 1    // NEED TO ADJUST Messages MANUALLY

struct thread_data{
  int thread_id;
  int sum;
  char *message;
};

pthread_mutex_t mutexA;   // declare a global mutex

/*************************************************************************/
void *PrintHello(void *threadarg)
{
  long taskid, sum;
  struct thread_data *my_data;
  char *message;

  my_data = (struct thread_data *) threadarg;
  taskid = my_data->thread_id;
  sum = my_data->sum;
  message = my_data->message;

  while (pthread_mutex_trylock(&mutexA));  // wait until released
  
  printf("\n   I, thread #%lu (sum = %d message = %s) am now unblocked!\n",
	 taskid, sum, message);

  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t threads[NUM_THREADS];
  struct thread_data thread_data_array[NUM_THREADS];
  int rc;
  long t;
  char *Messages[NUM_THREADS] = {"First Message" /*,     //Adjust manually
				 "Second Message",
				 "Third Message",
				 "Fourth Message",
				 "Fifth Message" */ };
  char dummy[10];

  if (pthread_mutex_init(&mutexA, NULL)) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  printf("\n Hello World!  It's me, MAIN!");

  for (t = 0; t < NUM_THREADS; t++) {
    if (pthread_mutex_lock(&mutexA)) printf("\nERROR on lock\n");
    thread_data_array[t].thread_id = t;
    thread_data_array[t].sum = t+28;
    thread_data_array[t].message = Messages[t];
    rc = pthread_create(&threads[t], NULL, PrintHello,
			(void*) &thread_data_array[t]);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  printf("\nIn main: created thread %ld, which is blocked -- press a char and enter\n", t);
  scanf("%s", dummy);
  if (pthread_mutex_unlock(&mutexA)) printf("\nERROR on unlock\n");  // unlock thread

  for (t = 0; t < NUM_THREADS; t++) {
    if (pthread_join(threads[t],NULL)){
      printf("\n ERROR on join\n");
      scanf("%s", dummy);
      exit(19);
    }
  }
  printf("\nAfter joining\n");

  printf("\nGOODBYE WORLD! \n");
  return(0);

}/* end main */

