#define KERNEL_MODULE
#include "../common.h"

#define DEV_MAJOR_NUM   101
#define DEV_MINOR_NUM   0
#define DEV_CLASS       "gboxs"
#define DEV_NAME        "gboxs_dev"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
MODULE_DESCRIPTION("Char Driver");
MODULE_VERSION("1.0");

// ioctl macro
#define DEV_CMD_WRITE_CONFIG (_IOW(DEV_MAJOR_NUM, 0, uint8_t *))
#define DEV_CMD_READ_CONFIG (_IOR(DEV_MAJOR_NUM, 1, uint8_t *))
//

/* File operaitons definition*/
ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
long dev_unlocked_ioctl(struct file *, unsigned int, unsigned long);
long dev_compat_ioctl(struct file *, unsigned int, unsigned long);

static dev_t dev;
static struct class *dev_class;
static struct cdev c_dev;
static struct device *device;
static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_unlocked_ioctl,
    .compat_ioctl = dev_compat_ioctl,
};

static struct transfer_data
{
    uint8_t cmd;
    uint8_t buffer[256];
} data;

static struct config_data
{
    uint8_t config_item;
    uint8_t payload[512];
} config_data;

static uint8_t *kernel_buffer;

static struct task_struct *task1;
static struct timer_list timer1;
/* File operation declearing */

ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    pr_info("File read\n");
    memset(kernel_buffer, 0, 1024);
    memcpy(kernel_buffer, (uint8_t *)&data, sizeof(struct transfer_data));
    return copy_to_user(buf, kernel_buffer, len);
}

ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("File write\n");
    pr_info("File len = %ld\n", len);
    copy_from_user(kernel_buffer, buf, len);
    memcpy(&data, kernel_buffer, sizeof(struct transfer_data));
    pr_info("write data = %s\n", (char *)data.buffer);
    return 0;
}

int dev_open(struct inode *node, struct file *file)
{
    pr_info("File open\n");
    /*Creating Physical memory*/
    if ((kernel_buffer = kmalloc(1024, GFP_KERNEL)) == NULL)
    {
        pr_info("Cannot allocate memory in kernel\n");
        return -ENOMEM;
    }
    return 0;
}
int dev_release(struct inode *node, struct file *file)
{
    pr_info("File close\n");
    kfree(kernel_buffer);
    return 0;
}
long dev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("File ioctl\n");
    size_t len = sizeof(struct config_data);
    switch (cmd)
    {
    case DEV_CMD_WRITE_CONFIG:
        if (access_ok(arg, len))
        {
            uint8_t *buf = kmalloc(len, GFP_KERNEL);
            if (buf)
            {
                copy_from_user(buf, (uint8_t *)arg, len);
                memcpy(&config_data, buf, len);
                pr_info("Write configuration payload: %s\n", (char *)config_data.payload);
                kfree(buf);
                return len;
            }
        }
        else
        {
            pr_err("Data is not accessible\n");
        }
        break;

    case DEV_CMD_READ_CONFIG:
        if (access_ok(arg, len))
        {
            pr_info("Configuraion payload = %s\n",(char*) config_data.payload);
                copy_to_user((uint8_t*)arg,(uint8_t *)&config_data, len);
                pr_info("Read configuration\n");
                return len;
        }
        else
        {
            pr_err("Data is not accessible\n");
        }
        break;

    default:
        break;
    }
    return 0;
}
long dev_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("File compat ioctl\n");
    return 0;
}

/* End of file operation declearing*/

// Device thread managerment

static int thread_1_func(void *arg)
{
    static int counter = 0;
    // Allow the SIGKILL signal
    // allow_signal(SIGKILL);
    for (; !kthread_should_stop();)
    {
        if (counter++ >= 100)
        {
            counter=0;
            pr_info("Task state = %ld\n", task1->state);
            pr_info("Task stop!\n");
        }

        // if (signal_pending(task1))
        //     break;

        pr_info("Current task pid = %d\n", current->pid);
        msleep(100);
    }

    do_exit(0);
    return 0;
}

static int count;
void timer1_timeout(struct timer_list *timer)
{
    pr_info("Timer Callback function Called [%d]\n", count++);
    /*
       Re-enable timer. Because this function will be called only first time. 
       If we re-enable this will work like periodic timer. 
    */
    mod_timer(&timer1, jiffies + msecs_to_jiffies(1000));
}

static int init_thread(void)
{
    if ((task1 = kthread_run(thread_1_func, NULL, "thread1")) == NULL)
    {
        pr_err("Created new task failed\n");
        return -1;
    }

    timer_setup(&timer1, timer1_timeout, 0);
    add_timer(&timer1);
    mod_timer(&timer1, jiffies + msecs_to_jiffies(1000));

    return 0;
}

static int device_init(void)
{
    int er = -ENOMEM;
    if (alloc_chrdev_region(&dev, DEV_MAJOR_NUM, DEV_MINOR_NUM, DEV_NAME) < 0)
    {
        pr_err("Can not alloc cherdev region\n");
        return -ENOMEM;
    }

    cdev_init(&c_dev, &fops);
    c_dev.owner = THIS_MODULE;

    if (cdev_add(&c_dev, dev, 1) < 0)
    {
        pr_err("Can not add cdev\n");
        goto r_err;
    }

    if ((dev_class = class_create(THIS_MODULE, DEV_CLASS)) < 0)
    {
        pr_err("Can not create dev class\n");
        goto r_err;
    }

    if ((device = device_create(dev_class, NULL, dev, NULL, DEV_NAME)) == NULL)
    {
        pr_err("Can not create device\n");
        goto c_err;
    }

    device->bus= 

    pr_info("Device %s inserted successfully\n", DEV_NAME);
    return 0;

c_err:
    class_destroy(dev_class);
r_err:
    unregister_chrdev_region(dev, 1);
    return er;
}

static void clean_up(void)
{
    if (!del_timer(&timer1))
    {
        pr_err("Delete timer failed\n");
        try_to_del_timer_sync(&timer1); /* Do the full CPU query loop */
    }

    if (task1)
    {
        pr_info("Task1 state = %ld\n", task1->state);
        kthread_stop(task1);
    }

    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, 1);
}

int __init initialize(void)
{
    if (device_init() < 0)
    {
        pr_err("Module initialized failed\n");
        return -ENOMEM;
    }

    if (init_thread() < 0)
    {
        pr_err("Can not create kthread\n");
    }

    pr_info("Module has installed successfully!");
    return 0;
}

void __exit remove(void)
{
    clean_up();
    pr_info("Module has removed successfully!");
}

module_init(initialize);
module_exit(remove);
