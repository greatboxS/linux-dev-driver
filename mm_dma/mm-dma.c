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
#include <linux/dma-mapping.h>

int __init mm_dma_init(void)
{
    pr_info("register mm-dma module successfully\n");
    return 0;
}


void __exit mm_dma_exit(void)
{
    pr_info("remove mm-dma module successfully\n");
}   


module_init(mm_dma_init);
module_exit(mm_dma_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greatboxs <https://github.com/greatboxs>");
MODULE_DESCRIPTION("Memory mapping and DMA demo driver");
MODULE_VERSION("1.0");
