
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/proc_fs.h>


static int demo_flags = 0;
static int proc_demo_show(struct seq_file *seq, void *v) 
{
	seq_printf(seq, "%d\n", demo_flags++);
	return 0;
}

static int proc_demo_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, proc_demo_show, NULL);
}

static ssize_t proc_demo_write(struct file *filp, const char __user *buf, size_t size, loff_t *off) 
{
	char buffer[128];
	copy_from_user(buffer, buf, size);
	sscanf(buffer, "%d", &demo_flags);
	return size;
}

struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.open = proc_demo_open,
	.write = proc_demo_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


static struct proc_dir_entry *proc_file = NULL;

static int __init proc_demo_init(void) 
{
	proc_file = proc_create("proc_demo", 0644, NULL, &proc_fops);
	if (!proc_file) 
		return -1;
	return 0;
}

static void __exit proc_demo_exit(void) {
	if (proc_file)
		proc_remove(proc_file);
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");

