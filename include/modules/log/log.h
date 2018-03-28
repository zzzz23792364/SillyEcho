/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : log.h
 * Author        : ZengChao
 * Date          : 2018-01-24
 * Description   : log.h header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-01-24
 *   Author      : ZengChao
 *   Modification: Created file

************************************************************************************/


#ifndef _LOG_H_H
#define _LOG_H_H

#include <stdarg.h>


#define LOG_TO_FILE


#define BSLSINK_SINK_NAME_MAX           32
#define BSLSINK_PREFIX_FORMAT_MAX       32

#ifndef BSL_FILE
#define BSL_FILE    __FILE__
#endif

#ifndef BSL_LINE
#define BSL_LINE    __LINE__
#endif

#ifndef BSL_FUNC
#define BSL_FUNC    __FUNCTION__
#endif

#define __NONE__         "\033[0m"
#define __BLACK__        "\033[0;30m"
#define __DARK_GRAY__    "\033[1;30m"
#define __BLUE__         "\033[0;34m"
#define __LIGHT_BLUE__   "\033[1;34m"
#define __GREEN__        "\033[0;32m"
#define __LIGHT_GREEN__  "\033[1;32m"
#define __CYAN__         "\033[0;36m"
#define __LIGHT_CYAN__   "\033[1;36m"
#define __RED__          "\033[0;31m"
#define __LIGHT_RED__    "\033[1;31m"
#define __PURPLE__       "\033[0;35m"
#define __LIGHT_PURPLE__ "\033[1;35m"
#define __BROWN__        "\033[0;33m"
#define __YELLOW__       "\033[1;33m"
#define __LIGHT_GRAY__   "\033[0;37m"
#define __WHITE__        "\033[1;37m"


typedef enum log_module_e {
	logModuleAppl             = 0,
	logModuleSal			  = 1,
	logModuleCount            = 2
} log_module_t;


typedef enum bsl_severity_e {
    bslSeverityOff            = 0,
    bslSeverityFatal          = 1,
    bslSeverityError          = 2,
    bslSeverityWarn           = 3,
    bslSeverityInfo           = 4,
    bslSeverityVerbose        = 5,
    bslSeverityDebug          = 6,
    bslSeverityNormal         = bslSeverityDebug, ///bslSeverityInfo,
    bslSeverityCount          = 7
} bsl_severity_t;


typedef struct log_sever_info_e
{
	char *severInstr;
	char *severColor;
}log_sever_info_t;



typedef struct bsl_meta_s {
    const char *func;
    const char *file;
    int line;

	log_module_t module;
    bsl_severity_t severity;
} bsl_meta_t;

typedef struct bslsink_severity_range_s {
    bsl_severity_t min;
    bsl_severity_t max;
} bslsink_severity_range_t;


typedef struct bslsink_sink_s {

    /* Next sink */
    struct bslsink_sink_s *next;

    /* Sink name */
    char name[BSLSINK_SINK_NAME_MAX];

    /* Unique ID for retrieval and removal */
    int sink_id;

    /* Low-level output function */
    int (*vfprintf)(void *,bsl_meta_t*,const char *, va_list);

    /* Opaque file handle */
    void *file;

    /* Messages within this severity range will be printed */
    bslsink_severity_range_t enable_range;

    /* Messages within this severity range will be prefixed */
    bslsink_severity_range_t prefix_range;

    /* Prefix configuration */
    char prefix_format[BSLSINK_PREFIX_FORMAT_MAX+1];

} bslsink_sink_t;


/* Message macro which includes the standard meta data */
#define BSL_META(str_) \
    "<c=%uf=%sl=%dF=%s>" str_, mchk_, BSL_FILE, BSL_LINE, BSL_FUNC

/* Organize layer/source/severity in a single 32-bit word */
//#define BSL_LAYER_SHFT          24
//#define BSL_LAYER_MASK          0xff
#define BSL_MODULE_SHFT         8
#define BSL_MODULE_MASK         0xffff
#define BSL_SEVERITY_SHFT       0
#define BSL_SEVERITY_MASK       0xff


#define BSL_SEVERITY_ENC(severity_) \
    ((severity_) << BSL_SEVERITY_SHFT)


/* Severity defines for LOG_* macros */
#define BSL_FATAL               BSL_SEVERITY_ENC(bslSeverityFatal)
#define BSL_ERROR               BSL_SEVERITY_ENC(bslSeverityError)
#define BSL_WARN                BSL_SEVERITY_ENC(bslSeverityWarn)
#define BSL_INFO                BSL_SEVERITY_ENC(bslSeverityInfo)
#define BSL_VERBOSE             BSL_SEVERITY_ENC(bslSeverityVerbose)
#define BSL_DEBUG               BSL_SEVERITY_ENC(bslSeverityDebug)


/* Get elements of packed meta data */
#define BSL_MODULE_GET(chk_) \
    (((chk_) >> BSL_MODULE_SHFT) & BSL_MODULE_MASK)
#define BSL_SEVERITY_GET(chk_) \
    (((chk_) >> BSL_SEVERITY_SHFT) & BSL_SEVERITY_MASK)


int bsl_printf(const char* format,...);


/* Macro for invoking "fast" checker */
#define BSL_LOG(chk_, stuff_) do \
{ \
    unsigned int mchk_ = chk_;	\
    (void)mchk_;	\
    bsl_printf stuff_;	\
} while(0)


/* Any layer log macros */
#if 0
#define LOG_FATAL(ls_, stuff_)          BSL_LOG(ls_|BSL_FATAL, stuff_)
#define LOG_ERROR(ls_, stuff_)          BSL_LOG(ls_|BSL_ERROR, stuff_)
#define LOG_WARN(ls_, stuff_)           BSL_LOG(ls_|BSL_WARN, stuff_)
#define LOG_INFO(ls_, stuff_)           BSL_LOG(ls_|BSL_INFO, stuff_)
#define LOG_VERBOSE(ls_, stuff_)        BSL_LOG(ls_|BSL_VERBOSE, stuff_)
#define LOG_DEBUG(ls_, stuff_)          BSL_LOG(ls_|BSL_DEBUG, stuff_)
#endif

#define LOG_FATAL(stuff_)          BSL_LOG(BSL_FATAL, stuff_)
#define LOG_ERROR(stuff_)          BSL_LOG(BSL_ERROR, stuff_)
#define LOG_WARN(stuff_)           BSL_LOG(BSL_WARN, stuff_)
#define LOG_INFO(stuff_)           BSL_LOG(BSL_INFO, stuff_)
#define LOG_VERBOSE(stuff_)        BSL_LOG(BSL_VERBOSE, stuff_)
#define LOG_DEBUG(stuff_)          BSL_LOG(BSL_DEBUG, stuff_)


#define cli_out bsl_printf 


extern log_sever_info_t log_sever_array[];


extern int log_file_severity_set(bsl_severity_t min,bsl_severity_t max);
extern int log_console_severity_set(bsl_severity_t min, bsl_severity_t max);


extern int bslsink_sink_add(bslsink_sink_t * sink);
extern void bslsink_sink_t_init(bslsink_sink_t *sink);
extern int log_init();

#endif
