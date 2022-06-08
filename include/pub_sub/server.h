#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include <pthread.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#include "pub_sub/util.h"
#include "pub_sub/log.h"
#include "pub_sub/basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int alg_sdk_push2q(const void* msg);
int alg_sdk_init_server(const char* url);
int alg_sdk_stop_server(void);

#ifdef __cplusplus
}
#endif
#endif
