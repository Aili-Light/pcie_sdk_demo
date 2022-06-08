#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <netinet/in.h>  /* For htonl and ntohl */
#include <unistd.h>

#include <pthread.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#include "pub_sub/util.h"
#include "pub_sub/log.h"
#include "pub_sub/basic_types.h"

// #define BUFFER_SIZE 1024

void *alg_sdk_publisher (void *arg);
// void alg_sdk_publisher_cmd_msg (void *arg);
int alg_sdk_push2q(const void* msg);
int alg_sdk_init_server(const char* url);
int alg_sdk_stop_server(void);
int alg_sdk_encode_msg (struct nn_msghdr *hdr, struct nn_iovec *iov, const pcie_item_manager_t* msg);

#endif
