/**************************************************************************************************
 * Filename:       zbSocPrivate.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:   zigbee芯片相关信息的私有定义.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    新建文件，添加了对节点的定义
 *
 */

#ifndef ZB_SOC_PRIVATE_H
#define ZB_SOC_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

// General Clusters
#define ZCL_CLUSTER_ID_GEN_BASIC                             0x0000
#define ZCL_CLUSTER_ID_GEN_POWER_CFG                         0x0001
#define ZCL_CLUSTER_ID_GEN_DEVICE_TEMP_CONFIG                0x0002
#define ZCL_CLUSTER_ID_GEN_IDENTIFY                          0x0003
#define ZCL_CLUSTER_ID_GEN_GROUPS                            0x0004
#define ZCL_CLUSTER_ID_GEN_SCENES                            0x0005
#define ZCL_CLUSTER_ID_GEN_ON_OFF                            0x0006
#define ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG              0x0007
#define ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL                     0x0008
#define ZCL_CLUSTER_ID_GEN_ALARMS                            0x0009
#define ZCL_CLUSTER_ID_GEN_TIME                              0x000A
#define ZCL_CLUSTER_ID_GEN_LOCATION                          0x000B
#define ZCL_CLUSTER_ID_GEN_ANALOG_INPUT_BASIC                0x000C
#define ZCL_CLUSTER_ID_GEN_ANALOG_OUTPUT_BASIC               0x000D
#define ZCL_CLUSTER_ID_GEN_ANALOG_VALUE_BASIC                0x000E
#define ZCL_CLUSTER_ID_GEN_BINARY_INPUT_BASIC                0x000F
#define ZCL_CLUSTER_ID_GEN_BINARY_OUTPUT_BASIC               0x0010
#define ZCL_CLUSTER_ID_GEN_BINARY_VALUE_BASIC                0x0011
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_INPUT_BASIC            0x0012
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_OUTPUT_BASIC           0x0013
#define ZCL_CLUSTER_ID_GEN_MULTISTATE_VALUE_BASIC            0x0014
#define ZCL_CLUSTER_ID_GEN_COMMISSIONING                     0x0015
#define ZCL_CLUSTER_ID_GEN_PARTITION                         0x0016

#define ZCL_CLUSTER_ID_OTA                                   0x0019

#define ZCL_CLUSTER_ID_GEN_POWER_PROFILE                     0x001A
#define ZCL_CLUSTER_ID_GEN_APPLIANCE_CONTROL                 0x001B

#define ZCL_CLUSTER_ID_GEN_POLL_CONTROL                      0x0020

#define ZCL_CLUSTER_ID_GREEN_POWER_PROXY                     0x0021
//Add By zxb，用于上报节点心跳包
#define ZCL_CLUSTER_ID_GEN_HEARTBEAT_REPORT                	 0x0022

// Closures Clusters
#define ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG                 0x0100
#define ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK                    0x0101
#define ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING              0x0102
#define ZCL_CLUSTER_ID_CLOSURES_POWER_SWITCH                 0x0103//取电开关

// HVAC Clusters
#define ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL              0x0200
#define ZCL_CLUSTER_ID_HVAC_THERMOSTAT                       0x0201
#define ZCL_CLUSTER_ID_HVAC_FAN_CONTROL                      0x0202
#define ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL         0x0203
#define ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG            0x0204

// Lighting Clusters
#define ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL                0x0300
#define ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG               0x0301

// Measurement and Sensing Clusters
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT            0x0400
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG   0x0401
#define ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT            0x0402
#define ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT               0x0403
#define ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT                   0x0404
#define ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY                  0x0405
#define ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING                  0x0406
#define ZCL_CLUSTER_ID_MS_SIMPLE_METER 						 0x0407//取电开关电量查询

// Security and Safety (SS) Clusters
#define ZCL_CLUSTER_ID_SS_IAS_ZONE                           0x0500
#define ZCL_CLUSTER_ID_SS_IAS_ACE                            0x0501
#define ZCL_CLUSTER_ID_SS_IAS_WD                             0x0502

// Protocol Interfaces
#define ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL                     0x0600
#define ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL             0x0601
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_REG            0x0602
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_EXT            0x0603
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_REG           0x0604
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_EXT           0x0605
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_REG            0x0606
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_EXT            0x0607
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_REG            0x0608
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_EXT            0x0609
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_REG           0x060A
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_EXT           0x060B
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_REG            0x060C
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_EXT            0x060D
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_REG        0x060E
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_EXT        0x060F
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_REG       0x0610
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_EXT       0x0611
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_REG        0x0612
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_EXT        0x0613
#define ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL              0x0614

// Advanced Metering Initiative (SE) Clusters
#define ZCL_CLUSTER_ID_SE_PRICING                            0x0700
#define ZCL_CLUSTER_ID_SE_LOAD_CONTROL                       0x0701
#define ZCL_CLUSTER_ID_SE_SIMPLE_METERING                    0x0702
#define ZCL_CLUSTER_ID_SE_MESSAGE                            0x0703
#define ZCL_CLUSTER_ID_SE_SE_TUNNELING                       0x0704
#define ZCL_CLUSTER_ID_SE_PREPAYMENT                         0x0705
#ifdef SE_UK_EXT
#define ZCL_CLUSTER_ID_SE_TOU_CALENDAR                       0x0706
#define ZCL_CLUSTER_ID_SE_DEVICE_MGMT                        0x0707
#endif  // SE_UK_EXT

