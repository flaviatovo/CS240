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
#define KSM_MAX_ATTACHMENTS 20

struct ksm_object {
  char name[KSM_NAME_SIZE]; // The name of the shared memory
  int ksm_id;
  
  uint ksmsz;       // The size of the shared memory
  int cpid;         // PID of the creator
  int mpid;         // PID of last modifier
  uint attached_nr; // Number of attached processes
  uint atime;       // Last attach time
  uint dtime;       // Last detach time
  
  uint pages_number;
  
  //int attached_pids [KSM_MAX_ATTACHMENTS]; // Array to store the pid of all attached process
  
  char* pages [KSM_MAX_NUMBER_PAGES]; // Array containing the pages of the shared memory
  int marked_for_deletion;  // Define the state of the shared memory
};

struct ksm_link {
  int pid;       // pid + handle + permission is key
  int handle;    // pid + handle + permission is key
  int permission;// pid + handle + permission is key
  int ksm_id;    // used to do the link between ksm_object, pid with permissions
  int address;   // value returned by ksm_attached, address of the first page on va.
};

struct {
  struct spinlock lock;
  
  struct ksm_object ksms[KSM_NUMBER];
  struct ksm_link attachments [KSM_MAX_ATTACHMENTS];
  
  uint total_shrg_nr; // Total number of existing shared regions
  uint total_shpg_nr; // Total number of existing shared pages
  int next_handle;
  int next_ksmid;
} ksmtable;

int ksmattachhelper(struct ksm_link * content);
int ksmdetachhelper(struct ksm_link * content);
int ksmreleasepages(int i);

void ksminit(void){
  int i;
  initlock(&ksmtable.lock, "ksmtable");
  
  for (i = 0; i < KSM_NUMBER; i++) {
    ksmtable.ksms[i].ksm_id = 0;
    ksmtable.ksms[i].marked_for_deletion = 0;
    ksmtable.ksms[i].name[0] = '\0';
  }
  
  for (i = 0; i < KSM_MAX_ATTACHMENTS; i++) {
    ksmtable.attachments[i].pid = 0;
    ksmtable.attachments[i].handle = 0;
	ksmtable.attachments[i].permission = -1;
  }
  
  ksmtable.total_shrg_nr = 0;
  ksmtable.total_shpg_nr = 0;

  ksmtable.next_handle = 1;
  ksmtable.next_ksmid = 1;
}

