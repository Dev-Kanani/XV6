#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

uint total_counter_for_ticks = 0;

struct
{
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;
struct spinlock print_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

int current_time = 0;

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    char *pa = kalloc();
    if (pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int)(p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table.
void procinit(void)
{
  struct proc *p;

  initlock(&pid_lock, "nextpid");
  initlock(&print_lock, "print_lock");
  initlock(&wait_lock, "wait_lock");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    initlock(&p->lock, "proc");
    p->state = UNUSED;
    p->kstack = KSTACK((int)(p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *
mycpu(void)
{
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc *
myproc(void)
{
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid()
{
  int pid;

  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *
allocproc(void)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == UNUSED)
    {
      goto found;
    }
    else
    {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;

  // Allocate a trapframe page.
  if ((p->trapframe = (struct trapframe *)kalloc()) == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if (p->pagetable == 0)
  {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  p->rtime = 0;
  p->etime = 0;
  p->ctime = ticks;
  p->mask = 0;

  for (int i = 0; i < 31; i++)
  {
    p->syscall_counts[i] = 0; // Initialize syscall counts to 0
  }

  p->syscount_trace_enabled = 0;

  p->alarm_handler = 0;
  p->alarm_ticks = 0; // Initialize to 0
  p->ticks_left = 0;  // Initialize to 0
  p->alarm_active = 0;
  p->in_alarm_handler = 0;
  p->alarm_interval = 0;
  p->scheduler_count = 0;
  p->queue_level = 0;
  p->ticks_used = 0;

  p->alarm_tf = (struct trapframe *)kalloc();

  if (p->alarm_tf)
  {
  }
  else
  {
    release(&p->lock);
    return 0;
  }

  p->tickets = 1;                            // Default ticket number
  p->arrival_time = total_counter_for_ticks; // Set the creation time

  if (p->parent)
  {
    p->tickets = p->parent->tickets;
  }

  return p;
}

// struct proc *
// findproc(int pid)
// {
//   struct proc *p;

//   // Acquire the process table lock to safely search through the process table
//   acquire(&ptable.lock);

//   // Iterate through the process table to find the process with the matching pid
//   for (p = proc; p < &proc[NPROC]; p++)
//   {
//     if (p->pid == pid)
//     {
//       release(&ptable.lock); // Release the lock after finding the process
//       return p;              // Return the found process
//     }
//   }

//   release(&ptable.lock); // Release the lock if no process was found
//   return 0;              // Return null if no process was found with the given pid
// }

struct proc *
findproc(int pid)
{
  struct proc *p;

  // Acquire the process table lock to safely search through the process table
  acquire(&ptable.lock);

  // Iterate through the process table to find the process with the matching pid
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->pid == pid)
    {
      // Acquire the process lock before returning the process
      acquire(&p->lock);

      // Release the process table lock after acquiring the process lock
      release(&ptable.lock);

      return p; // Return the found process with the lock held
    }
  }

  // If no process is found, release the process table lock
  release(&ptable.lock);
  return 0; // Return null if no process was found with the given pid
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  if (p->trapframe)
    kfree((void *)p->trapframe);
  p->trapframe = 0;
  if (p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  if (p->alarm_tf)
  {
    kfree((void *)p->alarm_tf); // Free the allocated memory
    p->alarm_tf = 0;            // Set to NULL to avoid dangling pointer
  }
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE,
               (uint64)trampoline, PTE_R | PTE_X) < 0)
  {
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE,
               (uint64)(p->trapframe), PTE_R | PTE_W) < 0)
  {
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
uchar initcode[] = {
    0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
    0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
    0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
    0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
    0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
    0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

// Set up first user process.
void userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;

  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;     // user program counter
  p->trapframe->sp = PGSIZE; // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n)
{
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if (n > 0)
  {
    if ((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0)
    {
      return -1;
    }
  }
  else if (n < 0)
  {
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // struct proc *parent = p->parent;

  // // Inherit syscall counts from the parent
  // for (int i = 0; i < 31; i++)
  // {
  //   p->syscall_counts[i] = parent->syscall_counts[i];
  // }

  // Allocate process.
  if ((np = allocproc()) == 0)
  {
    return -1;
  }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0)
  {
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  np->queue_level = 0; // Start at the highest priority queue
  // np->queue_level = p->queue_level; // Inherit queue level
  np->ticks_used = 0;
  np->tickets = p->tickets;

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p)
{
  struct proc *pp;

  for (pp = proc; pp < &proc[NPROC]; pp++)
  {
    if (pp->parent == p)
    {
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

extern void switchuvm(struct proc *);
extern void switchkvm(void);
// Simple pseudo-random number generator
static unsigned long next = 1;

int rand(void)
{
  next = next * 1103515245 + 12345;
  return (unsigned)(next / 65536) % 32768;
}

// Function to generate a random number between 0 and max
int random_at_most(int max)
{
  return rand() % (max + 1);
}

void exit(int status)
{
  struct proc *p = myproc();
  struct proc *parent = p->parent;

  if (p->syscount_trace_enabled) // Check if syscount tracing is enabled
  {
    for (int i = 0; i < 31; i++)
    {
      if (p->syscall_counts[i] > 0)
      {
        if ((1 << i) & p->mask)
        {
          parent->syscall_counts[i] += p->syscall_counts[i];
          printf("PID %d called %s: %d times\n", p->pid, getSyscallName(i), p->syscall_counts[i]);
        }
      }
    }
  }

  // printf("The process named %s with PID %d is getting exited.\n", p->name, p->pid);

  if (p == initproc)
    panic("init exiting");

  // Close all open files.
  for (int fd = 0; fd < NOFILE; fd++)
  {
    if (p->ofile[fd])
    {
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  acquire(&p->lock);

  p->xstate = status;
  p->state = ZOMBIE;
  p->etime = ticks;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
// void exit(int status)
// {
//   struct proc *p = myproc();
//   struct proc *parent = p->parent;

//   // Print total syscall usage summary for the current process
//   printf("PID %d syscall usage summary:\n", p->pid);
//   // printf("  Child %d:\n", p->pid);
//   // for (int i = 0; i < 31; i++)
//   // {
//   //   if (p->syscall_counts[i] > 0)
//   //   {
//   //     printf("    %s: %d times\n", getSyscallName(i), p->syscall_counts[i]);
//   //   }
//   // }

//   // Check if the parent exists before accessing its syscall counts
//   if (parent) // Only proceed if the parent exists
//   {
//     // printf("  Parent %d:\n", parent->pid);
//     // for (int i = 0; i < 31; i++)
//     // {
//     //   if (parent->syscall_counts[i] > 0)
//     //   {
//     //     printf("    %s: %d times\n", getSyscallName(i), parent->syscall_counts[i]);
//     //   }
//     // }

//     // Now aggregate the counts from both processes
//     // printf("  Total syscall counts:\n");
//     for (int i = 0; i < 31; i++)
//     {
//       int total_count = p->syscall_counts[i] + parent->syscall_counts[i];
//       if (total_count > 0)
//       {
//         printf("    %s: %d times\n", getSyscallName(i), total_count);
//       }
//     }
//   }
//   else
//   {
//     // Handle case where there is no parent
//     printf("  No parent process.\n");
//     printf("  Total syscall counts for PID %d:\n", p->pid);
//     for (int i = 0; i < 31; i++)
//     {
//       if (p->syscall_counts[i] > 0)
//       {
//         printf("    %s: %d times\n", getSyscallName(i), p->syscall_counts[i]);
//       }
//     }
//   }

//   if (p == initproc)
//     panic("init exiting");

//   // Close all open files.
//   for (int fd = 0; fd < NOFILE; fd++)
//   {
//     if (p->ofile[fd])
//     {
//       struct file *f = p->ofile[fd];
//       fileclose(f);
//       p->ofile[fd] = 0;
//     }
//   }

//   begin_op();
//   iput(p->cwd);
//   end_op();
//   p->cwd = 0;

//   acquire(&wait_lock);

//   // Give any children to init.
//   reparent(p);

//   // Parent might be sleeping in wait().
//   wakeup(p->parent);

//   acquire(&p->lock);

//   p->xstate = status;
//   p->state = ZOMBIE;
//   p->etime = ticks;

//   release(&wait_lock);

//   // Jump into the scheduler, never to return.
//   sched();
//   panic("zombie exit");
// }

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
  struct proc *pp;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (pp = proc; pp < &proc[NPROC]; pp++)
    {
      if (pp->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&pp->lock);

        havekids = 1;
        if (pp->state == ZOMBIE)
        {
          // Found one.
          pid = pp->pid;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate,
                                   sizeof(pp->xstate)) < 0)
          {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || killed(p))
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

int prev_ticks = 0;
int rr_index = 0; // Round-robin index for queue 3

void scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

#ifdef LBS

  printf("LBS\n");

  for (;;)
  {
    // Enable interrupts on this processor.
    intr_on();

    struct proc *p;
    struct proc *winner = 0;
    winner = 0;
    int total_tickets = 0;

    // Calculate total tickets of runnable processes
    for (p = proc; p < &proc[NPROC]; p++)
    {
      acquire(&p->lock);
      if (p->state == RUNNABLE)
      {
        total_tickets += p->tickets;
      }
      release(&p->lock);
    }

    if (total_tickets > 0)
    {
      // Generate a random ticket between 0 and total_tickets-1
      int random_ticket = rand() % total_tickets;

      // Find the winner based on random ticket
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);
        if (p->state == RUNNABLE)
        {
          if (random_ticket < p->tickets)
          {
            if (!winner || (winner->tickets == p->tickets && winner->arrival_time > p->arrival_time))
            {
              winner = p; // Pick the earlier arrival if ticket counts are the same
            }
            release(&p->lock);
            break;
          }
          random_ticket -= p->tickets;
        }
        release(&p->lock);
      }
    }

    if (winner)
    {
      int winner_tickets_found = winner->tickets;

      struct proc *yo = 0;
      for (yo = proc; yo < &proc[NPROC]; yo++)
      {
        acquire(&yo->lock);
        if (yo->state == RUNNABLE)
        {
          if (yo->tickets == winner_tickets_found)
          {
            if (yo->arrival_time < winner->arrival_time)
            {
              winner = yo;
            }
          }
        }
        release(&yo->lock);
      }
    }

    // If there's a winner, schedule it
    if (winner)
    {
      acquire(&winner->lock);
      if (winner->state == RUNNABLE)
      {
        winner->scheduler_count += 1;
        winner->state = RUNNING;
        c->proc = winner;
        swtch(&c->context, &winner->context); // Switch to the selected process
        c->proc = 0;
      }
      release(&winner->lock);
    }
  }
#endif

#ifdef MLFQ
  // Implement Multi-Level Feedback Queue scheduling logic here

  printf("MLFQ\n");

  int current_time = 0;

  for (;;)
  {
    // Enable interrupts on this processor.
    intr_on();

    if (ticks % 48 == 0)
    {
      for (p = proc; p < &proc[NPROC]; p++)
      {
        acquire(&p->lock);
        if (p->state == RUNNABLE)
        {
          // printf("Boosting process %d\n", p->pid);
          p->queue_level = 0; // Boost to highest queue
          p->ticks_used = 0;  // Reset ticks used
        }
        release(&p->lock);
      }

      // ticks++;
    }

    struct proc *p = 0; // pointer to the process to run

    for (int queue = 0; queue < NQUEUE; queue++)
    {
      if (queue == 3)
      {
        p = &proc[(rr_index) % NPROC];
        acquire(&p->lock);
        if (p->state == RUNNABLE && p->queue_level == 3)
        {
          acquire(&print_lock);
          // printf("timestamp=%d process_id=%d queue_level=%d\n", ticks, p->pid, p->queue_level);
          release(&print_lock);

          current_time++;
          p->state = RUNNING;
          c->proc = p;
          swtch(&c->context, &p->context); // Context switch to the process
          c->proc = 0;
          release(&p->lock);

          acquire(&print_lock);
          // printf("p->ticks_used : %d\n", p->ticks_used);
          release(&print_lock);
          if (p->ticks_used % 16 == 0)
          {
            // printf("rr_index made ++ : %d\n", rr_index);
            rr_index = (rr_index + 1) % NPROC; // Update rr_index for the next round
          }
        }
        else
        {
          rr_index = (rr_index + 1) % NPROC; // Update rr_index for the next round
          release(&p->lock);
        }
      }
      else // For queues 0, 1, and 2, schedule based on arrival time
      {
        struct proc *earliest_proc = 0;
        int earliest_time = -1;

        for (p = proc; p < &proc[NPROC]; p++)
        {
          acquire(&p->lock);
          if (p->state == RUNNABLE && p->queue_level == queue)
          {
            // Check if this process has the earliest arrival time
            if (earliest_time == -1 || p->arrival_time < earliest_time)
            {
              earliest_time = p->arrival_time;
              earliest_proc = p; // Select the earliest process
            }
          }
          release(&p->lock);
        }

        // If a process was found, run it
        if (earliest_proc)
        {
          p = earliest_proc;
          acquire(&p->lock);

          // Check if the process's time slice has expired
          int time_slice = (queue == 0)   ? TIME_SLICE_0
                           : (queue == 1) ? TIME_SLICE_1
                                          : TIME_SLICE_2;

          total_counter_for_ticks++;
          acquire(&print_lock);
          // printf("timestamp=%d process_id=%d queue_level=%d\n", ticks, p->pid, p->queue_level);
          // printf("total_counuter_for_ticks = %d\n", total_counter_for_ticks);
          release(&print_lock);

          current_time++;
          p->state = RUNNING;
          c->proc = p;
          swtch(&c->context, &p->context); // Context switch to the process

          c->proc = 0;

          release(&p->lock);
          break; // Exit the loop as a process has been found and executed
        }
      }
    }
  }
  if (!p)
  {
    // If no process was found, yield or sleep
    yield(); // This avoids busy waiting
  }

#endif

#ifdef DEFAULT

  printf("DEFAULT\n");
  // Default Round Robin Scheduling
  for (;;)
  {
    intr_on();

    for (p = proc; p < &proc[NPROC]; p++)
    {
      acquire(&p->lock);
      if (p->state == RUNNABLE)
      {
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);
        c->proc = 0;
      }
      release(&p->lock);
    }
  }
#endif
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
  int intena;
  struct proc *p = myproc();

  if (!holding(&p->lock))
    panic("sched p->lock");
  if (mycpu()->noff != 1)
    panic("sched locks");
  if (p->state == RUNNING)
    panic("sched running");
  if (intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;

  // acquire(&ptable.lock);
  // p->state = SLEEPING; // Example: changing state before switching
  // Perform context switch...
  // release(&ptable.lock);
}

// Give up the CPU for one scheduling round.
void yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  // p->ticks_used = 0; // Reset ticks used
  p->queue_level = p->queue_level;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void)
{
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first)
  {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock); // DOC: sleeplock1
  release(lk);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void *chan)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p != myproc())
    {
      acquire(&p->lock);
      if (p->state == SLEEPING && p->chan == chan)
      {
        p->state = RUNNABLE;
      }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->pid == pid)
    {
      p->killed = 1;
      if (p->state == SLEEPING)
      {
        // Wake process from sleep().
        p->state = RUNNABLE;
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

void setkilled(struct proc *p)
{
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int killed(struct proc *p)
{
  int k;

  acquire(&p->lock);
  k = p->killed;
  release(&p->lock);
  return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if (user_dst)
  {
    return copyout(p->pagetable, dst, src, len);
  }
  else
  {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if (user_src)
  {
    return copyin(p->pagetable, dst, src, len);
  }
  else
  {
    memmove(dst, (char *)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void)
{
  static char *states[] = {
      [UNUSED] "unused",
      [USED] "used",
      [SLEEPING] "sleep ",
      [RUNNABLE] "runble",
      [RUNNING] "run   ",
      [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;

  printf("\n");
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}

// waitx
int waitx(uint64 addr, uint *wtime, uint *rtime)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;)
  {
    // Scan through table looking for exited children.
    havekids = 0;
    for (np = proc; np < &proc[NPROC]; np++)
    {
      if (np->parent == p)
      {
        // make sure the child isn't still in exit() or swtch().
        acquire(&np->lock);

        havekids = 1;
        if (np->state == ZOMBIE)
        {
          // Found one.
          pid = np->pid;
          *rtime = np->rtime;
          *wtime = np->etime - np->ctime - np->rtime;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0)
          {
            release(&np->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&wait_lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->killed)
    {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); // DOC: wait-sleep
  }
}

void update_time()
{
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->state == RUNNING)
    {
      p->rtime++;
    }
    release(&p->lock);
  }
}