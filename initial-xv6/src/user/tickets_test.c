// // #include "types.h"
// // #include "user.h"

// // #define NUM_CHILDREN 15
// // #define TICK_COUNT 100

// // #define O_RDONLY 0x000
// // #define O_WRONLY 0x001
// // #define O_RDWR 0x002
// // #define O_CREATE 0x200
// // #define O_TRUNC 0x400

// // int schedule_count[NUM_CHILDREN];

// // void spin_for_ticks(int ticks)
// // {
// //     int start_ticks = uptime();
// //     while (uptime() - start_ticks < ticks)
// //     {
// //     }
// // }

// // #include "types.h"
// // #include "user.h"

// // // Function to convert an integer to a string
// // void itoa(int num, char *str)
// // {
// //     int i = 0;
// //     int is_negative = 0;

// //     // Handle 0 explicitly
// //     if (num == 0)
// //     {
// //         str[i++] = '0';
// //         str[i] = '\0';
// //         return;
// //     }

// //     // Handle negative numbers if needed
// //     if (num < 0)
// //     {
// //         is_negative = 1;
// //         num = -num;
// //     }

// //     // Process individual digits
// //     while (num != 0)
// //     {
// //         str[i++] = (num % 10) + '0'; // Convert digit to character
// //         num /= 10;
// //     }

// //     // If negative, add '-'
// //     if (is_negative)
// //     {
// //         str[i++] = '-';
// //     }

// //     str[i] = '\0'; // Null-terminate the string

// //     // Reverse the string (since digits are stored in reverse order)
// //     for (int j = 0; j < i / 2; j++)
// //     {
// //         char temp = str[j];
// //         str[j] = str[i - j - 1];
// //         str[i - j - 1] = temp;
// //     }
// // }

// // // Function to format strings like snprintf
// // int custom_snprintf(char *buffer, int size, const char *format, int arg1, int arg2)
// // {
// //     char temp1[10], temp2[10]; // Buffers for integers
// //     itoa(arg1, temp1);         // Convert first integer to string
// //     itoa(arg2, temp2);         // Convert second integer to string

// //     // Manually copy formatted string to buffer
// //     int pos = 0;
// //     const char *child_prefix = "Child ";
// //     const char *iteration_prefix = ": Iteration ";

// //     // Add "Child " to buffer
// //     for (int i = 0; child_prefix[i] != '\0' && pos < size - 1; i++)
// //     {
// //         buffer[pos++] = child_prefix[i];
// //     }

// //     // Add child ID to buffer
// //     for (int i = 0; temp1[i] != '\0' && pos < size - 1; i++)
// //     {
// //         buffer[pos++] = temp1[i];
// //     }

// //     // Add ": Iteration " to buffer
// //     for (int i = 0; iteration_prefix[i] != '\0' && pos < size - 1; i++)
// //     {
// //         buffer[pos++] = iteration_prefix[i];
// //     }

// //     // Add iteration number to buffer
// //     for (int i = 0; temp2[i] != '\0' && pos < size - 1; i++)
// //     {
// //         buffer[pos++] = temp2[i];
// //     }

// //     // Add newline character
// //     if (pos < size - 1)
// //     {
// //         buffer[pos++] = '\n';
// //     }
// //     buffer[pos] = '\0'; // Null-terminate the string

// //     return pos; // Return length of formatted string
// // }

// // int main(void)
// // {
// //     int pid;
// //     int status; // Variable to store the exit status of child processes
// //     char filename[16];

// //     printf("Starting lottery scheduling test...\n");

// //     for (int i = 0; i < NUM_CHILDREN; i++)
// //     {
// //         pid = fork();
// //         if (pid < 0)
// //         {
// //             printf("Fork failed!\n");
// //             exit(1); // Exit with failure if fork fails
// //         }
// //         else if (pid == 0)
// //         {
// //             // Child process
// //             int tickets = (i + 1) * 30; // Assigning increasing tickets
// //             settickets(tickets);
// //             printf("Child %d: Tickets set to %d\n", getpid(), tickets); // Debug output

// //             // Create the filename manually
// //             filename[0] = 'c';
// //             filename[1] = 'h';
// //             filename[2] = 'i';
// //             filename[3] = 'l';
// //             filename[4] = 'd';
// //             filename[5] = '0'; // Convert int to char
// //             filename[6] = '.';
// //             filename[7] = 't';
// //             filename[8] = 'x';
// //             filename[9] = 't';
// //             filename[10] = '\0'; // Null-terminate the string

// //             int fd = open(filename, O_RDWR | O_CREATE); // Open the file
// //             if (fd < 0)
// //             {
// //                 printf("Error opening log file for child %d\n", i + 1);
// //                 exit(1); // Exit with failure if file operation fails
// //             }

