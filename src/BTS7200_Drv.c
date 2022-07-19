/**
 * @copyright Copyright (c) 2020 - 2022, Sankuai
 * All rights reserved.
 * @file BTS7200_Drv.c
 * @brief BTS7200驱动源文件,对驱动函数进行实现
 * @details 需要对Cfg文件进行配置,选择负载类型
 * 在if文件中,对函数接口进行匹配,需要IO和ADC支持
 * @author sunkailiang，sunkailiang@meituan.com
 * @date 2022-7-19
 */

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/
#include "BTS7200_Drv.h"

#include "BTS7200_Drv_Cfg.h"
#include "BTS7200_if.h"

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

static void BTS7200_DiagnosticChannel1ChipX(enum BTS7200_ChipType ChipId);

static void BTS7200_DiagnosticChannel2ChipX(enum BTS7200_ChipType ChipId);
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
/* @brief 模块内部使用的结构体,记录BTS7200的状态 */
struct BTS7200_StateInfoType myStateInfo = {
    .InitInfo = BTS7200_UNINIT,
    .Dsel = BTS7200_CHANNEL_OUT1,
    .U4100 =
        {

            .ChipId = BTS7200_CHIP_U4100,
            .OUT1 =
                {
                    .ChannelId = BTS7200_CHANNEL_OUT1,
                    .Level = BTS7200_CHANNEL_LOW,
                    .Ctrl = BTS7200_CHANNEL_HIGH,
                },
            .OUT2 =
                {
                    .ChannelId = BTS7200_CHANNEL_OUT2,
                    .Level = BTS7200_CHANNEL_LOW,
                    .Ctrl = BTS7200_CHANNEL_HIGH,
                },
        },
    .U4101 = {
        .ChipId = BTS7200_CHIP_U4101,
        .OUT1 =
            {
                .ChannelId = BTS7200_CHANNEL_OUT1,
                .Level = BTS7200_CHANNEL_LOW,
                .Ctrl = BTS7200_CHANNEL_HIGH,
            },
        .OUT2 =
            {
                .ChannelId = BTS7200_CHANNEL_OUT2,
                .Level = BTS7200_CHANNEL_LOW,
                .Ctrl = BTS7200_CHANNEL_HIGH,
            },
    }};
/* @brief 模块内部使用的结构体,记录要输出的诊断信息 */
struct BTS7200_DiagnosticInfoType DiagnosticInfo = {
    .U4100 =
        {

            .OUT1Result = BTS7200_NOT_INIT,
            .OUT2Result = BTS7200_NOT_INIT,
        },
    .U4101 = {
        .OUT1Result = BTS7200_NOT_INIT,
        .OUT2Result = BTS7200_NOT_INIT,

    }};
/*******************************************************************************
**                      Global Function Definitions **
*******************************************************************************/

/**
 * @fn BTS7200_Init
 * @brief 初始化BTS7200的信息和控制引脚的设置,初始化ADC
 * @param[in] None
 * @param[out] None
 * @return None
 * @retval None
 */
void BTS7200_Init(void) {
  DiagnosticInfo.U4100.OUT1Result = BTS7200_NORMAL;
  DiagnosticInfo.U4100.OUT2Result = BTS7200_NORMAL;
  DiagnosticInfo.U4101.OUT1Result = BTS7200_NORMAL;
  DiagnosticInfo.U4101.OUT2Result = BTS7200_NORMAL;

  BTS7200_CtrlPinControl(BTS7200_CHIP_U4100, BTS7200_CHANNEL_OUT1,
                         BTS7200_CHANNEL_HIGH);
  BTS7200_CtrlPinControl(BTS7200_CHIP_U4100, BTS7200_CHANNEL_OUT2,
                         BTS7200_CHANNEL_HIGH);
  BTS7200_CtrlPinControl(BTS7200_CHIP_U4101, BTS7200_CHANNEL_OUT1,
                         BTS7200_CHANNEL_HIGH);
  BTS7200_CtrlPinControl(BTS7200_CHIP_U4101, BTS7200_CHANNEL_OUT2,
                         BTS7200_CHANNEL_HIGH);

  BTS7200_InPinControl(BTS7200_CHIP_U4100, BTS7200_CHANNEL_OUT1,
                       BTS7200_CHANNEL_LOW);
  BTS7200_InPinControl(BTS7200_CHIP_U4100, BTS7200_CHANNEL_OUT2,
                       BTS7200_CHANNEL_LOW);
  BTS7200_InPinControl(BTS7200_CHIP_U4101, BTS7200_CHANNEL_OUT1,
                       BTS7200_CHANNEL_LOW);
  BTS7200_InPinControl(BTS7200_CHIP_U4101, BTS7200_CHANNEL_OUT2,
                       BTS7200_CHANNEL_LOW);

  BTS7200_DselPinControl(BTS7200_CHANNEL_OUT1);
  myStateInfo.Dsel = BTS7200_CHANNEL_OUT1;
  myStateInfo.InitInfo = BTS7200_INIT;

  Adc_Init();
  return;
}

