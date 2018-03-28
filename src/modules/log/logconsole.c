/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : logconsole.c
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
#include <string.h>

#include "log.h"

bslsink_sink_t console_sink;

int
log_console_printf(void *file,bsl_meta_t* meta,       const char* format,va_list args)
{
	char log_prefix[256];
	
	if( meta->func && meta->file && (meta->line!=-1))
	{
		char *severInstr = log_sever_array[meta->severity].severInstr;
		char *severColor = log_sever_array[meta->severity].severColor;

		sprintf(log_prefix,"[%s in %s:%d]%s %s: %s",meta->func,meta->file,meta->line,
											   severColor,severInstr,__NONE__);
		printf("%s",log_prefix);
	}

	return vprintf(format,args);
}


int 
log_console_severity_set(bsl_severity_t min,bsl_severity_t max)
{
	if(console_sink.vfprintf == NULL)
		return -1;
	
	console_sink.enable_range.min = min;
	console_sink.enable_range.max = max;
	return 0;
}


int 
log_console_init()
{
	bslsink_sink_t *sink;

	sink = &console_sink;

	bslsink_sink_t_init(sink);
	strncpy(sink->name,"console_log",sizeof(sink->name));
	sink->vfprintf = log_console_printf;
	sink->enable_range.min = bslSeverityOff+1;
    sink->enable_range.max = bslSeverityCount-1;

	bslsink_sink_add(sink);

	return 0;
}
