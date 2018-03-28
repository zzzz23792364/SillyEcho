/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : vad.h
 * Author        : ZengChao
 * Date          : 2018-03-15
 * Description   : vad.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-15
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __VAD_H__
#define __VAD_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "webrtc_vad.h"

typedef enum vad_status_e
{
	ERROR = -1,
	RUNNING = 0,
	LISTENING = 1
}vad_status_t;


extern vad_status_t voice_data_process(VadInst* pVad,short *audio_data,int audio_len,
         							int frame_len,int sample_rate);
extern void webrtc_vad_free(VadInst* pVad);
extern VadInst* webrtc_vad_init();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VAD_H__ */
