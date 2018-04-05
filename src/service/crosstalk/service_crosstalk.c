/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : service_crosstalk.c
 * Author        : Zengchao
 * Date          : 2018-04-06
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-06
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "webapi.h"
#include "http_post.h"
#include "tts.h"
#include "service_crosstalk.h"
#include "play_audio.h"


	
crosstalk_list_t* crosstalk_url_head = NULL;
pthread_mutex_t crosstalk_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t crosstalk_ready = PTHREAD_COND_INITIALIZER;


static inline void 
sal_sys_faad(const U8 *src_path,const U8 *dst_path)
{
	char sys_cmd[256];

	memset(sys_cmd,0,sizeof(sys_cmd));

	strcpy(sys_cmd,"./faad -o ");
	strcat(sys_cmd,dst_path);
	strcat(sys_cmd," ");
	strcat(sys_cmd,src_path);

	sal_system(sys_cmd);
}


/*****************************************************************************
 * Function      : service_crosstalk_thread
 * Description   : 相声服务线程
 * Input         : void *arg  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180406
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
void 
service_crosstalk_thread(void *arg)
{
	for(;;)
	{
		int ret,talk_cnt = 0;
		char url[256],talk_name[256];
		char temp_m4a[] = "crosstalk_temp.m4a";
		
		for(;;)
		{
			post_response_data_t back_data;

			back_data.data = NULL;
			back_data.len  = 0;
				
			pthread_mutex_lock(&crosstalk_lock);
			while(crosstalk_url_head == NULL)
				pthread_cond_wait(&crosstalk_ready,&crosstalk_lock);

			crosstalk_list_t *talk_entry = crosstalk_url_head;
			memset(url,0,sizeof(url));
			strncpy(url,talk_entry->url,sizeof(url));
			free(talk_entry->url);
			
			crosstalk_url_head = talk_entry->next;
			free(talk_entry);
			pthread_mutex_unlock(&crosstalk_lock);

			if(!strncmp(url,"https",strlen("https")))
				memmove(url+4,url+5,strlen(url)-4);

			/*从URL下载资源*/
			ret = http_get_request(url,&back_data);
			if(ret<0)
			{
				LOG_ERROR((BSL_META("http_get_request failed\n")));
				continue;
			}
			
			memset(talk_name,0,sizeof(talk_name));
			snprintf(talk_name,sizeof(talk_name),"crosstalk-%d.wav",talk_cnt);

			/*将下载资源写入文件保存*/
			FILE *fp;
			
			fp = fopen(temp_m4a,"w+");
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
			fclose(fp);

			/*通过faad将MPEG-4标准的音频文件解码成WAV格式*/
			sal_sys_faad(temp_m4a,talk_name);

			/*将音频文件名发给播放线程播放*/
			pthread_mutex_lock(&play_lock);
			play_list_t *play_entry = malloc(sizeof(play_list_t));
			if(NULL == play_entry)
			{
				talk_cnt++;
				LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
				pthread_mutex_unlock(&play_lock);
				continue;
			}

			memset(play_entry->audio_name,0,sizeof(play_entry->audio_name));
			strncpy(play_entry->audio_name,talk_name,sizeof(play_entry->audio_name));
			play_entry->audio_type = AUDIO_WAV;
			play_entry->next = play_list_head;
			play_list_head = play_entry;
			talk_cnt++;
			pthread_mutex_unlock(&play_lock);
			pthread_cond_signal(&play_ready);
		}
		}
}