/**
 * @fn BTS7200_OpenChannel
 * @brief 打开所选通道的输出,过程中会检测初始化是否完成
 * @param[in] ChipId 芯片选择值
 * @param[in] ChannelId 通道选择值
 * @param[out] None
 * @return enum BTS7200_ReturnType
 * @retval
 */
enum BTS7200_ReturnType BTS7200_OpenChannel(
    enum BTS7200_ChipType ChipId, enum BTS7200_ChannelType ChannelId) {
  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChannelStateType *pChannelId;

  enum BTS7200_ReturnType ret = BTS7200_RETURN_NORMAL;
  if (myStateInfo.InitInfo != BTS7200_INIT) {
    ret = BTS7200_RETURN_UNINIT;
  } else {
    switch (ChipId) {
      case BTS7200_CHIP_U4100:
        pChipId = &myStateInfo.U4100;
        break;
      case BTS7200_CHIP_U4101:
        pChipId = &myStateInfo.U4101;
        break;
      default:
        ret |= BTS7200_RETURN_ENTER_ERROR;
        break;
    }

    switch (ChannelId) {
      case BTS7200_CHANNEL_OUT1:
        pChannelId = &(pChipId->OUT1);
        break;
      case BTS7200_CHANNEL_OUT2:
        pChannelId = &(pChipId->OUT2);
        break;
      default:
        ret |= BTS7200_RETURN_ENTER_ERROR;
        break;
    }

    pChannelId->Level = BTS7200_CHANNEL_HIGH;
    pChannelId->Ctrl = BTS7200_CHANNEL_HIGH;

    BTS7200_InPinControl(pChipId->ChipId, pChannelId->ChannelId,
                         pChannelId->Level);
    BTS7200_CtrlPinControl(pChipId->ChipId, pChannelId->ChannelId,
                           pChannelId->Ctrl);
  }

  return ret;
}

/**
 * @fn BTS7200_CloseChannel
 * @brief 关闭所选通道的输出,过程中会检测初始化是否完成
 * @param[in] ChipId 芯片选择值
 * @param[in] ChannelId 通道选择值
 * @param[out] None
 * @return enum BTS7200_ReturnType
 * @retval BTS7200_RETURN_NORMAL 正常状态
 * BTS7200_RETURN_UNINIT 模块未初始化
 * BTS7200_RETURN_ENTER_ERROR 函数传入的参数有误
 */
enum BTS7200_ReturnType BTS7200_CloseChannel(
    enum BTS7200_ChipType ChipId, enum BTS7200_ChannelType ChannelId) {
  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChannelStateType *pChannelId;

  enum BTS7200_ReturnType ret = BTS7200_RETURN_NORMAL;
  if (myStateInfo.InitInfo != BTS7200_INIT) {
    ret = BTS7200_RETURN_UNINIT;
  } else {
    switch (ChipId) {
      case BTS7200_CHIP_U4100:
        pChipId = &myStateInfo.U4100;
        break;
      case BTS7200_CHIP_U4101:
        pChipId = &myStateInfo.U4101;
        break;
      default:
        ret |= BTS7200_RETURN_ENTER_ERROR;
        break;
    }

    switch (ChannelId) {
      case BTS7200_CHANNEL_OUT1:
        pChannelId = &(pChipId->OUT1);
        break;
      case BTS7200_CHANNEL_OUT2:
        pChannelId = &(pChipId->OUT2);
        break;
      default:
        ret |= BTS7200_RETURN_ENTER_ERROR;
        break;
    }

    pChannelId->Level = BTS7200_CHANNEL_LOW;
    pChannelId->Ctrl = BTS7200_CHANNEL_HIGH;

    BTS7200_InPinControl(pChipId->ChipId, pChannelId->ChannelId,
                         pChannelId->Level);
    BTS7200_CtrlPinControl(pChipId->ChipId, pChannelId->ChannelId,
                           pChannelId->Ctrl);
  }

  return ret;
}

