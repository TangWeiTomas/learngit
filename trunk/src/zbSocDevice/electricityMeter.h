/***********************************************************************************
 * 文 件 名   : electricityMeter.h
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 电表操作接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#ifndef __ELECTRICITY_METER_H__
#define __ELECTRICITY_METER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "Types.h"
#include "zbSocCmd.h"

/*********************************************************************
 * CONSTANTS
 */
 
#define OFF_WARN_VALUE  500 // 5.00KWH拉闸报警值 

enum{
	ELEC_METER_RW_ADDR = 0x00,   /*1*/
	ELEC_METER_R_CONSTANT, 	//
	ELEC_METER_RW_TOTAL_ELEC,
	ELEC_METER_R_SURPLUS_ELEC,
	ELEC_METER_R_OVERDRAFT_ELEC,
	ELEC_METER_R_BUY_CNT,
	ELEC_METER_R_BUY_ELEC,
	ELEC_METER_R_TOTAL_BUY_ELEC,
	ELEC_METER_R_GALARM_ELEC,
	ELEC_METER_R_ALARM_ELEC,
	ELEC_METER_R_HOARD_ELEC,
	ELEC_METER_R_CREDIT_ELEC,		//12
	ELEC_METER_R_POWER,
	ELEC_METER_R_CURNT_POWER,
	ELEC_METER_R_POWER_WAY,
	ELEC_METER_R_STATUS,
	ELEC_METER_W_REMOTE_CTRL,
	ELEC_METER_W_RECHARGE,
	ELEC_METER_R_DATA,
	ELEC_METER_W_CLEAR,
	ELEC_METER_R_VOLTAGE,
	ELEC_METER_R_CURRENT,
	ELEC_METER_R_HPOWRS,
	ELEC_METER_R_NPOWRS,
	ELEC_METER_R_CPOWRS,
	ELEC_METER_R_VPOWRS,
	ELEC_METER_R_HZ,
  ELEC_METER_R_WARN,
};

/*********************************************************************
 * MACROS 
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
 
    
/*********************************************************************
 * FUNCTIONS
 */
 
int meter_SerialMsgProcess(epInfo_t *epInfo,hostCmd *cmd);
int meter_read(epInfo_t *epInfo,uint8_t cmds);
int meter_write(epInfo_t *epInfo,uint8_t cmds,uint8_t *data,uint8_t len);
void meter_serverProcess(uint16_t nCMD, epInfo_t *epInfo, hostCmd *cmd);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __ELECTRICITY_METER_H__ */

