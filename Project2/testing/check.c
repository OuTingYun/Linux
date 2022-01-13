#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <syscall.h>

int bss_value;
int data_value = 123;
int code_function() {
    return 0;
}

int main() {
    sleep(2);
    int block_input;
    int stack_value = 10;
    unsigned long stack = (unsigned long)&stack_value;
    unsigned long lib = (unsigned long)getpid;
    unsigned long heap = (unsigned long)malloc(sizeof(int));
    unsigned long bss = (unsigned long)&bss_value;
    unsigned long data = (unsigned long)&data_value;
    unsigned long code = (unsigned long)code_function;

    int len = 6;
    unsigned long vir_addrs[6] = {stack, lib, heap, bss, data, code};
    unsigned long phy_addrs[6];

    long copy = syscall(548, vir_addrs, len, phy_addrs, len);
    if (copy < 0) {
        printf("address transfer failed!!");
        exit(1);
    }

    while(1) {
        printf("block...");
        scanf("%d", &block_input);


        printf("============= main =============\n");
        printf("pid = %d\n", (int)getpid());
        printf("segment\tvalue\tvir_addr\tphy_addr\n");
        printf("stack\t%d\t%lx\t%lx\n", stack_value, vir_addrs[0], phy_addrs[0]);
        printf("getpid\t%d\t%lx\t%lx\n", (int)getpid(), vir_addrs[1], phy_addrs[1]);
        printf("heap\t%d\t%lx\t%lx\n", *(int*)heap, vir_addrs[2], phy_addrs[2]);
        printf("bss\t%d\t%lx\t%lx\n", bss_value, vir_addrs[3], phy_addrs[3]);
        printf("data\t%d\t%lx\t%lx\n", data_value, vir_addrs[4], phy_addrs[4]);
        printf("code\t%d\t%lx\t%lx\n", code_function(), vir_addrs[5], phy_addrs[5]);
    }



    return 0;
}