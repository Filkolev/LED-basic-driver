#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>

#define GPIO_DEFAULT 6
#define HIGH 1
#define LOW 0

static dev_t dev;

static int gpio_num = GPIO_DEFAULT;
module_param(gpio_num, int, S_IRUGO);
MODULE_PARM_DESC(gpio_num, "The gpio where the LED is connected (default = 6)");

static int __init init_led(void)
{
	int ret;

	ret = alloc_chrdev_region(&dev, 0, 1, "simple-led");
	if (ret) {
		pr_err("Failed to allocate char device region.\n");
		return ret;
	}

	ret = setup_led_gpio(gpio_num);
	if (ret) {
		pr_err("Failed to setup GPIO.\n");
		/* TODO: chrdev region */
		return ret;
	}	
}

static void __exit exit_led(void)
{

}

static int setup_led_gpio(int gpio)
{
	int ret;

	ret = 0;
	if (!gpio_is_valid(gpio)) {
		pr_err("Failed validation of GPIO %d\n", gpio);
		return -EINVAL;
	}

	pr_info("Validation succeeded for GPIO %d\n", gpio);

	ret = gpio_request(gpio, "sysfs");
	if (ret < 0) {
		pr_err("GPIO request failed. Exiting.\n");
		return ret;
	}

	gpio_direction_output(gpio, LOW);
	gpio_export(gpio, true);

	return ret;
}

module_init(init_led);
module_exit(exit_led);

MODULE_AUTHOR("Filip Kolev");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Very simple driver to control a LED using ioremap()");
