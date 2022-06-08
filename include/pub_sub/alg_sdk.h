#ifndef __ALG_SDK_H__
#define __ALG_SDK_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include "pub_sub/util.h"
#include "pub_sub/log.h"
#include "pub_sub/basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int alg_sdk_init(const int frame_rate);
int alg_sdk_stop();
int alg_sdk_append_task(alg_sdk_amp_msg_t* ptr, void* control);

#ifdef __cplusplus
}
#endif
#endif