#define ZCL_CLUSTER_ID_GEN_KEY_ESTABLISHMENT                 0x0800

#define ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION           0x0B00
#define ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION               0x0B01
#define ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS            0x0B02
#define ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS               0x0B03
#define ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT             0x0B04
#define ZCL_CLUSTER_ID_HA_DIAGNOSTIC                         0x0B05

// Light Link cluster
#define ZCL_CLUSTER_ID_LIGHT_LINK                           0x1000

/*** Frame Control bit mask ***/
#define ZCL_FRAME_CONTROL_TYPE                          0x03
#define ZCL_FRAME_CONTROL_MANU_SPECIFIC                 0x04
#define ZCL_FRAME_CONTROL_DIRECTION                     0x08
#define ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RSP           0x10

/*** Frame Types ***/
#define ZCL_FRAME_TYPE_PROFILE_CMD                      0x00
#define ZCL_FRAME_TYPE_SPECIFIC_CMD                     0x01

/*** Frame Client/Server Directions ***/
#define ZCL_FRAME_CLIENT_SERVER_DIR                     0x00
#define ZCL_FRAME_SERVER_CLIENT_DIR                     0x01

/*** Chipcon Manufacturer Code ***/
#define CC_MANUFACTURER_CODE                            0x1001

/*** Foundation Command IDs ***/
#define ZCL_CMD_READ                                    0x00
#define ZCL_CMD_READ_RSP                                0x01
#define ZCL_CMD_WRITE                                   0x02
#define ZCL_CMD_WRITE_UNDIVIDED                         0x03
#define ZCL_CMD_WRITE_RSP                               0x04
#define ZCL_CMD_WRITE_NO_RSP                            0x05
#define ZCL_CMD_CONFIG_REPORT                           0x06
#define ZCL_CMD_CONFIG_REPORT_RSP                       0x07
#define ZCL_CMD_READ_REPORT_CFG                         0x08
#define ZCL_CMD_READ_REPORT_CFG_RSP                     0x09
#define ZCL_CMD_REPORT                                  0x0a
#define ZCL_CMD_DEFAULT_RSP                             0x0b
#define ZCL_CMD_DISCOVER_ATTRS                          0x0c
#define ZCL_CMD_DISCOVER_ATTRS_RSP                      0x0d
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED                  0x11
#define ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP              0x12
#define ZCL_CMD_DISCOVER_CMDS_GEN                       0x13
#define ZCL_CMD_DISCOVER_CMDS_GEN_RSP                   0x14
#define ZCL_CMD_DISCOVER_ATTRS_EXT                      0x15
#define ZCL_CMD_DISCOVER_ATTRS_EXT_RSP                  0x16

#define ZCL_CMD_MAX           							ZCL_CMD_DISCOVER_ATTRS_EXT_RSP//ZCL_CMD_ONLINE_HEARTBEAT_REPORT

// define reporting constant
#define ZCL_REPORTING_OFF     							0xFFFF  // turn off reporting (maxReportInt)

// define command direction flag masks
#define CMD_DIR_SERVER_GENERATED          				0x01
#define CMD_DIR_CLIENT_GENERATED          				0x02
#define CMD_DIR_SERVER_RECEIVED           				0x04
#define CMD_DIR_CLIENT_RECEIVED           				0x08

/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10
#define ZCL_DATATYPE_BITMAP8                            0x18
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30
#define ZCL_DATATYPE_ENUM16                             0x31
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1
#define ZCL_DATATYPE_UTC                                0xe2
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8
#define ZCL_DATATYPE_ATTR_ID                            0xe9
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATATYPE_UNKNOWN                            0xff

/*** Error Status Codes ***/
#define ZCL_STATUS_SUCCESS                              0x00
#define ZCL_STATUS_FAILURE                              0x01
// 0x02-0x7D are reserved.
#define ZCL_STATUS_NOT_AUTHORIZED                       0x7E
#define ZCL_STATUS_MALFORMED_COMMAND                    0x80
#define ZCL_STATUS_UNSUP_CLUSTER_COMMAND                0x81
#define ZCL_STATUS_UNSUP_GENERAL_COMMAND                0x82
#define ZCL_STATUS_UNSUP_MANU_CLUSTER_COMMAND           0x83
#define ZCL_STATUS_UNSUP_MANU_GENERAL_COMMAND           0x84
#define ZCL_STATUS_INVALID_FIELD                        0x85
#define ZCL_STATUS_UNSUPPORTED_ATTRIBUTE                0x86
#define ZCL_STATUS_INVALID_VALUE                        0x87
#define ZCL_STATUS_READ_ONLY                            0x88
#define ZCL_STATUS_INSUFFICIENT_SPACE                   0x89
#define ZCL_STATUS_DUPLICATE_EXISTS                     0x8a
#define ZCL_STATUS_NOT_FOUND                            0x8b
#define ZCL_STATUS_UNREPORTABLE_ATTRIBUTE               0x8c
#define ZCL_STATUS_INVALID_DATA_TYPE                    0x8d
#define ZCL_STATUS_INVALID_SELECTOR                     0x8e
#define ZCL_STATUS_WRITE_ONLY                           0x8f
#define ZCL_STATUS_INCONSISTENT_STARTUP_STATE           0x90
#define ZCL_STATUS_DEFINED_OUT_OF_BAND                  0x91
#define ZCL_STATUS_INCONSISTENT                         0x92
#define ZCL_STATUS_ACTION_DENIED                        0x93
#define ZCL_STATUS_TIMEOUT                              0x94
#define ZCL_STATUS_ABORT                                0x95
#define ZCL_STATUS_INVALID_IMAGE                        0x96
#define ZCL_STATUS_WAIT_FOR_DATA                        0x97
#define ZCL_STATUS_NO_IMAGE_AVAILABLE                   0x98
#define ZCL_STATUS_REQUIRE_MORE_IMAGE                   0x99

