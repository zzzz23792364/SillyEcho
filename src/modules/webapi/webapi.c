/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : webapi.c
 * Author        : Zengchao
 * Date          : 2018-03-27
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/


#include "webapi.h"

web_api_t webapi;

/*****************************************************************************
 * Function      : web_api_config
 * Description   : 配置WEBAPI所用的接口
 * Input         : web_api_t *api  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
void 
web_api_config(web_api_t *api)
{
	api->semantic = iflytek_semantic_api;
	api->tts = iflytek_tts_api;
	api->diag = NULL;
}



