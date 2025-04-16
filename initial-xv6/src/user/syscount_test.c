#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Function to simulate syscalls in child process
void child_work()
{
    char buf[10];

    // Simulate syscalls in child process
    // Read from standard input (invokes the read syscall)
    read(0, buf, sizeof(buf));

    // Write to standard output (invokes the write syscall)
    write(1, "Testing syscount in child\n", 26);

    // Exit the child process
    exit(0);
}

// Main function to test syscall counting with mask
int main(int argc, char **argv)
{
    // Check if the correct number of arguments is passed
    if (argc < 2)
    {
        printf("Usage: syscount_test <mask>\n");
        exit(1);
    }

    // Parse the mask for syscall tracing
    int mask = atoi(argv[1]);

    // Validate the mask
    if (mask < 0)
    {
        printf("Invalid mask value\n");
        exit(1);
    }

    // Call getSysCount to start tracing for the specified syscalls
    getSysCount(mask, getpid());

    // Fork a child process
    int pid = fork();

    if (pid < 0)
    {
        // Fork failed
        printf("Fork failed!\n");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process
        child_work();
    }
    else
    {
        // Parent process waits for the child to finish
        wait(0);

        // Print syscall count for the parent
        printf("Parent process (PID: %d) finished execution\n", getpid());
        
        // The syscalls invoked in child and parent are printed automatically upon exit due to your syscount mechanism
        exit(0);
    }

    return 0;
}
