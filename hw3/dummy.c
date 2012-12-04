#include "types.h"
#include "stat.h"
#include "user.h"
#include "ksm.h"

int
main(int argc, char *argv[])
{
//  int pid;
  int ksm1,ksm2, ksm3, ksm4;
  
  printf(1,"\nUsed pages=%d before start\n",pgused());

  ksm1 = ksmget("casa", 1024);
  printf(1,"Used pages=%d after get ksm1\n",pgused());
  ksm2 = ksmget("casa", 4096);
  printf(1,"Used pages=%d after get ksm2\n",pgused());
  ksm3 = ksmget("really_big_name_to_see_what", 5120);
  printf(1,"Used pages=%d after get ksm3\n",pgused());
  ksm4 = ksmget("really_big_name_to_", 5120);
  printf(1,"Used pages=%d after get ksm4\n",pgused());
  printf(1,"Handlers: ksm1=%d ksm2=%d ksm3=%d ksm4=%d\n",ksm1, ksm2, ksm3, ksm4);
  
  ksmattach(ksm1,KSM_READWRITE);
  ksmdetach(ksm1);
  ksmdelete(ksm1);
  printf(1,"Used pages=%d after delete ksm1\n",pgused());
  ksmdelete(ksm2);
  printf(1,"Used pages=%d after delete ksm2\n",pgused());
  ksmdelete(ksm3);
  printf(1,"Used pages=%d after delete ksm3\n",pgused());
  
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
