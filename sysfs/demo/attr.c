

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kobject.h>

struct my_kobj {
	struct kobject kobj;
	int val;
};

struct my_attribute {
	struct attribute attr;
	ssize_t (*show)(struct my_kobj *obj, struct my_attribute *attr, char *buf);
	ssize_t (*store)(struct my_kobj *obj, struct my_attribute *attr, const char *buf, size_t count);
};



static ssize_t my_attr_show(struct kobject *kobj, struct attribute *attr, char *buf) 
{
	struct my_attribute *my_attr;
	ssize_t ret = -EIO;
	
	my_attr = container_of(attr, struct my_attribute, attr);
	if (my_attr->show) 
		ret = my_attr->show(container_of(kobj, struct my_kobj, kobj), my_attr, buf);
	
	return ret;
}


static ssize_t my_attr_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count) 
{
	struct my_attribute *my_attr;
	ssize_t ret = -EIO;

	my_attr = container_of(attr, struct my_attribute, attr);

	if (my_attr->store)
		ret = my_attr->store(container_of(kobj, struct my_kobj, kobj), my_attr, buf, count);
	return ret;
}

ssize_t name_show(struct my_kobj *obj, struct my_attribute *attr, char *buf) 
{
	return sprintf(buf, "%s\n", kobject_name(&obj->kobj));
}

ssize_t val_show(struct my_kobj *obj, struct my_attribute *attr, char *buf) 
{
	return sprintf(buf, "%d\n", obj->val);
}

ssize_t val_store(struct my_kobj *obj, struct my_attribute *attr, const char *buf, size_t size) 
{
	sscanf(buf, "%d", &obj->val);
	return size;
}

static const struct sysfs_ops my_sysfs_ops = {
	.show = my_attr_show,
	.store = my_attr_store,
};

#define _ATTR(_name, _mode, _show, _store) { \
	.attr = { .name = __stringify(_name), .mode = _mode }, \
	.show = _show, \
	.store = _store, \
}

struct my_attribute name_attr = _ATTR(name, 0444, name_show, NULL);
struct my_attribute val_attr = _ATTR(val, 0666, val_show, val_store);

struct attribute *my_attr[] = {
	&name_attr.attr,
	&val_attr.attr,
	NULL,
};

struct attribute_group my_group = {
	.name = "my_group",
	.attrs = my_attr,
};


void obj_release(struct kobject *kobj) 
{
	struct my_kobj *obj = container_of(kobj, struct my_kobj, kobj);
	printk(KERN_INFO "obj_release %s\n", kobject_name(&obj->kobj));
	kfree(obj);
}

static struct kobj_type my_ktype = {
	.release = obj_release,
	.sysfs_ops = &my_sysfs_ops,
};

static struct my_kobj *obj;

static int __init mykobj_init(void)
{
	int ret;
	printk(KERN_INFO "mykobj init\n");

	obj = kmalloc(sizeof(struct my_kobj), GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	obj->val = 1;
	memset(&obj->kobj, 0, sizeof(obj->kobj));
	kobject_init_and_add(&obj->kobj, &my_ktype, NULL, "my_kobj");

	ret = sysfs_create_files(&obj->kobj, (const struct attribute **)my_attr);
	if (ret)
		goto kobject_error_exit;

	ret = sysfs_create_group(&obj->kobj, &my_group);
	if (ret) 
		goto sysfs_remove_file_eixt;

	return 0;	

sysfs_remove_file_eixt:
	sysfs_remove_files(&obj->kobj, (const struct attribute **)my_attr);
kobject_error_exit:
	kobject_put(&obj->kobj);
mem_free_exit:
	kfree(obj);
	return ret;
}


static void __exit mykobj_exit(void) 
{
	printk(KERN_INFO "mykobj exit\n");

	sysfs_remove_group(&obj->kobj, &my_group);

	sysfs_remove_files(&obj->kobj, (const struct attribute **)my_attr);
	kobject_put(&obj->kobj);
	kfree(obj);
}

module_init(mykobj_init);
module_exit(mykobj_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang");



