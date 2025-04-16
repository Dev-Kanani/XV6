// // #include "kernel/param.h"
// // #include "kernel/types.h"
// // #include "kernel/stat.h"
// // #include "kernel/riscv.h"
// // #include "user/user.h"

// // void big_test();
// // void signal_handler();
// // void nested_handler();
// // void syscall_handler();
// // void child_process_test();
// // void multithread_test();
// // void periodic();

// // // Global variable for tracking alarm handler calls.
// // volatile static int handler_count;
// // volatile static int syscall_triggered = 0;
// // volatile static int nested_count = 0;
// // volatile static int child_signal_triggered = 0;

// // int main(int argc, char *argv[])
// // {
// //     big_test();
// //     exit(0);
// // }

// // // Simple periodic alarm signal handler.
// // void periodic()
// // {
// //     handler_count++;
// //     printf("Periodic alarm! Count = %d\n", handler_count);
// //     sigreturn();
// // }

// // // Signal handler for nested alarms.
// // void nested_handler()
// // {
// //     nested_count++;
// //     printf("Nested alarm! Nested count = %d\n", nested_count);

// //     // Trigger a second signal while handling the first one.
// //     if (nested_count == 1)
// //         sigalarm(1, nested_handler); // Trigger a new alarm with a smaller interval.

// //     sigreturn();
// // }

// // // Signal handler that interacts with a syscall.
// // void syscall_handler()
// // {
// //     syscall_triggered++;
// //     printf("Syscall triggered alarm! Syscall count = %d\n", syscall_triggered);

// //     // Perform a syscall within the handler to see interaction.
// //     if (syscall_triggered == 2)
// //         printf("Syscall inside alarm handler.\n");

// //     sigreturn();
// // }

// // // Signal handler for child process test.
// // void child_signal_handler()
// // {
// //     child_signal_triggered++;
// //     printf("Child process signal handler triggered. Count = %d\n", child_signal_triggered);
// //     sigreturn();
// // }

// // // Test involving multiple nested alarms, syscalls, and child processes.
// // void big_test()
// // {
// //     printf("Big test start\n");
// //     handler_count = 0;
// //     nested_count = 0;
// //     syscall_triggered = 0;
// //     child_signal_triggered = 0;

// //     // Part 1: Periodic alarm test
// //     printf("Part 1: Testing periodic alarms.\n");
// //     sigalarm(2, periodic); // Set alarm for every 2 ticks.

// //     for (int i = 0; i < 500000000; i++)
// //     {
// //         if (handler_count >= 5)
// //             break;
// //     }

// //     if (handler_count >= 5)
// //         printf("Periodic alarm test passed.\n");
// //     else
// //         printf("Periodic alarm test failed.\n");

// //     // Part 2: Nested alarm signal test
// //     printf("Part 2: Testing nested alarms.\n");
// //     sigalarm(4, nested_handler);

// //     for (int i = 0; i < 500000000; i++)
// //     {
// //         if (nested_count >= 2)
// //             break;
// //     }

// //     if (nested_count >= 2)
// //         printf("Nested alarm test passed.\n");
// //     else
// //         printf("Nested alarm test failed.\n");

// //     // Part 3: Alarm during syscalls
// //     printf("Part 3: Testing alarms during syscalls.\n");
// //     sigalarm(2, syscall_handler);

// //     for (int i = 0; i < 500000000; i++)
// //     {
// //         if (syscall_triggered >= 3)
// //             break;
// //     }

// //     if (syscall_triggered >= 3)
// //         printf("Syscall alarm test passed.\n");
// //     else
// //         printf("Syscall alarm test failed.\n");

// //     // Part 4: Fork and signal handling in child process.
// //     printf("Part 4: Testing signal handling in child process.\n");

// //     int pid = fork();
// //     if (pid == 0)
// //     {
// //         // Child process
// //         sigalarm(1, child_signal_handler);
// //         for (int i = 0; i < 50000000; i++)
// //         {
// //             if (child_signal_triggered >= 3)
// //                 break;
// //         }

// //         if (child_signal_triggered >= 3)
// //         {
// //             printf("Child process alarm test passed.\n");
// //         }
// //         else
// //         {
// //             printf("Child process alarm test failed.\n");
// //         }
// //         exit(0);
// //     }
// //     else
// //     {
// //         wait(0);
// //         printf("Parent process continues.\n");
// //     }

