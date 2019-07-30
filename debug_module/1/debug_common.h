

#ifndef _DEBUG_COMMON_H_
#define _DEBUG_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h> 
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>  
#include <semaphore.h>
#include <fcntl.h>
#include <sys/socket.h>  
#include <linux/in.h>  

#include <sys/shm.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int debug_out_func(int level, const char *file, const char *func, int line, const char *info);
extern int debug_out_func_v(int level, const char *file, const char *func, int line, const char *fmt, ...);


/*日志信息发送函数*/
extern int log_out_func(const char *info);
 
/* 同上，只不过改为可变参数 */
extern int log_out_func_v(const char *fmt, ...);


//设置调试信息输出级别，0--关闭输出，1--只输出err，2--输出err+warning，3及以上--输出err+warning+info
//当前默认全都输出
extern void SetDebugLevel(int level);
//发送指定消息id和数据到debug
extern void SendMsgToDebug(int msgid, int proc_id, char* data);

/* 封装调试函数 */
#define debug_print(info)  (debug_out_func(3, __FILE__, __func__, __LINE__, (info)))
#define debugw_print(info)  (debug_out_func(12, __FILE__, __func__, __LINE__, (info)))
#define debuge_print(info)  (debug_out_func(11, __FILE__, __func__, __LINE__, (info))) 
 
/* 封装带可变参数的调试函数 */
#define debug_printf(fmt, ...)  (debug_out_func_v(3, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__))
#define debugw_printf(fmt, ...)  (debug_out_func_v(12, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__))
#define debuge_printf(fmt, ...)  (debug_out_func_v(11, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__))

/* 封装调试函数 */
#define warn_print(info)  (debug_out_func(2, __FILE__, __func__, __LINE__, (info))) 
/* 封装带可变参数的调试函数 */
#define warn_printf(fmt, ...)  (debug_out_func_v(2, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__))

/* 封装调试函数 */
#define err_print(info)  (debug_out_func(1, __FILE__, __func__, __LINE__, (info))) 
/* 封装带可变参数的调试函数 */
#define err_printf(fmt, ...)  (debug_out_func_v(1, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__))

/* 封装日志输出函数 */
#define log_print(info)  (log_out_func(info))
/* 封装带可变参数的调试函数 */
#define log_printf(fmt, ...)  (log_out_func_v(fmt, ##__VA_ARGS__))

#ifdef __cplusplus
}
#endif


#endif


