#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "basic_types.h"
#include "log.h"

typedef struct alg_sdk_pcie_item_manager{
    uint8_t                 msg_type;
    pcie_common_head_t      common_head;
    pcie_image_info_meta_t  image_info_meta;
    pcie_msgs_meta_t        msgs_meta;
    pcie_poc_info_meta_t    poc_info_meta;
    void                    *payload;
}pcie_item_manager_t;

typedef struct alg_sdk_pcie_item_queue {
    pcie_item_manager_t     buffer[ALG_SDK_IMAGE_QUEUE_MAX_LEN];
    int                     head;
    int                     tail;
}pcie_item_queue_t;

int alg_sdk_queue_item_pop(pcie_item_queue_t* p, pcie_item_manager_t* t);
int alg_sdk_queue_item_push(pcie_item_queue_t* p, pcie_item_manager_t* t);
int alg_sdk_queue_item_is_empty(pcie_item_queue_t* p);
int alg_sdk_queue_item_is_full(pcie_item_queue_t* p);
int alg_sdk_queue_item_clear(pcie_item_queue_t* p);
int alg_sdk_queue_item_init(pcie_item_queue_t* p);
void alg_sdk_move_on(int *v);
unsigned char crc_array(unsigned char *p, unsigned char counter);
uint64_t milliseconds (void);
unsigned short crc16_array(void *buf,int count);

#endif
