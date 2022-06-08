#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_DEBUG 0
#define LOG_VERBOSE 1
#define LOG_NOTICE 2
#define LOG_WARNING 3
#define LOG_ERROR 4
#define REDIS_LOG_RAW (1<<10) /* Modifier to log without timestamp */

#ifdef __cplusplus
extern "C" {
#endif
extern void alg_sdk_set_level(uint32_t level);
extern void alg_sdk_log(int level, const char *fmt, ...);
#ifdef __cplusplus
}
#endif

#endif
