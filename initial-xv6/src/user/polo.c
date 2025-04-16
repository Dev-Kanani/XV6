#include "kernel/types.h"

int main(void)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("Running schedulertest: Iteration %d\n", i + 1);
        if (fork() == 0)
        {
            // Child process runs schedulertest
            char *argv[] = {"schedulertest", 0};
            exec("schedulertest", argv);
            // exec only returns if it fails
            printf("exec schedulertest failed\n");
            exit(1);
        }
        // Wait for child process to finish
        wait(0);
    }

    exit(0);
}
