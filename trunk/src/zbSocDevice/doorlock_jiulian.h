/**************************************************************************
 * Filename:       doorlock_jiulian.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    久联门锁接口
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __DOORLOCK_JIULIAN__H__
#define __DOORLOCK_JIULIAN__H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zbSocCmd.h"
#include "doorlock.h"

/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
 extern zbSocDoorLockCallbacks_t zbSocDoorLock_JiuLianCallBack_t;
/*********************************************************************
 * FUNCTIONS
 */
 
extern void zbSocDoorlock_JiuLianSerialNetProcess(epInfo_t *epInfo,hostCmd *cmd);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __DOORLOCK_JIULIAN__H__ */

