#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/completion.h>           // for thread completed check


/**
 * @brief 
 * MODULE_LICENSE("GPL");
 * MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
 * MODULE_DESCRIPTION("Module descriptor");
 * MODULE_VERSION("1.0");
 * 
 */
#endif // __COMMON_H__