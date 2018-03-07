/**************************************************************************
 * Filename:       zbSocZcl.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    ZCL协议层处理程序
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __ZB_SOC_ZCL_H__
#define __ZB_SOC_ZCL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "GwComDef.h"
#include "comParse.h"
#include "zbSocCmd.h"
/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * MACROS
 */
 
// Security and Safety (SS) Clusters
#define ZCL_CLUSTER_ID_SS_IAS_ZONE                           0x0500
#define ZCL_CLUSTER_ID_SS_IAS_ACE                            0x0501
#define ZCL_CLUSTER_ID_SS_IAS_WD                             0x0502

// Server commands generated (Server-to-Client in ZCL Header)
#define COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION                   0x00
#define COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST                        0x01

/*** Zone Status Attribute values ***/
#define SS_IAS_ZONE_STATUS_ALARM1_ALARMED                                0x0001
#define SS_IAS_ZONE_STATUS_ALARM2_ALARMED                                0x0002
#define SS_IAS_ZONE_STATUS_TAMPERED_YES                                  0x0004
#define SS_IAS_ZONE_STATUS_BATTERY_LOW                                   0x0008
#define SS_IAS_ZONE_STATUS_SUPERVISION_REPORTS_YES                       0x0010
#define SS_IAS_ZONE_STATUS_RESTORE_REPORTS_YES                           0x0020
#define SS_IAS_ZONE_STATUS_TROUBLE_YES                                   0x0040
#define SS_IAS_ZONE_STATUS_AC_MAINS_FAULT                                0x0080

#define SS_IAS_ZONE_STATUS_BATTERY_LOW_ALARM			0x01
#define SS_IAS_ZONE_STATUS_BATTERY_LOW_UNALARM		0x00
/*********************************************************************
 * TYPEDEFS
 */
 
typedef uint8_t ZStatus_t;

// Function pointer type to handle incoming messages.
//	 msg - incoming message
//	 logicalClusterID - logical cluster ID
typedef ZStatus_t (*zclInHdlr_t)( hostCmd *cmd,uint8_t zclFrameLen, uint16_t clusterID, uint16_t nwkAddr, uint8_t endpoint);

typedef struct zclLibPlugin
{
	uint16_t							startClusterID; 	 // starting cluster ID
	uint16_t							endClusterID; 		 // ending cluster ID
	zclInHdlr_t 				pfnIncomingHdlr;		// function to handle incoming message
} zclLibPlugin_t;

/*********************************************************************
 * VARIABLES
 */
 
    
/*********************************************************************
 * FUNCTIONS
 */
 zclLibPlugin_t *zclFindplugin(uint16_t clusterID);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __ZB_SOC_ZCL_H__ */

