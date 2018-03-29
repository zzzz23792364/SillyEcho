/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : iflytek_semantic.c
 * Author        : ZengChao
 * Date          : 2018-03-27
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>	

#include "b64.h"
#include "cJSON.h"
#include "curl.h"
#include "http_post.h"

#include "semantic.h"
#include "log.h"

#define MAX_BUFFER_SIZE 512

#define IFLY_APPID    "5aa3be70"
#define IFLY_SEMANTIC_URL  "http://api.xfyun.cn/v1/aiui/v1/voice_semantic"
#define IFLY_APPKEY   "cef0d11b03074451b2f5b57a51480e08"

extern int
calculate_md5(unsigned char *data,size_t len,unsigned char *md5);


/*****************************************************************************
 * Function      : iflytek_get_semantic_param
 * Description   : 计算讯飞HTTP头部所需X-Param
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180312
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
char* 
iflytek_get_semantic_param()
{	
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "auf", "16k");
	cJSON_AddStringToObject(root, "aue", "raw");
	cJSON_AddStringToObject(root, "scene", "main");
	cJSON_AddStringToObject(root, "userid", "user_0001");
	
	int len;
	char buf[MAX_BUFFER_SIZE];

	len = strlen(cJSON_PrintUnformatted(root));
	memcpy(buf, cJSON_PrintUnformatted(root), len);
	buf[len] = '\0';

	char *param = b64_encode((const unsigned char *)buf,len);

	cJSON_Delete(root);
	return param;
}

/*****************************************************************************
 * Function      : iflytek_get_semantic_checksum
 * Description   : 计算讯飞HTTP HEADER所需的X-CheckSum
 * Input         : char *param       
                char* time        
                char *audio_data  
                char *md5_str     
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180312
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int  
iflytek_get_semantic_checksum(char *param,char* time,char *audio_data,char *md5_str)
{
	int token_len = sizeof(IFLY_APPKEY) + strlen(time) + strlen(param)
					+ strlen("data=") + strlen(audio_data);

	char *token = malloc(token_len);
	if(NULL == token)
	{
		printf("malloc failed -->[%s]:[%d]\n",__FILE__,__LINE__);
		return -1;	
	}

	strcpy(token,IFLY_APPKEY);
	strcat(token,time);
	strcat(token,param);
	strcat(token,"data=");
	strcat(token,audio_data);

	calculate_md5((unsigned char *)token, (size_t)strlen(token),md5_str);
	md5_str[32] = 0;

	free(token);
	return 0;
}


/*****************************************************************************
 * Function      : iflytek_headers_generator
 * Description   : 产生访问讯飞WebAPI所需的HTTP HEADER
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180312
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int 
iflytek_headers_generator(char *audio_data,int audio_len,
								   struct curl_slist ** header)
{
	int ret;
	char time_str[32];
	char temp[MAX_BUFFER_SIZE];	
	char *param; 
	char md5_str[33];
	memset(time_str,0,sizeof(time_str));
	memset(md5_str,0,sizeof(md5_str));
	
	int cur_time = (int)time(0);
//	cur_time = 1520833340;
	sprintf(time_str,"%d",cur_time);

	param = iflytek_get_semantic_param();

	ret = iflytek_get_semantic_checksum(param,time_str,audio_data,md5_str);
	if(ret<0)
	{
		printf("get_iFlytek_checksum failed\n");
		return -1;
	}

	/*******CREATE HTTP HEADER*******/
	struct curl_slist *headerlist = NULL;

	//X-Appid	
	memset(temp,0,sizeof(temp));
	sprintf(temp,"X-Appid:%s",IFLY_APPID);
	headerlist = curl_slist_append(headerlist,temp);
	//X-CurTime
	memset(temp,0,sizeof(temp));
	sprintf(temp,"X-CurTime:%s",time_str);
	headerlist = curl_slist_append(headerlist,temp);
	//X-Param
	memset(temp,0,sizeof(temp));
	sprintf(temp,"X-Param:%s",param);
	headerlist = curl_slist_append(headerlist,temp);
	//X-CheckSum
	memset(temp,0,sizeof(temp));
	sprintf(temp,"X-CheckSum:%s",md5_str);
	headerlist = curl_slist_append(headerlist,temp);

	*header = headerlist;

	return 0;
}


