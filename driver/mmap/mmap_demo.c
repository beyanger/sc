

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define MM_SIZE	4096

static char *buffer = NULL;

static int dev_open(struct inode *inode, struct file *filp)
{
	printk("device open\n");
	return 0;
}

static int dev_close(struct inode *inode, struct file *filp)
{
	printk("device close\n");
	return 0;
}

static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	printk("device mmap\n");
	vma->vm_flags |= VM_IO;
	vma->vm_flags |= (VM_DONTEXPAND|VM_DONTDUMP);


	printk("virt_to_phys: %llx\n", virt_to_phys(buffer));
	printk("PAGE_SHIFT: %d\n", PAGE_SHIFT);
	printk("end: %lx, start: %lx\n", vma->vm_end, vma->vm_start);


	if (remap_pfn_range(vma, vma->vm_start, virt_to_phys(buffer)>>PAGE_SHIFT,
				vma->vm_end-vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

static struct file_operations dev_fops = {
	.owner		= THIS_MODULE,
	.open		= dev_open,
	.release	= dev_close,
	.mmap		= dev_mmap,
};

#define MISC_NAME "misc"
static struct miscdevice misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MISC_NAME,
	.fops	= &dev_fops,
	.mode	= 0644,
};

static int __init dev_mmap_init(void)
{
	int ret;
	buffer = (char *)kmalloc(1024, GFP_KERNEL);
	ret = misc_register(&misc);
	printk("%s misc dev init %s\n", MISC_NAME, ret==0 ? "successed" : "failed");
	return ret;
}


static void __exit dev_mmap_exit(void)
{
	printk("misc exit\n");
	misc_deregister(&misc);
	kfree(buffer);
}

module_init(dev_mmap_init);
module_exit(dev_mmap_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");

