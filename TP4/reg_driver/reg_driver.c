#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

struct cdev * p_vircdev;
dev_t dev_num;
int major_number;
int minor_number;

#define DEVICE_NAME "my_virtual_device"
#define FIRST_MINOR 0
#define NB_MINOR_ID 1

static int driver_init(void) 
{
	int err; 
	
	// ...

	if (err < 0) {
		printk(KERN_ALERT "%s: failed to obtain device numbers\n", DEVICE_NAME);
		return err;
	}

	// ...

	if (err < 0) {
		printk(KERN_ALERT "%s: unable to add cdev to kernel\n", DEVICE_NAME);
		return err;
	}
        printk(KERN_ALERT "%s: module successfully loaded\n", DEVICE_NAME);
	printk(KERN_INFO "\tuse \"mknod /dev/%s c %d %d\" for device file", DEVICE_NAME, major_number, minor_number);
	return 0;
}

static void driver_cleanup(void)
{
	// ...

	printk(KERN_ALERT "Module %s successfully unloaded\n", DEVICE_NAME);
}

module_init(driver_init);
module_exit(driver_cleanup);
