#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>


struct data_segment {
    unsigned long start_code;
    unsigned long end_code;
};

// asmlinkage long __x64_sys_get_pid_text_segment_addr(pid_t user_pid, void __user* user_address)

SYSCALL_DEFINE2(get_pid_text_segment_addr, pid_t, user_pid, void __user*, user_address) {

    struct task_struct *task;
    struct data_segment my_data_segment;
 
    for_each_process(task) {
        if (task->pid == user_pid) {

            my_data_segment.start_code = task->mm->start_code;
            my_data_segment.end_code = task->mm->end_code;
            
            long a = copy_to_user(user_address, &my_data_segment, sizeof(struct data_segment));
            break;
        }
    }

    return 0;
}