#include <stdio.h>
#include <pthread.h>
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
static __thread int thread_local_storage_value = 246;


void *thread1(void *arg) {
    sleep(2);
    int stack_value = 100;
    unsigned long TLS = (unsigned long)&thread_local_storage_value;
    unsigned long stack = (unsigned long)&stack_value;
    unsigned long lib = (unsigned long)getpid;
    unsigned long heap = (unsigned long)malloc(sizeof(int));
    unsigned long bss = (unsigned long)&bss_value;
    unsigned long data = (unsigned long)&data_value;
    unsigned long code = (unsigned long)code_function;

    int len = 7;
    unsigned long vir_addrs[7] = {TLS, stack, lib, heap, bss, data, code};
    unsigned long phy_addrs[7];

    long copy = syscall(548, vir_addrs, len, phy_addrs, len);
    if (copy < 0) {
        printf("address transfer failed!!");
        exit(1);
    }


    printf("============= thread1 =============\n");
    printf("pid = %d  tid = %d\n", (int)getpid(), (int)gettid());
    printf("segment\tvir_addr\tphy_addr\n");
    printf("TLS\t%lx\t%lx\n", vir_addrs[0], phy_addrs[0]);
    printf("stack\t%lx\t%lx\n", vir_addrs[1], phy_addrs[1]);
    printf("lib\t%lx\t%lx\n", vir_addrs[2], phy_addrs[2]);
    printf("heap\t%lx\t%lx\n", vir_addrs[3], phy_addrs[3]);
    printf("bss\t%lx\t%lx\n", vir_addrs[4], phy_addrs[4]);
    printf("data\t%lx\t%lx\n", vir_addrs[5], phy_addrs[5]);
    printf("code\t%lx\t%lx\n", vir_addrs[6], phy_addrs[6]);

    pthread_exit(NULL); // 離開子執行緒
}

void *thread2(void *arg) {
    sleep(4);
    int stack_value = 200;
    unsigned long TLS = (unsigned long)&thread_local_storage_value;
    unsigned long stack = (unsigned long)&stack_value;
    unsigned long lib = (unsigned long)getpid;
    unsigned long heap = (unsigned long)malloc(sizeof(int));
    unsigned long bss = (unsigned long)&bss_value;
    unsigned long data = (unsigned long)&data_value;
    unsigned long code = (unsigned long)code_function;

    int len = 7;
    unsigned long vir_addrs[7] = {TLS, stack, lib, heap, bss, data, code};
    unsigned long phy_addrs[7];

    long copy = syscall(548, vir_addrs, len, phy_addrs, len);
    if (copy < 0) {
        printf("address transfer failed!!");
        exit(1);
    }


    printf("============= thread2 =============\n");
    printf("pid = %d  tid = %d\n", (int)getpid(), (int)gettid());
    printf("segment\tvir_addr\tphy_addr\n");
    printf("TLS\t%lx\t%lx\n", vir_addrs[0], phy_addrs[0]);
    printf("stack\t%lx\t%lx\n", vir_addrs[1], phy_addrs[1]);
    printf("lib\t%lx\t%lx\n", vir_addrs[2], phy_addrs[2]);
    printf("heap\t%lx\t%lx\n", vir_addrs[3], phy_addrs[3]);
    printf("bss\t%lx\t%lx\n", vir_addrs[4], phy_addrs[4]);
    printf("data\t%lx\t%lx\n", vir_addrs[5], phy_addrs[5]);
    printf("code\t%lx\t%lx\n", vir_addrs[6], phy_addrs[6]);


    pthread_exit(NULL); // 離開子執行緒
}

void *thread3(void *arg) {
    sleep(6);
    int stack_value = 300;
    unsigned long TLS = (unsigned long)&thread_local_storage_value;
    unsigned long stack = (unsigned long)&stack_value;
    unsigned long lib = (unsigned long)getpid;
    unsigned long heap = (unsigned long)malloc(sizeof(int));
    unsigned long bss = (unsigned long)&bss_value;
    unsigned long data = (unsigned long)&data_value;
    unsigned long code = (unsigned long)code_function;

    int len = 7;
    unsigned long vir_addrs[7] = {TLS, stack, lib, heap, bss, data, code};
    unsigned long phy_addrs[7];

    long copy = syscall(548, vir_addrs, len, phy_addrs, len);
    if (copy < 0) {
        printf("address transfer failed!!");
        exit(1);
    }


    printf("============= thread3 =============\n");
    printf("pid = %d  tid = %d\n", (int)getpid(), (int)gettid());
    printf("segment\tvir_addr\tphy_addr\n");
    printf("TLS\t%lx\t%lx\n", vir_addrs[0], phy_addrs[0]);
    printf("stack\t%lx\t%lx\n", vir_addrs[1], phy_addrs[1]);
    printf("lib\t%lx\t%lx\n", vir_addrs[2], phy_addrs[2]);
    printf("heap\t%lx\t%lx\n", vir_addrs[3], phy_addrs[3]);
    printf("bss\t%lx\t%lx\n", vir_addrs[4], phy_addrs[4]);
    printf("data\t%lx\t%lx\n", vir_addrs[5], phy_addrs[5]);
    printf("code\t%lx\t%lx\n", vir_addrs[6], phy_addrs[6]);


    pthread_exit(NULL); // 離開子執行緒
}

// 主程式
int main() {
    pthread_t t1, t2, t3;

    printf("syscall\n"); // Syscall

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_create(&t3, NULL, thread3, NULL);
    
    sleep(8);
    int stack_value = 10;
    unsigned long TLS = (unsigned long)&thread_local_storage_value;
    unsigned long stack = (unsigned long)&stack_value;
    unsigned long lib = (unsigned long)getpid;
    unsigned long heap = (unsigned long)malloc(sizeof(int));
    unsigned long bss = (unsigned long)&bss_value;
    unsigned long data = (unsigned long)&data_value;
    unsigned long code = (unsigned long)code_function;

    int len = 7;
    unsigned long vir_addrs[7] = {TLS, stack, lib, heap, bss, data, code};
    unsigned long phy_addrs[7];

    long copy = syscall(548, vir_addrs, len, phy_addrs, len);
    if (copy < 0) {
        printf("address transfer failed!!");
        exit(1);
    }


    printf("============= main =============\n");
    printf("pid = %d  tid = %d\n", (int)getpid(), (int)gettid());
    printf("segment\tvir_addr\tphy_addr\n");
    printf("TLS\t%lx\t%lx\n", vir_addrs[0], phy_addrs[0]);
    printf("stack\t%lx\t%lx\n", vir_addrs[1], phy_addrs[1]);
    printf("lib\t%lx\t%lx\n", vir_addrs[2], phy_addrs[2]);
    printf("heap\t%lx\t%lx\n", vir_addrs[3], phy_addrs[3]);
    printf("bss\t%lx\t%lx\n", vir_addrs[4], phy_addrs[4]);
    printf("data\t%lx\t%lx\n", vir_addrs[5], phy_addrs[5]);
    printf("code\t%lx\t%lx\n", vir_addrs[6], phy_addrs[6]);


    printf("----------- thread address ------------\n");
    printf("t1 = %p\n", &t1);
    printf("t2 = %p\n", &t2);
    printf("t3 = %p\n", &t3);

    while(1);

    return 0;
}