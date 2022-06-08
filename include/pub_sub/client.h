#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>

#include "pub_sub/log.h"
#include "pub_sub/basic_types.h"
#include "pub_sub/util.h"
typedef void(*alg_sdk_callback_func)(void* p);

int alg_sdk_subscribe (const char* topic, alg_sdk_callback_func consumer);
int alg_sdk_stop_client();
int alg_sdk_init_client();

#endif
