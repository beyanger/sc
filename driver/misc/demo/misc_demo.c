

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>


#define MISC_NAME	"dev_misc"


static struct file_operations misc_ops = {
	.owner	= THIS_MODULE,
};

static struct miscdevice misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MISC_NAME,
	.fops	= &misc_ops,
};



static int __init misc_demo_init(void)
{
	int ret;
	ret = misc_register(&misc);
	printk("%s misc dev init %s\n", MISC_NAME, ret==0 ? "successed" : "failed");
	return ret;

}

static void __exit misc_demo_exit(void) 
{
	misc_deregister(&misc);
}

module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");