int ksmget(char * name, uint size){
  cprintf("ksmget called with name=%s size=%d\n", name, size);
  int i, j;
  uint allocated_size = 0;
  
  acquire(&ksmtable.lock);
  
  // Looking to see if the shared memory already exists
  for(i = 0; i < KSM_NUMBER; i++){
    // Shared Memory found
    if (strncmp(ksmtable.ksms[i].name, name, KSM_NAME_SIZE) == 0){
      for (j = 0; j < KSM_MAX_ATTACHMENTS; j ++){
        // Link between ksm_id and pid already exists, returning the same handle
        if ((ksmtable.attachments[i].pid == proc->pid) &&
            (ksmtable.attachments[i].ksm_id == ksmtable.ksms[i].ksm_id)){
          release(&ksmtable.lock);
          return ksmtable.attachments[i].handle;
        }
      }
      
      // Creating link
      // Finding empty slot
      for (j = 0; j < KSM_MAX_ATTACHMENTS; j ++){
        if(ksmtable.attachments[i].pid == 0){
          ksmtable.attachments[i].pid = proc->pid;
          ksmtable.attachments[i].ksm_id = ksmtable.ksms[i].ksm_id;
          ksmtable.attachments[i].handle = ksmtable.next_handle;
          ksmtable.attachments[i].permission = -1;
          
          ksmtable.next_handle ++;
          release(&ksmtable.lock);
          return ksmtable.attachments[i].handle;
        }
      }
      
      release(&ksmtable.lock);
      // Empty link not found
      return 0;
    }
  }
  
  // No Shared memory with that name, creating a new one
  // Finding free slot
  for(i = 0; i < KSM_NUMBER; i++){
    if (ksmtable.ksms[i].ksm_id == 0){
      // Empty slot found
      ksmtable.ksms[i].ksm_id = ksmtable.next_ksmid;
      strncpy(ksmtable.ksms[i].name,name, KSM_NAME_SIZE);
      
      ksmtable.ksms[i].cpid = proc->pid;
      ksmtable.ksms[i].pages_number = 0;
      
      // Puting pages on array, do not attach them
      while((allocated_size < size) && (ksmtable.ksms[i].pages_number < KSM_MAX_NUMBER_PAGES)){
        if ((ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number] = kalloc()) == 0){
          cprintf("allocuvm out of memory, allocated %d pages\n", ksmtable.ksms[i].pages_number);
          break;
        }

        memset(ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number],0,PGSIZE);
        ksmtable.ksms[i].pages_number ++;
        allocated_size += PGSIZE;
      }
      ksmtable.ksms[i].ksmsz = allocated_size;
      
      // initializing others variables
      ksmtable.ksms[i].mpid = ksmtable.ksms[i].cpid;
      ksmtable.ksms[i].attached_nr = 0;
      ksmtable.ksms[i].atime = 0;
      ksmtable.ksms[i].dtime = 0;      
      
      ksmtable.next_ksmid ++;
      ksmtable.total_shrg_nr ++;
      ksmtable.total_shpg_nr += ksmtable.ksms[i].pages_number;
      
      // Creating a new link
      for (j = 0; j < KSM_MAX_ATTACHMENTS; j ++){
        if(ksmtable.attachments[i].pid == 0){
          ksmtable.attachments[i].pid = proc->pid;
          ksmtable.attachments[i].ksm_id = ksmtable.ksms[i].ksm_id;
          ksmtable.attachments[i].handle = ksmtable.next_handle;
          ksmtable.attachments[i].permission = -1;
          
          ksmtable.next_handle ++;
          release(&ksmtable.lock);
          return ksmtable.attachments[i].handle;
        }
      }
      
      // No empty link slot found after creating a new ksm
      release(&ksmtable.lock);
      return 0;
    }
  }
  
  // No empty ksm slot found
  release(&ksmtable.lock);
  return 0;
}

int ksminfo(int handle, struct ksminfo_t* info){
  int i;
  int ksm_id = 0;
  cprintf("ksminfo called with handle=%d\n", handle);
  acquire(&ksmtable.lock);
  
  info->total_shrg_nr = ksmtable.total_shrg_nr;
  info->total_shpg_nr = ksmtable.total_shpg_nr;
  
  if (handle > 0){
    // Finding out what is the ksm_id for that handle
    for(i = 0; i < KSM_MAX_ATTACHMENTS; i++){
      if (ksmtable.attachments[i].handle == handle){
        ksm_id = ksmtable.attachments[i].ksm_id;
        break;
      }
    }
    
    // Finding the good ksm to get info
    if (ksm_id > 0) {
      for(i = 0; i < KSM_NUMBER; i++){
        if(ksmtable.ksms[i].ksm_id == ksm_id) {
          info->ksmsz = ksmtable.ksms[i].ksmsz;
          info->cpid = ksmtable.ksms[i].cpid;
          info->mpid = ksmtable.ksms[i].mpid;
          info->attached_nr = ksmtable.ksms[i].attached_nr;
          info->atime = ksmtable.ksms[i].atime;
          info->dtime = ksmtable.ksms[i].dtime;
        }
      }
    }
  }
  
  release(&ksmtable.lock);
  return 0;
}

