/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : service_news.h
 * Author        : Zengchao
 * Date          : 2018-04-04
 * Description   : service_news.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-04
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __SERVICE_NEWS_H__
#define __SERVICE_NEWS_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <pthread.h>
	
#include "cbb_types.h"


typedef struct news_list_s
{
	char *url;
	struct news_list_s *next;
}news_list_t;


extern pthread_mutex_t news_lock;
extern pthread_cond_t news_ready;
extern news_list_t* news_url_head;
extern void service_news_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SERVICE_NEWS_H__ */
