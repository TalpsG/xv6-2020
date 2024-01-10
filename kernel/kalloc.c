// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
char ref[PHYSTOP>>12];
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

int refadj(uint64 pa,int val){
	acquire(&kmem.lock);
	ref[pa>>12] += val;
	release(&kmem.lock);
	return ref[pa>>12];
}
int refset(uint64 pa){
	ref[pa>>12] = 1;
	return ref[pa>>12];
}
int refdec(uint64 pa){
	ref[pa>>12] -= 1;
	return ref[pa>>12];
}
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
	refset((uint64)p);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.

  acquire(&kmem.lock);
  if(refdec((uint64)pa) == 0){
	memset(pa, 1, PGSIZE);
	r = (struct run*)pa;
	r->next = kmem.freelist;
	kmem.freelist = r;
  }
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
	refset((uint64)r);
  }
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
  }
  return (void*)r;
}

int cowalloc(pagetable_t p, uint64 va){
	pte_t *pte = walk(p,va,0);
	if(pte == 0 || (*pte & PTE_V) ==0 ){
		printf("cowalloc pte invalid\n");
		return -1;
	}
	uint64 old = PTE2PA(*pte);
	uint64 flag = PTE_FLAGS(*pte);
	uint64 new = (uint64)kalloc();
	if(new == 0){
		return -1;
	}
	memmove((void*)new,(void*)old,PGSIZE);
	kfree((void*)old);
	*pte = PA2PTE(new) | flag | PTE_W;
	return 0;
}