# Intro to Xv6

## System calls

- **`syscount`**: it count how many times some syscall happen n prints it for process based on mask.
- **`sigalarm`**: sets a alarm to run handler after some cpu ticks, user gives interval.
- **`sigreturn`**: after handler run, this return process to normal state to continue working.


## Syscount

- `syscount_trace_enabled` is the flag to know if we want the syscount of that process.

   1. **User Program**:  
       Calls `getSysCount(mask, getpid())`.

    2. **uSys.S**:  
       Loads the system call number into a7.

    3. **Kernel (sys_getSysCount) (sysproc.c)**:  
       Handles the system call, processes the mask and pid. Gets the mask and pid using arguments.  
   Here is where the flag is enabled, and the mask of that process is set.

    4. **exit() Function**:  
   The syscall counts get printed.

    5. **syscall.c**:  
   This is where the counter for the syscalls made by the process are increased.


## Sigalarm

- Made the changes in usertrap that if the alarm is active for that process, then first count the time, then if it has reached the limit, alarm is triggered.

- Set alarm is deactivated.

-  The saved kernel state (stored in `alarm_tf`) is restored to the process's current trapframe.

- When the handler did its work, after then it calls the sigreturn.

- The purpose of sys_sigreturn() is to restore the process's original execution state after the alarm handler completes, to resume normal execution from where it left off.

# Scheduling

## Changes in makefile

- when you set SCHEDULER to LBS, it adds -DLBS flag to compile code in lottery-based schedulr
- if SCHEDULER is MLFQ, it adds -DMLFQ flag to use multi-level feedback queue scheduler
- if nothing set, it use -DDEFAULT flag for default round robin in the scheduler

## LBS

- Made the initial tickets of every process = 1

- And the arrival time for that process = the current tick

- Also as we wanted the child process to start with the same number of tickets as the parent, we changed the `fork()` system call for:
  `np->tickets = p->tickets`

- Time slice for 1 second is ensured because the scheduler, every time picks the winner ticket randomly, so after every `ticks++` in the `clockintr()`, we are scheduling a new process randomly again.

- **Finding the winner (for least arrival time) in nested for loop, the preempt was not passing in the usertests.** 

**`What is the implication of adding the arrival time in the lottery based scheduling policy?`**

- As we wanted to give the priority to the process that came earlier if their ticket number is equal to the winner ticket number, that's why we had to keep the arrival timer variable for every process.

- The reason is that to give fairness and prevent the starvation of older process by giving them chance to run in the CPU

**`What happens if all processes have the same number of tickets? `**

- if all the processes have the same tickets, then the FCFS scheduling will take place, as tickets are same, then we are checking for the arrival time minimum, which is the FCFS.

**`Are there any pitfalls to watch out for? `**

- **Fairness**: If many processes have the same number of tickets, then the ones which have arrived the latest will get the least fair and sometimes only the old ones will be executed due to their earlier aarrival time and if they consume more CPU then the newer ones will not get the CPU.

- **Starvation**: If the random number generator(also a pitfall of random number generator) generates only large ticket values due to its nature, then the process with less tickets will starve and not get the CPU.

**In my MLFQ plot, the process are not going to queue 3 beacuse of the boost interval, if I increase the interval, then some process go to queue 3.**


## MLFQ

- Here we have maintained four queues with time slices of each as 1, 4, 8 and 16.

- I have a variable in each process which indicates the queue in which it is currently present.

- I have also have the counter for the number of ticks used by the process in the queue in which it is currently in and i am updating the counter to zero if queue changes or boosted.

- We also the variable for arrival time in the particular queue in which it is currently in.

- For the queues **`0,1,2`** , we have implemented **`FCFS`** and for the queue **`3`** , we have implemented the **`round robin`**, which is the default scheduling policy of XV6.

*If a process voluntarily relinquishes control of the CPU(ex: for doing I/O operations), it leaves the queuing network, and when the process becomes ready again after the I/O operation, it is inserted at the tail of the same queue, from which it is relinquished earlier*

To implement the above:

- we wanted to pop and push the I/O bound process to be at the end of the queue if its time slice is used, we are updating its arrival time in the **`kerneltrap()`** function, which is where the I/O based interrupts are been handled. (the variable this thing in my code is `total_counter_for_ticks` which i am increasing every time when a process is scheduled, its just like ticks)

- Normal ticks do not get updated for the I/O bound processes, thats why i have maintained the variable above to update the arrival time , to make it at the end of the queue.

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the xv6 part of the Mini Project. 
- work inside the `networks/` directory for the Networking part of the Mini Project.

- You are free to delete these instructions and add your report before submitting.

![image](https://github.com/user-attachments/assets/c724819f-e288-48be-bf4c-dfc6cabef336)

## AVG TIMES

| Scheduler | CPUs | RTime | WTime |
|-----------|------|-------|-------|
| LBS       | 2    | 12    | 127   |
| LBS       | 1    | 19    | 180   |
| MLFQ      | 1    | 19    | 162   |
| LBS       | 1    | 19    | 176   |


![image](https://github.com/user-attachments/assets/01a1be42-1bad-4355-9b04-8e5e860b93e4)
![image](https://github.com/user-attachments/assets/4af8cb58-ab22-4bb7-abd5-206150e19d88)
![image](https://github.com/user-attachments/assets/80eb14a2-4b05-401d-a208-490b3a26eebd)
![image](https://github.com/user-attachments/assets/e451bccf-b0c1-462d-a94e-a63e3d2c4d44)



