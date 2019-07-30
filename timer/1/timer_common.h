

#ifndef _TIMER_COMMON_H_
#define _TIMER_COMMON_H_


#include "camera_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TimerFunc)();

extern int StartTimer(TimerFunc pTimerHandle, int timer_sec, int timer_usec);

//sleep指定的ms时间（毫秒）
extern void milliseconds_sleep(unsigned long mSec);

//sleep指定的us时间（微秒）
extern void microseconds_sleep(unsigned long uSec);

typedef struct stTimerMsgInfo
{
	long timer_us_len;  //定时器微秒数
	long timer_ms_len; //定时器毫秒数
	int msg_id;   //定时时发送的超时消息
	int msg_que_id; //消息发送的队列
}TimerMsgInfo, *PTimerMsgInfo;

//设置毫秒级定时器
extern void SetMillisecondsTimer(unsigned long timer_len, int msg_que_id);

//设置微秒级定时器
extern void SetMicrosendsTimer(unsigned long timer_len, int msg_que_id);

#ifdef __cplusplus
}
#endif


#endif


