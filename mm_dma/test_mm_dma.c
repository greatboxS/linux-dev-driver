#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#define DEV_NAME "/dev/mm-dma-dev"
#define PAGE_SIZE 4096
#define DMA_BUFFER_SIZE (PAGE_SIZE*2)

// ioctl macro
#define DEV_CMD_WRITE_CONFIG (_IOW(0, 0, uint8_t *))
#define DEV_CMD_READ_CONFIG (_IOR(0, 1, uint8_t *))
#define DEV_CMD_QBUF (_IOWR(0, 2, uint8_t *))
#define DEV_CMD_DQBUF (_IOWR(0, 3, uint8_t *))

int main()
{
    int fd = open(DEV_NAME, O_RDWR);

    if (fd > 0)
    {
        uint8_t *dma_buffer = mmap(NULL, DMA_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        if(dma_buffer != MAP_FAILED)
        {
            printf("mapping buffer successfully. Virtual address = %p\n", dma_buffer);

            int8_t data = 0;
            if(ioctl(fd, DEV_CMD_DQBUF, &data) == 0)
            {
                printf("dequeue successed\n");
                printf("%s\n", (char*)dma_buffer);
            }
            else 
            {
                printf("dequeue failed\n");
            }

            if(ioctl(fd, DEV_CMD_QBUF, &data) == 0)
            {
                printf("queue successed\n");
            }
            else 
            {
                printf("queue failed\n");
            }


        }
        else{
            printf("mapping failed\n");
        }

        close(fd);
        return 0;
    }

    printf("Open file failed\n");
    return 0;
}