// 0xbd-bf are reserved.
#define ZCL_STATUS_HARDWARE_FAILURE                     0xc0
#define ZCL_STATUS_SOFTWARE_FAILURE                     0xc1
#define ZCL_STATUS_CALIBRATION_ERROR                    0xc2
// 0xc3-0xff are reserved.
#define ZCL_STATUS_CMD_HAS_RSP                          0xFF // Non-standard status (used for Default Rsp)


/********************************/
/*** Basic Cluster Attributes ***/
/********************************/
// Basic Device Information
#define ATTRID_BASIC_ZCL_VERSION                          0x0000
#define ATTRID_BASIC_APPL_VERSION                         0x0001
#define ATTRID_BASIC_STACK_VERSION                        0x0002
#define ATTRID_BASIC_HW_VERSION                           0x0003
#define ATTRID_BASIC_MANUFACTURER_NAME                    0x0004
#define ATTRID_BASIC_MODEL_ID                             0x0005
#define ATTRID_BASIC_DATE_CODE                            0x0006
#define ATTRID_BASIC_POWER_SOURCE                         0x0007
// Basic Device Settings
#define ATTRID_BASIC_LOCATION_DESC                        0x0010
#define ATTRID_BASIC_PHYSICAL_ENV                         0x0011
#define ATTRID_BASIC_DEVICE_ENABLED                       0x0012
#define ATTRID_BASIC_ALARM_MASK                           0x0013

#define ATTRID_BASIC_LOCK_UNLOCK                          0x0015
#define ATTRID_BASIC_DEFAULT_IO_CFG                       0x0016
#define ATTRID_BASIC_GET_STATE                            0x0017 //获取设备启用/禁用状态
#define ATTRID_BASIC_POWER_VALUE                          0x0018 //总用电量
#define ATTRID_BASIC_CURRENT_POWER_VALUE                  0x0019 //当前用电量

#define ATTRID_BASIC_IR_OPER                           	  0x0020


#define ATTRID_BASIC_HEARTBEAT                            0x0023//心跳包操作
#define ATTRID_BASIC_POWER_SWITCH                   	  0x0024//取电开关操作
#define ATTRID_MS_ELECTRICAL_METER_MEASURED_VALUE		  0x0024//取电开关操作
#define ATTRID_BASIC_LX_MASTER                            0x0025
#define ATTRID_BASIC_IRSENSOR_ONOFF						  0x0026//红外人体感应启动/禁用操作
#define ATTRID_BASIC_MOTOR_SPEED						  0x0027//门市转数时间设置
#define ATTRID_BASIC_MOTOR_SPEED_CHECK					  0x0028//门市转数时间查询

#define ATTRID_BASIC_KG_OPERATION						  0x0029

#define ATTRID_BASIC_KG_DEFAULT_STATUS					  0x0030//开关默认状态
#define ATTRID_BASIC_TEMP_HUM_VALUE						  0x0031 //获取温湿度
#define ATTRID_BASIC_DOORLOCK_UART_MSG					  0x0032 //力维门锁
#define ATTRID_BASIC_UART_MSG					          0x0032 //串口透传

#define ATTRID_BASIC_UART_MSG							  0x0032


#define ATTRID_MOVE_LEVEL								  0x0020 //窗帘百分比
#define ATTRID_BASIC_CL_STATUS							  0x0014 //窗帘状态上报

#define ATTRID_LXZK_DEV_SET                             0x0025 
#define ATTRID_LXZK_DEV_GET                             0x0026 


/*****************************************************************************/
/***    Temperature Measurement Cluster Attributes                         ***/
/*****************************************************************************/
// Temperature Measurement Information attributes set
#define ATTRID_MS_TEMPERATURE_MEASURED_VALUE                             0x0000 // M, R, INT16
#define ATTRID_MS_TEMPERATURE_MIN_MEASURED_VALUE                         0x0001 // M, R, INT16
#define ATTRID_MS_TEMPERATURE_MAX_MEASURED_VALUE                         0x0002 // M, R, INT16
#define ATTRID_MS_TEMPERATURE_TOLERANCE                                  0x0003 // O, R, UINT16
#define ATTRID_MS_HUMIDITY_MEASURED_VALUE                                0x0004 // M, R, INT16
#define ATTRID_MS_REPORT_INTERVAL_VALUE                                  0x0005 // M, R, INT16

