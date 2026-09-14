// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define PA2INDEX(pa) (((uint64)(pa) - KERNBASE) / PGSIZE)
#define MAX_PAGES    ((PHYSTOP - KERNBASE) / PGSIZE)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;


struct spinlock ref_lock;
int ref_count[MAX_PAGES];

void
ref_init(void)
{
  initlock(&ref_lock, "ref_lock");
  acquire(&ref_lock);
  for(int i = 0; i < MAX_PAGES; i++) {
    ref_count[i] = 0;
  }
  release(&ref_lock);
}

void
inc_ref(uint64 pa)
{
  int idx = PA2INDEX(pa);
  acquire(&ref_lock);
  ref_count[idx]++;
  release(&ref_lock);
}

int
dec_ref(uint64 pa)
{
  int idx = PA2INDEX(pa);
  int count;
  acquire(&ref_lock);
  if(ref_count[idx] <= 0) {
    ref_count[idx] = 0;
    count = 0;
  } else {
    ref_count[idx]--;
    count = ref_count[idx];
  }
  release(&ref_lock);
  return count;
}


void
kinit()
{
  ref_init(); // Inicializar el cerrojo del contador
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Decrementar contador. Si aún hay procesos apuntando a la página, no liberar.
  if(dec_ref((uint64)pa) > 0)
    return;

  memset(pa, 1, PGSIZE);

  acquire(&kmem.lock);
  r = (struct run*)pa;
  r->next = kmem.freelist;
  kmem.freelist = r;
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // Llenar con basura
    int idx = PA2INDEX((uint64)r);
    acquire(&ref_lock);
    ref_count[idx] = 1; // La nueva página inicia con 1 referencia
    release(&ref_lock);
  }
  return (void*)r;
}



uint64
count_free_bytes(void)
{
  struct run *r;
  uint64 free_bytes = 0;

  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    free_bytes += PGSIZE;
    r = r->next;
  }
  release(&kmem.lock);

  return free_bytes;
}