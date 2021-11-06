#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

void main()
{
    int fd = open("/dev/i2c-22", O_RDWR);
    if (fd < 0)
    {
        printf("Open failed\n");
        return;
    }

    int addr = 0x40; /* The I2C address */
    // if (ioctl(fd, I2C_SLAVE, addr) < 0)
    // {
    //     printf("Slave address error\n");
    //     /* ERROR HANDLING; you can check errno to see what went wrong */
    //     exit(1);
    // }
    __u8 reg = 0x10; /* Device register to access */
    __s32 res;
    char buf[10];
    /* Using SMBus commands */
    res = i2c_smbus_read_word_data(fd, reg);
    if (res < 0)
    {
        printf("ERROR HANDLING: I2C transaction failed\n");
    }
    else
    {
        printf("res contains the read word\n");
    }
    /*
* Using I2C Write, equivalent of
* i2c_smbus_write_word_data(file, reg, 0x6543)
*/
    buf[0] = reg;
    buf[1] = 0x43;
    buf[2] = 0x65;
    if (write(fd, buf, 3) != 3)
    {
        printf("ERROR HANDLING: I2C transaction failed\n");
    }
    /* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
    if (read(fd, buf, 1) != 1)
    {
        printf("ERROR HANDLING: I2C transaction failed\n");
    }
    else
    {
        /* buf[0] contains the read byte */
    }
}