// Temperature Measurement Settings attributes set
#define ATTRID_MS_TEMPERATURE_MIN_PERCENT_CHANGE                         0x0010
#define ATTRID_MS_TEMPERATURE_MIN_ABSOLUTE_CHANGE                        0x0011

/*****************************************************************************/
/***    Pressure Measurement Cluster Attributes                            ***/
/*****************************************************************************/
// Pressure Measurement Information attribute set
#define ATTRID_MS_PRESSURE_MEASUREMENT_MEASURED_VALUE                    0x0000
#define ATTRID_MS_PRESSURE_MEASUREMENT_MIN_MEASURED_VALUE                0x0001
#define ATTRID_MS_PRESSURE_MEASUREMENT_MAX_MEASURED_VALUE                0x0002
#define ATTRID_MS_PRESSURE_MEASUREMENT_TOLERANCE                         0x0003

// Pressure Measurement Settings attribute set
// #define ATTRID_MS_PRESSURE_MEASUREMENT_MIN_PERCENT_CHANGE                0x0100
// #define ATTRID_MS_PRESSURE_MEASUREMENT_MIN_ABSOLUTE_CHANGE               0x0101

/*****************************************************************************/
/***        Flow Measurement Cluster Attributes                            ***/
/*****************************************************************************/
// Flow Measurement Information attribute set
#define ATTRID_MS_FLOW_MEASUREMENT_MEASURED_VALUE                        0x0000
#define ATTRID_MS_FLOW_MEASUREMENT_MIN_MEASURED_VALUE                    0x0001
#define ATTRID_MS_FLOW_MEASUREMENT_MAX_MEASURED_VALUE                    0x0002
#define ATTRID_MS_FLOW_MEASUREMENT_TOLERANCE                             0x0003

// Flow Measurement Settings attribute set
// #define ATTRID_MS_FLOW_MEASUREMENT_MIN_PERCENT_CHANGE                    0x0100
// #define ATTRID_MS_FLOW_MEASUREMENT_MIN_ABSOLUTE_CHANGE                   0x0101

/*****************************************************************************/
/***        Relative Humidity Cluster Attributes                           ***/
/*****************************************************************************/
// Relative Humidity Information attribute set
#define ATTRID_MS_RELATIVE_HUMIDITY_MEASURED_VALUE                       0x0000
#define ATTRID_MS_RELATIVE_HUMIDITY_MIN_MEASURED_VALUE                   0x0001
#define ATTRID_MS_RELATIVE_HUMIDITY_MAX_MEASURED_VALUE                   0x0002
#define ATTRID_MS_RELATIVE_HUMIDITY_TOLERANCE                            0x0003

/*****************************************************************************/
/***         Occupancy Sensing Cluster Attributes                          ***/
/*****************************************************************************/
// Occupancy Sensor Configuration attribute set
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_OCCUPANCY                     0x0000 // M, R, BITMAP8
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_OCCUPANCY_SENSOR_TYPE         0x0001 // M, R, ENUM8

/*** Occupancy Sensor Type Attribute values ***/
#define MS_OCCUPANCY_SENSOR_TYPE_PIR                                     0x00
#define MS_OCCUPANCY_SENSOR_TYPE_ULTRASONIC                              0x01
#define MS_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC                      0x02

// PIR Configuration attribute set
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_PIR_O_TO_U_DELAY              0x0010 // O, R/W, UINT16
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_PIR_U_TO_O_DELAY              0x0011 // O, R/W, UINT16
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_PIR_U_TO_O_THRESH             0x0012 // O, R/W, UINT8

// Ultrasonic Configuration attribute set
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_ULTRASONIC_O_TO_U_DELAY       0x0020 // O, R/W, UINT16
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_ULTRASONIC_U_TO_O_DELAY       0x0021 // O, R/W, UINT16
#define ATTRID_MS_OCCUPANCY_SENSING_CONFIG_ULTRASONIC_U_TO_O_THRESH      0x0022 // O, R/W, UINT8

/*********************************************/
/*******  Diagnostic Cluster Attributes ******/
/*********************************************/
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI								0x011C  // O, R, UINT8
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI                          	0x011D  // O, R, INT8

/**********************************************/
/*** Power Configuration Cluster Attributes ***/
/**********************************************/
  // Mains Information
#define ATTRID_POWER_CFG_MAINS_VOLTAGE                    0x0000
#define ATTRID_POWER_CFG_MAINS_FREQUENCY                  0x0001

  // Mains Settings
#define ATTRID_POWER_CFG_MAINS_ALARM_MASK                 0x0010
#define ATTRID_POWER_CFG_MAINS_VOLT_MIN_THRES             0x0011
#define ATTRID_POWER_CFG_MAINS_VOLT_MAX_THRES             0x0012
#define ATTRID_POWER_CFG_MAINS_DWELL_TRIP_POINT           0x0013

// Battery Information
#define ATTRID_POWER_CFG_BATTERY_VOLTAGE                  0x0020
#define ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING     0x0021

// Battery Information Default Attribute Value
#define ATTR_DEFAULT_POWER_CFG_BATTERY_PERCENTAGE_REMAINING    0

