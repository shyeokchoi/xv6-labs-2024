// clang-format off
#include "kernel/types.h"
#include "kernel/stat.h"
// clang-format on
#include "user/user.h"

#define STDOUT_FD 1

int main() {
    int p[2];
    char dummy = 'd';  // dummy byte to exchange between parent and child
    char buf[100];     // temp buffer for the child

    pipe(p);
    if (fork() == 0) {
        // chile process
        close(p[1]);
        read(p[0], buf, 1);
        close(p[0]);
        fprintf(STDOUT_FD, "%d: received ping\n", getpid());
        exit(0);
    }

    // parent process
    close(p[0]);
    write(p[1], &dummy, 1);
    close(p[1]);
    wait(0);
    fprintf(STDOUT_FD, "%d: received pong\n", getpid());
    exit(0);
}
