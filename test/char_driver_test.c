#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>

#define DEV_MAJOR_NUM 101

#define DEV_CMD_WRITE_CONFIG (_IOW(DEV_MAJOR_NUM, 0, uint8_t *))
#define DEV_CMD_READ_CONFIG (_IOR(DEV_MAJOR_NUM, 1, uint8_t *))

#pragma pack(1)
struct transfer_data{
    uint8_t cmd;
    uint8_t buffer[256];
}data;

static struct config_data
{
    uint8_t config_item;
    uint8_t payload[512];
} config_data;


#pragma pack_pop()

void main()
{
    printf("hello world\n");
    int fd = open("/dev/gboxs_dev", O_RDWR);
    printf("fd = %d\n", fd);

    if (fd < 0)
    {
        printf("Error when open dev file\n");
        return;
    }
    uint8_t buffer[1024] = {0};
    snprintf(data.buffer, sizeof(data.buffer), "Hello world\n");

    memcpy(buffer, (uint8_t*) &data, sizeof(struct transfer_data));

    int bytes = write(fd, buffer, sizeof(buffer));

    printf("Total sent bytes = %d\n", bytes);
    // fsync(fd);
    uint8_t read_buff[1024] = {0};
    read(fd, read_buff, sizeof(read_buff));
    printf("read data: %s\n", read_buff);

    memcpy((uint8_t*)&data, read_buff, sizeof(read_buff));
    printf("buffer = %s\n", (char*)data.buffer);

    size_t len = sizeof(struct config_data);

    snprintf((char*) config_data.payload, sizeof(config_data.payload), "This is configuration payload\n");

    ioctl(fd, DEV_CMD_WRITE_CONFIG, (uint8_t*)&config_data);

    struct config_data* c_data = (struct config_data*)malloc(sizeof(struct config_data));
    
    ioctl(fd, DEV_CMD_READ_CONFIG, (uint8_t*) c_data);

    printf("Read payload = %s\n", (char*)c_data->payload);

    int a = 0;
    scanf("%d\n", &a);

    close(fd);
}