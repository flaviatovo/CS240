#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;
  printf(1,"\nUsed pages: %d\n",pgused());

  pid = fork();
  printf(1,"\nPID: %d\n",pid);

  exit();
}
