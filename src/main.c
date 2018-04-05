/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : main.c
 * Author        : ZengChao
 * Date          : 2018-03-27
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sal/core/thread.h>
	
#include <fcntl.h>          
#include <sys/stat.h>       
#include <mqueue.h>


#include "log.h"
#include "detect.h"
#include "tts.h"
#include "webapi.h"
#include "play_audio.h"
#include "service_news.h"
#include "service_crosstalk.h"



#define VOICE_QUEUE_NAME   "/void_queue"

#define SND_IPC_MQ_MAXMSG   10
#define SND_IPC_MQ_MSGSIZE  32



/*****************************************************************************
 * Function      : main
 * Description   : MAIN就是MAIN啊
 * Input         : int argc     
				   char **argv  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int 
main(int argc,char **argv)
{
	int ret;
	mqd_t snd_mqdid;
	struct mq_attr setattr; 	

	log_init();
	log_console_severity_set(bslSeverityFatal,bslSeverityNormal);
	log_file_severity_set(bslSeverityFatal,bslSeverityInfo);

	system("rm record-*");
	system("rm news-*");
	system("rm crosstalk*");

	web_api_config(&webapi);

	ret = mq_unlink(VOICE_QUEUE_NAME);
	if(ret<0)
		perror("mq_unlink");

	setattr.mq_flags = 0;
	setattr.mq_maxmsg = SND_IPC_MQ_MAXMSG;	
	setattr.mq_msgsize = SND_IPC_MQ_MSGSIZE; 
	setattr.mq_curmsgs = 0;

	snd_mqdid = mq_open(VOICE_QUEUE_NAME,O_RDWR|O_CREAT|O_EXCL,0644, &setattr);
    if (snd_mqdid < 0) 
	{  
		LOG_FATAL((BSL_META("mq_open: %s\n"),strerror(errno)));
		return -1;
    }  

	/*创建声音检测和录制线程*/
	sal_thread_t detect_threadid = SAL_THREAD_ERROR;
	detect_threadid = sal_thread_create("detect thread",SAL_THREAD_STKSZ*256, 
										0,voice_detect_thread,&snd_mqdid);
	if(detect_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("voice_detect_thread create failed\n")));
		return -1;
	}

	/*创建语义识别线程*/
	sal_thread_t semantic_threadid = SAL_THREAD_ERROR;
	semantic_threadid = sal_thread_create("semantic thread",SAL_THREAD_STKSZ*256, 
										0,voice_semantic_thread,&snd_mqdid);	
	if(semantic_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("voice_upload_thread create failed\n")));
		return -1;
	}

	/*创建语音合成线程*/
	sal_thread_t tts_threadid = SAL_THREAD_ERROR;
	tts_threadid = sal_thread_create("tts thread",SAL_THREAD_STKSZ*256, 
										0,voice_tts_thread,NULL);	
	if(tts_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("voice_tts_thread create failed\n")));
		return -1;
	}


	/*创建音频播放线程*/
	sal_thread_t play_threadid = SAL_THREAD_ERROR;
	play_threadid = sal_thread_create("play thread",SAL_THREAD_STKSZ*256, 
										0,play_audio_thread,NULL); 
	if(play_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("service_news_thread create failed\n")));
		return -1;
	}

	/*创建新闻服务线程*/
	sal_thread_t news_threadid = SAL_THREAD_ERROR;
	news_threadid = sal_thread_create("news thread",SAL_THREAD_STKSZ*256, 
										0,service_news_thread,NULL); 
	if(news_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("service_news_thread create failed\n")));
		return -1;
	}

	/*创建新闻服务线程*/
	sal_thread_t talk_threadid = SAL_THREAD_ERROR;
	talk_threadid = sal_thread_create("crosstalk thread",SAL_THREAD_STKSZ*256, 
										0,service_crosstalk_thread,NULL); 
	if(talk_threadid == SAL_THREAD_ERROR)
	{
		LOG_FATAL((BSL_META("service_crosstalk_thread create failed\n")));
		return -1;
	}


	LOG_INFO(("###################################################\n"));
	LOG_INFO(("############    Hello,SillyEcho    ################\n"));
	LOG_INFO(("###################################################\n\n"));
		
	while(1)
	{
		sleep(10);
	}
	/*Never Reach Here*/
	return 0;
}


