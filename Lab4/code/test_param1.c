/*************************************************************************/
// gcc -pthread -o test_param test_param1.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

//NB task 8
//
//only thing that noticeably changes from runs is different print orders
//creation prints always in numerical order
// task 7 - hello pairs always in set print order
// threads print statements can interleave to some degree
// good bye can come before final thread prints
//
// locked in by pthread_exit???

/************************************************************************/
void *PrintHello(void *threadid)
{
  printf(" Task 7: value before casting %p\n", threadid);  //all numbers are the same, but printed in hex

  long unsigned int tid = (long unsigned int) threadid;
  
  printf(" Hello World!  It's me, thread # %lx ! \n", tid);

  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t threads[NUM_THREADS];
  int rc;
  long unsigned int t;
  char no_bueno = -5;   // when we pass this where t is, compile fails
                        // reason being is a char/int is a different size from a pointer

  printf("\n Hello World!  It's me, MAIN!\n");

  for (t = 0; t < NUM_THREADS; t++) {
    printf("In main:  creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, (void*) t);
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  printf("\n It's me MAIN -- Good Bye World!\n");

  pthread_exit(NULL);

}/* end main */

