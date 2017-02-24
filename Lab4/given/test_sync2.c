/*************************************************************************/
// gcc -pthread -o test_sync2 test_sync2.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 7

struct thread_data{
  int thread_id;
  int sum;
  char *message;
};

pthread_mutex_t mutexA[NUM_THREADS+1];

/********************/
void *PrintHello(void *threadarg)
{
  long taskid, sum;
  struct thread_data *my_data;
  char *message;

  my_data = (struct thread_data *) threadarg;
  taskid = my_data->thread_id;
  sum = my_data->sum;
  message = my_data->message;

  //printf("\n Hello World!  It's me, thread #%ld! sum = %ld message = %s",
  // taskid, sum, message);

  switch (taskid) {
  case 2:
    printf("\nIt's me, thread #%ld! I'm waiting for 1 ...", taskid);
    pthread_mutex_lock(&mutexA[1]);
    pthread_mutex_unlock(&mutexA[1]);
    break;
  case 3:
    printf("\nIt's me, thread #%ld! I'm waiting for 1 ...", taskid);
    pthread_mutex_lock(&mutexA[1]);   
    pthread_mutex_unlock(&mutexA[1]);
    break;
  case 4:
    printf("\nIt's me, thread #%ld! I'm waiting for 2 ...", taskid);
    pthread_mutex_lock(&mutexA[2]);   
    pthread_mutex_unlock(&mutexA[2]);
    break;
  case 5:
    printf("\nIt's me, thread #%ld! I'm waiting for 4 ...", taskid);
    pthread_mutex_lock(&mutexA[4]);
    pthread_mutex_unlock(&mutexA[4]);
    break;
  case 6:
    printf("\nIt's me, thread #%ld! I'm waiting 3 and 4 ...", taskid);
    pthread_mutex_lock(&mutexA[3]);
    pthread_mutex_unlock(&mutexA[3]);
    pthread_mutex_lock(&mutexA[4]);
    pthread_mutex_unlock(&mutexA[4]);
    break;
  case 7:
    printf("\nIt's me, thread #%ld! I'm waiting 5 and 6 ...", taskid);
    pthread_mutex_lock(&mutexA[5]);
    pthread_mutex_unlock(&mutexA[5]);
    pthread_mutex_lock(&mutexA[6]);
    pthread_mutex_unlock(&mutexA[6]);
    break;
  default:
    printf("\nIt's me, thread #%ld! I'm waiting ...", taskid);
    break;
  }
  printf("\nIt's me, thread #%ld! I'm unlocking %ld...", taskid, taskid);

  if (pthread_mutex_unlock(&mutexA[taskid])) printf("\nERROR on unlock\n");

  //  printf("\nIt's me, thread #%ld! I'm done ...\n", taskid);

  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t threads[NUM_THREADS+1];
  struct thread_data thread_data_array[NUM_THREADS+1];
  int rc;
  long t;
  char *Messages[NUM_THREADS+1] = {"Zeroth Message",
				 "First Message",
				 "Second Message",
				 "Third Message",
				 "Fourth Message",
				 "Fifth Message",
				 "Sixth Message",
				 "Seventh Message"};
  char dummy[1];

  for (i = 0; i <= NUM_THREADS; i++) {
    if (pthread_mutex_init(&mutexA[i], NULL)) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
    }
  }

  for (i = 0; i <= NUM_THREADS; i++) {
    if (pthread_mutex_lock(&mutexA[i])) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
    }
  }

  printf("\n Hello World!  It's me, MAIN!\n");

  for (t = 2; t <= NUM_THREADS; t++) {

    thread_data_array[t].thread_id = t;
    thread_data_array[t].sum = t+28;
    thread_data_array[t].message = Messages[t];
    //    printf("In main:  creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello,
			(void*) &thread_data_array[t]);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  sleep(1);
  printf("\n Created threads 2-7, type any character and <enter>\n");

  scanf("%s", dummy);

  printf("\nWaiting for thread 7 to finish, UNLOCK LOCK 1\n");
  
  if (pthread_mutex_unlock(&mutexA[1])) printf("\nERROR on unlock\n");

  printf("\n Done unlocking 1 \n");
    
  pthread_mutex_lock(&mutexA[7]);

  for (t = 2; t <= NUM_THREADS; t++) {
    if (pthread_join(threads[t],NULL)){
      printf("\n ERROR on join\n");
      exit(19);
    }
  }
  printf("\nAfter joining");

  printf("\nGOODBYE WORLD! \n");
  return(0);

}/* end main */

