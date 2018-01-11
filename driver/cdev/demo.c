#include <linux/module.h>  
#include <linux/fs.h>  
#include <linux/init.h>  
#include <linux/cdev.h>  
#include <linux/slab.h>  
#include <linux/uaccess.h>  
  
#include <linux/device.h>  




static int __init dev_init(void)
{
	int ret, err;



	return 0;
}

static void __exit dev_exit(void) 
{

}


module_init(dev_init);
module_exit(dev_exit);


MODULE_AUTOR("Yang");
MODULE_LICENSE("GPL");
