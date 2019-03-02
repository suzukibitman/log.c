/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

#define LOG_VERSION "0.1.0"
#define LOG_WORD "logme"

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define LOG_LOG(logType, ...) log_log(logType, __FUNCTION__, __LINE__, pthread_self(),  __VA_ARGS__);
	
#define LOG_T(...) LOG_LOG(LOG_TRACE,  __VA_ARGS__) 
#define LOG_D(...) LOG_LOG(LOG_DEBUG,  __VA_ARGS__)
#define LOG_I(...) LOG_LOG(LOG_INFO ,  __VA_ARGS__)
#define LOG_W(...) LOG_LOG(LOG_WARN ,  __VA_ARGS__)
#define LOG_E(...) LOG_LOG(LOG_ERROR,  __VA_ARGS__)
#define LOG_F(...) LOG_LOG(LOG_FATAL,  __VA_ARGS__)
 
void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const unsigned long threadID, const char *fmt, ...);

#endif
