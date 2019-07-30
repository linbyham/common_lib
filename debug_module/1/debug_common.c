
#include "debug_common.h"

#include "camera_common.h"

#define DEBUG_BUF_LEN 3096
#define LOG_BUF_LEN 2048

int debug_level = 3;
int debug_msgque_id = -1; //调试信息输出进程消息队列
char debug_str_head[4][10] = {"erro--", "warn--", "info--", "log-- "};

//设置消息队列数据空间尺寸，默认16384 （ipcs -l）,不足以传递数据，导致消息队列中可缓存的消息数很少。
//注意需要root权限			
int D_SetMsgQueSize(int msg_queue_id)
{
	int ret = -1;
	struct msqid_ds msqinfo;
	
	if(msgctl(msg_queue_id, IPC_STAT, &msqinfo) == 0)
	{
		if (msqinfo.msg_qbytes != (MAX_MSG_NUM * MAX_MSG_DATA_LEN))
		{
			//注意需要root权限			
			msqinfo.msg_qbytes = (MAX_MSG_NUM * MAX_MSG_DATA_LEN);
			if (msgctl(msg_queue_id, IPC_SET, &msqinfo) == 0)
			{				
				ret = 0;
			}
		}
		else
		{
			ret = 0;
		}
	}

	return ret;
}

void D_ClearMsgQue(int msg_que_id)
{
	int ret = 0;
	MsgBuff msg;

	if (msg_que_id <= 0) return;
	
	while(1)
	{
		ret = msgrcv(msg_que_id, (void *)&msg, MAX_MSG_DATA_LEN, 0, IPC_NOWAIT);
		if(ret < 0) break;
	}
}


//初始化所有消息队列ID，注意需要ROOT权限才能成功设置消息队列缓冲区大小
int D_InitMsgQue()
{
    key_t key;
	int msgque_id = 0;
	int i = 0;

	//创建消息队列
	key = ftok(APP_PATH, MSG_QUE_DEBUGOUT);
    debug_msgque_id = msgget(key, 0666|IPC_CREAT);
	if (debug_msgque_id == -1) return -1;
	if (D_SetMsgQueSize(debug_msgque_id) < 0) return -1;	

	return 0;
}

void SetDebugLevel(int level)
{
	debug_level = level;
}

void SendMsgToDebug(int msgid, int proc_id, char* data)
{
	MsgBuff msg;
	int ret = 0;
	int len = 0;

	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = msgid;
	msg.mtext[0] = proc_id;

	if (data != NULL)
	{
		len = strlen(data);
		if (len > 3096) len = 3096;
		memcpy(&(msg.mtext[MSG_DATA_OFFSET]), data, len);
		msg.mtext[MSG_DATA_OFFSET + len] = '\0';
	}
	
	if (debug_msgque_id == -1)
	{
		D_InitMsgQue();
	}

	if (debug_msgque_id != -1)
	{

		ret = msgsnd(debug_msgque_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);

		if(ret < 0)
		{
			//printf("debugcommon send debug info msg error %d. \r\n", errno);
		}
	}
}

void SendDebugInfoMsg(char* info)
{
	//定时向调试输出进程发调试信息消息
	MsgBuff msg;
	int ret = 0;
	int len = 0;

	if (info == NULL) return;

	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = MSG_ID_DEBUG_OUT;
	msg.mtext[0] = 0;
	len = strlen(info);
	if (len > 3096) len = 3096;
	memcpy(&(msg.mtext[MSG_DATA_OFFSET]), info, len);
	msg.mtext[MSG_DATA_OFFSET + len] = '\0';

	if (debug_msgque_id == -1)
	{
		D_InitMsgQue();
	}

	if (debug_msgque_id != -1)
	{

		ret = msgsnd(debug_msgque_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);

		if(ret < 0)
		{
			//printf("debugcommon send debug info msg error %d. \r\n", errno);
		}
	}
}

void SendLogInfoMsg(char* info)
{
	//定时向调试输出进程发日志消息
	MsgBuff msg;
	int ret = 0;
	int len = 0;

	if (info == NULL) return;

	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = MSG_ID_LOG_OUT;
	msg.mtext[0] = 0;
	len = strlen(info);
	if (len > 3096) len = 3096;
	memcpy(&(msg.mtext[MSG_DATA_OFFSET]), info, len);
	msg.mtext[MSG_DATA_OFFSET + len] = '\0';

	if (debug_msgque_id == -1)
	{
		D_InitMsgQue();
	}

	if (debug_msgque_id != -1)
	{

		ret = msgsnd(debug_msgque_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);

		if(ret < 0)
		{
			//printf("debugcommon send debug info msg error %d. \r\n", errno);
		}
	}
}

