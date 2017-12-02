#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

#define BUFFER_LENGTH	64

static char sysbuff_genius[BUFFER_LENGTH];
static char sysbuff_stupid[BUFFER_LENGTH];

static ssize_t hoge_show(
	struct kobject *kobj,
	struct attribute *attr,
	char *buf)
{

	if (!strcmp(attr->name, "genius")) {
		sprintf(buf, "from genius %s", sysbuff_genius);
	} else {
		sprintf(buf, "from stupid %s", sysbuff_stupid);
	}
	return strlen(buf);
}

static ssize_t hoge_store(
	struct kobject *kobj,
	struct attribute *attr,
	const char *buf,
	size_t len)
{
	if (!strcmp(attr->name, "genius")) {
		strcpy(sysbuff_genius, buf);
	} else {
		strcpy(sysbuff_stupid, buf);
	}
	return len;
}

static const struct sysfs_ops hoge_sysfs_ops = {
	.show = hoge_show,
	.store = hoge_store,
};

static void hoge_release(struct kobject *kobj)
{
	kfree(kobj);
}

static struct attribute hoge_attr1 = {
	"genius", 0666,
};
static struct attribute hoge_attr2 = {
	"stupid", 0666,
};

static struct attribute *hoge_attrs[] = {
	&hoge_attr1,
	&hoge_attr2,
	NULL,
};

static struct kobj_type hoge_ktype = {
	.sysfs_ops = &hoge_sysfs_ops,
	.release = hoge_release,
	.default_attrs = hoge_attrs,
};

static struct kset *hoge_kset;

struct kobject *kobj;

static int __init hoge_init(void)
{
	int     ret;
	hoge_kset = kset_create_and_add("kset_hoge", NULL, kernel_kobj);
	if (!hoge_kset)
		return -ENOMEM;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (!kobj)
		return -EINVAL;
	kobj->kset = hoge_kset;

	ret = kobject_init_and_add(kobj, &hoge_ktype, NULL, "%s", "hoge");
	return 0;
}

static void __exit hoge_exit(void)
{
	kobject_put(kobj);
	kset_unregister(hoge_kset);
}

module_init(hoge_init);
module_exit(hoge_exit);

#if 0 /* refernce from linux */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>

struct foo_obj {
	struct kobject kobj;
	int foo;
	int baz;
	int bar;
};
#define to_foo_obj(x) container_of(x, struct foo_obj, kobj)

struct foo_attribute {
	struct attribute attr;
	ssize_t (*show)(struct foo_obj *foo, struct foo_attribute *attr, char *buf);
	ssize_t (*store)(struct foo_obj *foo, struct foo_attribute *attr, const char *buf, size_t count);
};
#define to_foo_attr(x) container_of(x, struct foo_attribute, attr)

static ssize_t foo_attr_show(struct kobject *kobj,
			     struct attribute *attr,
			     char *buf)
{
	struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(foo, attribute, buf);
}

static ssize_t foo_attr_store(struct kobject *kobj,
			      struct attribute *attr,
			      const char *buf, size_t len)
{
	struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(foo, attribute, buf, len);
}

static const struct sysfs_ops foo_sysfs_ops = {
	.show = foo_attr_show,
	.store = foo_attr_store,
};

static void foo_release(struct kobject *kobj)
{
	struct foo_obj *foo;

	foo = to_foo_obj(kobj);
	kfree(foo);
}

static ssize_t foo_show(struct foo_obj *foo_obj, struct foo_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo_obj->foo);
}

static ssize_t foo_store(struct foo_obj *foo_obj, struct foo_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &foo_obj->foo);
	return count;
}

static struct foo_attribute foo_attribute =
	__ATTR(foo, 0666, foo_show, foo_store);

static ssize_t b_show(struct foo_obj *foo_obj, struct foo_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "baz") == 0)
		var = foo_obj->baz;
	else
		var = foo_obj->bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct foo_obj *foo_obj, struct foo_attribute *attr,
		       const char *buf, size_t count)
{
	int var;

	sscanf(buf, "%du", &var);
	if (strcmp(attr->attr.name, "baz") == 0)
		foo_obj->baz = var;
	else
		foo_obj->bar = var;
	return count;
}

static struct foo_attribute baz_attribute =
	__ATTR(baz, 0666, b_show, b_store);
static struct foo_attribute bar_attribute =
	__ATTR(bar, 0666, b_show, b_store);

static struct attribute *foo_default_attrs[] = {
	&foo_attribute.attr,
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,   /* need to NULL terminate the list of attributes */
};

static struct kobj_type foo_ktype = {
	.sysfs_ops = &foo_sysfs_ops,
	.release = foo_release,
	.default_attrs = foo_default_attrs,
};

static struct kset *example_kset;
static struct foo_obj *foo_obj;
static struct foo_obj *bar_obj;
static struct foo_obj *baz_obj;

static struct foo_obj *create_foo_obj(const char *name)
{
	struct foo_obj *foo;
	int retval;

	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return NULL;

	foo->kobj.kset = example_kset;

	retval = kobject_init_and_add(&foo->kobj, &foo_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&foo->kobj);
		return NULL;
	}

	kobject_uevent(&foo->kobj, KOBJ_ADD);

	return foo;
}

static void destroy_foo_obj(struct foo_obj *foo)
{
	kobject_put(&foo->kobj);
}

static int __init example_init(void)
{
	example_kset = kset_create_and_add("kset_example", NULL, kernel_kobj);
	if (!example_kset)
		return -ENOMEM;

	foo_obj = create_foo_obj("foo");
	if (!foo_obj)
		goto foo_error;

	bar_obj = create_foo_obj("bar");
	if (!bar_obj)
		goto bar_error;

	baz_obj = create_foo_obj("baz");
	if (!baz_obj)
		goto baz_error;

	return 0;

baz_error:
	destroy_foo_obj(bar_obj);
bar_error:
	destroy_foo_obj(foo_obj);
foo_error:
	return -EINVAL;
}

static void __exit example_exit(void)
{
	destroy_foo_obj(baz_obj);
	destroy_foo_obj(bar_obj);
	destroy_foo_obj(foo_obj);
	kset_unregister(example_kset);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
#endif
