#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// 子執行緒函數
void *child(void *data)
{
    int time = strtol((char *)data, NULL, 10);
    printf("%d start\n", time);
    sleep(time);
    printf("%d end\n", time);
    pthread_exit(NULL); // 離開子執行緒
}

// 主程式
int main()
{
    pthread_t t1, t2, t3;

    printf("syscall\n"); // Syscall

    pthread_create(&t1, NULL, child, "5");
    pthread_create(&t2, NULL, child, "10");
    pthread_create(&t3, NULL, child, "15");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}