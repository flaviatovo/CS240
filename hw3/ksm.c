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
#include "proc.h"
#include "spinlock.h"

#define KSM_NAME_SIZE 10
#define KSM_NUMBER 10
#define KSM_MAX_NUMBER_PAGES 10

struct ksm_object {
  char name[KSM_NAME_SIZE]; // The name of the shared memory
  int handle;
  
  uint ksmsz;       // The size of the shared memory
  int cpid;         // PID of the creator
  int mpid;         // PID of last modifier
  uint attached_nr; // Number of attached processes
  uint atime;       // Last attach time
  uint dtime;       // Last detach time
  
  uint pages_number;
  char* pages [KSM_MAX_NUMBER_PAGES]; // Array containing the pages of the shared memory
  int marked_for_deletion;  // Define the state of the shared memory
};

struct {
  struct spinlock lock;
  
  struct ksm_object ksms[KSM_NUMBER];
  
  uint total_shrg_nr; // Total number of existing shared regions
  uint total_shpg_nr; // Total number of existing shared pages
  int next_handle;
} ksmtable;

void ksminit(void){
  int i;
  initlock(&ksmtable.lock, "ksmtable");
  
  for (i = 0; i < KSM_NUMBER; i++) {
    ksmtable.ksms[i].handle = 0;
    ksmtable.ksms[i].marked_for_deletion = 0;
    ksmtable.ksms[i].name[0] = '\0';
  }
  
  ksmtable.total_shrg_nr = 0;
  ksmtable.total_shpg_nr = 0;

  ksmtable.next_handle = 1;
}

int ksmget(char * name, uint size){
  cprintf("ksmdelete called with name=%s size=%d\n", name, size);
  int i;
  uint allocated_size = 0;
  
  acquire(&ksmtable.lock);
  
  // Looking to see if the shared memory already exists
  for(i = 0; i < KSM_NUMBER; i++){
    if (strncmp(ksmtable.ksms[i].name, name, KSM_NAME_SIZE) == 0){
      // Shared Memory found
      release(&ksmtable.lock);
      return ksmtable.ksms[i].handle;
    }
  }
  
  // No Shared memory with that name, creating a new one
  // Finding free slot
  for(i = 0; i < KSM_NUMBER; i++){
    if (ksmtable.ksms[i].handle == 0){
      // Empty slot found
      ksmtable.ksms[i].handle = ksmtable.next_handle;
      strncpy(ksmtable.ksms[i].name,name, KSM_NAME_SIZE);
      
      ksmtable.ksms[i].cpid = proc->pid;
      ksmtable.ksms[i].pages_number = 0;
      
      // Puting pages on array, do not attach them
      while((allocated_size < size) && (ksmtable.ksms[i].pages_number < KSM_MAX_NUMBER_PAGES)){
        if ((ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number] = kalloc()) == 0){
          cprintf("allocuvm out of memory, allocated %d pages\n", ksmtable.ksms[i].pages_number);
          break;
        }
      
        ksmtable.ksms[i].pages_number ++;
        allocated_size += PGSIZE;
      }
      ksmtable.ksms[i].ksmsz = allocated_size ;
      
      ksmtable.next_handle ++;
      
      ksmtable.total_shrg_nr ++;
      ksmtable.total_shpg_nr += ksmtable.ksms[i].pages_number;
      
      release(&ksmtable.lock);
      return ksmtable.ksms[i].handle;
    }
  }
  
  release(&ksmtable.lock);
  return 0;
}

int ksminfo(int handle, struct ksminfo_t* info){
  //TODO
  cprintf("ksminfo called with handle=%d\n", handle);
  return 0;
}

int ksmattach(int handle, int flags){
  //TODO
  cprintf("ksmattach called with handle=%d and flags=%d\n", handle, flags);
  return 0;
}

int ksmreleasepages(){
  ksmtable.total_shpg_nr -= ksmtable.ksms[i].pages_number;
  for (;ksmtable.ksms[i].pages_number > 0; ksmtable.ksms[i].pages_number--){
    kfree(ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number -1]);
	ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number -1] = 0;
  }
  return 0;
}

int ksmdetach(int handle){
  //TODO
  cprintf("ksmdetach called with handle=%d\n", handle);
  return 0;
}

int ksmdelete(int handle){
  cprintf("ksmdelete called with handle=%d\n", handle);
  // Looking to see if the shared memory already exists
  for(i = 0; i < KSM_NUMBER; i++){
    if (ksmtable.ksms[i].handle == handle){
      if (ksmtable.ksms[i].attached_nr > 0){
	    ksmtable.ksms[i].marked_for_deletion = 1;
        release(&ksmtable.lock);
        return 0;
      }
	  
	  ksmreleasepages();
	  ksmtable.ksms[i].handle = 0;
	  ksmtable.ksms[i].marked_for_deletion = 0;
      ksmtable.ksms[i].name[0] = '\0';
	
	  ksmtable.total_shrg_nr --;
	  
      release(&ksmtable.lock);
      return ksmtable.ksms[i].handle;
    }
  }
  
  release(&ksmtable.lock);
  cprintf("ksmdelete: shared memory with handle=%d does not exist\n", handle);
  return 0;
}