// Battery Settings
#define ATTRID_POWER_CFG_BAT_MANU                         0x0030
#define ATTRID_POWER_CFG_BAT_SIZE                         0x0031
#define ATTRID_POWER_CFG_BAT_AHR_RATING                   0x0032
#define ATTRID_POWER_CFG_BAT_QUANTITY                     0x0033
#define ATTRID_POWER_CFG_BAT_RATED_VOLTAGE                0x0034
#define ATTRID_POWER_CFG_BAT_ALARM_MASK                   0x0035
#define ATTRID_POWER_CFG_BAT_VOLT_MIN_THRES               0x0036
#define ATTRID_POWER_CFG_BAT_VOLT_THRES_1                 0x0037
#define ATTRID_POWER_CFG_BAT_VOLT_THRES_2                 0x0038
#define ATTRID_POWER_CFG_BAT_VOLT_THRES_3                 0x0039
#define ATTRID_POWER_CFG_BAT_PERCENT_MIN_THRES            0x003A
#define ATTRID_POWER_CFG_BAT_PERCENT_THRES_1              0x003B
#define ATTRID_POWER_CFG_BAT_PERCENT_THRES_2              0x003C
#define ATTRID_POWER_CFG_BAT_PERCENT_THRES_3              0x003D
#define ATTRID_POWER_CFG_BAT_ALARM_STATE                  0x003E

/**********************************************/
/*** Window Covering Cluster Attribute Sets ***/
/**********************************************/
#define ATTRSET_WINDOW_COVERING_INFO                        0x0000
#define ATTRSET_WINDOW_COVERING_SETTINGS                    0x0010

/******************************************/
/*** Window Covering Cluster Attributes ***/
/******************************************/
//Window Covering Information
#define ATTRID_CLOSURES_WINDOW_COVERING_TYPE                ( ATTRSET_WINDOW_COVERING_INFO + 0x0000 )
#define ATTRID_CLOSURES_PHYSICAL_CLOSE_LIMIT_LIFT_CM        ( ATTRSET_WINDOW_COVERING_INFO + 0x0001 )
#define ATTRID_CLOSURES_PHYSICAL_CLOSE_LIMIT_TILT_DDEGREE   ( ATTRSET_WINDOW_COVERING_INFO + 0x0002 )
#define ATTRID_CLOSURES_CURRENT_POSITION_LIFT_CM            ( ATTRSET_WINDOW_COVERING_INFO + 0x0003 )
#define ATTRID_CLOSURES_CURRENT_POSITION_TILT_DDEGREE       ( ATTRSET_WINDOW_COVERING_INFO + 0x0004 )
#define ATTRID_CLOSURES_NUM_OF_ACTUATION_LIFT               ( ATTRSET_WINDOW_COVERING_INFO + 0x0005 )
#define ATTRID_CLOSURES_NUM_OF_ACTUATION_TILT               ( ATTRSET_WINDOW_COVERING_INFO + 0x0006 )
#define ATTRID_CLOSURES_CONFIG_STATUS                       ( ATTRSET_WINDOW_COVERING_INFO + 0x0007 )
#define ATTRID_CLOSURES_CURRENT_POSITION_LIFT_PERCENTAGE    ( ATTRSET_WINDOW_COVERING_INFO + 0x0008 )
#define ATTRID_CLOSURES_CURRENT_POSITION_TILT_PERCENTAGE    ( ATTRSET_WINDOW_COVERING_INFO + 0x0009 )

//Window Covering Setting
#define ATTRID_CLOSURES_INSTALLED_OPEN_LIMIT_LIFT_CM        ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0000 )
#define ATTRID_CLOSURES_INSTALLED_CLOSED_LIMIT_LIFT_CM      ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0001 )
#define ATTRID_CLOSURES_INSTALLED_OPEN_LIMIT_TILT_DDEGREE   ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0002 )
#define ATTRID_CLOSURES_INSTALLED_CLOSED_LIMIT_TILT_DDEGREE ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0003 )
#define ATTRID_CLOSURES_VELOCITY_LIFT                       ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0004 )
#define ATTRID_CLOSURES_ACCELERATION_TIME_LIFT              ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0005 )
#define ATTRID_CLOSURES_DECELERATION_TIME_LIFT              ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0006 )
#define ATTRID_CLOSURES_WINDOW_COVERING_MODE                ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0007 )
#define ATTRID_CLOSURES_INTERMEDIATE_SETPOINTS_LIFT         ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0008 )
#define ATTRID_CLOSURES_INTERMEDIATE_SETPOINTS_TILT         ( ATTRSET_WINDOW_COVERING_SETTINGS + 0x0009 )
//fxkj add 
#define ATTRID_CLOSURES_WINDOW_COVERING_STATUS				( ATTRSET_WINDOW_COVERING_SETTINGS + 0x000A )

