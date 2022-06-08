#ifndef __ALG_SDK_H__
#define __ALG_SDK_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <netinet/in.h>  /* For htonl and ntohl */
#include <unistd.h>

#include <pthread.h>
#include "util.h"
#include "log.h"
#include "basic_types.h"

int alg_sdk_init(const int frame_rate);
int alg_sdk_stop();
int alg_sdk_appeend_task(alg_sdk_amp_msg_t* ptr, void* control);

#endif