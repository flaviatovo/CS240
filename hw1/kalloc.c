// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define NUMBEROFPAGES 57069
#define BITMAPARRAYSIZE 1784 // Number of 32 bits in entries on bitmap table
#define BITMAPENTRYSIZE 32 // Number of bits per entry in the bitmap table
#define BITMAPENTRYSIZEB 4  // Number of Bytes in BITMAPENTRYSIZE

void initbitmaptable(void *vstart);
extern char end[]; // first address after kernel loaded from ELF file

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int used_pages;
  uint hint;
  unsigned long int * bitmaptable;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  initbitmaptable(vstart);
  kmem.used_pages = 0;
  kmem.hint = 0;
}

void
kinit2(void *vstart, void *vend)
{
  kmem.use_lock = 1;
}

void
initbitmaptable(void *vstart){
  kmem.bitmaptable = (unsigned long int*)PGROUNDUP((uint)vstart);

  // Setting values
  // First 2 pages are used for the table
  kmem.bitmaptable[0] = 3;
  // Middle pages are free
  memset(kmem.bitmaptable + BITMAPENTRYSIZEB, 0, BITMAPARRAYSIZE*BITMAPENTRYSIZEB);
  // Last entry uses only 13 bits
  kmem.bitmaptable[BITMAPARRAYSIZE-1] = 0xFFFFE000;
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  int index_in_bitmap = 0;
  int index_in_entry = 0;

  if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  // Get correct possition inside bitmap
  index_in_bitmap = v2p(v)>>5;
  index_in_entry = v2p(v) & 0x1F;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  kmem.bitmaptable[index_in_bitmap] &= ~(0x1 << index_in_entry);
  if(kmem.hint > index_in_bitmap)
    kmem.hint = index_in_bitmap;

  kmem.used_pages --;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  uint address = 0;
  uint index_in_bitmap = 0;
  int index_in_entry = -1;
  int found_empty_page = 0;
  uint chor_result = 0;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  index_in_bitmap = kmem.hint;

  do{
    chor_result = kmem.bitmaptable[index_in_bitmap] ^ 0xFFFFFFFF;
    if (chor_result){
      found_empty_page = 1;
      break;
    }
    else {
      index_in_bitmap ++;
      if( index_in_bitmap == BITMAPARRAYSIZE)
        index_in_bitmap = 0;
    }
  } while(index_in_bitmap != kmem.hint);

  if(!found_empty_page) {
    address = 0;
  }
  else{
    do {
      chor_result >>=1;
      index_in_entry ++;
    } while (chor_result);

    address = ((index_in_bitmap << 4) + index_in_entry)*PGSIZE + (uint) kmem.bitmaptable;

    kmem.bitmaptable[index_in_bitmap] |= 0x1 << index_in_entry;

    kmem.used_pages ++;
  }

  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)address;
}

