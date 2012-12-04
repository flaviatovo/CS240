#include "types.h"
#include "stat.h"
#include "user.h"
#include "ksm.h"

int
main(int argc, char *argv[])
{
  int pid;
  int ksm1,ksm2, ksm3;
  
  printf(1,"\nUsed pages: %d\n",pgused());

  ksm1 = ksmget("casa", 1024);
  ksm2 = ksmget("casa", 4096);
  ksm3 = ksmget("vida", 5120);
  printf(1,"\nHandlers: ksm1=%d ksm2=%d ksm3=%d\n",ksm1, ksm2, ksm3);
  
  ksmattach(ksm1,KSM_READWRITE);
  ksmdetack(ksm1);
  ksmdelete(ksm1);
  
  // pid = fork();
  
  // if (pid == 0){
    // pid = fork();
    // printf(1,"\nBefore calling wait on %d\n",pid);
    // if (sem_wait(sem) != 0)
       // printf(1,"\nERROR on wait\n");
    // printf(1,"\nAfter calling wait\n");
	// if (pid != 0)
      // wait();
  // } else {
    // printf(1,"\nBefore calling signal 1 on %d\n",pid);
    // if(sem_signal(sem) != 0)
       // printf(1,"\nERROR on signal\n");
    // printf(1,"\nAfter calling signal 1\n");
    // printf(1,"\nBefore calling signal 2 on %d\n",pid);
    // if(sem_signal(sem) != 0)
       // printf(1,"\nERROR on signal\n");
    // printf(1,"\nAfter calling signal 2\n");
    // wait();
    // sem_delete(sem);
  // }
  
  exit();
}
