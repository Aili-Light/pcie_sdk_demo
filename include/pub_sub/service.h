#ifndef __SERVICE_H__
#define __SERVICE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <netinet/in.h>  /* For htonl and ntohl */
#include <unistd.h>
#include <pthread.h>

#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include "pub_sub/util.h"
#include "pub_sub/log.h"
#include "pub_sub/basic_types.h"

int alg_sdk_init_service(const char* url);
int alg_sdk_stop_service();
int alg_sdk_call_service (const char *topic_name, void* msg, int timeout);
void alg_sdk_service_append_task(alg_sdk_amp_msg_t* ptr, void* control);

#endif