/**
 * @fn BTS7200_Diagnostic
 * @brief 诊断函数,周期性执行,执行结果通过反馈到诊断结构体
 * 由于引脚状态和诊断信息之间存在滞后性,这里通过交替执行的方式进行延时
 * @param[in] None
 * @param[out] None
 * @return None
 * @retval None
 */
void BTS7200_Diagnostic(void) {
  if (myStateInfo.InitInfo == BTS7200_INIT) {
    //交替查询
    switch (myStateInfo.Dsel) {
      case BTS7200_CHANNEL_OUT1:
        BTS7200_DiagnosticChannel1ChipX(BTS7200_CHIP_U4100);
        BTS7200_DiagnosticChannel1ChipX(BTS7200_CHIP_U4101);
        BTS7200_DselPinControl(BTS7200_CHANNEL_OUT2);
        myStateInfo.Dsel = BTS7200_CHANNEL_OUT2;
        break;
      case BTS7200_CHANNEL_OUT2:
        BTS7200_DiagnosticChannel2ChipX(BTS7200_CHIP_U4100);
        BTS7200_DiagnosticChannel2ChipX(BTS7200_CHIP_U4101);
        BTS7200_DselPinControl(BTS7200_CHANNEL_OUT1);
        myStateInfo.Dsel = BTS7200_CHANNEL_OUT1;
        break;
      default:
        break;
    }
  }
  return;
}

/**
 * @fn BTS7200_Diagnostic
 * @brief 返回诊断信息,传入结构体指针
 * @param[in] None
 * @param[out] pDiagnosticInfo
 * 传入一个BTS7200_DiagnosticInfoType结构体指针,函数会将信息拷贝到目标地址
 * @return enum BTS7200_ReturnType
 * @retval
 */
enum BTS7200_ReturnType BTS7200_GetDiagnosticInfo(
    struct BTS7200_DiagnosticInfoType *pDiagnosticInfo) {
  enum BTS7200_ReturnType ret = BTS7200_RETURN_NORMAL;
  if (pDiagnosticInfo == 0) {
    ret = BTS7200_RETURN_ENTER_ERROR;
  } else {
    pDiagnosticInfo->U4100.OUT1Result = DiagnosticInfo.U4100.OUT1Result;
    pDiagnosticInfo->U4100.OUT2Result = DiagnosticInfo.U4100.OUT2Result;
    pDiagnosticInfo->U4101.OUT1Result = DiagnosticInfo.U4101.OUT1Result;
    pDiagnosticInfo->U4101.OUT2Result = DiagnosticInfo.U4101.OUT2Result;
  }
  return ret;
}

#if BTS7200_HIGH_OUT_RESISTANCE

/**
 * @fn BTS7200_DiagnosticChannel1ChipX
 * @brief 高负载电阻的情况下,对芯片通道1的诊断
 * @param[in] ChipId 芯片选择值
 * @param[out] None
 * @return None
 * @retval  None
 */
