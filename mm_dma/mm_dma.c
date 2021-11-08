/**
 * @file mm-dma.c
 * @author greatboxs <https://github.com/greatboxs>
 * @brief 
 * @version 0.1
 * @date 2021-11-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "../common.h"
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>

#define DEV_MAJOR_NUM 0
#define DEV_MINOR_NUM 1
#define DEV_BUFF_SIZE (PAGE_SIZE * 2)
#define DEV_CLASS "mm-dma-dev"
#define DEV_NAME "mm-dma-dev"

#define DMA_BUFF_SIZE DEV_BUFF_SIZE

#if defined(CONFIG_ARM)
#define _PGPROT_NONCACHED(vm_page_prot) pgprot_noncached(vm_page_prot)
#define _PGPROT_WRITECOMBINE(vm_page_prot) pgprot_writecombine(vm_page_prot)
#define _PGPROT_DMACOHERENT(vm_page_prot) pgprot_dmacoherent(vm_page_prot)
#elif defined(CONFIG_ARM64)
#define _PGPROT_NONCACHED(vm_page_prot) pgprot_noncached(vm_page_prot)
#define _PGPROT_WRITECOMBINE(vm_page_prot) pgprot_writecombine(vm_page_prot)
#define _PGPROT_DMACOHERENT(vm_page_prot) pgprot_writecombine(vm_page_prot)
#else
#define _PGPROT_NONCACHED(vm_page_prot) pgprot_noncached(vm_page_prot)
#define _PGPROT_WRITECOMBINE(vm_page_prot) pgprot_writecombine(vm_page_prot)
#define _PGPROT_DMACOHERENT(vm_page_prot) pgprot_writecombine(vm_page_prot)
#endif

// ioctl macro
#define DEV_CMD_WRITE_CONFIG (_IOW(DEV_MAJOR_NUM, 0, uint8_t *))
#define DEV_CMD_READ_CONFIG (_IOR(DEV_MAJOR_NUM, 1, uint8_t *))
#define DEV_CMD_QBUF (_IOWR(DEV_MAJOR_NUM, 2, uint8_t *))
#define DEV_CMD_DQBUF (_IOWR(DEV_MAJOR_NUM, 3, uint8_t *))
//

/* File operaitons definition*/
ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
int dev_open(struct inode *, struct file *);
int dev_release(struct inode *, struct file *);
long dev_unlocked_ioctl(struct file *, unsigned int, unsigned long);
long dev_compat_ioctl(struct file *, unsigned int, unsigned long);
int dev_mmap(struct file *file, struct vm_area_struct *vma);
//

static struct char_dev_t
{
    dev_t dev;               // request major and minor number
    struct class *dev_class; // device class
    struct cdev c_dev;       // device control
    struct device *device;   // device instance
    uint8_t *buffer;         // device buffer (kzalloc())
    struct mutex mtx;
    int sync_mode;
    int is_open;
    int mtx_lock;
    struct dma_dev_t
    {
        uint8_t *buffer;
        size_t size;
        dma_addr_t dma_handler;
        void *virt_addr;
        int created;
    } dma; // dma
} mm_dma_dev;

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_unlocked_ioctl,
    .compat_ioctl = dev_compat_ioctl,
    .mmap = dev_mmap};

/* File operation declearing */

ssize_t dev_read(struct file *file, char __user *buff, size_t len, loff_t *off)
{
    struct char_dev_t *this = file->private_data;
    int result = 0;
    size_t xfer_size;
    size_t remain_size;
    dma_addr_t handler;
    void *virt_addr;

    if (mutex_lock_interruptible(&this->mtx))
    {
        pr_err("Mutex lock interruptible failed\n");
        return -ERESTARTSYS;
    }

    if (*off >= this->dma.size)
    {
        pr_err("The offset is greatter than dma buffer size\n");
        result = 0;
        goto return_unlock;
    }

    handler = this->dma.dma_handler + *off;
    virt_addr = this->dma.virt_addr + *off;
    xfer_size = (*off + len >= this->dma.size) ? this->dma.size - *off : len;

    // if ((file->f_flags & O_SYNC))
    //     dma_sync_single_for_cpu(this->device, handler, xfer_size, DMA_FROM_DEVICE);

    if ((remain_size = copy_to_user(buff, virt_addr, xfer_size)) != 0)
    {
        pr_err("Write to dma buffer failed\n");
        result = 0;
        goto return_unlock;
    }

    // if ((file->f_flags & O_SYNC))
    //     dma_sync_single_for_device(this->device, handler, xfer_size, DMA_FROM_DEVICE);

    *off += xfer_size;
    result = xfer_size;
return_unlock:
    mutex_unlock(&this->mtx);
    return result;
}

