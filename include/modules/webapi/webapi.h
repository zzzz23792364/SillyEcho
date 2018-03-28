/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : webapi.h
 * Author        : Zengchao
 * Date          : 2018-03-27
 * Description   : webapi.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __WEBAPI_H__
#define __WEBAPI_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#include "iflytek_semantic.h"
#include "iflytek_tts.h"


typedef struct web_api_s
{
	int (*semantic)(char*,U32,char **,semantic_code_t*);
	int (*tts)(char*,char*);
	int (*diag)(void);
}web_api_t;


//web_api_t webapi = 
//{
//	.semantic = iflytek_semantic_api,
//	.tts = iflytek_tts_api,
//	.diag = NULL,
//};

extern web_api_t webapi;

extern void web_api_config(web_api_t *api);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WEBAPI_H__ */
