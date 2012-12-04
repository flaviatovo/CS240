//
// File modified by Flavia Tovo as work for project 3
// of OS class on KAUST
// Fall 2012
//

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_pgused(void){
  return pgused();
}

// int ksmget(char *, uint);
// int ksmattach(int, int);
// int ksmdetack(int);
// int ksminfo(int, struct ksminfo_t*);
// int ksmdelete(int);
int
sys_ksmget(void){
  char * name;
  uint size;
  int size_in;

  if(argstr(0, &name) < 0)
    return -1;
  if((argint(1, &size_in) < 0) || (size_in < 0))
    return -1;
  size = (unsigned) size_in;
  
  return ksmget(name, size);
}
int
sys_ksmattach(void){
  int handle;
  int flags;
  
  if(argint(0, &handle) < 0)
    return -1;
  if(argint(1, &flags) < 0))
    return -1;

  return ksmattach(handle,flags);
}
int
sys_ksmdetach(void){
  int handle;

  if(argint(0, &handle) < 0)
    return -1;
  return ksmdetach(handle);
}
int
sys_ksminfo(void){
  int handle;
  struct ksminfo_t * info;
  
  if(argint(0, &handle) < 0)
    return -1;
  if(argptr(1, &info, sizeof(struct ksminfo_t)) < 0))
    return -1;

  return ksminfo(handle,info);
}
int
sys_ksmdelete(void){
  int handle;

  if(argint(0, &handle) < 0)
    return -1;
  return ksmdelete(handle);
}