static void BTS7200_DiagnosticChannel1ChipX(enum BTS7200_ChipType ChipId) {
  float DohValue = 0;
  float IsValue = 0;

  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChipDiagnosticType *pDiagnosticChipId;
  if (ChipId == BTS7200_CHIP_U4100) {
    pChipId = &myStateInfo.U4100;
    pDiagnosticChipId = &DiagnosticInfo.U4100;
  } else {
    pChipId = &myStateInfo.U4101;
    pDiagnosticChipId = &DiagnosticInfo.U4101;
  }

  IsValue = BTS7200_IsAdc(pChipId->ChipId);
  DohValue = BTS7200_DohAdc(pChipId->ChipId);

  if (pChipId->OUT1.Level == BTS7200_CHANNEL_LOW) {
    if (pChipId->OUT1.Ctrl == BTS7200_CHANNEL_LOW) {
      if ((IsValue >= BTS7200_IS_GND_MIN) && (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT1Result = BTS7200_NORMAL;
      } else {
        pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_VS;
      }
      pChipId->OUT1.Ctrl = BTS7200_CHANNEL_HIGH;
      BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT1,
                             BTS7200_CHANNEL_HIGH);
    } else {
      if ((IsValue >= BTS7200_IS_CLOSE_MIN) &&
          (IsValue < BTS7200_IS_CLOSE_MAX)) {
        pChipId->OUT1.Ctrl = BTS7200_CHANNEL_LOW;
        BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT1,
                               BTS7200_CHANNEL_LOW);
      } else if ((IsValue >= BTS7200_IS_GND_MIN) &&
                 (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_GND;
      } else {
        pDiagnosticChipId->OUT1Result = BTS7200_UNKNOWN;
      }
    }
  } else {
    if ((IsValue >= BTS7200_IS_MIN) && (IsValue < BTS7200_IS_MAX) &&
        (DohValue >= BTS7200_DOH_VS_MIN) && (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_NORMAL;
    } else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
               (IsValue < BTS7200_IS_FAULT_MAX)) {
      if ((DohValue >= 2) && (DohValue < BTS7200_DOH_VS_MIN))
        pDiagnosticChipId->OUT1Result = BTS7200_OVERCURRENT;
      else if ((DohValue >= BTS7200_DOH_VS_MIN) &&
               (DohValue < BTS7200_DOH_VS_MAX))
        pDiagnosticChipId->OUT1Result = BTS7200_OVERTEMPERATURE;
      else if ((DohValue >= BTS7200_DOH_GND_MIN) &&
               (DohValue < BTS7200_DOH_GND_MAX))
        pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_GND;
    } else if ((IsValue >= BTS7200_IS_OL_OPEN_MIN) &&
               (IsValue < BTS7200_IS_OL_OPEN_MAX) &&
               (DohValue >= BTS7200_DOH_VS_MIN) &&
               (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_OPEN_LOAD;
    } else if ((IsValue >= BTS7200_IS_MIN - BTS7200_ERROR) &&
               (IsValue < BTS7200_IS_MIN) && (DohValue >= BTS7200_DOH_VS_MIN) &&
               (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_VS;
    } else {
      pDiagnosticChipId->OUT1Result = BTS7200_UNKNOWN;
    }
  }
}

/**
 * @fn BTS7200_DiagnosticChannel2ChipX
 * @brief 高负载电阻的情况下,对芯片通道2的诊断
 * @param[in] ChipId 芯片选择值
 * @param[out]  None
 * @return None
 * @retval  None
 */
static void BTS7200_DiagnosticChannel2ChipX(enum BTS7200_ChipType ChipId) {
  float IsValue = 0;

  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChipDiagnosticType *pDiagnosticChipId;
  if (ChipId == BTS7200_CHIP_U4100) {
    pChipId = &myStateInfo.U4100;
    pDiagnosticChipId = &DiagnosticInfo.U4100;
  } else {
    pChipId = &myStateInfo.U4101;
    pDiagnosticChipId = &DiagnosticInfo.U4101;
  }

  IsValue = BTS7200_IsAdc(pChipId->ChipId);

  if (pChipId->OUT2.Level == BTS7200_CHANNEL_LOW) {
    if (pChipId->OUT2.Ctrl == BTS7200_CHANNEL_LOW) {
      if ((IsValue >= BTS7200_IS_GND_MIN) && (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_NORMAL;
      } else {
        pDiagnosticChipId->OUT2Result = BTS7200_SHORT_CIRCUITED_VS;
      }
      pChipId->OUT2.Ctrl = BTS7200_CHANNEL_HIGH;
      BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT2,
                             BTS7200_CHANNEL_HIGH);
    } else {
      if ((IsValue >= BTS7200_IS_CLOSE_MIN) &&
          (IsValue < BTS7200_IS_CLOSE_MAX)) {
        pChipId->OUT2.Ctrl = BTS7200_CHANNEL_LOW;
        BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT2,
                               BTS7200_CHANNEL_LOW);
      } else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
                 (IsValue < BTS7200_IS_FAULT_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_OVERTEMPERATURE;
      } else if ((IsValue >= BTS7200_IS_GND_MIN) &&
                 (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_SHORT_CIRCUITED_GND;
      }

      else {
        pDiagnosticChipId->OUT2Result = BTS7200_UNKNOWN;
      }
    }
  } else {
    if ((IsValue >= BTS7200_IS_MIN) && (IsValue < BTS7200_IS_MAX)) {
      pDiagnosticChipId->OUT2Result = BTS7200_NORMAL;
    } else if ((IsValue >= BTS7200_IS_OL_OPEN_MIN) &&
               (IsValue < BTS7200_IS_OL_OPEN_MAX)) {
      pDiagnosticChipId->OUT2Result = BTS7200_OPEN_LOAD;
    } else if ((IsValue >= BTS7200_IS_MIN - BTS7200_ERROR) &&
               (IsValue < BTS7200_IS_MIN)) {
      pDiagnosticChipId->OUT2Result = BTS7200_SHORT_CIRCUITED_VS;
    } else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
               (IsValue < BTS7200_IS_FAULT_MAX)) {
      pDiagnosticChipId->OUT2Result = BTS7200_SHORT_CIRCUITED_GND;
      // BTS7200_OVERCURRENT;
      // BTS7200_OVERTEMPERATURE;
    } else {
      pDiagnosticChipId->OUT2Result = BTS7200_UNKNOWN;
    }
  }
}
#else

/**
 * @fn BTS7200_DiagnosticChannel1ChipX
 * @brief 低负载电阻的情况下,对芯片通道1的诊断
 * @param[in] ChipId 芯片选择值
 * @param[out]  None
 * @return None
 * @retval  None
 */
static void BTS7200_DiagnosticChannel1ChipX(enum BTS7200_ChipType ChipId) {
  float DohValue = 0;
  float IsValue = 0;

  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChipDiagnosticType *pDiagnosticChipId;
  if (ChipId == BTS7200_CHIP_U4100) {
    pChipId = &myStateInfo.U4100;
    pDiagnosticChipId = &DiagnosticInfo.U4100;
  } else {
    pChipId = &myStateInfo.U4101;
    pDiagnosticChipId = &DiagnosticInfo.U4101;
  }

  IsValue = BTS7200_IsAdc(pChipId->ChipId);
  DohValue = BTS7200_DohAdc(pChipId->ChipId);

  if (pChipId->OUT1.Level == BTS7200_CHANNEL_LOW) {
    if (pChipId->OUT1.Ctrl == BTS7200_CHANNEL_LOW) {
      if ((IsValue >= BTS7200_IS_GND_MIN) && (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT1Result = BTS7200_OPEN_LOAD;
      } else {
        pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_VS;
      }
      pChipId->OUT1.Ctrl = BTS7200_CHANNEL_HIGH;
      BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT1,
                             BTS7200_CHANNEL_HIGH);
    } else {
      //正常情况
      //短接地情况 输出端地,诊断端地
      //无法区分
      if ((IsValue >= BTS7200_IS_CLOSE_MIN) &&
          (IsValue < BTS7200_IS_CLOSE_MAX) &&
          (DohValue >= BTS7200_DOH_CLOSE_MIN) &&
          (DohValue < BTS7200_DOH_CLOSE_MAX)) {
        pDiagnosticChipId->OUT1Result = BTS7200_NORMAL;
      }
      //高温情况 电流报错 加对外高阻抗
      else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
               (IsValue < BTS7200_IS_FAULT_MAX) &&
               (DohValue >= BTS7200_DOH_OL_MIN) &&
               (DohValue < BTS7200_DOH_OL_MAX)) {
        pDiagnosticChipId->OUT1Result = BTS7200_OVERTEMPERATURE;
      }
      //丢失负载和短接电源两种情况
      //对应开负载电流和高电平
      //此时拉低CTRL,开负载应该低is,doh也低,而短接vs表现为isol,dohol,表现不变
      else if ((IsValue >= BTS7200_IS_OL_MIN) &&
               (IsValue < BTS7200_IS_OL_MAX)) {
        if ((DohValue >= BTS7200_DOH_OL_MIN) &&
            (DohValue < BTS7200_DOH_OL_MAX)) {
          pChipId->OUT1.Ctrl = BTS7200_CHANNEL_LOW;
          BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT1,
                                 BTS7200_CHANNEL_LOW);
        }
      } else {
        pDiagnosticChipId->OUT1Result = BTS7200_UNKNOWN;
      }
    }
  } else {
    // 0.25到3认为正常,5.5左右认为过载,可能短路,0-0.25,开负载或短路电源
    // oh检查输出是否一致

    //普通模式out 12 vs为一个小范围内的值
    if ((IsValue >= BTS7200_IS_MIN) && (IsValue < BTS7200_IS_MAX) &&
        (DohValue >= BTS7200_DOH_VS_MIN) && (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_NORMAL;
    }
    //过电流,过温度,短接地
    //都表现为is
    else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
             (IsValue < BTS7200_IS_FAULT_MAX)) {
      if ((DohValue >= 2) && (DohValue < BTS7200_DOH_VS_MIN))
        pDiagnosticChipId->OUT1Result = BTS7200_OVERCURRENT;
      else if ((DohValue >= BTS7200_DOH_VS_MIN) &&
               (DohValue < BTS7200_DOH_VS_MAX))
        pDiagnosticChipId->OUT1Result = BTS7200_OVERTEMPERATURE;
      else if ((DohValue >= BTS7200_DOH_GND_MIN) &&
               (DohValue < BTS7200_DOH_GND_MAX))
        pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_GND;
    }
    //开路负载,输出为一低值
    else if ((IsValue >= BTS7200_IS_OL_OPEN_MIN) &&
             (IsValue < BTS7200_IS_OL_OPEN_MAX) &&
             (DohValue >= BTS7200_DOH_VS_MIN) &&
             (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_OPEN_LOAD;
    }
    //短接电源,输出电流比目标低
    else if ((IsValue >= BTS7200_IS_MIN - BTS7200_ERROR) &&
             (IsValue < BTS7200_IS_MIN) && (DohValue >= BTS7200_DOH_VS_MIN) &&
             (DohValue < BTS7200_DOH_VS_MAX)) {
      pDiagnosticChipId->OUT1Result = BTS7200_SHORT_CIRCUITED_VS;
    }

    else {
      pDiagnosticChipId->OUT1Result = BTS7200_UNKNOWN;
    }
  }
}

