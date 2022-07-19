/**
 * @copyright Copyright (c) 2020 - 2022, Sankuai
 * All rights reserved.
 * @file BTS7200_if.h
 * @brief BTS7200驱动的接口文件
 * @details 需要关注的是IO的输出,和ADC读取
 * 对ADC的值需要做处理,使其在合理范围
 * @author sunkailiang，sunkailiang@meituan.com
 * @date 2022-7-19
 */

#ifndef BTS7200_IF_C
#define BTS7200_IF_C

#include "BTS7200_Drv.h"
#include "BTS7200_Drv_Cfg.h"
#include "IO.h"

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
static void BTS7200_InPinControl(enum BTS7200_ChipType ChipId,
                                 enum BTS7200_ChannelType ChannelId,
                                 enum BTS7200_LevelType Level);

static void BTS7200_CtrlPinControl(enum BTS7200_ChipType ChipId,
                                   enum BTS7200_ChannelType ChannelId,
                                   enum BTS7200_LevelType Level);

static void BTS7200_DselPinControl(enum BTS7200_ChannelType ChannelId);

static float BTS7200_IsAdc(enum BTS7200_ChipType ChipId);

static float BTS7200_DohAdc(enum BTS7200_ChipType ChipId);

/*! @brief 引脚信息转化宏 */
#define DIO_NUM(x, y) (((x) << 8) + (y))
#define DIO_NUM_PORT(x) (((x)&0xFFFF) >> 8)
#define DIO_NUM_PIN(x) ((x)&0xFF)

/* @brief 使用方法 InputPinIDMatrix[BTS7200_CHIP_U4100][BTS7200_CHANNEL_OUT1]*/
unsigned int InputPinIDMatrix[2][2] = {

    {DIO_NUM(33, 0), DIO_NUM(33, 2)}, {DIO_NUM(33, 3), DIO_NUM(33, 4)}};

/* @brief 使用方法 DselPinIDMatrix*/
unsigned int DselPinIDMatrix = DIO_NUM(33, 10);

/* @brief 使用方法 CtrlPinIDMatrix[BTS7200_CHIP_U4100][BTS7200_CHANNEL_OUT1]*/
unsigned int CtrlPinIDMatrix[2][2] = {

    {DIO_NUM(33, 6), DIO_NUM(33, 5)}, {DIO_NUM(33, 7), DIO_NUM(20, 10)}};

/* @brief 使用方法 IsPinIDMatrix[BTS7200_CHIP_U4100]*/
unsigned int IsPinIDMatrix[2] = {

    DIO_NUM(1, 3),
    DIO_NUM(0, 2),
};

/* @brief 使用方法 DohPinIDMatrix[BTS7200_CHIP_U4100]*/
unsigned int DohPinIDMatrix[2] = {

    DIO_NUM(8, 5),
    DIO_NUM(8, 6),
};

/**
 * @fn BTS7200_InPinControl
 * @brief 控制到BTS7200的INx引脚的电平,高电平对应通道高电平
 * @param[in] ChipId 芯片选择值
 * @param[in] ChannelId 通道选择值
 * @param[in] Level 输出等级选择
 * @param[out]
 * @return None
 * @retval None
 */
static void BTS7200_InPinControl(enum BTS7200_ChipType ChipId,
                                 enum BTS7200_ChannelType ChannelId,
                                 enum BTS7200_LevelType Level) {
  Dio_WriteChannel(InputPinIDMatrix[ChipId][ChannelId], Level);
  return;
}

/**
 * @fn BTS7200_CtrlPinControl
 * @brief 控制到Ctrl控制端的电平,高电平对应通道Ctrl端高电平
 * @param[in] ChipId 芯片选择值
 * @param[in] ChannelId 通道选择值
 * @param[in] Level 输出等级选择
 * @param[out]
 * @return None
 * @retval None
 */
static void BTS7200_CtrlPinControl(enum BTS7200_ChipType ChipId,
                                   enum BTS7200_ChannelType ChannelId,
                                   enum BTS7200_LevelType Level) {
  Dio_WriteChannel(CtrlPinIDMatrix[ChipId][ChannelId], Level);
  return;
}

/**
 * @fn BTS7200_DselPinControl
 * @brief 控制Dsel引脚的电平,低电平对应通道1,高电平对应通道2
 * @param[in] ChannelId   通道选择值
 * @param[out]
 * @return None
 * @retval None
 */
static void BTS7200_DselPinControl(enum BTS7200_ChannelType ChannelId) {
  switch (ChannelId) {
    case BTS7200_CHANNEL_OUT1:
      Dio_WriteChannel(DselPinIDMatrix, BTS7200_CHANNEL_LOW);

      break;
    case BTS7200_CHANNEL_OUT2:
      Dio_WriteChannel(DselPinIDMatrix, BTS7200_CHANNEL_HIGH);
      break;
    default:
      break;
  }
  return;
}

/**
 * @fn BTS7200_IsAdc
 * @brief 获取Is诊断引脚的电平
 * @param[in] ChipId  芯片选择值
 * @param[out]
 * @return float
 * @retval 电压值的浮点形式
 */
static float BTS7200_IsAdc(enum BTS7200_ChipType ChipId) {
  int adc = get_adc(IsPinIDMatrix[ChipId]);
  float r_adc = ((float)adc) / 200;
  return r_adc;
}

/**
 * @fn BTS7200_DohAdc
 * @brief 获取Doh诊断引脚的电平
 * @param[in] ChipId  芯片选择值
 * @param[out]
 * @return float
 * @retval 电压值的浮点形式
 */
static float BTS7200_DohAdc(enum BTS7200_ChipType ChipId) {
  int adc = get_adc(DohPinIDMatrix[ChipId]);
  float r_adc = ((float)adc) / 200;
  return r_adc;
}

#endif
