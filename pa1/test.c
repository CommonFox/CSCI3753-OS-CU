#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    int *result = new int;
    pid_t tid;

    tid = syscall(327, 1, 2, result);

    //std::cout << tid;

    printf("The result is: %d", *result);

    return 0;
}
