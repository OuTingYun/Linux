# Linux Project2 WriteUp

> 第 23 組
107502533 張文耀
107502502 林欣蓓
107502504 歐亭昀
> 

# 目標

實作一個 system call ，功能是將複數個 virtual address 轉為 physical address ，並利用這個 system call 來檢查不同的 thread/process 的哪些 segment 會共用 memory。

# System call

- 我們的 system call
    
    ```c
    #include <linux/kernel.h>
    #include <linux/module.h>
    #include <linux/init.h>
    #include <linux/uaccess.h>
    #include <linux/types.h>
    #include <linux/string.h>
    #include <linux/mm.h>
    #include <linux/mm_types.h>
    #include <linux/sched.h>
    #include <linux/syscalls.h>
    #include <linux/init_task.h>
    #include <linux/pgtable.h>
    #include <linux/delay.h>
    #include <asm/io.h>
    
    unsigned long vir2phy(unsigned long vir_addr) {
    
        pgd_t* pgd;
        p4d_t* p4d;
        pud_t* pud;
        pmd_t* pmd;
        pte_t* pte;
        unsigned long phy_addr = 0;
        unsigned long page_addr = 0;
        unsigned long page_offset = 0;
    
        pgd = pgd_offset(current->mm, vir_addr);
        printk("pgd_val = 0x%lx, pgd_index = %lu\n", pgd_val(*pgd), pgd_index(vir_addr));
        if(pgd_none(*pgd)) {
            printk("not mapped in pgd\n");
            return -1;
        }
    
        p4d = p4d_offset(pgd, vir_addr);
        printk("p4d_val = 0x%lx, p4d_index = %lu\n", p4d_val(*p4d), p4d_index(vir_addr));
        if(p4d_none(*p4d)) {
            printk("not mapped in p4d\n");
            return -1;
        }
    
        pud = pud_offset(p4d, vir_addr);
        printk("pud_val = 0x%lx, pud_index = %lu\n", pud_val(*pud), pud_index(vir_addr));
        if(pud_none(*pud)) {
            printk("not mapped in pud\n");
            return -1;
        }
    
        pmd = pmd_offset(pud, vir_addr);
        printk("pmd_val = 0x%lx, pmd_index = %lu\n", pmd_val(*pmd), pmd_index(vir_addr));
        if(pmd_none(*pmd)) {
            printk("not mapped in pmd\n");
            return -1;
        }
    
        pte = pte_offset_map(pmd, vir_addr);
        printk("pte_val = 0x%lx, ptd_index = %lu\n", pte_val(*pte), pte_index(vir_addr));
        if(pte_none(*pte)) {
            printk("not mapped in pte\n");
            return -1;
        }
    
        struct page *pg = pte_page(*pte);
        page_addr = page_to_phys(pg);
        page_offset = vir_addr & ~PAGE_MASK;
        phy_addr = page_addr | page_offset;
        
    
        printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
        printk("vir_addr = %lx, phy_addr = %lx\n", vir_addr, phy_addr);
    
        return phy_addr;
    }
    
    SYSCALL_DEFINE4(my_get_physical_addresses, unsigned long*, initial, int, len_vir, unsigned long*, result, int, len_phy) {
    
        unsigned long vir_addr[len_vir];
        unsigned long phy_addr[len_vir];
    
        long vir_copy = copy_from_user(vir_addr, initial, sizeof(unsigned long)*len_vir);
    
        printk("len_vir = %d", len_vir);
        
        int i=0;
        for(i=0;i<len_vir;i++) {
            printk("i = %d", i);
            phy_addr[i] = vir2phy(vir_addr[i]);
        }
    
        long phy_copy = copy_to_user(result, phy_addr, sizeof(unsigned long)*len_vir);
        long phy_len_copy = copy_to_user(&len_phy, &len_vir, sizeof(int));    
    
        return 0;
    }
    ```
    
- 圖片支援
    
    ![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled.png)
    

