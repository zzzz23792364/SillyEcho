/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : detect.c
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
#include <alsa/asoundlib.h>
	
#include <fcntl.h>          
#include <sys/stat.h>        
#include <mqueue.h>
	
#include "cbb_types.h"
#include "record.h"
#include "vad.h"
#include "log.h"


//#define RECORD_PATH ""
#define VAD_WINDOWS_SIZE     (30*16)
#define SILENT_WINDOW_CNT    5


typedef enum silent_status_s
{
	DETECT_NO,  //不在静音区
	DETECT_IN 	//检测到静音区
}silent_status_t;

typedef struct silent_detect_s
{
	U8 cnt;
	silent_status_t status;
}silent_detect_t;


static voice_device_params_t vd_params;
static silent_detect_t silent = {0,DETECT_NO};
static vad_status_t    current_status = LISTENING;


//voice_rec_area_init()
//{
//	if(access(RECORD_PATH,F_OK) == 0)
//		
//}


/*****************************************************************************
 * Function      : stereo_to_mono
 * Description   : 将立体声音频数据转化成单声道数据
 * Input         : char *audio_data  
                U32 audio_len     
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
char*
stereo_to_mono(char *audio_data,U32 audio_len)
{
	U32 i;
	char *mono_data;

	mono_data = malloc(audio_len/2);
	if(NULL == mono_data)
	{
		LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
		return NULL;
	}
	
	for(i=0;i<audio_len/2;i+=2)
	{
		mono_data[i] = audio_data[i*2]; 
		mono_data[i+1] = audio_data[i*2+1]; 
	}

	return mono_data;
}


/*****************************************************************************
 * Function      : inject_silent_area
 * Description   : 在音频文件头尾注入一小段静音区，方便语音识别
 * Input         : char *pcm_file  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
inject_silent_area(char *pcm_file)
{
	FILE* fp = NULL;
	
	char buf_zero[4800] = {0};

	fp = fopen(pcm_file,"ab");

	fwrite(buf_zero,sizeof(char),sizeof(buf_zero),fp);

	fclose(fp);
}

/*****************************************************************************
 * Function      : voice_detect_thread
 * Description   : 监测声音线程，用来判断当前是否有声音以及端点检测
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
voice_detect_thread(void *arg)
{
	int i,ret,silent_cnt,silent_flag = 0;
	char *capture_data;
	snd_pcm_t *capture_handle;

	mqd_t snd_mqdid = *(mqd_t*)arg;

	/*初始化录音设备*/
	ret = pcm_device_init(&capture_handle,&vd_params);
	if(ret<0)
	{
		LOG_FATAL((BSL_META("pcm_device_init failed\n")));
		mq_close(snd_mqdid);
		return;
	}

	/*初始化VAD检测*/
	VadInst* vad_ptr = webrtc_vad_init();
	if(NULL == vad_ptr)
	{
		LOG_FATAL((BSL_META("webrtc_vad_init failed\n")));
		pcm_device_free(capture_handle);
		mq_close(snd_mqdid);
		return;
	}

#if 0
	/*在MINI2440跑降噪算法，太耗时，所以取消*/
	NsHandle* pNS_inst = webrtc_denoise_init(16000,2);
	if(NULL == pNS_inst)
	{
		printf("webrtc_denoise_init failed\n");
		return;
	}
