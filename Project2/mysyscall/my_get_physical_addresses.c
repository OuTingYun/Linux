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