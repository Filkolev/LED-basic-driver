#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <asm/io.h>

#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000)
#define GPIO_DEFAULT 6
#define HIGH 1
#define LOW 0

static ssize_t
led_read(struct file *filep, char __user *buf, size_t len, loff_t *off);

static ssize_t
led_write(struct file *filep, const char __user *buf, size_t len, loff_t *off);

static int led_open(struct inode *inodep, struct file *filep);
static int led_release(struct inode *inodep, struct file *filep);

static dev_t devt;
static struct cdev led_cdev;
static struct class *led_class;
static struct device *led_device;
static void __iomem *iomap;
static void *addr;

static unsigned int val;
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_release,
	.read = led_read,
	.write = led_write,
};

/* Only one process at a time will be allowed to work with the LED */
static DEFINE_MUTEX(led_mutex);

static int gpio_num = GPIO_DEFAULT;
module_param(gpio_num, int, S_IRUGO);
MODULE_PARM_DESC(gpio_num, "The gpio where the LED is connected (default = 6)");

static int __init init_led(void)
{
	int ret;

	ret = alloc_chrdev_region(&devt, 0, 1, "simple-led");
	if (ret) {
		pr_err("Failed to allocate char device region.\n");
		goto out;
	}

	cdev_init(&led_cdev, &fops);
	led_cdev.owner = THIS_MODULE;
	ret = cdev_add(&led_cdev, devt, 1);
	if (ret) {
		pr_err("Failed to add cdev.\n");
		goto cdev_err;
	}

	led_class = class_create(THIS_MODULE, "led-test");
	if (IS_ERR(led_class)) {
		pr_err("class_create() failed.\n");
		ret = PTR_ERR(led_class);
		goto class_err;
	}

	led_device = device_create(led_class, NULL, devt, NULL, "simple-led");
	if (IS_ERR(led_device)) {
		pr_err("device_create() failed.\n");
		ret = PTR_ERR(led_device);
		goto dev_err;
	}	

//	if (!request_mem_region(GPIO_BASE, 0x38, "led-gpio")) {
//		pr_err("Unable to request mem region.\n");
//		ret = -EINVAL;
//		goto region_err;
//	}		

	iomap = ioremap(GPIO_BASE, PAGE_ALIGN(0x38));
	if (!iomap) {
		pr_err("ioremap() failed.\n");
		ret = -EINVAL;
		goto remap_err;
	}
//	addr = phys_to_virt(GPIO_BASE);
	gpio_request(gpio_num, "simple-led");
	gpio_direction_output(gpio_num, LOW);

	mutex_init(&led_mutex);
	pr_info("BASE: %lld\n", (long long)GPIO_BASE);
//	pr_info("simple-led module loaded. addr = %lld\n", (unsigned long long)addr);

//	val = ioread32(iomap + (gpio_num / 10));
//	pr_info("readl: %ud", val);
//	val &= ~(7 << ((gpio_num % 10) * 3));
//	val |= 1 << ((gpio_num % 10) * 3);
//	iowrite32(val, iomap + (gpio_num / 10));
	int i;
	for (i = 0; i < 13; i++) { 
		pr_info("[low] offset %d: %ud", i, readl(iomap + i));
	}
//	iowrite32(1 << gpio_num, iomap + 7);
	gpio_set_value(gpio_num, HIGH);
	for (i = 0; i < 13; i++) { 
		pr_info("[high] offset %d: %ud", i, readl(iomap + i));
	};
	goto out;

remap_err:
//	release_mem_region(GPIO_BASE, 0x38);	
region_err:
	device_destroy(led_class, devt);
dev_err:
	class_unregister(led_class);
	class_destroy(led_class);	
class_err:
	cdev_del(&led_cdev);
cdev_err:
	unregister_chrdev_region(devt, 1);
out:
	return ret;	
}

static void __exit exit_led(void)
{
//	iowrite32(1 << gpio_num, iomap + 10);	
	gpio_set_value(gpio_num, LOW);
	gpio_free(gpio_num);
	mutex_destroy(&led_mutex);
	iounmap(iomap);
//	release_mem_region(GPIO_BASE, 0x38);
	device_destroy(led_class, devt);
	class_unregister(led_class);
	class_destroy(led_class);	
	cdev_del(&led_cdev);
	unregister_chrdev_region(devt, 1);
}

static int led_open(struct inode *inodep, struct file *filep)
{
	mutex_lock(&led_mutex);
	return 0;
}

static int led_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&led_mutex);
	return 0;
}

static ssize_t
led_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	return 0;
}

static ssize_t
led_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
	return 0;
}

module_init(init_led);
module_exit(exit_led);

MODULE_AUTHOR("Filip Kolev");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Very simple driver to control a LED using ioremap()");