int ksmattach(int handle, int flags){
  int i;
  int found_handle_i = 0;
  int empty_slot = -1;
  int ret = 0;
  
  cprintf("ksmattach called with handle=%d and flags=%d\n", handle, flags);
  acquire(&ksmtable.lock);
  
  // Finding the correct link
  for (i = 0; i < KSM_MAX_ATTACHMENTS; i ++){
    if (ksmtable.attachments[i].handle == handle){
      found_handle_i = i;
      if ((ksmtable.attachments[i].pid == proc->pid)&&
          (ksmtable.attachments[i].permission == flags)) {
        release(&ksmtable.lock);
        return ksmtable.attachments[i].address;
      }
    }
    if ((ksmtable.attachments[i].handle == 0) && (empty_slot == -1)){
      empty_slot = i;
    }
  }
  
  // Link found, seting other values on link
  if (ksmtable.attachments[i].permission == -1){
    ksmtable.attachments[i].permission = flags;
  }
  else {
    // The one we found has different permissions, need to create a new one
    ksmtable.attachments[empty_slot].pid = proc->pid;
    ksmtable.attachments[empty_slot].permission = flags;
    ksmtable.attachments[empty_slot].handle = handle;
    ksmtable.attachments[empty_slot].ksm_id = ksmtable.attachments[i].ksm_id;
    i = empty_slot;
  }
  
  ret = ksmattachhelper(&ksmtable.attachments[i]);
  
  release(&ksmtable.lock);
  return ret;
}

int ksmdetach(int handle){
  int i;
  int detached;
  
  cprintf("ksmdetach called with handle=%d\n", handle);
  acquire(&ksmtable.lock);
  
  // Finding the correct link
  for (i = 0; i < KSM_MAX_ATTACHMENTS; i ++){
    if (ksmtable.attachments[i].handle == handle){
      if (ksmtable.attachments[i].pid == proc->pid) {
	    detached = 1;
        ksmdetachhelper(& ksmtable.attachments[i]);
      }
    }
  }
  
  release(&ksmtable.lock);
  if (detached == 1)
    return 0;
  return -1;
}

int ksmdelete(int handle){
  int i, j;
  int ksm_id = 0;
  
  cprintf("ksmdelete called with handle=%d\n", handle);
  
  acquire(&ksmtable.lock);
  // Looking to see if the shared memory already exists
  for (j = 0; j < KSM_MAX_ATTACHMENTS; j++){
    if (ksmtable.attachments[j].handle == handle){
      ksm_id = ksmtable.attachments[j].ksm_id;
      
      ksmdetachhelper(& ksmtable.attachments[j]);
      
      break;
    }
  }
  if (ksm_id > 0) {
    for (i = 0; i < KSM_NUMBER; i++) {
      if (ksmtable.ksms[i].ksm_id == ksm_id) {
        if (ksmtable.ksms[i].attached_nr > 0){
          ksmtable.ksms[i].marked_for_deletion = 1;
          release(&ksmtable.lock);
          return 0;
        }
      
        ksmreleasepages(i);
        ksmtable.ksms[i].ksm_id = 0;
        ksmtable.ksms[i].marked_for_deletion = 0;
        ksmtable.ksms[i].name[0] = '\0';
        
        ksmtable.attachments[j].pid = 0;
        ksmtable.attachments[j].handle = 0;
        ksmtable.attachments[j].ksm_id = 0;
    
        ksmtable.total_shrg_nr --;
      
        release(&ksmtable.lock);
        return 0;
      }
    }
  }
  
  release(&ksmtable.lock);
  cprintf("ksmdelete: shared memory with handle=%d does not exist\n", handle);
  return 0;
}

int ksmreleasepages(int i){
  ksmtable.total_shpg_nr -= ksmtable.ksms[i].pages_number;
  for (;ksmtable.ksms[i].pages_number > 0; ksmtable.ksms[i].pages_number--){
    kfree(ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number -1]);
    ksmtable.ksms[i].pages[ksmtable.ksms[i].pages_number -1] = 0;
  }
  return 0;
}

