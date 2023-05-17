// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define COW

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld
#ifdef COW
struct {
	struct spinlock lock;
	int use_lock;
	int numfreepages; // 사용 가능한 free physical frame 개수 
	uint ref[PHYSTOP >> PGSHIFT]; // 현재 page를 참조하는 프로세스 개수  
} pmem;
#endif

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
#ifdef COW
	initlock(&pmem.lock, "pmemlock");
	pmem.use_lock = 0;
#endif
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
#ifdef COW
	pmem.use_lock = 1;
	memset(&pmem.ref, 0, sizeof(uint) * (PHYSTOP >> PGSHIFT));
	acquire(&pmem.lock);
	pmem.numfreepages = 0;
	release(&pmem.lock);
#endif
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
#ifdef COW
// reference counter APIs 
int freemem()
{
	int numfreepages; 
	acquire(&pmem.lock);
	numfreepages = pmem.numfreepages;
	release(&pmem.lock);
	return numfreepages;
}

// 수정해야 할 부분들
// PGSHIFT: 12
uint
get_ref(uint pa)
{
	// PHYSTOP 초과 시
	if (pa > PHYSTOP)
		panic("PHYSTOP 초과\n");
	
	return pmem.ref[pa >> PGSHIFT];

}

void
inc_ref(uint pa)
{
	// PHYSTOP 초과 시
	if (pa > PHYSTOP)
		return;

	// reference count 1 증가	
	pmem.ref[pa >> PGSHIFT]++;
	
	return;
}

void
dec_ref(uint pa)
{
	// PHYSTOP 초과 시
	if (pa > PHYSTOP)
		return;
	
	// reference count 1 감소	
	pmem.ref[pa >> PGSHIFT]--;
	
	return;
}
#endif
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
/*
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
*/

#ifdef COW
	uint pa = V2P(v);

	if (pmem.ref[pa >> PGSHIFT] > 0)
		pmem.ref[pa >> PGSHIFT]--;

	if (pmem.ref[pa >> PGSHIFT] == 0)
	{
		if (kmem.use_lock)
			acquire(&kmem.lock);
	
		r = (struct run*) v;
		memset(v, 1, PGSIZE);
		r->next = kmem.freelist;
		kmem.freelist = r;

		pmem.numfreepages++;

		if (kmem.use_lock)
			release(&kmem.lock);
	}

#endif
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = kmem.freelist;
  
  if(r){
    kmem.freelist = r->next;

#ifdef COW
	if(pmem.use_lock)
		acquire(&pmem.lock);
	
	// reference 증가
	uint pa = V2P((uint) r);
	pmem.ref[pa >> PGSHIFT]++;

	pmem.numfreepages--;
		
	if(pmem.use_lock)
		release(&pmem.lock);
#endif
	}
  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)r;
}

