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

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kfreesingle(void *pa, int cpuid, int flag)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpuid].lock);
  r->next = kmem[cpuid].freelist;
  kmem[cpuid].freelist = r;
  release(&kmem[cpuid].lock);
}

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int index = 0;

  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfreesingle(p, index, 0);
    index = (index + 1) % NCPU;
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  push_off();
  kfreesingle(pa, cpuid(), 1);
  pop_off();
}

void *
steal(void) {
  struct run *r = 0;
  int id = cpuid();
  int nextid = id;
  release(&kmem[id].lock);
  while (1) {
    nextid = (nextid + 1) % NCPU;
    if (nextid == id)
      break;
    acquire(&kmem[nextid].lock);
    r = kmem[nextid].freelist;
    if(r) {
      kmem[nextid].freelist = r->next;
      release(&kmem[nextid].lock);
      break;
    }
    release(&kmem[nextid].lock);
  }
  acquire(&kmem[id].lock);
  return (void*)r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int id = cpuid();
  acquire(&kmem[id].lock);
  r = kmem[id].freelist;
  if(r) {
    kmem[id].freelist = r->next;
  } else {
    // steal a page
    r = steal();
  }
  release(&kmem[id].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
