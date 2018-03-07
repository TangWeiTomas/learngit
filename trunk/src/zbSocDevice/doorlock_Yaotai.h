/***********************************************************************************
 * 文 件 名   : doorLock_Yaotai.h
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 遥泰门锁功能接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#ifndef __DOORLOCK_YAOTAI_H__
#define __DOORLOCK_YAOTAI_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "doorlock.h"
 

/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * MACROS
 */
 
#define MT_SOC_YAOTAI_ENDPOINT	0X07

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern zbSocDoorLockCallbacks_t	zbSocDoorLock_YaoTaiCallBack_t;

/*********************************************************************
 * FUNCTIONS
 */
 
//extern void doorLockYT_usrMngReq(epInfo_t *epInfo,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __DOORLOCK_YAOTAI_H__ */
