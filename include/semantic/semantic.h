/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : semantic.h
 * Author        : ZengChao
 * Date          : 2018-03-27
 * Description   : semantic.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "cbb_types.h"


typedef enum semantic_code_e
{
	RET_ERROR, //返回错误
	RET_TEXT,  //只返回了文本 	
	RET_ANSWER //返回语义回答
}semantic_code_t;


extern void voice_semantic_thread(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SEMANTIC_H__ */
