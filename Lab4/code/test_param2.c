/*************************************************************************/
// gcc -pthread -o test_param2 test_param2.c

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <time.h>

#define NUM_THREADS 10

/********************/
void *work(void *i)
{
  long int j, k;
  int f = *((int*)(i));  // get the value being pointed to
  int *g = (int*)(i);    // get the pointer itself

  for (j; j < 10000000; j++) k += j;  // busy work
  //sleep(3); //not the right way to stop threads from getting created
              //should probably manipulate join??  tired
  f += 2;  // since this value exists in main's stack and we modify here
  g += 3;  // this modification changes all threads
           // f quickly becomes 12 for all since main.i is 10 at the end of all issues
           // g is equal across all threads since they all refer to the same address plus an offset

  // printf("\nHello World from %lu with value %d\n", pthread_self(), f);
  printf("\nHello World! %d  %d",  f, *g);

  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t id[NUM_THREADS];

  for (i = 0; i < NUM_THREADS; ++i) {
    if (pthread_create(&id[i], NULL, work, (void *)(&i))) {
      printf("ERROR creating the thread\n");
      exit(19);
    }
  }

  for (j; j < 100000000; j++) k += j;  // busy work

  printf("\nAfter creating the thread.  My id is %lu, i = %d\n",
	 pthread_self(), i);

  return(0);

}/* end main */


