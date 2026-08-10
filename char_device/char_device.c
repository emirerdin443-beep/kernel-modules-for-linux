#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("emirerdin443-beep");
MODULE_DESCRIPTION("Simple character device driver");

#define DEVICE_NAME "simple_char"
#define CLASS_NAME "simple_class"
#define BUFFER_SIZE 256

static int majorNumber;
static struct class* simpleClass = NULL;
static struct device* simpleDevice = NULL;
static char kernelBuffer[BUFFER_SIZE] = {0};

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    int read_size = len > BUFFER_SIZE ? BUFFER_SIZE : len;
    
    error_count = copy_to_user(buffer, kernelBuffer, read_size);
    
    if (error_count == 0) {
        printk(KERN_INFO "Sent %d characters to user\n", read_size);
        return read_size;
    } else {
        printk(KERN_INFO "Failed to send %d characters to user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int write_size = len > BUFFER_SIZE ? BUFFER_SIZE : len;
    
    copy_from_user(kernelBuffer, buffer, write_size);
    
    printk(KERN_INFO "Received %d characters from user\n", write_size);
    return write_size;
}

static struct file_operations fops = {
    .read = dev_read,
    .write = dev_write,
};

static int __init char_device_init(void) {
    printk(KERN_INFO "Character device module initializing...\n");
    
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return majorNumber;
    }
    
    simpleClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(simpleClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(simpleClass);
    }
    
    simpleDevice = device_create(simpleClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(simpleDevice)) {
        class_destroy(simpleClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create device\n");
        return PTR_ERR(simpleDevice);
    }
    
    printk(KERN_INFO "Character device created successfully (major: %d)\n", majorNumber);
    return 0;
}

static void __exit char_device_exit(void) {
    device_destroy(simpleClass, MKDEV(majorNumber, 0));
    class_unregister(simpleClass);
    class_destroy(simpleClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Character device module removed\n");
}

module_init(char_device_init);
module_exit(char_device_exit);
