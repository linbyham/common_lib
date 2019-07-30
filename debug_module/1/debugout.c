


#include "debugout.h"

#define DEBUG_BUF_LEN 3096

int sys_mgt_msgque_id = 0;  //sysmgt消息队列
int debug_out_msgque_id = 0; //调试信息输出进程消息队列
BOOL quit_flag = FALSE;  //退出标志

FILE* debug_file = NULL;
char debug_filename[256] = {0};
FILE* log_file = NULL;
char log_filename[256] = "camera.log";
int close_debug_delay_count = 0;
int debug_level = 3;

char debug_str_head[3][10] = {"erro--", "warn--", "info--"};

BOOL debug_buff_flag = FALSE;
char* debug_str_arr[500000] = {0};
int debug_str_num = 0;

void PrintDebugInfo(char* info)
{
    int i = 0;
	int len  = 0;
	char* data = NULL;

	//如果是缓存调试信息
	if ((debug_buff_flag == TRUE) && (info != NULL))
	{
		len = strlen(info);
		data = malloc(len + 4);
		if (data != NULL)
		{
			strcpy(data, info);
			data[len] = '\0';
			debug_str_arr[debug_str_num] = data;
			debug_str_num++;
		}
		else
		{
			printf("debug info malloc failed. \r\n");
		}
		return;
	}

	printf("%s", info);
	
	if (debug_file != NULL)
	{
		fprintf(debug_file,"%s", info);
		fflush(debug_file);
	}
}

void PrintLogInfo(char* info)
{
    int i = 0;
	char* write_pos = NULL;

	printf("%s", info);
	
	if (log_file != NULL)
	{
		fprintf(log_file,"%s", info);
		fflush(log_file);
	}
}

/* file， func， line为宏封装好的入参，info为调用者传入参数 */
int debug_out_func(int level, const char *file, const char *func, int line, const char *info)
{
	char buf[DEBUG_BUF_LEN];
	struct timespec time_s;	
	int index = 0;

	if (info == NULL) return 0;
	if (level > debug_level) return 0;
	if (level == 0) return 0;
	index = level - 1;

	if (index > 2) index = 2; 

	clock_gettime(CLOCK_REALTIME, &time_s);  //获取相对于1970到现在的秒数
	
	memset(buf, 0, DEBUG_BUF_LEN);	   
//	sprintf(buf, "Time %d.%06d, File: %s, Line: %d, Func: %s, Info: %s", 
//		time_s.tv_sec, (time_s.tv_nsec)/1000, file, line, func, info);

	sprintf(buf, "%s %d.%03d F: %s, L: %d, F: %s: %s", 
		debug_str_head[index], (int)time_s.tv_sec, (int)(time_s.tv_nsec)/1000000,
		file, line, func, info);

	PrintDebugInfo(buf);
	return 0;
}
 
/* 同上，只不过改为可变参数 */
int debug_out_func_v(int level, const char *file, const char *func, int line, const char *fmt, ...)
{
	char buf[DEBUG_BUF_LEN];
	struct timespec time_s;		
	int index = 0;	
	va_list args;

	if (level > debug_level) return 0;	
	if (level == 0) return 0;
	index = level - 1;

	if (index > 2) index = 2; 

	clock_gettime(CLOCK_REALTIME, &time_s);  //获取相对于1970到现在的秒数	

	memset(buf, 0, DEBUG_BUF_LEN);	 
	//snprintf(buf, DEBUG_BUF_LEN, "Time %d.%06d, File: %s, Line: %d, Func: %s, Info: ", 
	//	time_s.tv_sec, (time_s.tv_nsec)/1000, file, line, func);

	snprintf(buf, DEBUG_BUF_LEN, "%s %d.%03d F: %s, L: %d, F: %s: ", 
		debug_str_head[index], (int)time_s.tv_sec, (int)(time_s.tv_nsec)/1000000,
		file, line, func);
	
	/* 可变参的处理 */
	va_start(args, fmt);
	vsnprintf(buf + strlen(buf), DEBUG_BUF_LEN - strlen(buf), fmt, args);
	va_end(args);

	PrintDebugInfo(buf);	 
	return 0;
}

