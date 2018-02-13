#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define MISC_NAME	"misc"

static struct file_operations misc_ops = {
	.owner	= THIS_MODULE,
};

static int count = 1;
module_param(count, int, S_IRUGO);


static struct miscdevice **misc;

static char *strdup(char *str)
{
	int n = strlen(str) + 1;
	char *s = kmalloc(n, GFP_KERNEL);
	if (!s) 
		return NULL;
	return strcpy(s, str);
}


static int __init misc_demo_init(void)
{
	int ret, i;
	char name[32];
	struct miscdevice *mi;

	
	misc = (struct miscdevice **)kmalloc(sizeof(mi) * count, GFP_KERNEL);
	if (!misc) 
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		mi = kmalloc(sizeof(*mi), GFP_KERNEL);
		if (!mi) {
			ret = -ENOMEM;
			goto error_exit;
		}
		memset(mi, 0, sizeof(*mi));

		mi->minor = MISC_DYNAMIC_MINOR;
		mi->fops = &misc_ops;
		mi->mode = 0666;

		snprintf(name, sizeof(name), "%s%d", MISC_NAME, i);
		mi->name = strdup(name);
		if (!mi->name) {
			ret = -ENOMEM;
			kfree(mi);
			goto error_exit;
		}

		ret = misc_register(mi);

		if (ret) {
			kfree(mi->name);
			kfree(mi);
			goto error_exit;
		}
		misc[i] = mi;
		printk("misc: %s register ok\n", name); 
	}
	return 0;

error_exit:
	for (--i; i >= 0; i--) {
		misc_deregister(misc[i]);
		kfree(misc[i]->name);
		kfree(misc[i]);
	}

	kfree(misc);
	return ret;
}

static void __exit misc_demo_exit(void) 
{
	int i;
	for (i = count-1; i >= 0; i--) {
		misc_deregister(misc[i]);
		kfree(misc[i]->name);
		kfree(misc[i]);
	}
	kfree(misc);
}

module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");


