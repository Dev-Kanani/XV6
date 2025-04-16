#include "kernel/types.h"
#include "user/user.h"

void child_process(int tickets)
{
    // Each child process requests a specific number of tickets
    int newTicketNum = settickets(tickets);
    if (newTicketNum == -1)
    {
        fprintf(2, "Could not change tickets to %d for process with pid %d\n", tickets, getpid());
    }
    // Simulate some work
    for (int i = 0; i < tickets * 100; i++)
    {
        // printf("Process %d with %d tickets is running\n", getpid(), tickets);
        // sleep(1); // Sleep for 1 second
        // printf("\n");
    }

    exit(0); // Exit the child process
}

int main(int argc, char *argv[])
{
    // Create child processes with different numbers of tickets
    int tickets[3] = {10, 20, 30}; // Ticket values for each process
    int pids[3];

    for (int i = 0; i < 3; i++)
    {
        pids[i] = fork(); // Create child process
        if (pids[i] == 0)
        {
            child_process(tickets[i]); // Call child process function
        }
        else
        {
            // wait(0);
        }
    }

    // Parent process waits for child processes to finish
    for (int i = 0; i < 3; i++)
    {
        wait(0);
    }

    printf("All child processes have finished.\n");
    exit(0); // Exit the parent process
}
