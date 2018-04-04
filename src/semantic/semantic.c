/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : semantic.c
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
#include <fcntl.h>          
#include <sys/stat.h>    
#include <mqueue.h>

#include "tts.h"
#include "log.h"
#include "webapi.h"


/*****************************************************************************
 * Function      : get_audio_file
 * Description   : 读出音频文件内容
 * Input         : char *path   
                U32 *in_len  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180316
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
char *
get_audio_file(char *path,U32 *in_len)
{
	FILE *fp = NULL;

	if(NULL == path)
		return NULL;

	fp = fopen(path,"rb");
	if(NULL == fp)
	{
		printf("open [%s] failed -->[%s]:[%d]\n",path,__FILE__,__LINE__);
		return NULL;
	}
	
	U32 data_len;
	char *data_buf;

	fseek(fp,0,SEEK_END);
	data_len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	data_buf = malloc(data_len);
	if(NULL == data_buf)
	{
		printf("malloc failed -->[%s]:[%d]\n",__FILE__,__LINE__);
		return NULL;
	}

	if(data_len != fread(data_buf,sizeof(char),data_len,fp))
	{
		printf("fread failed -->[%s]:[%d]\n",__FILE__,__LINE__);
		free(data_buf);
		return NULL;
	}

	*in_len = data_len; 
	
	return data_buf;
}



/*****************************************************************************
 * Function      : voice_semantic_thread
 * Description   : 语义识别线程
 * Input         : void *arg  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180316
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
voice_semantic_thread(void *arg)
{
	int ret;
	ssize_t mq_res;
	mqd_t snd_mqdid = *(mqd_t*)arg;

	char pcm_file[32];
	
	for(;;)
	{	
		mq_res = mq_receive(snd_mqdid,pcm_file,sizeof(pcm_file),0);
		if(mq_res<0)
		{
			LOG_ERROR((BSL_META("mq_receive:%s\n"),strerror(errno)));
			continue;
		}

		LOG_DEBUG(("semantic:pcm_file = %s\n",pcm_file));

		int audio_len;
		char *audio_data;
		
		LOG_INFO(("#########################################################################\n"));
		LOG_INFO(("## 语音识别技术（SEMANTIC）能够将语音转换成对应的文字并根据语义返回结果。##\n"));
		LOG_INFO(("###########################################################################\n\n"));
		
		LOG_INFO(("读取音频文件>>>>>> "));
		audio_data = get_audio_file(pcm_file,&audio_len);
		if(NULL == audio_data)
		{
			LOG_INFO(("失败\n"));
			continue;
		}		
		LOG_INFO(("成功\n"));


		/*调用语义识别接口*/
		LOG_INFO(("上传音频文件>>>>>> "));
		
		char *res_text = NULL;
		semantic_code_t res_code;
		
		ret = webapi.semantic(audio_data,audio_len,&res_text,&res_code);
		if(ret<0)
		{
			LOG_INFO(("失败\n")); 
			free(audio_data);
			continue;
		}	
		LOG_INFO(("成功\n")); 	
		/*语义识别完成，释放内存*/
		free(audio_data);

		/*根据语义识别结果，分别进行处理*/
		if(res_code == RET_ERROR)
		{
			/*语义识别失败*/
			LOG_INFO((BSL_META("语义解析时发生了错误\n"))); 
		}
		else if(res_code == RET_ANSWER)
		{
			LOG_DEBUG(("语义识别成功，将进行语音合成\n"));
		
			/*语义识别成功，将进行语音合成*/
			tts_list_t *entry = malloc(sizeof(tts_list_t));
			if(NULL == entry)
			{
				LOG_ERROR((BSL_META("malloc:%s\n"),strerror(errno))); 
				free(res_text);
				continue;
			}

			pthread_mutex_lock(&tts_lock);
			entry->text = res_text;
			entry->next = tts_list_head;
			tts_list_head = entry;
			pthread_mutex_unlock(&tts_lock);
			pthread_cond_signal(&tts_ready);
		}
		else if(res_code == RET_TEXT)
		{
		#if 0
			/*语义识别失败，但是语音识别成功，返回了文本信息，将进行AI机器人对话*/
			diag_list_t *entry = malloc(sizeof(diag_list_t));
			if(NULL == entry)
			{
				LOG_ERROR(BSL_META("malloc:%s\n"),strerrno(errno)); 
				free(res_text);
				continue;
			}
			
			pthread_mutex_lock();
			entry->text = res_text;
			entry->next = diag_list_head;
			diag_list_head = entry->next;
			pthread_mutex_unlock();
		#endif
		}	
	}
}




