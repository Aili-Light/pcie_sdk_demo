#ifndef __SERVICE_H__
#define __SERVICE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include "alg_sdk/util.h"
#include "alg_sdk/log.h"
#include "alg_sdk/basic_types.h"

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

int alg_sdk_init_service(const char* url);
int alg_sdk_stop_service();
ALG_SDK_API int alg_sdk_call_service (const char *topic_name, void* msg, int timeout);
void alg_sdk_service_append_task(alg_sdk_amp_msg_t* ptr, void* control);

#ifdef __cplusplus
}
#endif
#endif
