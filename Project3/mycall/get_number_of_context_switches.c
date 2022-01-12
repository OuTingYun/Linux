#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>


SYSCALL_DEFINE1(get_number_of_context_switches, unsigned int*, count) {
    unsigned int answer = current->wq_count;
    printk("pid = %d ; wq_count = %u\n", current->pid, answer);
    return -copy_to_user(count, &(answer), sizeof(unsigned int));
}