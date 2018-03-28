/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : iflytek_semantic.h
 * Author        : Zengchao
 * Date          : 2018-03-27
 * Description   : iflytek_semantic.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __IFLYTEK_SEMANTIC_H__
#define __IFLYTEK_SEMANTIC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#include "curl.h"
#include "semantic.h"
#include "cbb_types.h"


extern int iflytek_semantic_api(char *audio_data,U32 audio_len,
         					char **res_text,semantic_code_t *res_code);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __IFLYTEK_SEMANTIC_H__ */
