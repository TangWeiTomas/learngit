/***********************************************************************************
 * 文 件 名   : indLight.h
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : Led 状态指示灯
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#ifndef __IND_LIGHT_H__
#define __IND_LIGHT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
	LIGHT_OFF=0X00,
	LIGHT_ON = 0x01
}IndLight_Type_t;

enum
{
	SYS_LED = 0,
	CONNECT_LED,
};

enum 
{
	LED_ON = 0,
	LED_OFF,
	LED_BLINK
};
/*********************************************************************
 * VARIABLES
 */
 
    
/*********************************************************************
 * FUNCTIONS
 */
 
int8_t  led_Serviceindicator(IndLight_Type_t status);
int8_t  led_Serviceindicator_Blink(uint64_t second);
int8_t	led_Serviceindicator_Blink_Off(void);
uint8_t led_SetSysLedStatus(uint8_t led,uint8_t status );

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __ENCRYPT_H__ */
