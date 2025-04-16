#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Main function
int main(int argc, char **argv)
{
    // Check if the correct number of arguments is passed
    if (argc < 3)
    {
        fprintf(2, "%s: execution failed - insufficient number of arguments\n", argv[0]);
        exit(1);
    }

    // Parse the mask from the command line argument
    int mask = atoi(argv[1]);

    // Validate the mask
    if (mask < 0)
    {
        fprintf(2, "%s: execution failed - invalid mask provided\n", argv[0]);
        exit(1);
    }

    getSysCount(mask, getpid()); // Call getSysCount for the specified mask

    exec(argv[2], argv + 2);
}