// //     // Part 5: Test for multiple handlers across threads (if applicable)
// //     printf("Part 5: Testing multiple threads (if applicable).\n");
// //     multithread_test(); // Stub function to simulate multithreading tests, could be implemented differently.

// //     printf("Big test finished.\n");
// // }

// // void multithread_test()
// // {
// //     // This is a stub for multithreaded alarms; implement based on your system's capability.
// //     // This can involve forking multiple child processes and testing alarm delivery to each.
// //     printf("Multithread test: Currently not implemented.\n");
// // }

// #include "kernel/param.h"
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "kernel/riscv.h"
// #include "user/user.h"

// void big_test();
// void signal_handler();
// void nested_handler();
// void syscall_handler();
// void child_process_test();
// void multithread_test();
// void periodic();

// // Global variable for tracking alarm handler calls.
// volatile static int handler_count;
// volatile static int syscall_triggered = 0;
// volatile static int nested_count = 0;
// volatile static int child_signal_triggered = 0;

// int main(int argc, char *argv[])
// {
//     big_test();
//     exit(0);
// }

// // Simple periodic alarm signal handler.
// void periodic()
// {
//     handler_count++;
//     printf("[Periodic Handler] Alarm! Count = %d\n", handler_count);
//     sigreturn();
// }

// // Signal handler for nested alarms.
// void nested_handler()
// {
//     nested_count++;
//     printf("[Nested Handler] Alarm! Nested count = %d\n", nested_count);

//     // Trigger a second signal while handling the first one.
//     if (nested_count == 1)
//     {
//         printf("[Nested Handler] Triggering a second alarm inside the first one.\n");
//         sigalarm(1, nested_handler); // Trigger a new alarm with a smaller interval.
//     }

//     sigreturn();
// }

// // Signal handler that interacts with a syscall.
// void syscall_handler()
// {
//     syscall_triggered++;
//     printf("[Syscall Handler] Alarm! Syscall count = %d\n", syscall_triggered);

//     // Perform a syscall within the handler to see interaction.
//     if (syscall_triggered == 2)
//     {
//         printf("[Syscall Handler] Performing syscall inside alarm handler.\n");
//     }

//     sigreturn();
// }

// // Signal handler for child process test.
// void child_signal_handler()
// {
//     child_signal_triggered++;
//     printf("[Child Process Handler] Alarm! Count = %d\n", child_signal_triggered);
//     sigreturn();
// }

// // Test involving multiple nested alarms, syscalls, and child processes.
// void big_test()
// {
//     printf("=== Big Test Start ===\n");
//     handler_count = 0;
//     nested_count = 0;
//     syscall_triggered = 0;
//     child_signal_triggered = 0;

//     // Part 1: Periodic alarm test
//     printf("\n--- Part 1: Periodic Alarm Test ---\n");
//     sigalarm(2, periodic); // Set alarm for every 2 ticks.

//     for (int i = 0; i < 500000000; i++)
//     {
//         if (handler_count >= 5)
//             break;
//     }

//     if (handler_count >= 5)
//         printf("[Test Result] Periodic alarm test passed. Handler count: %d\n", handler_count);
//     else
//         printf("[Test Result] Periodic alarm test failed. Handler count: %d\n", handler_count);

//     // Part 2: Nested alarm signal test
//     printf("\n--- Part 2: Nested Alarm Test ---\n");
//     sigalarm(4, nested_handler);

//     for (int i = 0; i < 500000000; i++)
//     {
//         if (nested_count >= 2)
//             break;
//     }

//     if (nested_count >= 2)
//         printf("[Test Result] Nested alarm test passed. Nested count: %d\n", nested_count);
//     else
//         printf("[Test Result] Nested alarm test failed. Nested count: %d\n", nested_count);

//     // Part 3: Alarm during syscalls
//     printf("\n--- Part 3: Syscall During Alarm Test ---\n");
//     sigalarm(2, syscall_handler);

//     for (int i = 0; i < 500000000; i++)
//     {
//         if (syscall_triggered >= 3)
//             break;
//     }

//     if (syscall_triggered >= 3)
//         printf("[Test Result] Syscall alarm test passed. Syscall triggered: %d\n", syscall_triggered);
//     else
//         printf("[Test Result] Syscall alarm test failed. Syscall triggered: %d\n", syscall_triggered);