先在 kernel stack 開兩個 array 一個放 virtual addresses 一個放 physical addresses ，用 copy_from_user 把傳進來的 addresses 複製到 kernel，接著用以下的 macro :

- pgd_offset
- p4d_offset
- pud_offset
- pmd_offset
- pte_offset_map

把 virtual address 轉成 physical address，再將結果用 copy_to_user 回傳給 user space。

# 驗證 physical address

我們使用 devmem2 來驗證轉換出來的 physical address 的正確性。devmem2 可以查看某 physical address 裡存放的數值並對其修改。

---

- check.c
    
    ```c
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
            printf("============= main =============\n");
            printf("pid = %d\n", (int)getpid());
            printf("segment\tvalue\tvir_addr\tphy_addr\n");
            printf("stack\t%d\t%lx\t%lx\n", stack_value, vir_addrs[0], phy_addrs[0]);
            printf("getpid\t%d\t%lx\t%lx\n", (int)getpid(), vir_addrs[1], phy_addrs[1]);
            printf("heap\t%d\t%lx\t%lx\n", *(int*)heap, vir_addrs[2], phy_addrs[2]);
            printf("bss\t%d\t%lx\t%lx\n", bss_value, vir_addrs[3], phy_addrs[3]);
            printf("data\t%d\t%lx\t%lx\n", data_value, vir_addrs[4], phy_addrs[4]);
            printf("code\t%d\t%lx\t%lx\n", code_function(), vir_addrs[5], phy_addrs[5]);
    
            printf("block...");
            scanf("%d", &block_input);
        }
    
        return 0;
    }
    ```
    

上面的 code 會不斷重複印出每個段的 value 、virtual address 和透過我們的 system call 轉換出來的 physical address，並且每個迴圈會使用 standard input block 住。以下為輸出畫面。

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%201.png)

---

以 data 段舉例，我們開啟 devmem2 檢查 data 段的 physical address (0x45c74010)，檢查其值是否為 123 (0x7B)。

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%202.png)

看來結果非常正確。

---

我們來嘗試修改其值，確保我們不是運氣好數值剛好對。我們透過以下指令把其值改成 **-321。**

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%203.png)

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%204.png)

可以發現 data 段的變數變成 -321 了，驗證非常成功。

# Shared Segment in Multi-thread Program

- 我們的 code
    
    ```c
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
    ```
    
- 我們的結果截圖
    
    ![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%205.png)
    

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%206.png)

上面是我們依輸出的結果，排序後的資料。資料分為三部分

- 左邊這次跑的執行順序為 thread1 → thread2 → thread3 → main
- 中間這次跑的執行順序為 thread2 → thread3 → thread1 → main
- 右邊為分析後的結果

用這個結果來對照 /proc/$(pid)/maps

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%207.png)

最後統整出以下結果 ↓

![thread-stack.png](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/thread-stack.png)

## 發現

         不同 thread 之間 code、data、bss、library 段是共用的，而 stack 以及 heap 段比較特別，在 maps 中可以看到 [heap] 下面額外配置了一段可讀可寫的記憶體，這一段就是 thread 配置的 heap 段 (以下簡稱 thread heap)，有趣的是 thread heap 和主程式 heap 之間有一段蠻大的差距，原因不明。

        maps 中可以發現 library 段之間有一大段可疑的空白段，對照發現這些是 thread stack 段，而且每個 thread stack 下方都會配置一段不可獨不可寫不可執行的保護段，可見這些 thread stack 會往下成長，是貨真價實的 stack。至於為什麼會配置在 library 段之間，我們推測是因為 thread 是由 library 的某個 function 生出來的，才會生在 library 附近。

        而 TLS(thread local storage) 會個複製一份到各自 thread stack 的頂端，而屬於 main thread 的則會配置在所有 thread stack 的上方。

# Shared Segment in Two Process

