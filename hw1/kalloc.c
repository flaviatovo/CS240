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
//#define BITS32
#define BITS16
//#define BITS8

#ifdef BITS32
////////////////////////////// 32 BITS ENTRY
#define BITMAPARRAYSIZE 1784 // Number of 32 bits in entries on bitmap table
#define BITMAPENTRYSIZE 32   // Number of bits per entry in the bitmap table
#define BITMAPENTRYSIZEB 4   // Number of bytes in the bitmap entry
                             // BITMAPENTRYSIZE/8
#endif
#ifdef BITS16
////////////////////////////// 16 BITS ENTRY
#define BITMAPARRAYSIZE 3567 // Number of 16 bits in entries on bitmap table
#define BITMAPENTRYSIZE 16   // Number of bits per entry in the bitmap table
#define BITMAPENTRYSIZEB 2   // Number of bytes in the bitmap entry
                             // BITMAPENTRYSIZE/8
#endif
#ifdef BITS8
////////////////////////////// 8 BITS ENTRY
#define BITMAPARRAYSIZE 7134 // Number of 16 bits in entries on bitmap table
#define BITMAPENTRYSIZE  8   // Number of bits per entry in the bitmap table
#define BITMAPENTRYSIZEB 1   // Number of bytes in the bitmap entry
                             // BITMAPENTRYSIZE/8
#endif

void initbitmaptable(void *vstart);
extern char end[]; // first address after kernel loaded from ELF file

struct {
  struct spinlock lock;
  int use_lock;
  uint hint;

#ifdef BITS8
///////////// 8 BITS
  uchar * bitmaptable;
#endif
#ifdef BITS16
///////////// 16 BITS
  uint * bitmaptable;
#endif
#ifdef BITS32
///////////// 32 BITS
  unsigned long int * bitmaptable;
#endif

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
  kmem.hint = 0;
}

void
kinit2(void *vstart, void *vend)
{
  kmem.use_lock = 1;
}

void
initbitmaptable(void *vstart){

#ifdef BITS8
///////////// 8 BITS
  kmem.bitmaptable = (uchar*)PGROUNDUP((uint)vstart);
#endif
#ifdef BITS16
///////////// 16 BITS
  kmem.bitmaptable = (uint*)PGROUNDUP((uint)vstart);
#endif
#ifdef BITS32
///////////// 32 BITS
  kmem.bitmaptable = (unsigned long int*)PGROUNDUP((uint)vstart);
#endif

  // Setting values
  // First 2 pages are used for the table
  kmem.bitmaptable[0] = 3;
  // Middle pages are free
  memset(kmem.bitmaptable + BITMAPENTRYSIZEB, 0, BITMAPARRAYSIZE*BITMAPENTRYSIZEB);

#ifdef BITS8
///////////// 8 BITS
  // Last entry uses only 5 bits
  kmem.bitmaptable[BITMAPARRAYSIZE-1] = 0xE0;
#endif
#ifdef BITS16
///////////// 16 BITS
  // Last entry uses only 13 bits
  kmem.bitmaptable[BITMAPARRAYSIZE-1] = 0xE000;
#endif
#ifdef BITS32
///////////// 32 BITS
  // Last entry uses only 13 bits
  kmem.bitmaptable[BITMAPARRAYSIZE-1] = 0xFFFFE000;
#endif

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
  uint temp = (((uint)v - (uint)kmem.bitmaptable)/PGSIZE);

#ifdef BITS32
///////////// 32 BITS
  index_in_bitmap = temp >> 5;
  index_in_entry = temp & 0x1F;
#endif
#ifdef BITS16
///////////// 16 BITS
  index_in_bitmap = temp >> 4;
  index_in_entry = temp & 0xF;
#endif
#ifdef BITS8
///////////// 8 BITS
  index_in_bitmap = temp >> 3;
  index_in_entry = temp & 0x7;
#endif

  if(kmem.use_lock)
    acquire(&kmem.lock);

  kmem.bitmaptable[index_in_bitmap] &= ~(0x1 << index_in_entry);
  if(kmem.hint > index_in_bitmap)
    kmem.hint = index_in_bitmap;

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

#ifdef BITS32
///////////// 32 BITS
    address = ((index_in_bitmap << 5) + index_in_entry)*PGSIZE + (uint) kmem.bitmaptable;
#endif
#ifdef BITS16
///////////// 16 BITS
    address = ((index_in_bitmap << 4) + index_in_entry)*PGSIZE + (uint) kmem.bitmaptable;
#endif
#ifdef BITS8
///////////// 8 BITS
    address = ((index_in_bitmap << 3) + index_in_entry)*PGSIZE + (uint) kmem.bitmaptable;
#endif

    kmem.bitmaptable[index_in_bitmap] |= 0x1 << index_in_entry;

    kmem.hint = index_in_bitmap;
  }

  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)address;
}