//     // Part 4: Fork and signal handling in child process.
//     printf("\n--- Part 4: Child Process Signal Handling Test ---\n");

//     int pid = fork();
//     if (pid == 0)
//     {
//         // Child process
//         printf("[Child Process] Starting child process signal test.\n");
//         sigalarm(1, child_signal_handler);
//         for (int i = 0; i < 50000000; i++)
//         {
//             if (child_signal_triggered >= 3)
//                 break;
//         }

//         if (child_signal_triggered >= 3)
//         {
//             printf("[Test Result] Child process alarm test passed. Child signal triggered: %d\n", child_signal_triggered);
//         }
//         else
//         {
//             printf("[Test Result] Child process alarm test failed. Child signal triggered: %d\n", child_signal_triggered);
//         }
//         exit(0);
//     }
//     else
//     {
//         wait(0);
//         printf("[Parent Process] Child process completed.\n");
//     }

//     // Part 5: Test for multiple handlers across threads (if applicable)
//     printf("\n--- Part 5: Multithreaded Alarm Test (Stub) ---\n");
//     multithread_test(); // Stub function to simulate multithreading tests, could be implemented differently.

//     printf("\n=== Big Test Finished ===\n");
// }

// void multithread_test()
// {
//     // This is a stub for multithreaded alarms; implement based on your system's capability.
//     // This can involve forking multiple child processes and testing alarm delivery to each.
//     printf("[Multithread Test] Currently not implemented.\n");
// }

#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

// Function prototypes
void big_test();
void periodic_handler();
void nested_handler();
void syscall_handler();
void io_handler();
void child_process_test();
void multithread_test();
void stress_test();
void edge_case_test();
void back_to_back_test();

// Global variables for tracking
volatile static int handler_count;
volatile static int syscall_count = 0;
volatile static int nested_count = 0;
volatile static int child_signal_count = 0;
volatile static int io_triggered = 0;
volatile static int stress_signal_count = 0;

int main(int argc, char *argv[])
{
    big_test();
    exit(0);
}

// Simple periodic alarm signal handler
void periodic_handler()
{
    handler_count++;
    printf("[Periodic Handler] Periodic alarm fired! Count = %d\n", handler_count);
    sigreturn();
}

// Signal handler for nested alarms
void nested_handler()
{
    nested_count++;
    printf("[Nested Handler] Alarm fired! Nested count = %d\n", nested_count);

    if (nested_count == 1)
    {
        printf("[Nested Handler] Triggering a nested alarm.\n");
        sigalarm(1, nested_handler); // Trigger a new alarm while handling the first one.
    }

    sigreturn();
}

// Signal handler interacting with a syscall
void syscall_handler()
{
    syscall_count++;
    printf("[Syscall Handler] Alarm fired! Syscall count = %d\n", syscall_count);

    if (syscall_count == 2)
    {
        printf("[Syscall Handler] Making a syscall inside alarm handler.\n");
        // Example syscall
        int x = uptime();
        printf("[Syscall Handler] Syscall (uptime) returned: %d\n", x);
    }

    sigreturn();
}

// Signal handler triggered during an I/O operation
void io_handler()
{
    io_triggered++;
    printf("[IO Handler] Alarm fired during I/O! Triggered count = %d\n", io_triggered);
    sigreturn();
}

// Stress test handler
void stress_test_handler()
{
    stress_signal_count++;
    if (stress_signal_count % 100 == 0)
    {
        printf("[Stress Test] Stress signal count: %d\n", stress_signal_count);
    }
    sigreturn();
}

// Back-to-back alarm handler
void back_to_back_handler()
{
    printf("[Back-to-Back Handler] Alarm fired back-to-back!\n");
    sigalarm(1, back_to_back_handler); // Reset alarm immediately
    sigreturn();
}