/* file， func， line为宏封装好的入参，info为调用者传入参数 */
int debug_out_func(int level_in, const char *file, const char *func, int line, const char *info)
{
	char buf[DEBUG_BUF_LEN];
	struct timespec time_s;	
	int index = 0;
	int level = 0;

	if (info == NULL) return 0;

	level = level_in%10;
	if (level > debug_level) return 0;
	if (level == 0) return 0;

	index = level_in - 1;
	if (index > 2) index = 2; 

	clock_gettime(CLOCK_REALTIME, &time_s);  //获取相对于1970到现在的秒数
	
	memset(buf, 0, DEBUG_BUF_LEN);	   
	sprintf(buf, "%s %d.%03d F: %s, L: %d, F: %s : %s", 
		debug_str_head[index], (int)time_s.tv_sec, (int)(time_s.tv_nsec)/1000000,
		file, line, func, info);

	SendDebugInfoMsg(buf);
	return 0;
}
 
/* 同上，只不过改为可变参数 */
int debug_out_func_v(int level_in, const char *file, const char *func, int line, const char *fmt, ...)
{
	char buf[DEBUG_BUF_LEN];
	struct timespec time_s;		
	int index = 0;	
	int level = 0;	
	va_list args;

	level = level_in%10;
	if (level > debug_level) return 0;	
	if (level == 0) return 0;
	
	index = level_in - 1;
	if (index > 2) index = 2; 

	clock_gettime(CLOCK_REALTIME, &time_s);  //获取相对于1970到现在的秒数	

	memset(buf, 0, DEBUG_BUF_LEN);	 
	//snprintf(buf, DEBUG_BUF_LEN, "Time %d.%06d, File: %s, Line: %d, Func: %s, Info: ", 
	//	time_s.tv_sec, (time_s.tv_nsec)/1000, file, line, func);

	snprintf(buf, DEBUG_BUF_LEN, "%s %d.%03d F: %s, L: %d, F: %s: ", 
		debug_str_head[index],  (int)time_s.tv_sec, (int)(time_s.tv_nsec)/1000000,
		file, line, func);
	
	/* 可变参的处理 */
	va_start(args, fmt);
	vsnprintf(buf + strlen(buf), DEBUG_BUF_LEN - strlen(buf), fmt, args);
	va_end(args);

	SendDebugInfoMsg(buf);	 
	return 0;
}


/*日志信息发送函数*/
int log_out_func(const char *info)
{
	char buf[LOG_BUF_LEN];	
	time_t timep;	 
	struct tm *p = NULL; 

	if (info == NULL) return -1;

	memset(buf, 0, LOG_BUF_LEN);	 

	timep = time(NULL);   
	p = localtime(&timep);
	sprintf(buf, "%04d_%02d_%02d_%02d_%02d_%02d: %s\r\n",
		(1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday, 
		p->tm_hour, p->tm_min, p->tm_sec, info);	
	
	SendLogInfoMsg(buf);
	//SendDebugInfoMsg(buf);
	return 0;
}
 
/* 同上，只不过改为可变参数 */
int log_out_func_v(const char *fmt, ...)
{
	char buf[LOG_BUF_LEN];
	time_t timep;	 
	struct tm *p = NULL; 	
	va_list args;

	memset(buf, 0, LOG_BUF_LEN);	 

	timep = time(NULL);   
	p = localtime(&timep);
	sprintf(buf, "%04d_%02d_%02d_%02d_%02d_%02d: ",
		(1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday, 
		p->tm_hour, p->tm_min, p->tm_sec);	
	
	/* 可变参的处理 */
	va_start(args, fmt);
	vsnprintf(buf + strlen(buf), LOG_BUF_LEN - strlen(buf), fmt, args);
	va_end(args);

	strcpy(buf + strlen(buf), "\r\n");

	SendLogInfoMsg(buf);	 
	//SendDebugInfoMsg(buf);
	return 0;
}



