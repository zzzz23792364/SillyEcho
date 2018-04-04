/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : play_audio.h
 * Author        : Zengchao
 * Date          : 2018-04-05
 * Description   : play_audio.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-05
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __PLAY_AUDIO_H__
#define __PLAY_AUDIO_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum audio_type_s
{
	AUDIO_PCM,
	AUDIO_WAV,
	AUDIO_MP3
}audio_type_t;

typedef struct play_list_s
{
	char audio_name[256];
	audio_type_t audio_type;
	struct play_list_s *next;
}play_list_t;

extern play_list_t *play_list_head;
extern pthread_mutex_t play_lock;	
extern pthread_cond_t play_ready;

extern void play_audio_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PLAY_AUDIO_H__ */