ssize_t dev_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
    struct char_dev_t *this = file->private_data;
    int result = 0;
    size_t xfer_size;
    size_t remain_size;
    dma_addr_t handler;
    void *virt_addr;

    if (mutex_lock_interruptible(&this->mtx))
    {
        pr_err("Mutex lock interruptible failed\n");
        return -ERESTARTSYS;
    }

    if (*off >= this->dma.size)
    {
        pr_err("The offset is greatter than dma buffer size\n");
        result = 0;
        goto return_unlock;
    }

    handler = this->dma.dma_handler + *off;
    virt_addr = this->dma.virt_addr + *off;
    xfer_size = (*off + len >= this->dma.size) ? this->dma.size - *off : len;

    // if ((file->f_flags & O_SYNC))
    //     dma_sync_single_for_cpu(this->device, handler, xfer_size, DMA_FROM_DEVICE);

    if ((remain_size = copy_to_user(buff, virt_addr, xfer_size)) != 0)
    {
        pr_err("Write to dma buffer failed\n");
        result = 0;
        goto return_unlock;
    }

    // if ((file->f_flags & O_SYNC))
    //     dma_sync_single_for_device(this->device, handler, xfer_size, DMA_FROM_DEVICE);

    *off += xfer_size;
    result = xfer_size;
return_unlock:
    mutex_unlock(&this->mtx);
    return result;
}

int dev_open(struct inode *node, struct file *file)
{
    struct char_dev_t *this = &mm_dma_dev;
    pr_info("Open mm-dma device\n");

    //this = container_of(node->i_cdev, struct char_dev_t, c_dev);
    file->private_data = this;
    this->is_open = 1;
    return 0;
}

int dev_release(struct inode *node, struct file *file)
{
    struct char_dev_t *this = file->private_data;
    this->is_open = 0;
    pr_info("Close mm-dma device\n");
    return 0;
}
long dev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct char_dev_t *this = file->private_data;
    long result = -1;
    pr_info("File ioctl\n");
    int8_t *buffer = (int8_t *)arg;
    switch (cmd)
    {
    case DEV_CMD_WRITE_CONFIG:
        result = 0;
        break;

    case DEV_CMD_READ_CONFIG:
        result = 0;
        break;

    case DEV_CMD_QBUF:
        if (this->mtx_lock)
            mutex_unlock(&this->mtx);
        this->mtx_lock = 0;
        pr_warn("mutex unlock from userspace\n");
        result = 0;
        break;

    case DEV_CMD_DQBUF:
        if (mutex_trylock(&this->mtx))
        {
            this->mtx_lock = 1;
            pr_warn("mutex lock successfully from userspace\n");
            *buffer = 0;
            result = 0;
        }
        else
        {
            pr_warn("mutex lock failed\n");
            *buffer = -1;
        }
        break;

    default:
        result = -1;
        break;
    }
    return result;
}
long dev_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("File compat ioctl\n");
    return 0;
}

int dev_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct char_dev_t *this = file->private_data;

    pr_info("mapping data to userspace\n");

    pr_info("vma->vm_pgoff  = %ld\n", vma->vm_pgoff);
    pr_info("vma_pages(vma)  = %ld\n", vma_pages(vma));
    pr_info("(this->dma.size >> PAGE_SHIFT) = %d\n", (this->dma.size >> PAGE_SHIFT));

    if (vma->vm_pgoff + vma_pages(vma) > (this->dma.size >> PAGE_SHIFT))
    {
        return -EINVAL;
    }

    vma->vm_page_prot = _PGPROT_DMACOHERENT(vma->vm_page_prot);

    vma->vm_private_data = this;

    pr_info("this->dma.dma_handler = %lld\n", this->dma.dma_handler);

#if 0 // other solution, it works find 
    struct page *page = NULL;
    page = virt_to_page((unsigned long)this->dma.virt_addr + (vma->vm_pgoff << PAGE_SHIFT));
    if (remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), this->dma.size, vma->vm_page_prot) == 0)
    {
        pr_info("mapping successed\n");
        return 0;
    }
    else
        return -1;
