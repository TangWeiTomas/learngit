/*******************************************************************
    Filename:   errorCode.h
    Author:     zxb
    Timer:      2014.1.4
    Describe:       本文件用于将数据包解析为具体的指令
    Version 1.01 :

*******************************************************************/
#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#define YY_STATUS_SUCCESS                       0x00
#define YY_STATUS_FAIL                          0x01
#define YY_STATUS_UNREGISTER                  	0x02
#define YY_STATUS_ILLEGAL_PARAM     			0x03
#define YY_STATUS_ILLEGAL_TASKID          		0x04      //非法的任务ID
#define YY_STATUS_OUTLINE                      	0x05      //非法的任务ID

#define YY_STATUS_NODE_NO_EXIST     			0x10


#define YY_STATUS_UPGRADE_SUCCESS				0x00
#define YY_STATUS_DEGRADE_SUCCESS				0x01
#define YY_STATUS_UPGRADE_FAIL					0x02
#define YY_STATUS_SAME_VERSION					0x03
#define YY_STATUS_UP_PROGRESS					0x04
#define YY_STATUS_DOWNLOAD_PROGRESS				0x00
#define YY_STATUS_FLASH_PROGRESS				0x01



#endif