// Big test function covering multiple cases
void big_test()
{
    printf("=== Big Test Start ===\n");
    handler_count = 0;
    nested_count = 0;
    syscall_count = 0;
    child_signal_count = 0;
    io_triggered = 0;
    stress_signal_count = 0;

    // Part 1: Periodic alarm test
    printf("\n--- Part 1: Periodic Alarm Test ---\n");
    sigalarm(2, periodic_handler); // Set periodic alarm (2 ticks)

    for (int i = 0; i < 500000000; i++)
    {
        if (handler_count >= 5)
            break;
    }

    if (handler_count >= 5)
        printf("[Test Result] Periodic alarm test passed.\n");
    else
        printf("[Test Result] Periodic alarm test failed.\n");

    // Part 2: Nested alarms
    printf("\n--- Part 2: Nested Alarms Test ---\n");
    sigalarm(4, nested_handler);

    for (int i = 0; i < 500000000; i++)
    {
        if (nested_count >= 2)
            break;
    }

    if (nested_count >= 2)
        printf("[Test Result] Nested alarm test passed.\n");
    else
        printf("[Test Result] Nested alarm test failed.\n");

    // Part 3: Syscalls within alarms
    printf("\n--- Part 3: Syscall During Alarm Test ---\n");
    sigalarm(2, syscall_handler);

    for (int i = 0; i < 500000000; i++)
    {
        if (syscall_count >= 3)
            break;
    }

    if (syscall_count >= 3)
        printf("[Test Result] Syscall alarm test passed.\n");
    else
        printf("[Test Result] Syscall alarm test failed.\n");

    // Part 4: Signals during I/O operations
    printf("\n--- Part 4: I/O Alarm Test ---\n");
    printf("[Test] Triggering alarm during sleep (simulating I/O).\n");
    sigalarm(2, io_handler);
    sleep(10); // Simulate an I/O bound process (using sleep)

    if (io_triggered > 0)
        printf("[Test Result] I/O alarm test passed.\n");
    else
        printf("[Test Result] I/O alarm test failed.\n");

    // Part 5: Child process signal handling
    printf("\n--- Part 5: Child Process Signal Handling ---\n");

    int pid = fork();
    if (pid == 0)
    {
        printf("[Child Process] Starting child alarm test.\n");
        sigalarm(1, periodic_handler);

        for (int i = 0; i < 50000000; i++)
        {
            if (handler_count >= 3)
                break;
        }

        if (handler_count >= 3)
            printf("[Child Test Result] Child alarm test passed.\n");
        else
            printf("[Child Test Result] Child alarm test failed.\n");
        exit(0);
    }
    else
    {
        wait(0);
        printf("[Parent Process] Child process completed.\n");
    }

    // Part 6: Edge case test - back-to-back alarms
    printf("\n--- Part 6: Back-to-Back Alarm Test ---\n");
    sigalarm(1, back_to_back_handler);

    for (int i = 0; i < 1000000000; i++)
    {
        if (handler_count >= 5)
            break;
    }

    printf("[Test Result] Back-to-back alarm test: %d alarms handled.\n", handler_count);

    if (handler_count >= 5)
        printf("[Test Result] Back-to-back alarm test passed.\n");
    else
        printf("[Test Result] Back-to-back alarm test failed.\n");

    printf("[Test Result] Back-to-back alarm test finished.\n");

    // Part 7: Stress test with high-volume alarms
    printf("\n--- Part 7: Stress Test ---\n");
    sigalarm(1, stress_test_handler);

    for (int i = 0; i < 1000000000; i++)
    {
        if (stress_signal_count >= 1000)
            break;
    }

    printf("[Test Result] Stress test: %d signals handled.\n", stress_signal_count);
    if (stress_signal_count >= 1000)
        printf("[Test Result] Stress test passed (1000 signals handled).\n");
    else
        printf("[Test Result] Stress test failed.\n");

    // Part 8: Simulated multithreading (if applicable)
    printf("\n--- Part 8: Multithreaded Alarm Test (Simulated with Fork) ---\n");
    multithread_test();

    printf("\n=== Big Test Finished ===\n");
}

void multithread_test()
{
    int num_threads = 3;
    printf("[Multithread Test] Simulating %d processes with alarms.\n", num_threads);

    for (int i = 0; i < num_threads; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            sigalarm(2, periodic_handler);

            for (int j = 0; j < 500000000; j++)
            {
                if (handler_count >= 3)
                    break;
            }

            printf("[Multithread Test] Process %d handled %d alarms.\n", getpid(), handler_count);
            exit(0);
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        wait(0);
    }

    printf("[Multithread Test] All child processes completed.\n");
}
