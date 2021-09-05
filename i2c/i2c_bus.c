#include "../common.h"
#include <linux/i2c.h>

#define I2C_ADAPTER_NAME "ETX_I2C_ADAPTER"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
MODULE_DESCRIPTION("I2C Driver_Bus");
MODULE_VERSION("1.0");


static int dev_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
                           int num)
{
    pr_info("i2c_master_xfer\n");
    return 0;
}
static int dev_i2c_master_xfer_atomic(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    pr_info("i2c_master_xfer_atomic\n");
    return 0;
}

static int dev_i2c_smbus_xfer(struct i2c_adapter *adap, u16 addr, unsigned short flags, char read_write, u8 command,
                   int size, union i2c_smbus_data *data)
{
    pr_info("i2c_smbus_xfer\n");
    return 0;
}

static int dev_i2c_smbus_xfer_atomic(struct i2c_adapter *adap, u16 addr, unsigned short flags, char read_write, u8 command,
                          int size, union i2c_smbus_data *data)
{
    pr_info("i2c_smbus_xfer_atomic\n");
    return 0;
}

/* To determine what the adapter supports */
static u32 i2c_functionality(struct i2c_adapter *adap)
{
    return (I2C_FUNC_I2C |
            I2C_FUNC_SMBUS_QUICK |
            I2C_FUNC_SMBUS_BYTE |
            I2C_FUNC_SMBUS_BYTE_DATA |
            I2C_FUNC_SMBUS_WORD_DATA |
            I2C_FUNC_SMBUS_BLOCK_DATA);
}

static struct i2c_algorithm i2c_algo =
    {
        .master_xfer = dev_i2c_master_xfer,
        .master_xfer_atomic = dev_i2c_master_xfer_atomic,
        .smbus_xfer = dev_i2c_smbus_xfer,
        .smbus_xfer_atomic = dev_i2c_smbus_xfer_atomic,
        .functionality = i2c_functionality};

static struct i2c_adapter i2c_adapt =
    {
        .owner = THIS_MODULE,
        .class = I2C_CLASS_HWMON, /* classes to allow probing for */
        .algo = &i2c_algo,
        .name = I2C_ADAPTER_NAME};

static int __init initialize(void)
{
    int ret = i2c_add_adapter(&i2c_adapt);
    pr_info("Add new I2c Adapter to i2c bus successfully\n");
    return ret;
}

static void __exit remove(void)
{
    i2c_del_adapter(&i2c_adapt);
    pr_info("Module has removed successfully!");
}

module_init(initialize);
module_exit(remove);
