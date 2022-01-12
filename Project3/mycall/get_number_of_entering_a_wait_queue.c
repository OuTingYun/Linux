#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>


SYSCALL_DEFINE1(get_number_of_entering_a_wait_queue, unsigned int*, count) {
    unsigned int answer = current->cs_count;
    printk("pid = %d ; cs_count = %u\n", current->pid, answer);
    return -copy_to_user(count, &(answer), sizeof(unsigned int));
}