/**
 * @fn BTS7200_DiagnosticChannel2ChipX
 * @brief 低负载电阻的情况下,对芯片通道2的诊断
 * @param[in] ChipId 芯片选择值
 * @param[out]  None
 * @return None
 * @retval  None
 */
static void BTS7200_DiagnosticChannel2ChipX(enum BTS7200_ChipType ChipId) {
  float IsValue = 0;

  struct BTS7200_ChipStateType *pChipId;
  struct BTS7200_ChipDiagnosticType *pDiagnosticChipId;
  if (ChipId == BTS7200_CHIP_U4100) {
    pChipId = &myStateInfo.U4100;
    pDiagnosticChipId = &DiagnosticInfo.U4100;
  } else {
    pChipId = &myStateInfo.U4101;
    pDiagnosticChipId = &DiagnosticInfo.U4101;
  }

  IsValue = BTS7200_IsAdc(pChipId->ChipId);

  if (pChipId->OUT2.Level == BTS7200_CHANNEL_LOW) {
    if (pChipId->OUT2.Ctrl == BTS7200_CHANNEL_LOW) {
      if ((IsValue >= BTS7200_IS_GND_MIN) && (IsValue < BTS7200_IS_GND_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_OPEN_LOAD;
      } else {
        pDiagnosticChipId->OUT2Result = BTS7200_SHORT_CIRCUITED_VS;
      }
      pChipId->OUT2.Ctrl = BTS7200_CHANNEL_HIGH;
      BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT2,
                             BTS7200_CHANNEL_HIGH);
    } else {
      //正常和短接地无法区分
      //此时is 高阻 表现为gnd
      if ((IsValue >= BTS7200_IS_CLOSE_MIN) &&
          (IsValue < BTS7200_IS_CLOSE_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_NORMAL;
      }
      //高温
      // is错误状态
      else if ((IsValue >= BTS7200_IS_FAULT_MIN) &&
               (IsValue < BTS7200_IS_FAULT_MAX)) {
        pDiagnosticChipId->OUT2Result = BTS7200_OVERTEMPERATURE;
      }
      //如果在0.5--3.5,认为是短接电源,5.5周围认为是短接地,2.7-2.3可能是开路
      //短接电源和丢失负载
      //利用ctrl开关来控制判断
      else if ((IsValue >= BTS7200_IS_OL_MIN) &&
               (IsValue < BTS7200_IS_OL_MAX)) {
        pChipId->OUT2.Ctrl = BTS7200_CHANNEL_LOW;
        BTS7200_CtrlPinControl(ChipId, BTS7200_CHANNEL_OUT2,
                               BTS7200_CHANNEL_LOW);
      }

      else {
        pDiagnosticChipId->OUT2Result = BTS7200_UNKNOWN;
      }
    }
  }
}

#endif
