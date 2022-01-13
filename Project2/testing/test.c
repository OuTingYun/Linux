#include <syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

int main() {
    
    int vir_len = 3;
    int a=0, b=0, c=0;
    uintptr_t vir_addr[3] = {&a, &b, &c};

    int i=0;
    for(i=0;i<vir_len;i++) {
        printf("i = %d ; vir_addr = %lx\n", i, vir_addr[i]);
    }

    int phy_len = 3;
    uintptr_t phy_addr[3];

    int copy = syscall(548, vir_addr, vir_len, phy_addr, phy_len);

    printf("test\n");
    printf("len_phs = %d\n", phy_len);
    
    for(i=0;i<vir_len;i++) {
        printf("i = %d ; vir_addr = %lx ; phy_addr = %lx\n", i, vir_addr[i], phy_addr[i]);
    }

    while(1);

    return a;
}