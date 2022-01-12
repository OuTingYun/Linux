#include <syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

struct data_segment {
    unsigned long start_code;
    unsigned long end_code;
};

int main() {
    struct data_segment my_data_segment;

    pid_t pid = getpid();
    int a = syscall(548, pid, (void*)&my_data_segment);

    printf("pid=> %d\n", pid);
    printf("text segment => Start: %lx\nEnd: %lx\n", my_data_segment.start_code, my_data_segment.end_code);

    while(1);

    return a;
}