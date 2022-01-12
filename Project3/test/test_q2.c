#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#define NUMBER_OF_IO_ITERATIONS 6
#define NUMBER_OF_ITERATIONS 99999999

int main()
{
    char c;
    int i, t = 2, u = 3, v;
    unsigned int w;

    for (i = 0; i < NUMBER_OF_IO_ITERATIONS; i++) {
        v = 1;
        c = getchar();
    }

    for (i = 0; i < NUMBER_OF_ITERATIONS; i++)
        v = (++t) * (u++);

    long syscall548Result = syscall(548, &w);
    if(syscall548Result!=0)
        printf("Error (1)!\n");
    else
        printf("This process encounters %u times context switches.\n", w);

    long syscall549Result = syscall(549, &w);
    if(syscall549Result!=0)
        printf("Error (2)!\n");
    else
        printf("This process enters a wait queue %u times.\n", w);

    for (i = 0; i < NUMBER_OF_IO_ITERATIONS; i++) {
        v = 1;
        printf("I love my home.\n");
    }

    syscall549Result = syscall(549, &w);
    if(syscall549Result!=0)
        printf("Error (3)!\n");
    else
        printf("This process enters a wait queue %u times.\n", w);

    printf("pid=%d\n", getpid());
    while(1);
}
