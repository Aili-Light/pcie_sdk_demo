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

#ifdef __MINGW32__
    #ifdef ALG_SDK_EXPORT
        #define ALG_SDK_API __declspec(dllexport)
    #else
        #define ALG_SDK_API __declspec(dllimport)
    #endif // ALG_SDK_EXPORT
#else
    #define ALG_SDK_API extern
#endif // MINGW32

int alg_sdk_push2q(const void* msg);
int alg_sdk_init_server(const char* url);
int alg_sdk_stop_server(void);

#ifdef __cplusplus
}
#endif
#endif