#endif
    return dma_mmap_coherent(this->device, vma, this->dma.virt_addr, this->dma.dma_handler, this->dma.size);
}
/* End of file operation declearing*/

/**
 * @brief this function used to set the permission of dev file
 * 
 * @param dev 
 * @param mode 
 * @return char* 
 */
static char *devnode(struct device *dev, umode_t *mode)
{
    if (!mode)
        return NULL;
    *mode = 0666;   // rw-rw-rw
    return NULL;
}

static int device_init(void)
{
    int er = -ENOMEM;
    if (alloc_chrdev_region(&mm_dma_dev.dev, DEV_MAJOR_NUM, DEV_MINOR_NUM, DEV_NAME) < 0)
    {
        pr_err("Can not alloc cherdev region\n");
        return -ENOMEM;
    }

    cdev_init(&mm_dma_dev.c_dev, &fops);
    mm_dma_dev.c_dev.owner = THIS_MODULE;

    if (cdev_add(&mm_dma_dev.c_dev, mm_dma_dev.dev, 1) < 0)
    {
        pr_err("Can not add cdev\n");
        goto r_err;
    }

    if ((mm_dma_dev.dev_class = class_create(THIS_MODULE, DEV_CLASS)) < 0)
    {
        pr_err("Can not create dev class\n");
        goto r_err;
    }

    mm_dma_dev.dev_class->devnode = devnode;

    if ((mm_dma_dev.device = device_create(mm_dma_dev.dev_class, NULL, mm_dma_dev.dev, NULL, DEV_NAME)) == NULL)
    {
        pr_err("Can not create device\n");
        goto c_err;
    }

    mutex_init(&mm_dma_dev.mtx);

    mm_dma_dev.buffer = (uint8_t *)kzalloc(DEV_BUFF_SIZE, GFP_KERNEL);

    pr_info("Device %s inserted successfully\n", DEV_NAME);

    dev_set_drvdata(mm_dma_dev.device, &mm_dma_dev);
    return 0;

c_err:
    class_destroy(mm_dma_dev.dev_class);
r_err:
    unregister_chrdev_region(mm_dma_dev.dev, 1);
    return er;
}

static int device_dma_init(void)
{
    u64 mask = 0;
    mm_dma_dev.dma.size = DMA_BUFF_SIZE;
    mm_dma_dev.dma.created = 0;

    mask = dma_get_required_mask(mm_dma_dev.device);
    pr_info("Device mask is 0x%llX\n", mask);

    if (!dma_set_mask(mm_dma_dev.device, mask))
    {
        dma_set_coherent_mask(mm_dma_dev.device, mask);
    }
    else
    {
        pr_warn("No suitable DMA available.\n");
    }

    mm_dma_dev.dma.virt_addr = dma_alloc_coherent(mm_dma_dev.device, mm_dma_dev.dma.size, &mm_dma_dev.dma.dma_handler, GFP_KERNEL);

    memcpy(mm_dma_dev.dma.virt_addr, "Hello from kernel\n", sizeof("Hello from kernel\n"));

    if (mm_dma_dev.dma.virt_addr == NULL)
    {
        pr_err("dma allocate coherent failed\n");
        return -1;
    }

    mm_dma_dev.dma.created = 1;

    pr_info("Allocate and set mask to dma successfully\n");
    return 0;
}

static void clean_up(void)
{
    if (mm_dma_dev.buffer)
        kfree(mm_dma_dev.buffer);

    if (mm_dma_dev.dma.created)
        dma_free_coherent(mm_dma_dev.device, mm_dma_dev.dma.size, mm_dma_dev.dma.virt_addr, mm_dma_dev.dma.dma_handler);

    device_destroy(mm_dma_dev.dev_class, mm_dma_dev.dev);
    class_destroy(mm_dma_dev.dev_class);
    cdev_del(&mm_dma_dev.c_dev);
    unregister_chrdev_region(mm_dma_dev.dev, 1);
}

int __init mm_dma_init(void)
{
    if (device_init() == 0)
    {
        pr_info("register mm-dma module successfully\n");

        if (device_dma_init() != 0)
        {
            clean_up();
            return -1;
        }
    }

    return 0;
}

void __exit mm_dma_exit(void)
{
    clean_up();
    pr_info("remove mm-dma module successfully\n");
}

module_init(mm_dma_init);
module_exit(mm_dma_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
MODULE_DESCRIPTION("Memory mapping and DMA demo driver");
MODULE_VERSION("1.0");
