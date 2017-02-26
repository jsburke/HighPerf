/*************************************************************************/
// gcc -pthread -o test_param2 test_param2.c

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <time.h>

#define NUM_THREADS 10
#define BUSY 100
#define USE_PTHREAD_EXIT

int array_index = 0;

//above define forces main to wait for all threads to complete because of pthread_exit
//commenting it out will sometimes allow for a few to go, but not all of them

/********************/
void *work(void *i)
{
  long int j, k;
  int f = *((int*)(i));  // get the value being pointed to
  int *g = (int*)(i);    // get the pointer itself
  int* mod_arr_ptr = (int*)i;
  int my_index = array_index++;

  long unsigned int wait = BUSY*BUSY*BUSY;

  for (j; j < wait; j++) k += j;  // busy work
  //sleep(1);   //not the right way to stop threads from getting created
              //should probably manipulate join??  tired
  f += 2;  // since this value exists in main's stack and we modify here
  g += 3;  // this modification changes all threads
           // f quickly becomes 12 for all since main.i is 10 at the end of all issues
           // g is equal across all threads since they all refer to the same address plus an offset

  //printf("\nHello World from %lu with value %d\n", pthread_self(), f);
  //printf("Hello World! %d  %d\n",  f, *g); // part 9 related
  printf("\nMy index is %d", my_index);
  mod_arr_ptr[my_index] = my_index * array_index;

  pthread_exit(NULL);
}

/*************************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t id[NUM_THREADS];

  int mod_array [NUM_THREADS] ={0};  // initialize to zero


  printf("\nArray to be modified is: ");
  for (i = 0; i < NUM_THREADS; i++)
  {
    if (i != 0) printf(", ");
    printf("%d", mod_array[i]);
  }

  for (i = 0; i < NUM_THREADS; ++i) {
    if (pthread_create(&id[i], NULL, work, (void *)(&mod_array))) {
      printf("ERROR creating the thread\n");
      exit(19);
    }
  }

  //for (j; j < BUSY; j++) k += j;  // busy work

  printf("\nAfter creating the thread.  My id is %lu, i = %d\n",
	 pthread_self(), i);

  for (i = 0; i < NUM_THREADS; i++) // joins to ensure each thread mods the array
                                    // breaks part 9, so comment out for part 9 results
  {
    if (pthread_join(id[i],NULL)) exit(19);
  }

  printf("\nArray after modification: ");
  for (i = 0; i < NUM_THREADS; i++)
  {
    if (i != 0) printf(", ");
    printf("%d", mod_array[i]);
  }
  printf("\n");
  #ifdef USE_PTHREAD_EXIT
    pthread_exit(NULL);
  #endif

  return(0);

}/* end main */