#endif

	capture_data = malloc(vd_params.rec_len);
	if(NULL == capture_data)
	{
		LOG_FATAL((BSL_META("malloc failed:%s\n"),strerror(errno)));
		webrtc_vad_free(vad_ptr);
		pcm_device_free(capture_handle);
		mq_close(snd_mqdid);
		return;
	}

	int pcm_serial = 0;
	char pcm_file[32] = "record-0.pcm";
	inject_silent_area(pcm_file);
	
	for(;;)
	{			
		snd_pcm_sframes_t snd_res;
		snd_res = snd_pcm_readi(capture_handle,capture_data,vd_params.frames);
		if(snd_res == -EPIPE) /* EPIPE means overrun */
		{
			LOG_WARN((BSL_META("Overrun occurred\n")));
			snd_pcm_prepare(capture_handle);
			continue;
		}
		else if(snd_res != vd_params.frames)
		{
			LOG_WARN((BSL_META("read from audio interface failed (%s)\n"),snd_strerror(snd_res)));
			continue;
		}

		/*采集到完整数据，进行处理*/
		char *audio_data;

		if(vd_params.channel == 2)
		{
			audio_data = stereo_to_mono(capture_data,vd_params.rec_len);
			if(NULL == audio_data)
			{
				LOG_ERROR((BSL_META("stereo_to_mono\n")));
				continue;
			}
		}
		else 
			audio_data = capture_data;

	#if 0
		/*去除降噪处理*/
		char* denoise_data = denoise_process(pNS_inst,res_data,vd_params.rec_len/vd_params.channel);
		if(vd_params.channel == 2)
			free(res_data);
	#endif
	
		vad_status_t vad_res;
		vad_res = webrtc_vad_process(vad_ptr,(short*)audio_data,vd_params.frames,
										VAD_WINDOWS_SIZE,vd_params.rate);
		if(vad_res<0)
		{
			LOG_WARN((BSL_META("webrtc_vad_process failed\n")));
			free(audio_data);
			continue;
		}

		/*如果检测到声音就写入文件*/
		if(vad_res == RUNNING)  
		{
			LOG_DEBUG(("data ... yes\n"));
			silent.cnt = 0;
			silent.status = DETECT_NO;

			/*写文件*/
			FILE* fp = NULL;
			ssize_t wt_len;
			
			fp = fopen(pcm_file,"ab");
			if(NULL == fp)
			{
				LOG_ERROR((BSL_META("fopen:%s\n"),strerror(errno)));
				free(audio_data);
				continue;
			}
		
			wt_len = fwrite(audio_data,sizeof(char),vd_params.rec_len/vd_params.channel,fp);
			if(wt_len != vd_params.rec_len/vd_params.channel)
			{
				LOG_ERROR((BSL_META("fwrite:%s\n"),strerror(errno)));
				free(audio_data);
				fclose(fp);
				continue;
			}
			
			free(audio_data);
			fclose(fp);
			current_status = RUNNING;
		}
		else if(vad_res == LISTENING)
		{
			free(audio_data);
			
			LOG_DEBUG(("data ... no\n"));

			/*
				必须连续识别到SILENT_WINDOW_CNT个静音区才表示真的静音,防止出现说话中间停顿
				误判为说话停止。
			*/
			silent.cnt++;
			if(silent.cnt == SILENT_WINDOW_CNT)   
			{
				silent.cnt = 0;
				silent.status = DETECT_IN;
			}

			/*如果当前状态为运行态且连续检测到静音区，则表明到达语音末端*/
			if(current_status == RUNNING && 
				silent.status == DETECT_IN) 
			{	
				silent.status = DETECT_NO;
				current_status = LISTENING;
				
				LOG_DEBUG(("data ... end\n"));
				LOG_DEBUG(("detect: pcm_file ... %s\n",pcm_file));

				/*注入尾部静音区*/
				inject_silent_area(pcm_file); 
				ret = mq_send(snd_mqdid,pcm_file,sizeof(pcm_file),0);
				if(ret<0)
				{
					LOG_ERROR((BSL_META("mq_send:%s\n"),strerror(errno)));
				}
				
				/*开始写下一个文件*/	
				pcm_serial++;
				if(pcm_serial == 256)
					pcm_serial == 0;
				sprintf(pcm_file,"record-%d.pcm",pcm_serial);
				/*注入头部静音区*/
				inject_silent_area(pcm_file);			
			}
		}
	}	
}

