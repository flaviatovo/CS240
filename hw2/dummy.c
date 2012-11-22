#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  uint i = 0xFFFF;
  uchar * t1,* t2,* t3,* t4,* t5;

  printf(1,"\nUsed pages: %d\n",pgused());

  printf(1,"Malloc of %d \n", i);
  t1 = (uchar *)malloc(i);
  t2 = (uchar *)malloc(i);
  t3 = (uchar *)malloc(i);
  t4 = (uchar *)malloc(i);
  t5 = (uchar *)malloc(i);
  printf(1,"\nUsed pages: %d\n",pgused());
  printf(1,"Free of %d \n", i);
  free(t1);
  free(t2);
  free(t3);
  free(t4);
  free(t5);
  printf(1,"\nUsed pages: %d\n",pgused());

  exit();
}
