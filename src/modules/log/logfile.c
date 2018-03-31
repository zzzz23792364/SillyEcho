/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : logfile.c
 * Author        : ZengChao
 * Date          : 2018-01-24
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-01-24
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"


#define LOG_FILE "cbb.log"


bslsink_sink_t logfile_sink;


int 
log_file_vfprintf(void *file,bsl_meta_t *meta,const char*format,va_list args)
{
	int ret;
	char log_prefix[256];

	FILE *file_fp = fopen(LOG_FILE,"a");
	if(file_fp == NULL)
	{
		perror("log_file_vfprintf:");
		return -1;
	}

	if( meta->func && meta->file && (meta->line!=-1) && (meta->time != NULL))
	{
		char *severInstr = log_sever_array[meta->severity].severInstr;

		snprintf(log_prefix,sizeof(log_prefix),
				"[%s][%s in %s:%d] %s: ",
				meta->time,meta->func,meta->file,meta->line,severInstr);
		
		fwrite(log_prefix,sizeof(char),strlen(log_prefix),file_fp);
	}

	ret = vfprintf(file_fp,format,args);

	fclose(file_fp);

	return ret;
}


int 
log_file_severity_set(bsl_severity_t min,bsl_severity_t max)
{
	if(logfile_sink.vfprintf == NULL)
		return -1;
	
	logfile_sink.enable_range.min = min;
	logfile_sink.enable_range.max = max;
	return 0;
}


int
log_file_init()
{
	bslsink_sink_t *sink;

    /* Create logfile sink */
    sink = &logfile_sink;
    bslsink_sink_t_init(sink);
    strncpy(sink->name, "logfile", sizeof(sink->name));
    sink->vfprintf = log_file_vfprintf;
	sink->enable_range.min = bslSeverityOff+1;
    sink->enable_range.max = bslSeverityCount-1;

    bslsink_sink_add(sink);

	return 0;
}


