/********************************************************************************

 **** Copyright (C), 2018, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : dialog.h
 * Author        : ZengChao
 * Date          : 2018-03-27
 * Description   : dialog.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2018-03-27
 *   Author      : ZengChao
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __DIALOG_H__
#define __DIALOG_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef struct diag_list_s
{
	char *text;
	struct diag_list_s *next;
}diag_list_t;



diag_list_t *diag_list_head = NULL;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DIALOG_H__ */