/*** Window Covering Type Attribute types ***/
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE                       0x00
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_2_MOTOR               0x01
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR              0x02
#define CLOSURES_WINDOW_COVERING_TYPE_ROLLERSHADE_EXTERIOR_2_MOTOR      0x03
#define CLOSURES_WINDOW_COVERING_TYPE_DRAPERY                           0x04
#define CLOSURES_WINDOW_COVERING_TYPE_AWNING                            0x05
#define CLOSURES_WINDOW_COVERING_TYPE_SHUTTER                           0x06
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_TILT_ONLY              0x07
#define CLOSURES_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT          0x08
#define CLOSURES_WINDOW_COVERING_TYPE_PROJECTOR_SCREEN                  0x09

/****************************************/
/*** Window Covering Cluster Commands ***/
/****************************************/
#define COMMAND_CLOSURES_UP_OPEN                            ( 0x00 )
#define COMMAND_CLOSURES_DOWN_CLOSE                         ( 0x01 )
#define COMMAND_CLOSURES_STOP                               ( 0x02 )
#define COMMAND_CLOSURES_GO_TO_LIFT_VALUE                   ( 0x04 )
#define COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE              ( 0x05 )
#define COMMAND_CLOSURES_GO_TO_TILT_VALUE                   ( 0x07 )
#define COMMAND_CLOSURES_GO_TO_TILT_PERCENTAGE              ( 0x08 )

#define ZCL_WC_GOTOVALUEREQ_PAYLOADLEN                      ( 2 )
#define ZCL_WC_GOTOPERCENTAGEREQ_PAYLOADLEN                 ( 1 )


/*****************************************************************************/
/***         中央空调控制器 Cluster Attributes                          ***/
/*****************************************************************************/
#define ATTRID_BASIC_KTWKQ_OPER											 0x0016


/*****************************************************************************/
/***         智能插座 Cluster Attributes                          ***/
/*****************************************************************************/
//	ATTRID_BASIC_CARACITY_VALUE(0x0017) 	功率
//	ATTRID_BASIC_POWER_VALUE(0x0018)		电量
//	ATTRID_BASIC_VOLTAGE_VALUE(0x0019)		电压
//	ATTRID_BASIC_CURRENT_VALUE(0x0021)		电流

#define 	ATTRID_BASIC_CARACITY_VALUE 	0x0017
//#define 	ATTRID_BASIC_POWER_VALUE		0x0018
#define 	ATTRID_BASIC_VOLTAGE_VALUE		0x0019
#define 	ATTRID_BASIC_CURRENT_VALUE		0x0021

//返回的状态值
#define ZSuccess                    0x00
#define ZFailure                    0x01
#define ZInvalidParameter           0x02

// ZStack status values must start at 0x10, after the generic status values (defined in comdef.h)
#define ZMemError                   0x10
#define ZBufferFull                 0x11
#define ZUnsupportedMode            0x12
#define ZMacMemError                0x13

#define ZSapiInProgress             0x20
#define ZSapiTimeout                0x21
#define ZSapiInit                   0x22

#define ZNotAuthorized              0x7E

#define ZMalformedCmd               0x80
#define ZUnsupClusterCmd            0x81

// OTA Status values
#define ZOtaAbort                   0x95
#define ZOtaImageInvalid            0x96
#define ZOtaWaitForData             0x97
#define ZOtaNoImageAvailable        0x98
#define ZOtaRequireMoreImage        0x99

// APS status values
#define ZApsFail                    0xb1
#define ZApsTableFull               0xb2
#define ZApsIllegalRequest          0xb3
#define ZApsInvalidBinding          0xb4
#define ZApsUnsupportedAttrib       0xb5
#define ZApsNotSupported            0xb6
#define ZApsNoAck                   0xb7
#define ZApsDuplicateEntry          0xb8
#define ZApsNoBoundDevice           0xb9
#define ZApsNotAllowed              0xba
#define ZApsNotAuthenticated        0xbb

// Security status values
#define ZSecNoKey                   0xa1
#define ZSecOldFrmCount             0xa2
#define ZSecMaxFrmCount             0xa3
#define ZSecCcmFail                 0xa4

// NWK status values
#define ZNwkInvalidParam            0xc1
#define ZNwkInvalidRequest          0xc2
#define ZNwkNotPermitted            0xc3
#define ZNwkStartupFailure          0xc4
#define ZNwkAlreadyPresent          0xc5
#define ZNwkSyncFailure             0xc6
#define ZNwkTableFull               0xc7
#define ZNwkUnknownDevice           0xc8
#define ZNwkUnsupportedAttribute    0xc9
#define ZNwkNoNetworks              0xca
#define ZNwkLeaveUnconfirmed        0xcb
#define ZNwkNoAck                   0xcc  // not in spec
#define ZNwkNoRoute                 0xcd

// MAC status values
#define ZMacSuccess                 0x00
#define ZMacBeaconLoss              0xe0
#define ZMacChannelAccessFailure    0xe1
#define ZMacDenied                  0xe2
#define ZMacDisableTrxFailure       0xe3
#define ZMacFailedSecurityCheck     0xe4
#define ZMacFrameTooLong            0xe5
#define ZMacInvalidGTS              0xe6
#define ZMacInvalidHandle           0xe7
#define ZMacInvalidParameter        0xe8
#define ZMacNoACK                   0xe9
#define ZMacNoBeacon                0xea
#define ZMacNoData                  0xeb
#define ZMacNoShortAddr             0xec
#define ZMacOutOfCap                0xed
#define ZMacPANIDConflict           0xee
#define ZMacRealignment             0xef
#define ZMacTransactionExpired      0xf0
#define ZMacTransactionOverFlow     0xf1
#define ZMacTxActive                0xf2
#define ZMacUnAvailableKey          0xf3
#define ZMacUnsupportedAttribute    0xf4
#define ZMacUnsupported             0xf5
#define ZMacSrcMatchInvalidIndex    0xff


