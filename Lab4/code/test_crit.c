/*************************************************************************/
// gcc -pthread -o test_crit test_crit.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 100

pthread_mutex_t mt;

struct thread_data{
  int thread_id;
  int *balance;
};

// struct timespec t1;
// t1.tv_sec  = 0;      // would use sleep() ...
// t1.tv_nsec = 50000;
/********************/
void *PrintHello(void *threadarg)
{
  long int taskid;
  struct thread_data *my_data;
  int *balance, save;
  int slp;

  my_data = (struct thread_data *) threadarg;
  taskid = my_data->thread_id;
  balance = my_data->balance;

  //sleep(3);

  struct timespec t1;
  t1.tv_sec  = 0;      // would use sleep() ...
  t1.tv_nsec = 10000;

  pthread_mutex_lock(&mt);
  if (taskid % 2 == 0) *balance += 1;
  else {slp = nanosleep(&t1,NULL); *balance -= 1;}
  pthread_mutex_unlock(&mt);

  printf(" It's me, thread #%ld! balance = %d\n", taskid, *balance);

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
  int account = 1000;

  pthread_mutex_init(&mt,NULL);
  printf("\n Hello World!  It's me, MAIN!\n");

  for (t = 0; t < NUM_THREADS; t++) {
    thread_data_array[t].thread_id = t+1;
    thread_data_array[t].balance = &account;
    //    printf("In main:  creating thread %ld\n", t+1);
    rc = pthread_create(&threads[t], NULL, PrintHello,
			(void*) &thread_data_array[t]);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(threads[i],NULL)){ exit(19);
    }
  }

  pthread_mutex_destroy(&mt);
  printf("\n MAIN --> balance = %d\n", account); 

  pthread_exit(NULL);

}/* end main */

