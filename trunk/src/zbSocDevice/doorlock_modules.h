/**************************************************************************
 * Filename:       doorlock_modules.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    普通门锁模块控制接口
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __DOORLOCK_MODULES_H__
#define __DOORLOCK_MODULES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "doorlock_zbSocCmds.h"
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
 
extern zbSocDoorLockCallbacks_t	zbSocDoorLock_modulesCallBack_t ;   
/*********************************************************************
 * FUNCTIONS
 */
 

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __DOORLOCK_MODULES_H__ */

