/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : http-post.h
 * Author        : ZengChao
 * Date          : 2018-03-12
 * Description   : http-post.h header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-12
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __HTTP_POST_H__
#define __HTTP_POST_H__


#include "curl.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct post_response_data_s
{
	char *data;
	size_t len;
}post_response_data_t;


int http_post_request(char * host_url, struct curl_slist * headerlist, 
			char * body_data, int body_len, post_response_data_t * response_data);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HTTP-POST_H__ */
