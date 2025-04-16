#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main() {
    // Write "Hello" to stdout 5 times
    for (int i = 0; i < 10; i++) {
        write(1, "Hello\n", 6); // 6 includes the null terminator
    }
    exit(0);
}
