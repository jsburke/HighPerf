/*************************************************************************/
// gcc -pthread -o test_join test_join.c

// Simple thread create and exit test

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

/*************************************************************************/
void *work(void *i)
{
  printf(" Hello World!  It's me, thread #%lu --\n",
	 pthread_self());
  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t id[NUM_THREADS];

  printf("\n Hello World!  It's me, MAIN!\n");

  for (i = 0; i < NUM_THREADS; i++) {
    if (pthread_create(&id[i], NULL, work, NULL)) {
      exit(19);
    }
  }

  printf("\nmain() -- After creating the thread.  My id is: %lu\n",
	 pthread_self());

  for (i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(id[i],NULL)){ exit(19);
    }
  }

  printf("\nAfter joining, Good Bye World!\n");

  return(0);
}/* end main */

