//
// File created by Flavia Tovo as work for project 2
// of OS class on KAUST
// Fall 2012
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "semaphore.h"
#include "spinlock.h"

struct {
  struct spinlock lock;

  struct semaphore sems[NSEM];
  uint sem_handles[NSEM];

  uint next_handle;
} semtable;

void
seminit(void)
{
  initlock(&semtable.lock, "semtable");
  
  for (int i = 0; i < NSEM; i++)
    sem_handles[i] = 0;

  next_handle = 1;
}

// Function used to get or create a semaphore
// If a semaphore with name equal to name exists, than return 
// the handler of that semaphore;
// If a semaphore with name doesn't exist, create the semaphore,
// initialize the value as value and return the handler.
int sem_get(uint name, int value){
  
  acquire(&semtable.lock);
  
  // Looking to see if semaphore already exists
  for(int i = 0; i < NSEM; i ++){
    if (sems[i].name == name){
      // if the one I found is valid, return it
      if (sem_handles[i] != 0){
        release(&semtable.lock);
        return sem_handles[i];
      }
      // if I found the name, but there is no handle, create a new one
      sems[i].value = value;
      sem_handles[i] = next_handle;
      next_handle ++;

      release(&semtable.lock);
      return sem_handles[i];
	}
  }
  // No semaphore with that name
  // Looking for an empty one
  for(int i = 0; i < NSEM; i ++){
    // An empty one was found
    if (sem_handles[i] == 0){
      sems[i].name = name;
      sems[i].value = value;
      sem_handles[i] = next_handle;
      next_handle ++;

      release(&semtable.lock);
      return sem_handles[i];
    }
  }
  
  // Run out of semaphores
  release(&semtable.lock);
  return OUT_OF_SEM;
}

// Function used to delete a semaphore, if there are processes waiting
// on that semaphore, they can't be let spleeping forever!
int sem_delete(int handle){
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(int i = 0; i < NSEM; i ++){
    if (sem_handles[i] == handle){

      sem_handles[i] = 0;
      wakeup(sems[i].name);

      release(&semtable.lock);
      return SEM_OK;
	}
  }

not_found:
  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}

// Used to increase the value of the semaphore, if the value was previously
// 0, this should wake processes sleeping on name
int sem_signal(int handle){
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(int i = 0; i < NSEM; i ++){
    if (sem_handles[i] == handle){

      // Increasing the value
      sems[i].value ++;
      // Wakeup everyone
      wakeup(sems[i].name);

      release(&semtable.lock);
      return SEM_OK;
	}
  }

not_found:
  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}

// Used to wait for an event on the semaphore, if the value is greater than 0
// should just decrement the value and continue working
// if value is 0, sleep on name
int sem_wait(int handle){
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(int i = 0; i < NSEM; i ++){
test:
    if (sem_handles[i] == handle){
      if(sems[i].value > 0){
        // Decreasing the value
        sems[i].value --;

        release(&semtable.lock);
        return SEM_OK;
      }
      sleep(sems[i].name, &semtable.lock);
      goto test;
	}
  }

not_found:
  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}