/*****************************************************************************
* Function		: iflytek_back_data_process
* Description	: 解析讯飞返回的JSON格式数据
* Input 		: post_back_data_t *back_data  
			   char *back_res				
* Output		: None
* Return		: 
* Others		: 
* Record
* 1.Date		: 20180312
*	Author		: ZengChao
*	Modification: Created function

*****************************************************************************/
int
iflytek_semantic_process(post_response_data_t *back_data,
									char **res_result,semantic_code_t *res_code)
{

	/*入参检查*/
	if(NULL == back_data->data)
	{
		printf("iflytek_reponse_data_process --> response_data has no data\n");
		return -1;
	}

	/*解析JSON格式响应数据*/
	cJSON *root = cJSON_Parse(back_data->data);
	if(NULL == root)
	{
		printf("cJSON_Parse failed\n");
		goto err_exit;
	}

	/*判断结果码*/
	char *code = cJSON_GetObjectItem(root, "code")->valuestring;
	if(strcmp(code,"00000"))
	{
		/*如果出错，打印错误信息*/
		char *desc = cJSON_GetObjectItem(root, "desc")->valuestring;
		printf("err: code:[%s],desc:[%s]\n",code,desc);
		goto err_exit;
	}

	cJSON *data = cJSON_GetObjectItem(root,"data");
	if(NULL == data)
	{
		printf("err: cJSON_GetObjectItem --> no data\n");
		goto err_exit;
	}

	/*判断应答码(response code)*/
	int rc = cJSON_GetObjectItem(data,"rc")->valueint;
	if(rc != 0 && rc != 4)
	{
		printf("err: response code = [%d]\n",rc);
		printf("我不懂你在说什么\n");
		*res_code = RET_ERROR;
		goto err_exit;
	}
	else if(rc == 4)
	{
		*res_code = RET_TEXT;
		
	}
	else if(rc == 0)
	{
		/*应答成功，提取返回的数据*/
		cJSON *answer = cJSON_GetObjectItem(data,"answer");
		if(NULL == answer)
		{
			printf("err: cJSON_GetObjectItem --> no answer\n");
			goto err_exit;
		}

		/*拷贝语义结果*/
		char *text = cJSON_GetObjectItem(answer,"text")->valuestring;
		int len = strlen(text)+1;	
		*res_result = malloc(len);
		if(NULL == *res_result)
		{
			printf("malloc failed -->[%s]:[%d]\n",__FILE__,__LINE__);
			goto err_exit;
		}
		
		strncpy(*res_result,text,len);
		*res_code = RET_ANSWER;
	}
	

//	free(back_data->data);
	return 0;
	
err_exit:
//	free(back_data->data);
	cJSON_Delete(root);
	return -1;
}



/*****************************************************************************
 * Function      : iflytek_semantic_api
 * Description   : 讯飞语义听写接口
 * Input         : char *audio_data           
                U32 audio_len              
                char **res_text            
                semantic_code_t *res_code  
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180327
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
int 
iflytek_semantic_api(char *audio_data,U32 audio_len,
					char **res_text,semantic_code_t *res_code)
{
	int ret;
	int decode_len,body_len;
	char *decode_data,*body_data;
	struct curl_slist * headerlist = NULL;
	post_response_data_t response_data;

	/*对原始音频文件进行base64编码*/
	decode_data = b64_encode((const unsigned char *)audio_data,audio_len);
	decode_len = strlen(decode_data);
	
	/*生成讯飞语义识别所需的HTTP HEADER*/
	ret = iflytek_headers_generator(decode_data,decode_len,&headerlist);
	if(ret<0)
	{
		LOG_ERROR((BSL_META("iFlytek_headers_generator failed\n")));
		free(decode_data);
		return -1;
	}

	/*识别数据需加上"data="前缀*/
	body_len = decode_len + strlen("data=");
	body_data = malloc(body_len+1);  //第二个BUG	,不然335行的strcat的时候会内存越界访问
	if(NULL == body_data)
	{
		LOG_ERROR((BSL_META("malloc:%s\n"),strerror(errno)));
		free(decode_data);
		return -1;
	}
	strcpy(body_data,"data=");
	strcat(body_data,decode_data);
	free(decode_data);

	response_data.len = 0;
	response_data.data = NULL;

	ret = http_post_request(IFLY_SEMANTIC_URL,headerlist,
						body_data,body_len,&response_data);	
	if(ret!=CURLE_OK)
	{
		LOG_ERROR((BSL_META("http_post_request failed\n")));
		free(body_data);
		return -1;
	}
	/*使用完之后释放发送数据所占内存*/
	free(body_data);

	response_data.data = realloc(response_data.data,response_data.len+1);
	response_data.data[response_data.len] = 0;

	printf("response_data->data = %s\n",response_data.data);

	/*对返回的语义数据进行分析*/
	ret = iflytek_semantic_process(&response_data,res_text,res_code);
	if(ret<0)
	{
		LOG_ERROR((BSL_META("iflytek_semantic_process failed\n")));
		return -1;
	}

	return 0;
}


