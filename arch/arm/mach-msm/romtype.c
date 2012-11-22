/*
 * Copyright (C) 2012 Chad Goodman, All Rights Reserved
 *
 */

/*
 * Possible values for "rom_type" are :
 *
 *   0 - Sense (default)
 *   1 - AOSP/CM10
 *
 * used to allow camera driver to use correct settings depending on rom type.  
 */



#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/romtype.h>

int rom_type;

/* sysfs interface for "rom_type" */

static ssize_t rom_type_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
return sprintf(buf, "%d\n", rom_type);
}

static ssize_t rom_type_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
int new_rom_type;

sscanf(buf, "%du", &new_rom_type);
if (new_rom_type >= ROM_SENSE && new_rom_type <= ROM_AOSP) {
	rom_type = new_rom_type;
}

return count;
}

static struct kobj_attribute rom_type_attribute =
__ATTR(rom_type, 0666, rom_type_show, rom_type_store);

static struct attribute *rom_type_attrs[] = {
&rom_type_attribute.attr,
NULL,
};

static struct attribute_group rom_type_attr_group = {
.attrs = rom_type_attrs,
};

static struct kobject *rom_type_kobj;

int rom_type_init(void)
{
	int rom_type_retval;

	rom_type = ROM_SENSE; /* sense by default */

        rom_type_kobj = kobject_create_and_add("rom_type", kernel_kobj);
        if (!rom_type_kobj) {
                return -ENOMEM;
        }
        rom_type_retval = sysfs_create_group(rom_type_kobj, &rom_type_attr_group);



        if (rom_type_retval)
                kobject_put(rom_type_kobj);

        return (rom_type_retval);
}

void rom_type_exit(void)
{
	kobject_put(rom_type_kobj);
}

module_init(rom_type_init);
module_exit(rom_type_exit);

