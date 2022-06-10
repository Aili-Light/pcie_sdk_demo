#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>

#include "alg_sdk/log.h"
#include "alg_sdk/basic_types.h"
#include "alg_sdk/util.h"

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

typedef void(*alg_sdk_callback_func)(void* p);

ALG_SDK_API int alg_sdk_subscribe (const char* topic, alg_sdk_callback_func consumer);
ALG_SDK_API int alg_sdk_stop_client();
ALG_SDK_API int alg_sdk_init_client();

#ifdef __cplusplus
}
#endif
#endif