/*************************自定义******************************************************/


/*************************如下不规范*************************************************************/

#define ZLL_MT_APP_RPC_CMD_TOUCHLINK          	0x01
#define ZLL_MT_APP_RPC_CMD_RESET_TO_FN        	0x02
#define ZLL_MT_APP_RPC_CMD_CH_CHANNEL         	0x03
#define ZLL_MT_APP_RPC_CMD_JOIN_HA            	0x04
#define ZLL_MT_APP_RPC_CMD_PERMIT_JOIN        	0x05
#define ZLL_MT_APP_RPC_CMD_SEND_RESET_TO_FN   	0x06
#define ZLL_MT_APP_RPC_CMD_START_DISTRIB_NWK  	0x07

#define MT_APP_RSP                              0x80
#define MT_APP_ZLL_TL_IND                  		0x81
#define MT_APP_COORD_VERSION_RSP				0x82
#define MT_APP_DEVICE_OFF_LINE_RSP				0x83
#define MT_APP_SERIAL_RECEIVED_RSP				0x00


#define MT_APP_ZLL_NEW_DEV_IND       			0x82

#define MT_DEBUG_MSG                         	0x80

#define COMMAND_LIGHTING_MOVE_TO_HUE            0x00
#define COMMAND_LIGHTING_MOVE_TO_SATURATION 	0x03
#define COMMAND_LEVEL_MOVE_TO_LEVEL             0x00
#define COMMAND_IDENTIFY                    	0x00
#define COMMAND_IDENTIFY_TRIGGER_EFFECT     	0x40


//ZDO

#define ZCL_CLUSTER_ID_NO_USE                  	0xffff


/*******************************/
/*** Generic Cluster ATTR's  ***/
/*******************************/
#define ATTRID_BASIC_MODEL_ID                             0x0005
#define ATTRID_ON_OFF                                     0x0000
#define ATTRID_LEVEL_CURRENT_LEVEL                        0x0000

/*******************************/
/*** Master Control Cluster ATTR's  ***/
/*******************************/

#define ATTRID_MASTER_CONTROL_READ_VALUE				  0x0006

/*******************************/
/*** Lighting Cluster ATTR's  ***/
/*******************************/
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_HUE         0x0000
#define ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION  0x0001

/*******************************/
/*** Scenes Cluster Commands ***/
/*******************************/
#define COMMAND_SCENE_STORE                               0x04
#define COMMAND_SCENE_RECALL                              0x05
#define COMMAND_SCENE_GET_REMOVE                          0x02

/*******************************/
/*** Groups Cluster Commands ***/
/*******************************/
#define COMMAND_GROUP_ADD                                 0x00
#define COMMAND_GROUP_GET_REMOVE                          0x03


/* The 3 MSB's of the 1st command field byte are for command type. */
#define MT_RPC_CMD_TYPE_MASK  0xE0

/* The 5 LSB's of the 1st command field byte are for the subsystem. */
#define MT_RPC_SUBSYSTEM_MASK 0x1F



/*******************************/
/*** Bootloader Commands ***/
/*******************************/
#define SB_FORCE_BOOT               0xF8
#define SB_FORCE_RUN               (SB_FORCE_BOOT ^ 0xFF)
#define SB_FORCE_BOOT_1             0x10
#define SB_FORCE_RUN_1             (SB_FORCE_BOOT_1 ^ 0xFF)


// CapabilityFlags Bitmap values
#define CAPINFO_ALTPANCOORD           0x01
#define CAPINFO_DEVICETYPE_FFD        0x02
#define CAPINFO_DEVICETYPE_RFD        0x00
#define CAPINFO_POWER_AC              0x04
#define CAPINFO_RCVR_ON_IDLE          0x08
#define CAPINFO_SECURITY_CAPABLE      0x40
#define CAPINFO_ALLOC_ADDR            0x80


typedef enum
{
    MT_RPC_CMD_POLL = 0x00,
    MT_RPC_CMD_SREQ = 0x20,
    MT_RPC_CMD_AREQ = 0x40,
    MT_RPC_CMD_SRSP = 0x60,
    MT_RPC_CMD_RES4 = 0x80,
    MT_RPC_CMD_RES5 = 0xA0,
    MT_RPC_CMD_RES6 = 0xC0,
    MT_RPC_CMD_RES7 = 0xE0
} mtRpcCmdType_t;

