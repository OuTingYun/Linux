scp -r ./mysyscall cockroach@192.168.131.1:~/linux/project2/linux-5.14.14
scp ./Makefile cockroach@192.168.131.1:~/linux/project2/linux-5.14.14
scp ./syscall_64.tbl cockroach@192.168.131.1:~/linux/project2/linux-5.14.14/arch/x86/entry/syscalls
scp ./syscalls.h cockroach@192.168.131.1:~/linux/project2/linux-5.14.14/include/linux
scp -r ./testing cockroach@192.168.131.1:~/linux/project2