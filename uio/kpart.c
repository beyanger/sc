
#include <linux/module.h>
#include <linux/uio_driver.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/timer.h>

static struct timer_list tm;

struct uio_info kpart_info  = {
	.name = "kpart",
	.version = "0.1",
	.irq = UIO_IRQ_NONE,
};


void call_back(unsigned long arg) {
	char *mem = (char *)arg;
	if (mem[0] != '\0')
		printk("mem: %s\n", mem);
	mem[0] = '\0';
	tm.expires = jiffies + HZ;
	add_timer(&tm);
}


static int 
drv_kpart_probe(struct device *dev)
{
	int ret;
	char *mem;

	printk("drv_kpart_porbe(%p)\n", dev);

	mem = kmalloc(1024, GFP_KERNEL);

	if (!mem)
		return -ENOMEM;
	memset(mem, 0, 1024);
	kpart_info.mem[0].addr = (unsigned long)mem;
	kpart_info.mem[0].memtype = UIO_MEM_LOGICAL;
	kpart_info.mem[0].size = 1024;

	ret = uio_register_device(dev, &kpart_info);
	if (ret) {
		kfree(mem);
		return -ENODEV;
	}

	init_timer(&tm);
	tm.function = call_back;
	tm.data = (unsigned long)mem;
	tm.expires = jiffies + HZ;
	add_timer(&tm);

	return 0;
}
static int 
drv_kpart_remove(struct device *dev) 
{
	del_timer(&tm);
	kfree((void *)kpart_info.mem[0].addr);
	uio_unregister_device(&kpart_info);
	return 0;
}


static struct device_driver uio_dummy_driver = {
	.name = "kpart",
	.bus = &platform_bus_type,
	.probe = drv_kpart_probe,
	.remove = drv_kpart_remove,
};

static struct platform_device *uio_dummy_device;

static int __init
uio_kpart_init(void) 
{
	uio_dummy_device = platform_device_register_simple("kpart", -1, NULL, 0);
	return driver_register(&uio_dummy_driver);
}

static void __exit
uio_kpart_exit(void) 
{
	driver_unregister(&uio_dummy_driver);
	platform_device_unregister(uio_dummy_device);
}


module_init(uio_kpart_init);
module_exit(uio_kpart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");