- 我們的code
    
    ```c
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
    
        printf("============= main =============\n");
        printf("pid = %d\n", (int)getpid());
        printf("segment\tvir_addr\tphy_addr\n");
        printf("stack\t%lx\t%lx\n", vir_addrs[0], phy_addrs[0]);
        printf("lib\t%lx\t%lx\n", vir_addrs[1], phy_addrs[1]);
        printf("heap\t%lx\t%lx\n", vir_addrs[2], phy_addrs[2]);
        printf("bss\t%lx\t%lx\n", vir_addrs[3], phy_addrs[3]);
        printf("data\t%lx\t%lx\n", vir_addrs[4], phy_addrs[4]);
        printf("code\t%lx\t%lx\n", vir_addrs[5], phy_addrs[5]);
    
        return 0;
    }
    ```
    

我們用 crontab 讓兩個 process 同時執行，應該比用手執行還來的準確，以下是 crontab 設定

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%208.png)

然後是輸出結果

![Untitled](Linux%20Project2%20WriteUp%20c801a08b5c0246de843b02d369e58b8c/Untitled%209.png)

## 發現

可以發現只有 library 段和 code 段是共用的喔~

# 結論

為什麼會有以上神奇的結果呢? 我們對 question 1 的 code 進行 strace ，得到以下結果

