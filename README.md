## LINUX 
Praticing three Linux Kernel Modules.
### Environment
linux kernel version：5.14.14  
>[download here](https://www.kernel.org/)


### How to use
Compile kernel and use file in Test to represent result.

Reference to comiler kernel：
1. [Compile Kernel](https://discourse.ubuntu.com/t/how-to-compile-kernel-in-ubuntu-20-04/20268?fbclid=IwAR0hYkJaNDShEWH8Kv-ofXeHY2-o7ijfB84Oa_uBv8QOMuE3OcfeHytFtEg)

2. [Compile Kernel2](https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8?fbclid=IwAR1RKyDbEZPTtjBxKuthOqTrbyjR7LgEoAdRc47zpRCMKf9jO2KZO0ZCM9w)

3. [linux 編寫](https://blog.kaibro.tw/2016/11/07/Linux-Kernel%E7%B7%A8%E8%AD%AF-Ubuntu/)

### Project1
從 `init_task` 開始遍歷 task_list 至指定 task，傳出指定資料至指定記憶體段。

### Project2

**Question 1**

* write a new system  so that a process can use it to get the physical addresses of some virtual addresses.
` int my_get_physical_addresses(unsigned int * initial, int len_vir, unsigned int * result, int len_phy)`

* The return value of this system call is either 0 or a positive value. 0 means that an error occurs when executing this system call. A positive value means the system call is executed successfully.

* The first argument of this system call is the address of an unsigned integer array. Each element of the array stores a virtual address of a process.

* The second argument of this system call is the number of elements in the array.

* The third argument is the address of an unsigned integer array. Each element with index i of this array stores the physical address of the virtual address stored as element i in the array pointed by the first argument.

* The fourth argument is the number of elements stored in the array pointed by the third argument.

* 實作一個 system call ，功能是將複數個 virtual address 轉為 physical address ，並利用這個 system call 來檢查不同的 thread/process 的哪些 segment 會共用 memory。

**Question 2**

* Write a multi-thread program with three threads using the new system call to show how the following memory areas are shared by these threads. Your program must use variables with storage class __thread. The memory areas include code segments, data segments, BSS segments, heap segments, libraries, stack segments, and thread local storages. You need to draw a figure as follows to show your results.

* 找出在同個程式中，不同 thread 所共用的 segment。
### Project3

**Question 1**
* Write a new system call `int get_number_of_context_switches(unsigned int *)` so that a process can use it to get the number of context switches the scheduler makes upon it. If get_number_of_context_switches(unsigned int *) executes successfully, it returns 0; otherwise, it returns a negative value.

* 計算 context 的次數

**Question 2**
* Write a new system call `int get_number_of_entering_a_wait_queue(unsigned int *)` so that a process can use it to get the number of its entering a wait queue. If get_number_of_entering_a_wait_queue(unsigned int *) executes successfully, it returns 0; otherwise, it returns a negative value.

* 進入waiting queue 的次數

### Collaborators

國立中央大學 張XX

國立中央大學 林XX


