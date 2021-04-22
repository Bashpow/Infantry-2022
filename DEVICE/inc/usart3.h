#ifndef __USART3_H
#define __USART3_H

#include "sys.h"
#include "stdio.h"

#define DEBUG 1
#define INFORMATION 1

#if DEBUG
	#define DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
	#define DEBUG_LOG(format, arg...) printf("<DEBUG> " format "\r\n", ##arg)
	#define DEBUG_SHOWDATA1(name, data) printf("<DEBUG> %s = %d\r\n",name,data)
	#define DEBUG_SHOWDATA2(name, data) printf("<DEBUG> %s = %.2f\r\n",name,data)
	#define DEBUG_ERROR(err) printf("<ERROR> error:%d\r\n",err)
	#define DEBUG_ARRAY(name, data, len) { printf("<DEBUG>%s : {"); for(u16 __tmp_i=0; __tmp_i<len; __tmp_i++) printf("%d ", data[__tmp_i]);  printf("}\r\n"); }
#else
	#define DEBUG_PRINT(format, ...)
	#define DEBUG_LOG(format, arg...)
	#define DEBUG_SHOWDATA1(name, data)
	#define DEBUG_SHOWDATA2(name, data)
	#define DEBUG_ERROR(err)
	#define DEBUG_ARRAY(name, data, len)
#endif

#if INFORMATION
	#define INFO_PRINT(format, ...) printf(format, ##__VA_ARGS__)
	#define INFO_LOG(format, arg...) printf("<INFO> " format "\r\n", ##arg)
#else
	#define INFO_PRINT(format, ...)
	#define INFO_LOG(format, arg...)
#endif


void Usart3_Init(void);

#endif