- **strace -f ./question1**
    
    ```bash
    execve("./test", ["./test"], 0x7ffdb9299128 /* 27 vars */) = 0
    brk(NULL)                               = 0x561992579000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7ffd38a9ff30) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=32083, ...}) = 0
    mmap(NULL, 32083, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f5ed3c5f000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libpthread.so.0", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\220\201\0\0\0\0\0\0"..., 832) = 832
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\345Ga\367\265T\320\374\301V)Yf]\223\337"..., 68, 824) = 68
    fstat(3, {st_mode=S_IFREG|0755, st_size=157224, ...}) = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f5ed3c5d000
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\345Ga\367\265T\320\374\301V)Yf]\223\337"..., 68, 824) = 68
    mmap(NULL, 140408, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f5ed3c3a000
    mmap(0x7f5ed3c41000, 69632, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x7000) = 0x7f5ed3c41000
    mmap(0x7f5ed3c52000, 20480, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x18000) = 0x7f5ed3c52000
    mmap(0x7f5ed3c57000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1c000) = 0x7f5ed3c57000
    mmap(0x7f5ed3c59000, 13432, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f5ed3c59000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360q\2\0\0\0\0\0"..., 832) = 832
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\t\233\222%\274\260\320\31\331\326\10\204\276X>\263"..., 68, 880) = 68
    fstat(3, {st_mode=S_IFREG|0755, st_size=2029224, ...}) = 0
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\t\233\222%\274\260\320\31\331\326\10\204\276X>\263"..., 68, 880) = 68
    mmap(NULL, 2036952, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f5ed3a48000
    mprotect(0x7f5ed3a6d000, 1847296, PROT_NONE) = 0
    mmap(0x7f5ed3a6d000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x25000) = 0x7f5ed3a6d000
    mmap(0x7f5ed3be5000, 303104, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19d000) = 0x7f5ed3be5000
    mmap(0x7f5ed3c30000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e7000) = 0x7f5ed3c30000
    mmap(0x7f5ed3c36000, 13528, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f5ed3c36000
    close(3)                                = 0
    mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f5ed3a45000
    arch_prctl(ARCH_SET_FS, 0x7f5ed3a45740) = 0
    mprotect(0x7f5ed3c30000, 12288, PROT_READ) = 0
    mprotect(0x7f5ed3c57000, 4096, PROT_READ) = 0
    mprotect(0x561990f2d000, 4096, PROT_READ) = 0
    mprotect(0x7f5ed3c94000, 4096, PROT_READ) = 0
    munmap(0x7f5ed3c5f000, 32083)           = 0
    set_tid_address(0x7f5ed3a45a10)         = 6895
    set_robust_list(0x7f5ed3a45a20, 24)     = 0
    rt_sigaction(SIGRTMIN, {sa_handler=0x7f5ed3c41bf0, sa_mask=[], sa_flags=SA_RESTORER|SA_SIGINFO, sa_restorer=0x7f5ed3c4f3c0}, NULL, 8) = 0
    rt_sigaction(SIGRT_1, {sa_handler=0x7f5ed3c41c90, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART|SA_SIGINFO, sa_restorer=0x7f5ed3c4f3c0}, NULL, 8) = 0
    rt_sigprocmask(SIG_UNBLOCK, [RTMIN RT_1], NULL, 8) = 0
    prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
    fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0), ...}) = 0
    brk(NULL)                               = 0x561992579000
    brk(0x56199259a000)                     = 0x56199259a000
    write(1, "syscall\n", 8syscall
    )                = 8
    mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x7f5ed3244000
    mprotect(0x7f5ed3245000, 8388608, PROT_READ|PROT_WRITE) = 0
    clone(child_stack=0x7f5ed3a43fb0, flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTIDstrace: Process 6896 attached
    , parent_tid=[6896], tls=0x7f5ed3a44700, child_tidptr=0x7f5ed3a449d0) = 6896
    [pid  6896] set_robust_list(0x7f5ed3a449e0, 24 <unfinished ...>
    [pid  6895] mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0 <unfinished ...>
    [pid  6896] <... set_robust_list resumed>) = 0
    [pid  6896] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=2, tv_nsec=0},  <unfinished ...>
    [pid  6895] <... mmap resumed>)         = 0x7f5ed2a43000
    [pid  6895] mprotect(0x7f5ed2a44000, 8388608, PROT_READ|PROT_WRITE) = 0
    [pid  6895] clone(child_stack=0x7f5ed3242fb0, flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTIDstrace: Process 6897 attached
    , parent_tid=[6897], tls=0x7f5ed3243700, child_tidptr=0x7f5ed32439d0) = 6897
    [pid  6897] set_robust_list(0x7f5ed32439e0, 24 <unfinished ...>
    [pid  6895] mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0) = 0x7f5ed2242000
    [pid  6897] <... set_robust_list resumed>) = 0
    [pid  6895] mprotect(0x7f5ed2243000, 8388608, PROT_READ|PROT_WRITE) = 0
    [pid  6897] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=4, tv_nsec=0},  <unfinished ...>
    [pid  6895] clone(child_stack=0x7f5ed2a41fb0, flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTIDstrace: Process 6898 attached
    , parent_tid=[6898], tls=0x7f5ed2a42700, child_tidptr=0x7f5ed2a429d0) = 6898
    [pid  6898] set_robust_list(0x7f5ed2a429e0, 24 <unfinished ...>
    [pid  6895] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=8, tv_nsec=0},  <unfinished ...>
    [pid  6898] <... set_robust_list resumed>) = 0
    [pid  6898] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=6, tv_nsec=0},  <unfinished ...>
    [pid  6896] <... clock_nanosleep resumed>0x7f5ed3a43e10) = 0
    [pid  6896] mmap(NULL, 134217728, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0) = 0x7f5eca242000
    [pid  6896] munmap(0x7f5eca242000, 31186944) = 0
    [pid  6896] munmap(0x7f5ed0000000, 35921920) = 0
    [pid  6896] mprotect(0x7f5ecc000000, 135168, PROT_READ|PROT_WRITE) = 0
    [pid  6896] write(1, "============= thread1 =========="..., 36============= thread1 =============
    ) = 36
    [pid  6896] gettid()                    = 6896
    [pid  6896] getpid()                    = 6895
    [pid  6896] write(1, "pid = 6895  tid = 6896\n", 23pid = 6895  tid = 6896
    ) = 23
    [pid  6896] write(1, "segment\tvir_addr\n", 17segment   vir_addr
    ) = 17
    [pid  6896] write(1, "TLS\t7f5ed3a446fc\n", 17TLS       7f5ed3a446fc
    ) = 17
    [pid  6896] write(1, "stack\t7f5ed3a43e60\n", 19stack   7f5ed3a43e60
    ) = 19
    [pid  6896] write(1, "lib\t7f5ed3b2f240\n", 17lib       7f5ed3b2f240
    ) = 17
    [pid  6896] write(1, "heap\t7f5ecc000b60\n", 18heap     7f5ecc000b60
    ) = 18
    [pid  6896] write(1, "bss\t561990f2e018\n", 17bss       561990f2e018
    ) = 17
    [pid  6896] write(1, "data\t561990f2e010\n", 18data     561990f2e010
    ) = 18
    [pid  6896] write(1, "code\t561990f2b219\n", 18code     561990f2b219
    ) = 18
    [pid  6896] openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    [pid  6896] fstat(3, {st_mode=S_IFREG|0644, st_size=32083, ...}) = 0
    [pid  6896] mmap(NULL, 32083, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f5ed3c5f000
    [pid  6896] close(3)                    = 0
    [pid  6896] openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libgcc_s.so.1", O_RDONLY|O_CLOEXEC) = 3
    [pid  6896] read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\3405\0\0\0\0\0\0"..., 832) = 832
    [pid  6896] fstat(3, {st_mode=S_IFREG|0644, st_size=104984, ...}) = 0
    [pid  6896] mmap(NULL, 107592, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f5ed2227000
    [pid  6896] mmap(0x7f5ed222a000, 73728, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x3000) = 0x7f5ed222a000
    [pid  6896] mmap(0x7f5ed223c000, 16384, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x15000) = 0x7f5ed223c000
    [pid  6896] mmap(0x7f5ed2240000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x18000) = 0x7f5ed2240000
    [pid  6896] close(3)                    = 0
    [pid  6896] mprotect(0x7f5ed2240000, 4096, PROT_READ) = 0
    [pid  6896] munmap(0x7f5ed3c5f000, 32083) = 0
    [pid  6896] futex(0x7f5ed22411e0, FUTEX_WAKE_PRIVATE, 2147483647) = 0
    [pid  6896] madvise(0x7f5ed3244000, 8368128, MADV_DONTNEED) = 0
    [pid  6896] exit(0)                     = ?
    [pid  6896] +++ exited with 0 +++
    [pid  6897] <... clock_nanosleep resumed>0x7f5ed3242e10) = 0
    [pid  6897] write(1, "============= thread2 =========="..., 36============= thread2 =============
    ) = 36
    [pid  6897] gettid()                    = 6897
    [pid  6897] getpid()                    = 6895
    [pid  6897] write(1, "pid = 6895  tid = 6897\n", 23pid = 6895  tid = 6897
    ) = 23
    [pid  6897] write(1, "segment\tvir_addr\n", 17segment   vir_addr
    ) = 17
    [pid  6897] write(1, "TLS\t7f5ed32436fc\n", 17TLS       7f5ed32436fc
    ) = 17
    [pid  6897] write(1, "stack\t7f5ed3242e60\n", 19stack   7f5ed3242e60
    ) = 19
    [pid  6897] write(1, "lib\t7f5ed3b2f240\n", 17lib       7f5ed3b2f240
    ) = 17
    [pid  6897] write(1, "heap\t7f5ecc001270\n", 18heap     7f5ecc001270
    ) = 18
    [pid  6897] write(1, "bss\t561990f2e018\n", 17bss       561990f2e018
    ) = 17
    [pid  6897] write(1, "data\t561990f2e010\n", 18data     561990f2e010
    ) = 18
    [pid  6897] write(1, "code\t561990f2b219\n", 18code     561990f2b219
    ) = 18
    [pid  6897] madvise(0x7f5ed2a43000, 8368128, MADV_DONTNEED) = 0
    [pid  6897] exit(0)                     = ?
    [pid  6897] +++ exited with 0 +++
    [pid  6898] <... clock_nanosleep resumed>0x7f5ed2a41e10) = 0
    [pid  6898] write(1, "============= thread3 =========="..., 36============= thread3 =============
    ) = 36
    [pid  6898] gettid()                    = 6898
    [pid  6898] getpid()                    = 6895
    [pid  6898] write(1, "pid = 6895  tid = 6898\n", 23pid = 6895  tid = 6898
    ) = 23
    [pid  6898] write(1, "segment\tvir_addr\n", 17segment   vir_addr
    ) = 17
    [pid  6898] write(1, "TLS\t7f5ed2a426fc\n", 17TLS       7f5ed2a426fc
    ) = 17
    [pid  6898] write(1, "stack\t7f5ed2a41e60\n", 19stack   7f5ed2a41e60
    ) = 19
    [pid  6898] write(1, "lib\t7f5ed3b2f240\n", 17lib       7f5ed3b2f240
    ) = 17
    [pid  6898] write(1, "heap\t7f5ecc001290\n", 18heap     7f5ecc001290
    ) = 18
    [pid  6898] write(1, "bss\t561990f2e018\n", 17bss       561990f2e018
    ) = 17
    [pid  6898] write(1, "data\t561990f2e010\n", 18data     561990f2e010
    ) = 18
    [pid  6898] write(1, "code\t561990f2b219\n", 18code     561990f2b219
    ) = 18
    [pid  6898] madvise(0x7f5ed2242000, 8368128, MADV_DONTNEED) = 0
    [pid  6898] exit(0)                     = ?
    [pid  6898] +++ exited with 0 +++
    <... clock_nanosleep resumed>0x7ffd38a9fe20) = 0
    write(1, "============= main ============="..., 33============= main =============
    ) = 33
    gettid()                                = 6895
    getpid()                                = 6895
    write(1, "pid = 6895  tid = 6895\n", 23pid = 6895  tid = 6895
    ) = 23
    write(1, "segment\tvir_addr\n", 17segment       vir_addr
    )     = 17
    write(1, "TLS\t7f5ed3a4573c\n", 17TLS   7f5ed3a4573c
    )     = 17
    write(1, "stack\t7ffd38a9fe68\n", 19stack       7ffd38a9fe68
    )   = 19
    write(1, "lib\t7f5ed3b2f240\n", 17lib   7f5ed3b2f240
    )     = 17
    write(1, "heap\t561992579a40\n", 18heap 561992579a40
    )    = 18
    write(1, "bss\t561990f2e018\n", 17bss   561990f2e018
    )     = 17
    write(1, "data\t561990f2e010\n", 18data 561990f2e010
    )    = 18
    write(1, "code\t561990f2b219\n", 18code 561990f2b219
    )    = 18
    write(1, "----------- thread address -----"..., 40----------- thread address ------------
    ) = 40
    write(1, "t1 = 0x7ffd38a9fe70\n", 20t1 = 0x7ffd38a9fe70
    )   = 20
    write(1, "t2 = 0x7ffd38a9fe78\n", 20t2 = 0x7ffd38a9fe78
    )   = 20
    write(1, "t3 = 0x7ffd38a9fe80\n", 20t3 = 0x7ffd38a9fe80
    )   = 20
    ```
    

        大概可以得知當一個 process 執行時，會先 call execve()，初始化 stack、data、bss，以及把 elf 檔案讀進 text 段。

        接著會透過 mmap() 把 library 讀進連續的 memory 中，並且大多會設置 **MAP_PRIVATE** 的 flag，防止有心人士偷改 library 的文件。以上可以解釋為何第二題的 code 段和 library 段會共享。

        再來我們發現，在 create 一個新的 thread 的時候，會先 mmap() 一段 memory ，然後mprotect() 生成一段保護區，最後執行 clone()。

        clone() 跟 fork() 都會生出一個新的 task_struct ，但兩者最大的差別在於，fork() 會生出一個子 process ，和全新的 virtual memory， 而 clone() 可以選擇你想共享的 memory，是為 thread 設計的 system call。而上面可以看到，clone() 會先傳入一段 stack 段的 address，而傳入的 address 正是先前 mmap() 的那一段，這就是為甚麼每個 thread 會各有一段 stack 不共享。而 clone() 還有傳入一個 **CLONE_VM** 的 flag ，讓 main thread 和 children thread 運行在相同的 virtual memory 中，這也就是為甚麼 thread 其他的記憶體都共享。