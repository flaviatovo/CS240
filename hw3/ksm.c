//
// File created by Flavia Tovo as work for project 3
// of OS class on KAUST
// Fall 2012
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "ksm.h"
#include "spinlock.h"

#define KSM_NAME_SIZE 10
#define KSM_NUMBER 10
#define KSM_MAX_NUMBER_PAGES 10

struct ksm_object {
  char name[KSM_NAME_SIZE]; // The name of the shared memory
  
  uint ksmsz;       // The size of the shared memory
  int cpid;         // PID of the creator
  int mpid;         // PID of last modifier
  uint attached_nr; // Number of attached processes
  uint atime;       // Last attack time
  uint dtime;       // Last detach time
  
  uint pages [KSM_MAX_NUMBER_PAGES]; // Array containing the pages of the shared memory
  int marked_for_deletion;  // Define the state of the shared memory
};

struct ksmtable {
  struct spinlock lock;
  
  struct ksm_object ksms[KSM_NUMBER];
  int ksm_handlers [KSM_NUMBER];
  
  uint total_shrg_nr; // Total number of existing shared regions
  uint total_shpg_nr; // Total number of existing shared pages
};

void ksminit(void){

}
int ksmget(char *, uint){
  //TODO
  return 0;
}
int ksmattach(int, int){
  //TODO
  return 0;
}
int ksmdetack(int){
  //TODO
  return 0;
}
int ksminfo(int, struct ksminfo_t*){
  //TODO
  return 0;
}
int ksmdelete(int){
  //TODO
  return 0;
}
