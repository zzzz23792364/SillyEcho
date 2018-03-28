/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : tts.h
 * Author        : Zengchao
 * Date          : 2018-03-27
 * Description   : tts.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __TTS_H__
#define __TTS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#include "cbb_types.h"

typedef struct tts_list_s
{
	char *text;
	struct tts_list_s *next;
}tts_list_t;

extern tts_list_t *tts_list_head;


extern void voice_tts_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __TTS_H__ */
