/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "mylog.h"

#define MAX_FUNC_SAVE 32
#define MAX_FUNC_LENGTH 64

static struct {
	void *udata;
	log_LockFn lock;
	FILE *fp;
	int level;
	int quiet;
} L;


static struct func_info{
	bool inUse;
	int  recoderLine;
	char funcName[MAX_FUNC_LENGTH];
	long seqCounter; 
};

//staticの意味をもう少し知ろう
static struct func_info func_list[MAX_FUNC_SAVE] = {0}; 

static const char *level_names[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#define LOG_USE_COLOR

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static int trace_function(const int line, const char *function_name);

static void lock(void)   {
	if (L.lock) {
		L.lock(L.udata, 1);
	}
}


static void unlock(void) {
	if (L.lock) {
		L.lock(L.udata, 0);
	}
}


void log_set_udata(void *udata) {
	L.udata = udata;
}


void log_set_lock(log_LockFn fn) {
	L.lock = fn;
}


void log_set_fp(FILE *fp) {
	L.fp = fp;
}


void log_set_level(int level) {
	L.level = level;
}


void log_set_quiet(int enable) {
	L.quiet = enable ? 1 : 0;
}

/* TODO:
 * trace function 
 * semaphore
 * dumpFile
 * */

void log_log(int level, const char *function, int line, const unsigned long threadID,const char *fmt, ...) 
{
	if (level < L.level) 
	{
		return;
	}

	/* Acquire lock */
	//セマフォ管理しよう
	lock();
	int index = trace_function(line, function);
	//printf("idnex :%d \n" ,index);

	/* Get current time */
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);

	struct timeval logerTime;
	gettimeofday(&logerTime, NULL);

	/* Log to stderr */
	if (!L.quiet) {
		va_list args;
		char buf[16];
		//何この書き方・・・？
		buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
		fprintf(
				stderr, "[%s] %s:%06d [t]:%lx %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
				LOG_WORD,
				buf,
				logerTime.tv_usec,
				threadID,
				level_colors[level],
				level_names[level],
				function,
				line);
#else
		fprintf(stderr, 
				"[%s] %s:%06d [t]:%lx %-5s %s:%d:[seq]:%ld ", 
				LOG_WORD,
				buf, 
				logerTime.tv_usec,
				threadID,
				level_names[level],
				function,
				line,
				func_list[index].seqCounter);
#endif
		va_start(args, fmt);//これの使い方はまたイマイチ、何これ〜〜〜
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	/* Log to file */
	if (L.fp) 
	{
		va_list args;
		char buf[32];
		buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
		fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], function, line);
		va_start(args, fmt);
		vfprintf(L.fp, fmt, args);
		va_end(args);
		fprintf(L.fp, "\n");
		fflush(L.fp);
	}

	/* Release lock */
	unlock();
	//セマフォ管理しよう THE END
}


static int trace_function(const int line, const char *function_name)
{
	bool foundFuncInList = false;
	int  i,ret_index;

	i = 0;
	while(func_list[i].inUse)
	{
		//比較して、同じ関数があるあるならば
		if(0 == strcmp(function_name, func_list[i].funcName))
		{
			if(line == func_list[i].recoderLine)
			{
				func_list[i].seqCounter++;
			}
			ret_index = i;
			foundFuncInList = true;
			break;
		}
		i++;
	}

	/* バグが存在している */
	if (!foundFuncInList)
	{
		ret_index= i++;
		//printf("ret_index = %d\n", ret_index);
		func_list[ret_index].inUse = true;
		func_list[ret_index].recoderLine = line;
		func_list[ret_index].seqCounter = 1;
		strcpy(func_list[ret_index].funcName, function_name);
		//printf("function %s, list.funcName: %s\n", function_name, func_list[ret_index].funcName);
	}

	return ret_index;
}
