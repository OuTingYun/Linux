#!/bin/bash
user="outingyun"
path="/home/${user}/Linux/project3/linux-5.14.14"
cp -r ./mycall "${path}"
cp ./Makefile "${path}"
cp ./syscall_64.tbl "${path}/arch/x86/entry/syscalls"
cp ./syscalls.h "${path}/include/linux"
cp -r ./test "/home/${user}/Linux/project3"
cp ./sched.h "${path}/include/linux"
cp ./fork.c "${path}/kernel"
cp ./core.c "${path}/kernel/sched"
