/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : iflytek_service.c
 * Author        : Zengchao
 * Date          : 2018-04-06
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-04-06
 *   Author      : Zengchao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>	

#include "log.h"
#include "service_news.h"
#include "service_crosstalk.h"
#include "iflytek_service.h"



#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct iflytek_service_s
{
	char *service;
	int (*handler)(cJSON *service);
}iflytek_service_t;

int iflytek_crosstalk_service(cJSON * service);
int iflytek_news_service(cJSON * service);

iflytek_service_t services[] = 
{
	{"crossTalk", iflytek_crosstalk_service},
	{"news", iflytek_news_service},
};

/*****************************************************************************
 * Function      : iflytek_news_service
 * Description   : 讯飞新闻服务处理函数
 * Input         : cJSON *service  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180406
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
int 
iflytek_news_service(cJSON *service)
{
	int i,res_num;
	
	cJSON *data = cJSON_GetObjectItem(service,"data");
	if(NULL == data)
	{
		LOG_ERROR((BSL_META("cJSON_GetObjectItem ---> service data\n")));
		return;
	}

	cJSON *result = cJSON_GetObjectItem(data,"result");
	if(NULL == result)
	{
		LOG_ERROR((BSL_META("cJSON_GetObjectItem ---> result\n")));
		return;
	}
	
	res_num = cJSON_GetArraySize(result);

	printf("... res_num = %d ...\n",res_num);

	for(i=0;i<res_num;i++)
	{
		cJSON *item = cJSON_GetArrayItem(result,i);
		char* url_str = cJSON_GetObjectItem(item,"url")->valuestring;
		int url_len = strlen(url_str)+1;
		char *url_cpy = malloc(url_len);
		if(NULL == url_cpy)
		{
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			return;
		}

		printf("url = %s\n",url_str);
		
		strncpy(url_cpy,url_str,url_len);
		
		news_list_t *entry = malloc(sizeof(news_list_t));
		if(NULL == entry)
		{
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			free(url_cpy);
			return;
		}

		pthread_mutex_lock(&news_lock);
		entry->url = url_cpy;
		entry->next = news_url_head;
		news_url_head = entry;
		pthread_mutex_unlock(&news_lock);
		pthread_cond_signal(&news_ready);
	}
	
	return;

}


/*****************************************************************************
 * Function      : iflytek_crosstalk_service
 * Description   : 讯飞相声处理函数
 * Input         : cJSON *service  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180406
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
int 
iflytek_crosstalk_service(cJSON *service)
{
	int i,res_num;

	cJSON *data = cJSON_GetObjectItem(service,"data");
	if(NULL == data)
	{
		LOG_ERROR((BSL_META("cJSON_GetObjectItem ---> service data\n")));
		return;
	}

	cJSON *result = cJSON_GetObjectItem(data,"result");
	if(NULL == result)
	{
		LOG_ERROR((BSL_META("cJSON_GetObjectItem ---> result\n")));
		return;
	}
	
	res_num = cJSON_GetArraySize(result);

	printf("... res_num = %d ...\n",res_num);

	for(i=0;i<res_num;i++)
	{
		cJSON *item = cJSON_GetArrayItem(result,i);
		char* url_str = cJSON_GetObjectItem(item,"url")->valuestring;
		int url_len = strlen(url_str)+1;
		char *url_cpy = malloc(url_len);
		if(NULL == url_cpy)
		{
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			return;
		}

		printf("url = %s\n",url_str);
		
		strncpy(url_cpy,url_str,url_len);
		
		news_list_t *entry = malloc(sizeof(news_list_t));
		if(NULL == entry)
		{
			LOG_ERROR((BSL_META("malloc failed:%s\n"),strerror(errno)));
			free(url_cpy);
			return;
		}

		pthread_mutex_lock(&crosstalk_lock);
		entry->url = url_cpy;
		entry->next = crosstalk_url_head;
		crosstalk_url_head = entry;
		pthread_mutex_unlock(&crosstalk_lock);
		pthread_cond_signal(&crosstalk_ready);
	}
	
	return;
}


/*****************************************************************************
 * Function      : iflytek_service_process
 * Description   : 按讯飞返回的服务分别进行处理
 * Input         : cJSON *data  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180404
 *   Author      : Zengchao
 *   Modification: Created function

*****************************************************************************/
void  
iflytek_service_process(cJSON *data)
{
	int i;
	char *service = cJSON_GetObjectItem(data,"service")->valuestring;
	if(NULL == service)
	{
		LOG_ERROR((BSL_META("response data has no service\n")));
		return;
	}

	iflytek_service_t *entry = NULL;

	for(i=0;i<ARRAY_SIZE(services);i++)
	{
		if(!strcmp(service,services[i].service))
			entry = services + i;
	}

	if(NULL == entry)
	{
		LOG_ERROR((BSL_META("no found service in services\n")));
		return;
	}	

	(entry->handler)(data);
}

