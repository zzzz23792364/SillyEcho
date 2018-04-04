/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : service_news.c
 * Author        : ZengChao
 * Date          : 2018-04-03
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-03
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include "log.h"
#include "webapi.h"
#include "http_post.h"
#include "tts.h"
#include "service_news.h"
#include "play_audio.h"


news_list_t* news_url_head = NULL;
pthread_mutex_t news_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t news_ready = PTHREAD_COND_INITIALIZER;
	
/*****************************************************************************
 * Function      : service_news_thread
 * Description   : 播放新闻服务线程
 * Input         : void *arg  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180403
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
service_news_thread(void *arg)
{
	int ret,news_cnt = 0;
	char url[256],news_name[256];
	
	for(;;)
	{
		post_response_data_t back_data;

		back_data.data = NULL;
		back_data.len  = 0;
			
		pthread_mutex_lock(&news_lock);
		while(news_url_head == NULL)
			pthread_cond_wait(&news_ready,&news_lock);

		news_list_t *news_entry = news_url_head;
		memset(url,0,sizeof(url));
		strncpy(url,news_entry->url,sizeof(url));
		free(news_entry->url);
		
		news_url_head = news_entry->next;
		free(news_entry);
		pthread_mutex_unlock(&news_lock);

		if(!strncmp(url,"https",strlen("https")))
			memmove(url+4,url+5,strlen(url)-4);

		/*从URL下载资源*/
		ret = http_get_request(url,&back_data);
		if(ret<0)
		{
			LOG_ERROR((BSL_META("http_get_request failed\n")));
			continue;
		}

		memset(news_name,0,sizeof(news_name));
		snprintf(news_name,sizeof(news_name),"news-%d.mp3",news_cnt);

		/*将下载资源写入文件保存*/
		FILE *fp;
		
		fp = fopen(news_name,"w+");
		if(NULL == fp)
		{
			LOG_ERROR((BSL_META("fopen failed:%s\n"),strerror(errno)));
			continue;
		}

		if(	back_data.len !=
			fwrite(back_data.data,sizeof(char),back_data.len,fp))
		{
			LOG_ERROR((BSL_META("fwrite failed:%s\n"),strerror(errno)));
			free(back_data.data);
			fclose(fp);
			continue;
		}

		free(back_data.data);

		/*将音频文件名发给播放线程播放*/
		pthread_mutex_lock(&play_lock);
		play_list_t *play_entry = malloc(sizeof(play_list_t));
		if(NULL == play_entry)
		{
			news_cnt++;
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			pthread_mutex_unlock(&play_lock);
			continue;
		}

		memset(play_entry->audio_name,0,sizeof(play_entry->audio_name));
		strncpy(play_entry->audio_name,news_name,sizeof(play_entry->audio_name));
		play_entry->audio_type = AUDIO_MP3;
		play_entry->next = play_list_head;
		play_list_head = play_entry;
		news_cnt++;
		pthread_mutex_unlock(&play_lock);
		pthread_cond_signal(&play_ready);
	}
}