typedef enum
{
    MT_RPC_SYS_RES0, /* Reserved. */
    MT_RPC_SYS_SYS,
    MT_RPC_SYS_MAC,
    MT_RPC_SYS_NWK,
    MT_RPC_SYS_AF,
    MT_RPC_SYS_ZDO,
    MT_RPC_SYS_SAPI, /* Simple API. */
    MT_RPC_SYS_UTIL,
    MT_RPC_SYS_DBG,
    MT_RPC_SYS_APP,
    MT_RPC_SYS_OTA,
    MT_RPC_SYS_ZNP,
    MT_RPC_SYS_SPARE_12,
    MT_RPC_SYS_UBL = 13, // 13 to be compatible with existing RemoTI.
    MT_RPC_SYS_MAX // Maximum value, must be last (so 14-32 available, not yet assigned).
} mtRpcSysType_t;



/* SREQ/SRSP: */
#define MT_APP_MSG                           		0x00
#define MT_APP_USER_TEST                			0x01

#define ZDO_MGMT_LEAVE_REQ							0x34

/*****App *****/
#define APP_ZCL_CMD_REVERT_FACTORY_SETTING   		0x01
#define APP_ZCL_CMD_OPEN_NETWORK     				0x02
#define APP_ZCL_CMD_QUERY_DEVICEID     				0x03
#define APP_ZCL_CMD_COOR_SYSTEM_RESET     			0x05

#define APP_ZCL_CMD_BASIC_REVERT_FACTORY_DEFAULT 	0x00


//应用层端口号
#define MT_SOC_APP_ENDPOINT              			0x08


//NetWork Address
#define MT_NWKADDR_BROADCAST_ALL_ROUTER     		0xFFFC//the special broadcast address that denotes all routers and coordinator


/***************************Attribute ID***********************************/
//温湿度节点
#define     ZB_ATTID_TEMPERATURE          			0x0000
#define     ZB_ATTID_HUMIDITY                 		0x0004
#define     ZB_ATTID_CONFIG_REPORT       			0x0005
//插座节点
#define     ZB_ATTID_PLUG_ONOFF             		0x0000
#define     ZB_ATTID_POWER_VAL              		0x0019 //电量值
#define     ZB_ATTRID_POWER_SWITCH     				0x0024 //取电开关
#define     ZB_ATTID_OTHERS

//#define 	ZB_ATTID_MASTER_CONTROL					0x0025

/******************************************************************************
 * Types			设备类型定义DeviceID
 *****************************************************************************/
//开关设备
#define     ZB_DEV_ONOFF_PLUG          					0x0009   		//三口插座
#define     ZB_DEV_WIN_CURTAIN        					0x0002  		//窗帘	
#define     ZB_DEV_MINI_SWITCH          				0x0006   		//随意贴开关
#define     ZB_DEV_IR_SENSOR            				0x0107   		//红外热体感应 暂时废弃
#define     ZB_DEV_SMOKE_SENSOR            				0x0108   		//烟雾感应 暂时废弃
#define     ZB_DEV_MAGNETOMETER_SENSOR    				0x0109   		//门磁感应 暂时废弃
#define     ZB_DEV_GAS_SENSOR    						0x010a  		//燃气感应
#define     ZB_DEV_ONOFF_SWITCH          				0x0100  		//墙面开关
#define     ZB_DEV_POWER_SWITCH     					0x010c      	//取电开关操作
//最新门磁ID
#define     ZB_DEV_DOOR_SENSOR     						0x010d      	//门磁感应
//Infrared body sensor 红外人体感应
#define 	ZB_DEV_INFRARED_BODY_SENSOR					0X010e      	//红外人体感应
#define 	ZB_DEV_CENTRAL_AIR							0X010F			//中央空调控制面板
#define     ZB_DEV_IRC_LEARN_CTRL     					0x0020      	//红外学习控制器
#define     ZB_DEV_ENVIRONMENT_CTRL     				0x0021      	//环境监测器  暂时没有设备
#define     ZB_DEV_TEMP_HUM              				0x0302      	//温湿度
#define     ZB_DEV_TEMP_HUM_LIGHT   					0x0303
#define 	ZB_DEV_METER_ENERGY_DEVICE					0x0501			//智能电表
#define 	ZB_DEV_MASTER_CONTROL						0x01ff			//主控设备
#define     ZB_DEV_WATER_METER                  		0x0111      	//水表
//DoorLock Device ID
#define 	ZB_DEV_LEVEL_DOORLOCK						0x010a			//智能门锁		
#define   	ZB_DEV_ONOFF_DOORLOCK             			0x010B			//(智能门锁)
#define		ZCL_HA_DEVICEID_FXZB_DOORLOCK_YAOTAI		0x0110			//遥泰门锁
#define 	ZCL_HA_DEVICEID_FXZB_DOORLOCK_YGS			0X0112
#define		ZCL_HA_DEVICEID_FXZB_DOORLOCK_JIULIAN		0x0113

//红外人体感应
#define 	ZCL_HA_DEVICEID_IAS_ZONE                	0x0402			//海曼人体感应

// Closures Device IDs
#define ZCL_HA_DEVICEID_SHADE                                   0x0200
#define ZCL_HA_DEVICEID_SHADE_CONTROLLER                        0x0201
//飞雪内置窗帘控制器
#define ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE                  0x0202
#define ZCL_HA_DEVICEID_WINDOW_COVERING_CONTROLLER              0x0203
//飞雪外置窗帘控制器
#define ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE_FXKJ				0x02FF


#ifdef __cplusplus
}
#endif

#endif //ZB_SOC_PRIVATE_H