int ksmattachhelper(struct ksm_link * content){
  int i, j;
  pte_t * pte;
  pde_t * pde;
  int perm;
  uint va;
  
  // finding ksm
  for (i = 0; i < KSM_NUMBER; i ++){
    if (content->ksm_id == ksmtable.ksms[i].ksm_id){
	  break;
	}
  }
  
  ksmtable.ksms[i].mpid = proc->pid;
  
  content->address = (uint) p2v(PHYSTOP) - proc->ssm - ksmtable.ksms[i].ksmsz;
  if (content->address < proc->sz) {
    content->address = 0;
	return 0;
  }
  
  if (content->permission == KSM_READWRITE)
    perm = PTE_U | PTE_W;
  else
   perm = PTE_U;
  
  va = content->address;
  for (j = 0; j< ksmtable.ksms[i].pages_number; j ++){
    // work of waldpgdir
	pte = (pte_t*)ksmtable.ksms[i].pages[j];
	pde = &proc->pgdir[PDX((void*)va)];
	*pde = v2p(ksmtable.ksms[i].pages[j]) | PTE_P |PTE_W | PTE_U;
	
	// work of mappages
	pte = &pte[PTX((void *)va)];
    *pte = ((uint) ksmtable.ksms[i].pages[j]) | perm | PTE_P;
	
	va +=PGSIZE;
  }
  ksmtable.ksms[i].attached_nr ++;
  
  return content->address;
}

int ksmdetachhelper(struct ksm_link * content){
  int i;
  pte_t *pte;
  pde_t *pde;
  uint pages_number = 0;
  
  if (content->address == 0)
    return -1;
  
  // finding size
  for (i = 0; i < KSM_NUMBER; i ++){
    if (content->ksm_id == ksmtable.ksms[i].ksm_id){
	  pages_number = ksmtable.ksms[i].pages_number;
	  break;
	}
  }
  
  ksmtable.ksms[i].mpid = proc->pid;
  
  if (pages_number == 0){
    content->address = 0;
	return -1;
  }
  
  if (ksmtable.ksms[i].marked_for_deletion && (ksmtable.ksms[i].attached_nr <= 1)){
	ksmreleasepages(i);
  }
  ksmtable.ksms[i].attached_nr --;
  
  for (i = 0; i < pages_number; i++){
    //Doing the work of walkpgdir
    pde = &proc->pgdir[PDX((char*)content->address)];
	pte = (pte_t*)p2v(PTE_ADDR(*pde));
	pte = &pte[PTX((char*)content->address)];
	
	if(pte){
	  *pte = 0;
	}
	*pde = 0;
	content->address += PGSIZE;
  }
  return 0;
}

int ksminherit(struct proc * np) {
  int i, j;
  
  if (! np->parent)
    return 0;
  
  acquire(&ksmtable.lock);
  np->ssm = np->parent->ssm;
  
  for (i = 0; i < KSM_MAX_ATTACHMENTS; i++){
    if (ksmtable.attachments[i].pid == np->parent->pid) {
      // Creating a link
      for (j = 0; j < KSM_MAX_ATTACHMENTS; j++){
        if (ksmtable.attachments[j].pid == 0){
          ksmtable.attachments[j].pid = np->pid;
          ksmtable.attachments[j].address = ksmtable.attachments[i].address;
          ksmtable.attachments[j].ksm_id = ksmtable.attachments[i].ksm_id;
          ksmtable.attachments[j].handle = ksmtable.attachments[i].handle;
          ksmtable.attachments[j].permission = ksmtable.attachments[i].permission;
          break;
        }
      }
      
      // Cloning attachments
      if (ksmattachhelper(& ksmtable.attachments[i]) == 0)
        return -1;
    }
  }
  release(&ksmtable.lock);
  return 0;
}

int ksmdetachall() {
  int i;
  acquire(&ksmtable.lock);
  for (i = 0; i < KSM_MAX_ATTACHMENTS; i++){
    if (ksmtable.attachments[i].pid == proc->pid) {
      if (ksmdetachhelper(& ksmtable.attachments[i]) == 0)
        return -1;
      ksmtable.attachments[i].pid = 0;
      ksmtable.attachments[i].handle = 0;
      ksmtable.attachments[i].ksm_id = 0;
    }
  }
  
  proc->ssm = 0;
  
  release(&ksmtable.lock);
  return 0;
}