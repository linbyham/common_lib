
#include "timer_common.h"

TimerFunc g_pTimerHandle = NULL;

void SignalHandle(int signo)
{
	switch(signo)
	{
		case SIGALRM:
			if(g_pTimerHandle != NULL)
			{
				g_pTimerHandle();
			}

			//重置SIGALRM信号处理函数，避免丢失
			signal(SIGALRM, SignalHandle);
			break;
			
		default:
			break;
	}
}

//启动定时器，传入定时器处理函数，定时秒数、微秒数。
//成功则返回0，失败返回-1
//注意：涉及到全局变量g_pTimerHandle、SignalHandle函数和SIGALRM信号，一个进程只能调用一次。
int StartTimer(TimerFunc pTimerHandle, int timer_sec, int timer_usec)
{	
	int ret = 0;
	struct itimerval new_value, old_value;

	//定时器函数赋值；
	g_pTimerHandle = pTimerHandle;

	//设置SIGALRM信号处理函数
	signal(SIGALRM, SignalHandle);

	//设置定时器，会按定时间隔，触发SIGALRM信号
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_usec = 10000; //此值如果太小，可能导致定时器很快启动，出现一些异常问题
	new_value.it_interval.tv_sec = timer_sec;
	new_value.it_interval.tv_usec = timer_usec;
	ret = setitimer(ITIMER_REAL, &new_value, NULL); //&old_value);	
	return ret;
}

//可以不调用，关系不大
void CloseTimer()  
{  
	struct itimerval value;  
	value.it_value.tv_sec = 0;  
	value.it_value.tv_usec = 0;  
	value.it_interval = value.it_value;  
	setitimer(ITIMER_REAL, &value, NULL);  
}


//sleep指定的ms时间（毫秒）
void milliseconds_sleep(unsigned long mSec)
{
    struct timeval tv;
    tv.tv_sec=mSec/1000;
    tv.tv_usec=(mSec%1000)*1000;
    int err;
    do{
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}


//sleep指定的us时间（微秒）
void microseconds_sleep(unsigned long uSec)
{
    struct timeval tv;
    tv.tv_sec=uSec/1000000;
    tv.tv_usec=uSec%1000000;
    int err;
    do{
        err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}

//微秒级高精度定时线程
void* MicrosendsTimerThread(void* arg)
{
	MsgBuff msg;
	int ret = 0;
	unsigned long timer_len = 0;
	PTimerMsgInfo pInfo = NULL;
	int que_id = 0;

	if(arg == NULL) pthread_exit((void *)0); 

	pInfo = (PTimerMsgInfo)arg;
	timer_len = pInfo->timer_us_len;
	
	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = pInfo->msg_id;
	msg.mtext[0] = 0;
	que_id = pInfo->msg_que_id;

	while(1)
	{
		microseconds_sleep(timer_len);
		ret = msgsnd(que_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);
		if(ret < 0)
		{
			printf("timer_common send timer out msg error %d. \r\n", errno);
		}		
	}

	free(pInfo);		
	pthread_exit((void *)0); 
}


void SetMicrosendsTimer(unsigned long timer_len, int msg_que_id)
{
	pthread_t timer_thread; 
	PTimerMsgInfo pInfo = NULL;

	pInfo = (PTimerMsgInfo)malloc(sizeof(TimerMsgInfo));
	if (pInfo == NULL) 
	{
		printf("timer_common Create microseconds timer thread failed for memory. \r\n");	
		return;
	}
	
	pInfo->timer_us_len = timer_len;
	pInfo->msg_que_id = msg_que_id;
	pInfo->msg_id = MSG_ID_TIMER_OUT;

	if(pthread_create(&timer_thread, NULL, MicrosendsTimerThread, pInfo) != 0)
	{
		printf("timer_common Create microseconds timer thread failed. \r\n");			
	}
}

//毫秒级高精度定时线程
void* MillisecondsTimerThread(void* arg)
{
	MsgBuff msg;
	int ret = 0;
	unsigned long timer_len = 0;
	PTimerMsgInfo pInfo = NULL;
	int que_id = 0;

	if(arg == NULL) pthread_exit((void *)0); 

	pInfo = (PTimerMsgInfo)arg;
	timer_len = pInfo->timer_ms_len;
	
	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = pInfo->msg_id;
	msg.mtext[0] = 0;
	que_id = pInfo->msg_que_id;

	while(1)
	{
		milliseconds_sleep(timer_len);
		ret = msgsnd(que_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);
		if(ret < 0)
		{
			printf("timer_common send timer out msg error %d. \r\n", errno);
		}		
	}

	free(pInfo);		
	pthread_exit((void *)0); 
}

//设置毫秒级定时器
void SetMillisecondsTimer(unsigned long timer_len, int msg_que_id)
{
	pthread_t timer_thread; 
	PTimerMsgInfo pInfo = NULL;

	pInfo = (PTimerMsgInfo)malloc(sizeof(TimerMsgInfo));
	if (pInfo == NULL) 
	{
		printf("timer_common Create milliseconds timer thread failed for memory. \r\n");	
		return;
	}
	
	pInfo->timer_ms_len = timer_len;
	pInfo->msg_que_id = msg_que_id;
	pInfo->msg_id = MSG_ID_TIMER_OUT;

	if(pthread_create(&timer_thread, NULL, MillisecondsTimerThread, pInfo) != 0)
	{
		printf("timer_common Create milliseconds timer thread failed. \r\n");			
	}
}

 


