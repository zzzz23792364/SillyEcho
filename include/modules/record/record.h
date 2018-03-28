/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : record.h
 * Author        : ZengChao
 * Date          : 2018-03-15
 * Description   : record.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-15
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __RECORD_H__
#define __RECORD_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <alsa/asoundlib.h>

#include "cbb_types.h"

typedef struct voice_device_params_s
{
	U8 *name; 
	U32 rate; 
	U16 bits;
	U32 channel;
	U32 rec_time; //unit:ms
	U32 rec_len;
	snd_pcm_sframes_t frames; 
	snd_pcm_format_t format;
}voice_device_params_t;


extern void pcm_device_free(snd_pcm_t **handler);
extern int pcm_device_init(snd_pcm_t * * handler, voice_device_params_t * vd_params);
extern void pcm_hw_params_configs(voice_device_params_t *params);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RECORD_H__ */
