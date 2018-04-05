/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : service_crosstalk.h
 * Author        : Zengchao
 * Date          : 2018-04-06
 * Description   : service_crosstalk.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-06
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __SERVICE_CROSSTALK_H__
#define __SERVICE_CROSSTALK_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct crosstalk_list_s
{
	char *url;
	struct crosstalk_list_s *next;
}crosstalk_list_t;

extern crosstalk_list_t* crosstalk_url_head;
extern pthread_mutex_t crosstalk_lock;
extern pthread_cond_t crosstalk_ready;


extern void service_crosstalk_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SERVICE_CROSSTALK_H__ */
