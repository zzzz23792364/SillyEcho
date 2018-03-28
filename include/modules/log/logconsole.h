/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : logconsole.h
 * Author        : ZengChao
 * Date          : 2018-01-24
 * Description   : logconsole.h header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-01-24
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __LOGCONSOLE_H__
#define __LOGCONSOLE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


extern int log_console_init();
extern int log_console_severity_set(bsl_severity_t min,bsl_severity_t max);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LOGCONSOLE_H__ */

