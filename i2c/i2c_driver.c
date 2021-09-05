#include "../common.h"
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
MODULE_DESCRIPTION("I2C Driver");
MODULE_VERSION("1.0");

#define I2C_BUS_AVAILABLE (1)                     // I2C Bus that we have created
#define SLAVE_DEVICE_NAME ("esp32")             // Device and Driver Name
#define SSD1306_SLAVE_ADDR (0x3C)                  // SSD1306 OLED Slave Address
static struct i2c_adapter *dev_i2c_adapter = NULL; // I2C Adapter Structure
static struct i2c_client *dev_i2c_client = NULL;   // I2C Cient Structure (In our case it is OLED)

/* Standard driver model interfaces */
int dev_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_info("dev_probe\n");
    return 0;
}
int dev_remove(struct i2c_client *client)
{
    pr_info("dev_remove\n");
    return 0;
}
/* Device detection callback for automatic device creation */
int dev_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    pr_info("dev_detect\n");
    return 0;
}

// /* a ioctl like command that can be used to perform specific functions
//  * with the device.
//  */
// int (*command)(struct i2c_client *client, unsigned int cmd, void *arg);

static const struct i2c_device_id dev_i2c_id[] = {
    {SLAVE_DEVICE_NAME, 0},
    {}};

static struct i2c_driver dev_i2c_driver =
    {
        .probe = dev_probe,
        .remove = dev_remove,
        .detect = dev_detect,
        .id_table = dev_i2c_id,
        .driver = {
            .name = SLAVE_DEVICE_NAME,
            .owner = THIS_MODULE,
        },
};

/*
** Structure that has slave device id
*/
MODULE_DEVICE_TABLE(i2c, dev_i2c_id);
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info dev_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SSD1306_SLAVE_ADDR)};

/*
** Module Init function
*/
static int __init i2c_driver_init(void)
{
    dev_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if (dev_i2c_adapter != NULL)
    {
        dev_i2c_client = i2c_new_device(dev_i2c_adapter, &dev_i2c_board_info);

        if (dev_i2c_client != NULL)
        {
            i2c_add_driver(&dev_i2c_driver);
        }

        i2c_put_adapter(dev_i2c_adapter);
    }

    pr_info("Client Driver Added!!!\n");
    return 0;
}
/*
** Module Exit function
*/
static void __exit i2c_driver_exit(void)
{
    i2c_unregister_device(dev_i2c_client);
    i2c_del_driver(&dev_i2c_driver);
    pr_info("Client Driver Removed!!!\n");
}
module_init(i2c_driver_init);
module_exit(i2c_driver_exit);