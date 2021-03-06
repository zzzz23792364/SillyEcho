/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : tts.c
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
#include <string.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "webapi.h"
#include "tts.h"
#include "play_audio.h"


tts_list_t *tts_list_head = NULL;
extern web_api_t webapi;

pthread_mutex_t tts_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  tts_ready = PTHREAD_COND_INITIALIZER;

/*****************************************************************************
 * Function      : voice_tts_thread
 * Description   : 语音合成线程
 * Input         : void *arg  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
voice_tts_thread(void *arg)
{
	int ret;
	U32 text_len;
	char *tts_text;

	U8   wav_serial = 0;
	char wav_name[32];
	
	for(;;)
	{
		pthread_mutex_lock(&tts_lock);
		while(tts_list_head == NULL)
			pthread_cond_wait(&tts_ready,&tts_lock);

		LOG_DEBUG(("即将开始语音合成\n"));

		tts_list_t *tts_entry = tts_list_head;
		text_len = strlen(tts_entry->text)+1; //根据valgrind找到的第一个BUG
		tts_text = malloc(text_len);		
		if(NULL == tts_text)
		{
			LOG_ERROR((BSL_META("malloc:%s\n"),strerror(errno)));
			pthread_mutex_unlock(&tts_lock);
			continue;
		}

		/*拷贝需要进行语音合成的数据*/
		strncpy(tts_text,tts_entry->text,text_len);
		free(tts_entry->text);
		
		tts_list_head = tts_entry->next;
		free(tts_entry);
		pthread_mutex_unlock(&tts_lock);	

		/*开始语音合成*/
		memset(wav_name,0,sizeof(wav_name));
		snprintf(wav_name,sizeof(wav_name),"tts-%d.wav",wav_serial);
		
		ret = webapi.tts(tts_text,wav_name);
		if(ret<0)
		{
			LOG_INFO((BSL_META("webapi.tts failed\n")));
			free(tts_text);
			continue;
		}
		free(tts_text);
		
		/*将文件名传递给处理线程进行处理*/
		pthread_mutex_lock(&play_lock);
		play_list_t *play_entry = malloc(sizeof(play_list_t));
		if(NULL == play_entry)
		{
			wav_serial++;
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			pthread_mutex_unlock(&play_lock);
			continue;
		}

		memset(play_entry->audio_name,0,sizeof(play_entry->audio_name));
		strncpy(play_entry->audio_name,wav_name,sizeof(play_entry->audio_name));
		play_entry->audio_type = AUDIO_WAV;
		play_entry->next = play_list_head;
		play_list_head = play_entry;
		wav_serial++;
		pthread_mutex_unlock(&play_lock);
		pthread_cond_signal(&play_ready);
	}	
}
