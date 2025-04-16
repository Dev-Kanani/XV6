#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct
{
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static int syscall_count[31]; // Array to count syscall invocations

extern int read_count;

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64 sys_getSysCount(void)
{
  struct proc *p = myproc();
  p->syscount_trace_enabled = 1;

  int mask;
  int pid;
  argint(0, &mask);
  argint(1, &pid);

  p->mask = mask;

  // printf("MASK AND PID: %d %d\n", mask, pid);
  // printf("HERE\n");

  return read_count;
  // return getSysCount(mask, pid); // This is your custom function that does the counting
}

uint64 sys_sigalarm(void)
{
  uint64 handler;
  argaddr(1, &handler);

  if (handler < 0)
    return -1;

  int interval;
  argint(0, &interval);

  if (interval < 0)
    return -1;

  // printf("Setting alarm: interval = %d, handler = %p\n", interval, handler);

  struct proc *p = myproc();
  // printf("p->pid: %d\n", p->pid);
  p->alarm_ticks = 0;
  p->ticks_left = interval;
  p->alarm_handler = handler;
  p->alarm_active = 1;
  p->alarm_interval = interval;

  // printf("p->alarm_active: %d\n", p->alarm_active);

  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();

  memcpy(p->trapframe, p->alarm_tf, sizeof(struct trapframe));

  p->alarm_active = 1;

  return p->trapframe->a0;
}

uint64 sys_settickets(void)
{
  int n;
  argint(0, &n);

  // printf("Setting tickets: %d\n", n);

  if (n < 1)
  {
    return -1;
  }

  myproc()->tickets = n;

  // printf("myproc()->tickets: %d\n", myproc()->tickets);
  return 0;
}

// In sysproc.c
uint64 sys_getschedulecount(void)
{
  int pid;
  struct proc *p;

  argint(0, &pid); // Get PID from user space
  // printf("PID: %d\n", pid);

  for (p = proc; p < &proc[NPROC]; p++)
  {
    // printf("p->pid: %d\n", p->pid);
    if (p->pid == pid)
    {
      return p->scheduler_count; // Return the scheduling count for the given PID
    }
  }
  return 0; // Process not found
}

int find_first_set_bit(int mask)
{
  int position = 0;
  while (mask > 0)
  {
    if (mask & 1)
    {
      return position;
    }
    mask >>= 1;
    position++;
  }
  return -1; // If no bits are set
}

// int getSysCount(int mask)
// {
//   int syscall_index = find_first_set_bit(mask); // Find the index of the set bit

//   // Check if the index is valid
//   if (syscall_index < 0 || syscall_index >= 31)
//   {
//     return -1; // Invalid mask
//   }

//   // Reset count for the current process
//   syscall_count[syscall_index] = 0;

//   acquire(&ptable.lock);

//   struct proc *p;
//   for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
//   {
//     if (p->state == RUNNING || p->state == ZOMBIE)
//     {
//       syscall_count[syscall_index] += p->syscall_count[syscall_index];
//     }
//   }

//   release(&ptable.lock);

//   return syscall_count[syscall_index];
// }

// int getSysCount(int mask)
// {
//   // printf("HI\n");
//   int total_count = 0;

//   // Iterate through all bits in the mask
//   acquire(&ptable.lock);
//   for (int i = 0; i < 32; i++)
//   {
//     if (mask & (1 << i)) // Check if the bit is set
//     {
//       // Reset count for the current process for this syscall
//       syscall_count[i] = 0; // Assuming this is the correct array for the current process

//       struct proc *p;
//       for (p = proc; p < &proc[NPROC]; p++)
//       {
//         if (p->state == RUNNING)
//         {
//           total_count += p->syscall_count[i]; // Sum counts for only RUNNING processes
//         }
//       }
//     }
//   }
//   release(&ptable.lock);
//   // printf("TOTAL COUNT: %d\n", total_count);

//   return total_count; // Return total count for all syscalls represented in the mask
// }

// int getSysCount(int mask)
// {
//   int total_count = 0;
//   // printf("Entering getSysCount with mask: %d\n", mask);

//   acquire(&ptable.lock);
//   for (int i = 0; i < 32; i++) // Iterate through all syscalls
//   {
//     if (mask & (1 << i)) // Check if the bit is set
//     {
//       // printf("Checking syscall index: %d\n", i); // Debug print for syscall index
//       struct proc *p;
//       for (p = proc; p < &proc[NPROC]; p++) // Iterate through the process table
//       {
//         // Print the state of each process
//         // printf("Process PID: %d, State: %d, Count for syscall %d: %d\n", p->pid, p->state, i, p->syscall_count[i]);

//         // Count syscalls from running and zombie processes
//         // if (p->state == RUNNING || p->state == ZOMBIE)
//         // {
//         total_count = p->syscall_count[i]; // Aggregate syscall counts

//         // }
//       }

//       if (total_count)
//       {
//         release(&ptable.lock);
//         return total_count;
//       }
//     }
//   }

//   release(&ptable.lock);
//   // printf("Total count for mask %d: %d\n", mask, total_count);
//   return total_count; // Return total count for all syscalls represented in the mask
// }

// Kernel function to get syscall count
// int getSysCount(int mask)
// {
//   struct proc *calling_process = myproc(); // Get the calling process

//   // Acquire lock for process table
//   acquire(&ptable.lock);

//   // Check syscall counts for the calling process and its children
//   for (struct proc *p = proc; p < &proc[NPROC]; p++)
//   {
//     // Check the specific syscall indicated by the mask
//     for (int i = 0; i < 32; i++)
//     {
//       printf("p->syscall_count[%d]: %d\n", i, p->syscall_count[i]);
//       if (mask & (1 << i))
//       {                                  // If this bit is set
//         int count = p->syscall_count[i-1]; // Get the count for the specific syscall

//         release(&ptable.lock); // Release lock before returning
//         return count;          // Return the count for the requested syscall
//       }
//     }
//   }

//   release(&ptable.lock); // Release lock if no count was found
//   return 0;              // Return 0 if the syscall count is not found
// }

// int getSysCount(int mask)
// {
//   struct proc *calling_process = myproc(); // Get the calling process
//   calling_process->mask = mask;            // Set the mask for the calling process

//   // Acquire lock for process table
//   acquire(&ptable.lock);

//   printf("Calling process PID: %d\n", calling_process->pid);

//   // Check if the mask for the requested syscall is set for the calling process
//   for (int i = 0; i < 32; i++)
//   {
//     // If the syscall bit (1 << i) is set in the input mask
//     if (mask & (1 << i))
//     {
//       // Check if the corresponding bit in the process mask is set
//       if (calling_process->mask & (1 << i))
//       {
//         release(&ptable.lock); // Release lock before returning
//         return 1;              // The syscall is being logged (enabled by mask)
//       }
//       else
//       {
//         release(&ptable.lock); // Release lock before returning
//         return 0;              // The syscall is not being logged
//       }
//     }
//   }

//   release(&ptable.lock); // Release lock if no match was found
//   return 0;              // Return 0 if no syscalls are being logged for this mask
// }

// int getSysCount(int mask, int pid)
// {
//   printf("PID: %d\n", pid);
//   struct proc *calling_process; // Get the calling process
//   // calling_process = findproc(pid);
//   calling_process = myproc();

//   if (calling_process == 0)
//   {
//     // Handle case where process is not found
//     printf("Process with PID %d not found\n", pid);
//     return 0; // or other error handling
//   }

//   printf("calling->process->name: %s\n", calling_process->name);
//   calling_process->mask = mask; // Set the mask for the calling process

//   // Acquire lock for process table
//   acquire(&ptable.lock);

//   printf("Calling process PID: %d\n", calling_process->pid);

//   // Variable to store syscall index
//   int syscallIndex = -1;

//   // Check if the mask for the requested syscall is set for the calling process
//   for (int i = 0; i < 32; i++)
//   {
//     // printf("i and mask and 1<<i and calling_process->syscall_counts[i]: %d %d %d %d\n", i, mask, 1 << i, calling_process->syscall_counts[i]);
//     // If the syscall bit (1 << i) is set in the input mask
//     if (mask & (1 << i))
//     {
//       syscallIndex = i; // Record the syscall index
//       // break;            // Exit the loop since we found the corresponding syscall
//     }
//   }

//   // printf("WE ARE IN SYS COUNT\n");
//   // printf("syscallIndex: %d\n", syscallIndex);

//   if (syscallIndex == -1)
//   {
//     release(&ptable.lock); // No valid syscall index found
//     return 0;              // Return 0 if no syscalls are being logged for this mask
//   }

//   // for (int i = 0; i < 31; i++)
//   // {
//   //   printf("syscall_count[%d]: %d\n", i, calling_process->syscall_counts[i]);
//   // }

//   // Retrieve the count for the specific syscall
//   int count = calling_process->syscall_counts[syscallIndex]; // Get count for this syscall

//   release(&ptable.lock); // Release the lock before returning

//   printf("Count for syscall %d: %d\n", syscallIndex, count);

//   return count; // Return the count for the specific syscall
// }

int getSysCount(int mask, int pid)
{
  return 0;
}

int settickets(int tickets)
{
  struct proc *p;
  p->tickets = tickets;

  return tickets;
}

int getschedulercount(int pid)
{
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++)
  {
    printf("p->pid: %d\n", p->pid);

    if (p->pid == pid)
    {
      return p->scheduler_count;
    }
  }

  return 0;
}