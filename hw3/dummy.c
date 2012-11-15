#include "types.h"
#include "stat.h"
#include "user.h"

typedef unsigned long long ticks;

static __inline__ ticks getticks(void)
{
     unsigned a, d;
     asm("cpuid");
     asm volatile("rdtsc" : "=a" (a), "=d" (d));

//     printf(1,"\na: %d, d: %d\n",a,d);

     return (((ticks)a) | (((ticks)d) << 32));
}


int
main(int argc, char *argv[])
{
  uint i = 0xFFFF;
  uchar * t1,* t2,* t3,* t4,* t5;
  ticks tick,tick1;

  tick = getticks();

  printf(1,"Malloc of %d \n", i);
  t1 = (uchar *)malloc(i);
  t2 = (uchar *)malloc(i);
  t3 = (uchar *)malloc(i);
  t4 = (uchar *)malloc(i);
  t5 = (uchar *)malloc(i);
  printf(1,"Free of %d \n", i);
  free(t1);
  free(t2);
  free(t3);
  free(t4);
  free(t5);

  tick1 = getticks();

  printf(1,"\nStart time: %d\n",tick);
  printf(1,"End time: %d\n",tick1);
  printf(1,"Execution time: %d\n",tick1-tick);

  exit();
}
