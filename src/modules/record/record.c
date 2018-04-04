

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "record.h"

#define DEFAULT_RECORD_DEVICE "hw:0,0" //"plughw:1,0"

#define DETECT_WINDOWS_SIZE  300  //300

/*****************************************************************************
 * Function      : pcm_hw_params_configs
 * Description   : 配置声卡参数：声卡名、采样率、通道数、格式
 * Input         : voice_device_params_t *vd_params  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180314
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
pcm_hw_params_configs(voice_device_params_t *params)
{
	params->name = DEFAULT_RECORD_DEVICE;
	params->rate = 16000;
	params->bits = 16;
	params->channel = 2,
	params->format = SND_PCM_FORMAT_S16_LE;
	params->rec_time = DETECT_WINDOWS_SIZE;	
	params->frames = (params->rate*params->rec_time)/1000;
	params->rec_len = (params->frames * params->channel * params->bits)/8;
}


void 
pcm_device_free(snd_pcm_t **handler)
{
	snd_pcm_close(handler);
}

/*****************************************************************************
 * Function      : pcm_device_init
 * Description   : 初始化声卡设备
 * Input         : snd_pcm_t **handler  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180314
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int 
pcm_device_init(snd_pcm_t **handler,voice_device_params_t *vd_params)
{
	int ret;
	
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;

	pcm_hw_params_configs(vd_params);

	ret = snd_pcm_open(&capture_handle,vd_params->name,SND_PCM_STREAM_CAPTURE, 0);
	if(ret<0)
	{
		fprintf (stderr, "cannot open audio device %s (%s)\n",
						vd_params->name,snd_strerror(ret));
		return -1;
	}

	ret = snd_pcm_hw_params_malloc(&hw_params);
	if(ret<0)
	{
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
						snd_strerror(ret));
		snd_pcm_close(capture_handle);
		return -1;
	}

	ret = snd_pcm_hw_params_any(capture_handle, hw_params);
	if(ret<0)
	{
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
						snd_strerror(ret));
		goto err_exit;
	}
	
	ret = snd_pcm_hw_params_set_access(capture_handle, hw_params, 
								SND_PCM_ACCESS_RW_INTERLEAVED);
	if(ret<0)
	{
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror(ret));
		goto err_exit;
	}
	
	ret = snd_pcm_hw_params_set_format(capture_handle, hw_params,vd_params->format);
	if(ret<0)
	{
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (ret));
		return -1;
	}

	ret = snd_pcm_hw_params_set_rate_near(capture_handle,hw_params,&vd_params->rate,0);
	if(ret<0)
	{
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (ret));
		goto err_exit;
	}

	ret = snd_pcm_hw_params_set_channels(capture_handle,hw_params,vd_params->channel);
	if(ret<0)
	{
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (ret));
		goto err_exit;
	}

	ret = snd_pcm_hw_params (capture_handle,hw_params);
	if(ret<0)
	{
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (ret));
		goto err_exit;
	}

	*handler = capture_handle;

	snd_pcm_hw_params_free (hw_params);
	
	return 0;

err_exit:
	snd_pcm_hw_params_free (hw_params);
	snd_pcm_close (capture_handle);
	return -1;
}



