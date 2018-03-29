
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "http_post.h"


/*****************************************************************************
 * Function      : call_back_request
 * Description   : HTTP POST请求回调函数
 * Input         : void *contents  
                size_t size     
                size_t nmemb    
                void *userp     
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180312
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
size_t call_back_request(void *contents, size_t size, size_t nmemb, void *userp)
{
//	size_t realsize = size * nmemb;
//	struct MemoryStruct *mem = (struct MemoryStruct *) userp;
//	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
//	if (mem->memory == NULL)
//	{
//		err_log("not enough memory (realloc returned NULL)\n");
//		return 0;
//	}
//	memcpy(&(mem->memory[mem->size]), contents, realsize);
//	mem->size += realsize;
//	mem->memory[mem->size] = 0;
//	return realsize;


	size_t cur_size = size * nmemb;
	post_response_data_t *tmp = (post_response_data_t *)userp;
	
	tmp->data = realloc(tmp->data,tmp->len + cur_size);
	if(NULL == tmp->data)
	{
		LOG_ERROR((BSL_META("malloc:%s\n"),strerror(errno)));
		return 0;
	}

	memcpy(&tmp->data[tmp->len],contents,cur_size);
	tmp->len += cur_size;
//	tmp->data[tmp->len] = 0;
	return cur_size;

//	post_response_data_t *tmp = (post_response_data_t *)userp;

//	tmp->len = size * nmemb;
//	tmp->data = malloc(tmp->len);
//	if(NULL == tmp->data)
//	{
//		printf("not enough memory (realloc returned NULL)\n");
//		return 0;
//	}

//	memcpy(tmp->data,contents,tmp->len);
//	tmp->data[tmp->len] = '\0';
//	printf("tmp->data = %s\n",tmp->data);
//	free(tmp->data);

//	printf("tmp->data = %s\n",contents);
	return tmp->len;
}

/*****************************************************************************
 * Function      : http_post_request
 * Description   : HTTP POST请求函数
 * Input         : char *host_url                  
                struct curl_slist * headerlist  
                char *body_data                 
                int body_len                    
                post_back_data_t *back_data     
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180312
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int
http_post_request(char *host_url,struct curl_slist * headerlist,
						char *body_data,int body_len,post_response_data_t *back_data)
{
	CURL *curl;
	CURLcode ret;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, host_url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
//	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_data);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_len);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_back_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * )back_data);
	
	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK)
	{
		printf("perform curl error:%d.\n", ret);
		return ret;
	}

//	printf("\nback_data->data = %s\n",back_data->data);
	
	curl_slist_free_all(headerlist);
	curl_easy_cleanup(curl);
//	curl_global_cleanup();

	return 0;
}

