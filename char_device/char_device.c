#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/version.h>

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

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    size_t available = strnlen(kernelBuffer, BUFFER_SIZE);
    size_t read_size;

    if (*offset >= available)
        return 0;

    read_size = min_t(size_t, len, available - *offset);

    if (copy_to_user(buffer, kernelBuffer + *offset, read_size))
        return -EFAULT;

    *offset += read_size;

    printk(KERN_INFO "Sent %zu characters to user\n", read_size);
    return read_size;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    size_t write_size = min_t(size_t, len, BUFFER_SIZE - 1);

    memset(kernelBuffer, 0, BUFFER_SIZE);

    if (copy_from_user(kernelBuffer, buffer, write_size))
        return -EFAULT;

    kernelBuffer[write_size] = '\0';

    printk(KERN_INFO "Received %zu characters from user\n", write_size);
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
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    simpleClass = class_create(CLASS_NAME);
#else
    simpleClass = class_create(THIS_MODULE, CLASS_NAME);
#endif
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
