/*************************************************************************/
// gcc -pthread -o test_barrier test_barrier.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

struct thread_data{
  int thread_id;
  int sum;
  char *message;
};

// Barrier variable
pthread_barrier_t barr;

/********************/
void *PrintHello(void *threadarg)
{
  long taskid, sum, i;
  struct thread_data *my_data;
  char *message;

  my_data = (struct thread_data *) threadarg;
  taskid = my_data->thread_id;
  sum = my_data->sum;
  message = my_data->message;

  //print some thread-specific stuff, then wait for everybody else
  for (i = 0; i < 3; i++) {
    //    sleep(1);
    printf("Thread %ld printing before barrier %ld of 3\n", taskid, i+1);
  }

  //wait at barrier until all NUM_THREAD threads arrive
  int rc = pthread_barrier_wait(&barr);
  if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
    printf("Could not wait on barrier\n");
    exit(-1);
  }

  //now print some more thread-specific stuff
  for (i = 0; i < 2; i++)
    printf("Thread %ld printing after barrier %ld of 2\n", taskid, i+1);

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
  char *Messages[NUM_THREADS] = {"First Message",
				 "Second Message",
				 "Third Message",
				 "Fourth Message",
				 "Fifth Message"};
  char dummy[10];

  printf("\n Hello World!  It's me, MAIN!\n");

  // Barrier initialization -- spawned threads will wait until all arrive
  if(pthread_barrier_init(&barr, NULL, NUM_THREADS)) {
    printf("Could not create a barrier\n");
    return -1;
  } 
 
  for (t = 0; t < NUM_THREADS; t++) {        // create threads
    thread_data_array[t].thread_id = t;
    thread_data_array[t].sum = t+28;
    thread_data_array[t].message = Messages[t];
    printf("In main:  creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello,
			(void*) &thread_data_array[t]);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  printf("\nAfter creating the threads.  My id is:  %lu\n", pthread_self());

  for (t = 0; t < NUM_THREADS; t++) {
    if (pthread_join(threads[t],NULL)){
      printf("\n ERROR on join\n");
      exit(19);
    }
  }
  printf("\nAfter joining");

  printf("\nGOODBYE WORLD! \n");
  return(0);

}/* end main */

