#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("emirerdin443-beep");
MODULE_DESCRIPTION("Simple platform driver example");

#define DRIVER_NAME "simple_device"

static int simple_probe(struct platform_device *pdev) {
    printk(KERN_INFO "Simple driver probe called for device: %s\n", pdev->name);
    printk(KERN_INFO "Device ID: %d\n", pdev->id);
    dev_info(&pdev->dev, "Device initialized successfully\n");
    return 0;
}

static void simple_remove(struct platform_device *pdev) {
    printk(KERN_INFO "Simple driver remove called for device: %s\n", pdev->name);
    dev_info(&pdev->dev, "Device removed successfully\n");
}

static void simple_shutdown(struct platform_device *pdev) {
    printk(KERN_INFO "Simple driver shutdown called\n");
    dev_info(&pdev->dev, "Device shutdown\n");
}

static struct platform_driver simple_driver = {
    .probe = simple_probe,
    .remove = simple_remove,
    .shutdown = simple_shutdown,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
};

static int __init simple_driver_init(void) {
    printk(KERN_INFO "Simple driver module initializing...\n");
    int ret = platform_driver_register(&simple_driver);
    
    if (ret != 0) {
        printk(KERN_ERR "Failed to register platform driver\n");
        return ret;
    }
    
    printk(KERN_INFO "Simple platform driver registered successfully\n");
    return 0;
}

static void __exit simple_driver_exit(void) {
    printk(KERN_INFO "Simple driver module exiting...\n");
    platform_driver_unregister(&simple_driver);
    printk(KERN_INFO "Simple platform driver unregistered\n");
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);
