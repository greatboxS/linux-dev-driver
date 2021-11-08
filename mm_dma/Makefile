obj-m += mm_dma.o


KDIR = /lib/modules/$(shell uname -r)/build
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean

test:
	gcc test_mm_dma.c -o test_mm_dma

