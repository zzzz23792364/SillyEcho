/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : log.c
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


#include <stdlib.h>
#include <time.h>
#include <sal/core/libc.h>

#include "log.h"
#include "logconsole.h"
#include "logfile.h"


bslsink_sink_t *bslsink_sinks;

log_sever_info_t log_sever_array[]=
{
	{"Off",__WHITE__},
	{"Fatal",__RED__},
	{"Error",__LIGHT_RED__},
	{"Warning",__PURPLE__},
	{"Info",__GREEN__},
	{"Verbose",__GREEN__},
	{"Debug",__WHITE__}
};



static char log_time[256];

/*****************************************************************************
 * Function      : bsl_log_time
 * Description   : 获取LOG时间
 * Input          : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Record
 * 1.Date        : 20180330
 *   Author      : ZengChao
 *   Modification: Created function

*****************************************************************************/
char*
bsl_log_time()
{
	memset(log_time,0,sizeof(log_time));

	time_t cur_time = time(0);

	struct tm *tm_time = localtime(&cur_time);
	
	snprintf(log_time,sizeof(log_time),"%d.%02d.%02d %02d:%02d:%02d",
		1900+tm_time->tm_year,1+tm_time->tm_mon,tm_time->tm_mday,
		tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);

	return log_time;
}



static int
severity_in_range(bslsink_severity_range_t *range, bsl_severity_t val)
{
    if (range == NULL) {
        return 0;
    }
    return (val >= range->min && val <= range->max);
}


int
bslsink_out(bslsink_sink_t *sink, bsl_meta_t *meta,
            const char *format, va_list args)
{
    bsl_severity_t sev = meta->severity;

    if (!severity_in_range(&sink->enable_range, sev)) {
        return 0;
    }

    return sink->vfprintf(sink->file,meta,format, args);
}

void
bsl_meta_t_init(bsl_meta_t *meta)
{
	sal_memset(meta, 0, sizeof(*meta));
	meta->module = logModuleCount;
	meta->severity = bslSeverityCount;
	meta->file = NULL;
	meta->func = NULL;
	meta->line = -1;
	meta->time = bsl_log_time();
}


int 
log_out_hook(bsl_meta_t *meta, const char *format, va_list args)
{
    int rv = 0;
    int sink_rv;
    va_list args_copy;
    bslsink_sink_t *sink = bslsink_sinks;

    while (sink != NULL) 
	{
        /* Avoid consuming same arg list twice. */
        va_copy(args_copy, args);
        sink_rv = bslsink_out(sink, meta, format, args_copy);
        va_end(args_copy); 
        if (sink_rv > 0) {
            rv = sink_rv;
        }
        sink = sink->next;
    }
    return rv;
}

int 
bsl_vprintf(const char*format,va_list args)
{
	const char *fmt = format;
    bsl_meta_t meta_data, *meta = &meta_data;

    bsl_meta_t_init(meta);

    if (*fmt == '<') {
        fmt++;
        while (1) {
            if (strncmp(fmt, "c=%u", 4) == 0) {
                unsigned int chk = va_arg(args, unsigned int);
                meta->module = BSL_MODULE_GET(chk);
                meta->severity = BSL_SEVERITY_GET(chk);
                fmt += 4;
            } else if (strncmp(fmt, "f=%s", 4) == 0) {
                meta->file = va_arg(args, char *);
                fmt += 4;
            } else if (strncmp(fmt, "l=%d", 4) == 0) {
                meta->line = va_arg(args, int);
                fmt += 4;
            } else if (strncmp(fmt, "F=%s", 4) == 0) {
                meta->func = va_arg(args, char *);
                fmt += 4;
            } else {
                break;
            }
        }
        if (*fmt == '>') {
            fmt++;
        }
    }

    /* Default to shell output */
    if (meta->module == logModuleCount) {
        meta->module = logModuleAppl;
        meta->severity = bslSeverityNormal;
    }

    /* Call output hook with whatever is left over */
    return log_out_hook(meta, fmt, args);
}

int 
bsl_printf(const char* format,...)
{
	int rc;
	va_list vargs;

	va_start(vargs,format);
	rc = bsl_vprintf(format,vargs);
	va_end(vargs);
	
	return rc;
}


int
bslsink_init(void)
{
    bslsink_sinks = NULL;
    return 0;
}


void
bslsink_sink_t_init(bslsink_sink_t *sink)
{
    sal_memset(sink, 0, sizeof(*sink));
}


int 
bslsink_sink_add(bslsink_sink_t *sink)
{
	sink->next = bslsink_sinks;
	bslsink_sinks = sink;

	return 0;
}


int 
log_init()
{
	bslsink_init();

	log_console_init();

#ifdef LOG_TO_FILE
	log_file_init();
#endif

	return 0;
}


