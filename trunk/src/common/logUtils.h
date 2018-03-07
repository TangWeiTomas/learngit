#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

//控制日志输出
extern bool use_log_debug;

char*  getlocaltime(void);

#define NDEBUG 0

#if NDEBUG
#define log_debug(M,...)  NULL
#else
#define log_debug(M,...)  logUtils_msg("[Debug][%s][%s:%d]" M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);//log_printf( "Debug",__FILE__, __FUNCTION__,__LINE__, M, ##__VA_ARGS__)
#endif

//限制字符串长度输出实例
//#define log_debug(M,...)  logUtils_msg("[Debug][%s][%.*s:%d]" M,getlocaltime(),30,__FUNCTION__,__LINE__,##__VA_ARGS__);//log_printf( "Debug",__FILE__, __FUNCTION__,__LINE__, M, ##__VA_ARGS__)

#define clean_errno() 			(errno==0 ? "None":strerror(errno))
#define log_err(M,...)			fprintf(stderr,"[Error][%s][%s:%d] %s " M,getlocaltime(),__FUNCTION__,__LINE__,clean_errno(),##__VA_ARGS__);
#define log_warn(M,...) 		fprintf(stderr,"[Warring][%s][%s:%d] %s " M,getlocaltime(),__FUNCTION__,__LINE__,clean_errno(),##__VA_ARGS__);
#define log_info(M,...) 		fprintf(stderr,"[Info][%s][%s:%d] " M,getlocaltime(),__FUNCTION__,__LINE__,##__VA_ARGS__);

#define check(A,M,...) 			if((!A)){log_err(M,##__VA_ARGS__);errno = 0;goto error;}
#define sentinal(M,...) 		{log_err(M,##__VA_ARGS__);errno=0;goto error;}
#define check_mem(A)			check((A),"Out of memory.")
#define check_debug(A,M,...) 	if(!(A)){debug(M,##__VA_ARGS__);errno=0;goto error;}

#define log_debug_array(array,length,IntervalSymbol) 	logUtils_array(array,length,IntervalSymbol)

#if NDEBUG
#define ASSERT(expr) NULL
#else
#define ASSERT(expr)                                          \
	 do {													  \
	  if (!(expr)) {										  \
		fprintf(stderr, 									  \
				"Assertion failed in %s on line %d: %s\n",	  \
				__FILE__,									  \
				__LINE__,									  \
            #expr);                                       	  \
		abort();											  \
	  } 													  \
	 } while (0)
#endif

void logUtils_array(const uint8_t *array,uint16_t length,uint8_t *IntervalSymbol);
void logUtils_msg(const char *fmt ,...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