// //             // Perform fewer iterations with reduced work time
// //             for (int j = 0; j < TICK_COUNT / 50; j++)
// //             {
// //                 char buffer[64]; // Ensure buffer is large enough
// //                 int len = custom_snprintf(buffer, sizeof(buffer), "Child %d: Iteration %d\n", getpid(), j);
// //                 write(fd, buffer, len); // Write to file

// //                 // Directly format the string
// //                 spin_for_ticks(2); // Reduced spinning time
// //             }

// //             char buffer[64]; // Ensure buffer is large enough
// //             int len = custom_snprintf(buffer, sizeof(buffer), "%d: counts %d\n", getpid(), schedule_count[i]);
// //             write(fd, buffer, len);
// //             close(fd);

// //             schedule_count[i] = getschedulercount(getpid()); // Get the scheduling count

// //             exit(schedule_count[i]); // Child process exits successfully after work
// //         }
// //     }

// //     // Parent process waits for all children to finish
// //     for (int i = 0; i < NUM_CHILDREN; i++)
// //     {
// //         int finished_pid = wait(&status); // Wait for a child to finish
// //         schedule_count[i] = status;       // Store the exit status
// //         printf("Parent: Child with PID %d finished with count %d.\n", finished_pid, schedule_count[i]);
// //     }

// //     int pass = 1;
// //     for (int i = 0; i < NUM_CHILDREN; i++)
// //     {
// //         if (schedule_count[i] <= schedule_count[i - 1])
// //         {
// //             pass = 0;
// //             break;
// //         }
// //     }

// //     if (pass)
// //     {
// //         printf("\nTest PASSED: Processes with more tickets were scheduled more frequently!\n");
// //     }
// //     else
// //     {
// //         printf("\nTest FAILED: Processes were not scheduled according to ticket count.\n");
// //     }

// //     exit(0); // Parent exits successfully after the test
// // }

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROCS 5

void cpu_intensive_task(int id, int loops)
{
    for (int i = 0; i < loops; i++)
    {
        for (volatile int j = 0; j < 10000000; j++)
        {
            // Busy waiting to simulate CPU load
        }
    }
}

void mixed_task(int id, int loops)
{
    for (int i = 0; i < loops; i++)
    {
        if (i % 2 == 0)
        {
            // Busy wait (CPU task)
            for (volatile int j = 0; j < 5000000; j++)
            {
                // Busy waiting to simulate CPU load
            }
        }
        else
        {
            // Sleep for a bit (I/O task)
            sleep(5);
        }
    }
}

void io_task(int id, int loops)
{
    for (int i = 0; i < loops; i++)
    {
        sleep(10); // Simulate I/O task by sleeping
    }
}

int main(void)
{
    int pid;

    // Spawn CPU-intensive processes
    for (int i = 0; i < NUM_PROCS; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            if (i < NUM_PROCS / 2)
            {
                // Half of the processes are CPU intensive
                cpu_intensive_task(i, 5);
            }
            else
            {
                // Half are mixed tasks (CPU + I/O)
                mixed_task(i, 5);
            }
            exit(0);
        }
    }

    // Wait for all processes to finish
    for (int i = 0; i < NUM_PROCS; i++)
    {
        wait(0);
    }

    // Spawn an I/O-intensive process after CPU ones finish
    pid = fork();
    if (pid == 0)
    {
        io_task(NUM_PROCS, 5);
        exit(0);
    }

    // Wait for I/O-intensive process to finish
    wait(0);

    exit(0);
}

// #include "kernel/types.h"
// #include "kernel/stat.h"

// void cpu_bound_process(int duration)
// {
//     // This function simulates a CPU-bound process
//     for (int i = 0; i < duration * 100000000; i++)
//     {
//         asm("nop"); // Do nothing (consume CPU cycles)
//     }
//     exit();
// }

// void io_bound_process(int cycles)
// {
//     // This function simulates an I/O-bound process by alternating between CPU and sleep
//     for (int i = 0; i < cycles; i++)
//     {
//         for (int j = 0; j < 100000000; j++)
//         {
//             asm("nop"); // Simulate short CPU burst
//         }
//         sleep(5); // Simulate I/O wait (sleep for some ticks)
//     }
//     exit();
// }

// int main()
// {
//     // Create a CPU-bound process
//     if (fork() == 0)
//     {
//         cpu_bound_process(30); // Long-running CPU-bound process
//     }

//     // Create another CPU-bound process but with shorter duration
//     if (fork() == 0)
//     {
//         cpu_bound_process(20); // Shorter CPU-bound process
//     }

//     // Create an I/O-bound process
//     if (fork() == 0)
//     {
//         io_bound_process(50); // I/O-bound process (alternates between CPU and I/O)
//     }

//     // Create another I/O-bound process but with more cycles
//     if (fork() == 0)
//     {
//         io_bound_process(80); // More I/O cycles
//     }

//     // Wait for all processes to complete
//     for (int i = 0; i < 4; i++)
//     {
//         wait();
//     }

//     exit();
// }
