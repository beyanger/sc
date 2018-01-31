
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

static void *page_mem;
static void *kmalloc_mem;
static void *vmalloc_mem;

static int __init mem_module_init(void) 
{
	page_mem = (void *)__get_free_page(0);
	printk(KERN_INFO "page mem: \t %p, %llx\n", page_mem, virt_to_phys(page_mem));

	kmalloc_mem = kmalloc(100, GFP_KERNEL);
	printk(KERN_INFO "kmalloc mem:\t %p, %llx\n", kmalloc_mem, virt_to_phys(kmalloc_mem));

	vmalloc_mem = vmalloc(1000000);
	printk(KERN_INFO "vmalloc mem:\t %p\n", vmalloc_mem);

	return 0;
}


static void __exit mem_module_exit(void)
{
	free_page((unsigned long)page_mem);
	kfree(kmalloc_mem);
	vfree(vmalloc_mem);
}

module_init(mem_module_init);
module_exit(mem_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");


