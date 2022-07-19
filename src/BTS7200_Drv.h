/**
 * @copyright Copyright (c) 2020 - 2022, Sankuai
 * All rights reserved.
 * @file BTS7200_Drv.h
 * @brief BTS7200驱动头文件,包含对外部所需数据类型的定义,对外的函数定义,
 * @details 需要对Cfg文件进行配置,选择负载类型
 * 在if文件中,对函数接口进行匹配,需要IO和ADC支持
 * @author sunkailiang，sunkailiang@meituan.com
 * @date 2022-7-19
 */

#ifndef BTS7200_DRV_H
#define BTS7200_DRV_H

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/

/*******************************************************************************
**                      Imported Compiler Switch Check                        **
*******************************************************************************/

/*******************************************************************************
**                      Private Macro Definitions                             **
*******************************************************************************/

/*******************************************************************************
**                      Private Type Definitions                              **
*******************************************************************************/

/*******************************************************************************
**                      Private Function Declarations                         **
*******************************************************************************/

/*******************************************************************************
**                      Global Constant Definitions                           **
*******************************************************************************/

/*******************************************************************************
**                      Global Variable Definitions                           **
*******************************************************************************/

/*******************************************************************************
**                      Private Constant Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Private Variable Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Functon Definitions                            **
*******************************************************************************/

/*!
 * @enum BTS7200_ReturnType
 * @brief 函数返回值枚举
 */
enum BTS7200_ReturnType {
  BTS7200_RETURN_NORMAL = 0x00,
  BTS7200_RETURN_UNINIT = 0x01,
  BTS7200_RETURN_ENTER_ERROR = 0x02,
};
/*!
 * @enum BTS7200_ChipType
 * @brief 芯片编号枚举
 */
enum BTS7200_ChipType {
  BTS7200_CHIP_U4100 = 0x00,
  BTS7200_CHIP_U4101 = 0x01,
};
/*!
 * @enum BTS7200_ChannelType
 * @brief 芯片通道枚举
 * @details 由于DOH的检测只有通道1可以进行,所以通道的选择非常重要
 */
enum BTS7200_ChannelType {
  BTS7200_CHANNEL_OUT1 = 0x00,
  BTS7200_CHANNEL_OUT2 = 0x01,
};
/*!
 * @enum BTS7200_LevelType
 * @brief 电平等级枚举
 * @details 输出等级,CTRL引脚输出等级,DSEL选择等级都使用该枚举
 */
enum BTS7200_LevelType {
  BTS7200_CHANNEL_LOW = 0x00,
  BTS7200_CHANNEL_HIGH = 0x01,
};
/*!
 * @enum BTS7200_InitType
 * @brief 初始化状态枚举
 * @details 位于信息结构体,表示模块整体的初始化状态
 */
enum BTS7200_InitType {
  BTS7200_UNINIT = 0x00,
  BTS7200_INIT = 0x01,
};
/*!
 * @enum BTS7200_DiagnosticResultType
 * @brief 诊断信息枚举
 */
enum BTS7200_DiagnosticResultType {
  BTS7200_NORMAL = 0x00,
  BTS7200_OPEN_LOAD = 0x01,
  BTS7200_SHORT_CIRCUITED_VS = 0x02,
  BTS7200_SHORT_CIRCUITED_GND = 0x03,
  BTS7200_NOT_INIT = 0x04,
  BTS7200_OVERCURRENT = 0x05,
  BTS7200_OVERTEMPERATURE = 0x06,
  BTS7200_UNKNOWN = 0x07,
};
/*!
 * @struct BTS7200_ChannelStateType
 * @brief 通道信息结构体
 * @details 定义一个通道所包含的配置信息,用于构成上层芯片结构体
 */
struct BTS7200_ChannelStateType {
  enum BTS7200_ChannelType ChannelId;
  enum BTS7200_LevelType Level;
  enum BTS7200_LevelType Ctrl;
};
/*!
 * @struct BTS7200_ChipStateType
 * @brief 芯片信息结构体
 * @details 使用两个通道结构加上芯片信息构成,用于构成驱动结构体
 */
struct BTS7200_ChipStateType {
  enum BTS7200_ChipType ChipId;
  struct BTS7200_ChannelStateType OUT1;
  struct BTS7200_ChannelStateType OUT2;
};
/*!
 * @struct BTS7200_StateInfoType
 * @brief 7200状态结构体
 * @details 包含驱动的输出配置信息,初始化状态信息
 */
struct BTS7200_StateInfoType {
  enum BTS7200_InitType InitInfo;
  enum BTS7200_ChannelType Dsel;
  struct BTS7200_ChipStateType U4100;
  struct BTS7200_ChipStateType U4101;
};
/*!
 * @struct BTS7200_ChipDiagnosticType
 * @brief 芯片诊断结构体
 * @details 包含两个通道的诊断枚举
 */
struct BTS7200_ChipDiagnosticType {
  enum BTS7200_DiagnosticResultType OUT1Result;
  enum BTS7200_DiagnosticResultType OUT2Result;
};
/*!
 * @struct BTS7200_DiagnosticInfoType
 * @brief 驱动诊断信息结构体
 * @details 上层函数获取的诊断信息使用该结构体进行传递
 */
struct BTS7200_DiagnosticInfoType {
  struct BTS7200_ChipDiagnosticType U4100;
  struct BTS7200_ChipDiagnosticType U4101;
};

void BTS7200_Init();

enum BTS7200_ReturnType BTS7200_OpenChannel(enum BTS7200_ChipType ChipId,
                                            enum BTS7200_ChannelType ChannelId);

enum BTS7200_ReturnType BTS7200_CloseChannel(
    enum BTS7200_ChipType ChipId, enum BTS7200_ChannelType ChannelId);

void BTS7200_Diagnostic();

enum BTS7200_ReturnType BTS7200_GetDiagnosticInfo(
    struct BTS7200_DiagnosticInfoType *pDiagnosticInfo);

#endif