void CloseLogFile()
{
	FILE* tmp_file = NULL;

	if (log_file != NULL)
	{
		tmp_file = log_file;
		log_file = NULL;		
		fflush(tmp_file);
    	fclose(tmp_file);		
	}
}

FILE* OpenLogFile()
{
	time_t timep;	 
	struct tm *p = NULL; 

	timep = time(NULL);   
	p = localtime(&timep);

	sprintf(log_filename, "./log/camera_%04d_%02d_%02d.log", 
		(1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday);	

	CloseLogFile();

	log_file=fopen(log_filename, "a+");	 
    if(log_file == NULL)
    {
		err_printf("debug fopen %s error.\r\n", log_filename);
	}

	return log_file;
}

void CloseDebugFile()
{
	int i = 0;
	int count = 0;
	FILE* tmp_file = NULL;
	close_debug_delay_count = 0;

	count = debug_str_num;
	if (debug_file == NULL) return;

	//如果是缓存数据，则全部写入文件中，并释放内存
	debug_str_num = 0;
	if (debug_buff_flag == TRUE)
	{
		for(i = 0; i < count; i++)
		{
			fprintf(debug_file,"%s", debug_str_arr[i]);
			free(debug_str_arr[i]);
			debug_str_arr[i] = NULL;
		}
	}
	
	err_printf("Close debug file: %s.\r\n", debug_filename);
	
	tmp_file = debug_file;
	debug_file = NULL;		
	fflush(tmp_file);
	fclose(tmp_file);
}


FILE* OpenDebugFile(BOOL new_flag)
{
	static int count = 0;
	static char time_str[128] = {0};
	time_t timep;	 
	struct tm *p = NULL;

	if (new_flag == TRUE)
	{
		count = 0;
		timep = time(NULL);   
		p = localtime(&timep);
		sprintf(time_str, "%04d_%02d_%02d_%02d_%02d_%02d", 
			(1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday, 
			p->tm_hour, p->tm_min, p->tm_sec);	
		sprintf(debug_filename, "/media/dbg/camera_debug_%s_%03d.txt", time_str, count);	
	}
	else
	{
		count++;
		sprintf(debug_filename, "/media/dbg/camera_debug_%s_%03d.txt", time_str, count);
	}

	CloseDebugFile();
	debug_file=fopen(debug_filename, "wb+");	 
    if(debug_file == NULL)
    {
		err_printf("debug fopen %s error.\r\n", debug_filename);
	}
	else
	{
		debuge_printf("Open new debug file: %s.\r\n", debug_filename);
	}

	debug_str_num = 0;

	return debug_file;
}


//设置消息队列数据空间尺寸，默认16384 （ipcs -l）,不足以传递数据，导致消息队列中可缓存的消息数很少。
//注意需要root权限			
int SetMsgQueSize(int msg_queue_id)
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

void ClearMsgQue(int msg_que_id)
{
	int ret = 0;
	MsgBuff msg;

	if (msg_que_id < 0) return;
	
	while(1)
	{
		ret = msgrcv(msg_que_id, (void *)&msg, MAX_MSG_DATA_LEN, 0, IPC_NOWAIT);
		if(ret < 0) break;
	}
}


//初始化所有消息队列ID，注意需要ROOT权限才能成功设置消息队列缓冲区大小
int InitMsgQue()
{
    key_t key;
	int msgque_id = 0;
	int i = 0;

	//创建消息队列
	key = ftok(APP_PATH, MSG_QUE_SYSMGT);
	sys_mgt_msgque_id = msgget(key, 0666|IPC_CREAT);
	if (sys_mgt_msgque_id == -1) return -1;
	if (SetMsgQueSize(sys_mgt_msgque_id) < 0) return -1;

	key = ftok(APP_PATH, MSG_QUE_DEBUGOUT);
    debug_out_msgque_id = msgget(key, 0666|IPC_CREAT);
	if (debug_out_msgque_id == -1) return -1;
	if (SetMsgQueSize(debug_out_msgque_id) < 0) return -1;	

	//ClearMsgQue(debug_out_msgque_id);

	return 0;
}

//发送心跳函数
int SendHearbeatToSysmgt(int srcProcID)
{
	//定时向sysmgt进程发心跳消息
	MsgBuff msg;
	int ret = 0;

	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = MSG_ID_HEART;
	msg.mtext[0] = srcProcID;

	ret = msgsnd(sys_mgt_msgque_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);

	if(ret < 0)
	{
		//printf("debug_out Msg queue %d send sysmgt heartbeat msg error %d. \r\n", srcProcID, errno);
	}
	else
	{
		//printf("debug_out Msg queue %d send sysmgt heartbeat msg. \r\n", srcProcID);
	}

	return ret;	
}

//发送退出消息给指定队列
int SendQuitMsgToQueue(int proc_id, int msg_queue_id)
{
	MsgBuff msg;
	int ret = 0;

	//算法进程退出需要的消息数据
	char data[68] = {0};
	int32_t id32 = 0;
	int32_t width32 = 0;
	int32_t height32 = 0;
	int32_t version = 3;	
	

	memset(msg.mtext, 0, MAX_MSG_DATA_LEN);
	msg.mtype = MSG_ID_QUIT;
	msg.mtext[0] = proc_id;

	//算法进程退出需要的消息数据
	id32 = 100;
	width32 = 0;
	height32 = 0;
	memset(data, 0, 64);
	memcpy(&(data[0]), &version, 4);
	memcpy(&(data[4]), &id32, 4);
	memcpy(&(data[8]), &width32, 4);
	memcpy(&(data[12]), &height32, 4);
	strcpy(&(data[16]), "quit");
	memcpy(&(msg.mtext[MSG_DATA_OFFSET]), data, 64);		

	ret = msgsnd(msg_queue_id, &msg, MAX_MSG_DATA_LEN, IPC_NOWAIT);

	if(ret < 0)
	{
		//printf("debugout Proc id %d send quit msg to queue id = %d error %d. \r\n", 
		//	proc_id, msg_queue_id, errno);
	}
	else
	{
		//printf("debugout Proc id %d send quit msg to queue id = %d success. \r\n", 
		//	proc_id, msg_queue_id);
	}

	return ret;	
}

//处理sysmgt发送过来的quit退出消息
int HandleQuitMsg(MsgBuff* pMsg)      
{	
	//先置退出标志
	//quit_flag = TRUE;	
	return 0;			
}

//处理调试信息输出消息
int HandleDebugOutMsg(MsgBuff* pMsg)      
{	
	char* data = NULL;
	if (pMsg == NULL) return -1;

	data = &(pMsg->mtext[MSG_DATA_OFFSET]);
	PrintDebugInfo(data);
	
	return 0;			
}

//处理日志信息输出消息
int HandleLogOutMsg(MsgBuff* pMsg)      
{	
	char* data = NULL;
	if (pMsg == NULL) return -1;

	data = &(pMsg->mtext[MSG_DATA_OFFSET]);
	PrintLogInfo(data);

	PrintDebugInfo("log -- ");
	PrintDebugInfo(data);
	
	return 0;			
}


void DebugOutTimerHandler()
{
	static int count = 0;
	time_t timep;	 
	struct tm *p = NULL;
	char new_log_filename[128] = {0};

	//检查是否是新的一天，如果是，则重新生成新的日志文件
	timep = time(NULL);   
	p = localtime(&timep);
	sprintf(new_log_filename, "./camera_%04d_%02d_%02d.log", 
		(1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday);	

	if(strcmp(new_log_filename, log_filename) != 0)
	{
		CloseLogFile();
		OpenLogFile();
	}

	//每半小时保存一次新的调试文件，避免太大
	count++;
	if (count > 200)
	{
		count = 0;
		OpenDebugFile(FALSE);		
	}

	//延迟退出，保存所有调试信息
	if (close_debug_delay_count > 0)
	{
		close_debug_delay_count--;
		if (close_debug_delay_count == 0)
		{
			quit_flag = TRUE;
			//CloseDebugFile();
		}
	}

	//定时向sysmgt进程发心跳消息
	//不需要发，保持调试任务的独立性
	//SendHearbeatToSysmgt(MSG_QUE_DEBUGOUT);

}


int DebugOutMsgHandle(MsgBuff* pMsg)
{
	int ret = 0;
	if (pMsg == NULL) return -1;	
	
	switch(pMsg->mtype)
	{
		case MSG_ID_DEBUG_OUT:
			HandleDebugOutMsg(pMsg);
			ret = 0;
			break;

		case MSG_ID_LOG_OUT:
			HandleLogOutMsg(pMsg);
			ret = 0;
			break;

		case MSG_ID_TIMER_OUT:
			DebugOutTimerHandler();
			ret = 0;
			break;

		case MSG_ID_QUIT:
			HandleQuitMsg(pMsg);
			ret = 0;
			break;			

		case MSG_ID_ROOTMGT_START:
			//OpenNewDebugFile();
			debuge_print("Rootmgt start.\r\n");
			break;

		case MSG_ID_ROOTMGT_STOP:
			debuge_print("Rootmgt stop.\r\n");			
			//CloseDebugFile();
			//延迟关闭debug文件，部分调试信息可能会晚一些输出
			//close_debug_delay_count = 5;
			break;			
		
		default:
			debug_printf("debug_out receive msg type = %d, src = %d.\r\n", (int)pMsg->mtype, (int)pMsg->mtext[0]);		
			break;
	}

	return ret;
}


void QuitProc(int signo) 
{
	debuge_printf("debug_out proc will quit after 5s.\r\n");	
	close_debug_delay_count = 5;
	return; 

	/*
    quit_flag = TRUE;
	PrintDebugInfo("debug_out proc quit.\r\n");
	SendQuitMsgToQueue(MSG_QUE_DEBUGOUT, debug_out_msgque_id);	
	CloseDebugFile();			
    _exit(0);
    */
}

int main()
{    
    MsgBuff msg;
	int ret = 0;

	//设置Ctrl+C中断信号处理函数
	quit_flag = FALSE;
	signal(SIGINT, QuitProc); 

	debug_str_num = 0;	

	log_file = NULL;
	OpenLogFile();

	debug_file = NULL;
	if (OpenDebugFile(TRUE) == NULL)
	{
		return 0;
	}	

	//创建消息队列
	//初始化所有消息队列ID，注意需要ROOT权限才能成功设置消息队列缓冲区大小	
    if(InitMsgQue() == -1)
    {
        err_print("debug_out Open all msg queue error! maybe you aren't root user. quit! \r\n");
        return 0;
    }
   
    SetMillisecondsTimer(HEARTBEAT_TIME_LEN * 1000, debug_out_msgque_id);

	debugw_printf("debug_out start! msgqueue =%d. \r\n", debug_out_msgque_id);
	
    while(1)
    {
		//消息数据清0
		memset(msg.mtext, 0, MAX_MSG_DATA_LEN);

		ret = msgrcv(debug_out_msgque_id, (void *)&msg, MAX_MSG_DATA_LEN, 0, 0);
		if(ret < 0)
        {
			if(errno == EINTR)
			{
				//printf("debug_out receive %d EINTR! \r\n", ret);
			}
			else
			{
				err_printf("debug_out receive msg error %d ! sleep a little time.\r\n", ret);
            	sleep(1);
			}
        }
		else
		{
			DebugOutMsgHandle(&msg);
		}
		
		if (quit_flag == TRUE)
		{
			debuge_printf("debug_out proc quit...\r\n");
			break; 
		}

    }

	CloseLogFile();
	CloseDebugFile();
	
    return 0;
}




