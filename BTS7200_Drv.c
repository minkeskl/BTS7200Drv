#include "BTS7200_Drv.h"
// KILIS~~670
// DEN  high 测量开启
// DSEL low  测量输出0
//      high 测量输出1

//最基本的是 正常使用情况下,is=il/kilis
//开关延时
//变更通道时间 -- 5 20 us
//小负载情况 -- 250 400 us

//除此以外,有一些故障状态
//如短接到地时,会主动断输出,并重试,此时诊断输出故障电流
//故障检测电流4.4 5.5 10 mA

//在输出处于关闭状态时,测量输出电压,可以检测负载是否丢失
//正常情况下off应当无检测电流,开路时会出现高电流
//开路检测电流1.9 2.5 3.5 mA
//开路检测时间30 70 120 us
//开路检测电压1.3 1.8 2.3 V
//电压值需要重新算算

//下面是各种情况下输出结果的表格
// IN      V_DO    V_IS    result
// low     low     low     正常(可能短路)
// low     high    OL      负载丢失
// high    low     fault   输出短路
// high    high    il/k    正常输出,可以计算电流

#include "IO.h"
#define DIO_NUM(x, y) ((x) << 8 + (y))

unsigned int InputPinIDMatrix[2][2] = {
    //   InputPinIDMatrix[BTS7200_PORT_U4100][BTS7200_CHANNEL_OUT1]
    DIO_NUM(33, 0), DIO_NUM(33, 2),
    DIO_NUM(33, 3), DIO_NUM(33, 4)};

unsigned int DselPinIDMatrix = DIO_NUM(33, 10);
//   DselPinIDMatrix

unsigned int CtrlPinIDMatrix[2][2] = {
    // CtrlPinIDMatrix[BTS7200_PORT_U4100][BTS7200_CHANNEL_OUT1]
    DIO_NUM(33, 5), DIO_NUM(33, 6),
    DIO_NUM(33, 7), DIO_NUM(20, 10)};

unsigned int IsPinIDMatrix[2] = {
    // IsPinIDMatrix[BTS7200_PORT_U4100]
    11,
    2,
};

unsigned int DohPinIDMatrix[2] = {
    //   DohPinIDMatrix[BTS7200_PORT_U4100]
    37,
    38,
};

static unsigned char BTS7200_GetPinId(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId);

static void BTS7200_InputOut(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId, enum BTS7200_LevelType Level);

static void BTS7200_CtrlOut(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId, enum BTS7200_LevelType Level);

static void BTS7200_DselOut(enum BTS7200_ChannelType ChannelId);

static int BTS7200_ISAdc(enum BTS7200_PortType PortId);

static int BTS7200_DohAdc(enum BTS7200_PortType PortId);

static enum BTS7200_InitType BTS7200_IsInit(enum BTS7200_PortType PortId);

static void BTS7200_DiagnosticHighChannel(struct BTS7200_ChannelStateType *pChannelId);

static void BTS7200_DiagnosticLowChannel(struct BTS7200_ChannelStateType *pChannelId);

static void BTS7200_DiagnosticChannel(struct BTS7200_ChannelStateType *pChannelId);

static void BTS7200_DiagnosticPort(struct BTS7200_PortStateType *pPortId);

void BTS7200_InitPort(enum BTS7200_PortType PortId)
{
    struct BTS7200_PortStateType *pPortId;

    switch (PortId)
    {
    case BTS7200_PORT_U4100:
        pPortId = &myStateInfo.U4100;
        break;

    case BTS7200_PORT_U4101:
        pPortId = &myStateInfo.U4101;
        break;

    default:
        break;
    }

    pPortId->OUT1.DiagnosticResult = BTS7200_NORMAL;
    pPortId->OUT1.DiagnosticDone = BTS7200_NO_DONE;

    pPortId->OUT2.DiagnosticResult = BTS7200_NORMAL;
    pPortId->OUT2.DiagnosticDone = BTS7200_NO_DONE;

    pPortId->InitInfo = BTS7200_PORT_INIT;

    BTS7200_InputOut(pPortId->PortId, pPortId->OUT1.ChannelId, pPortId->OUT1.Level);
    BTS7200_InputOut(pPortId->PortId, pPortId->OUT2.ChannelId, pPortId->OUT2.Level);
    return;
}

void BTS7200_OpenChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId)
{
    struct BTS7200_PortStateType *pPortId;
    struct BTS7200_ChannelStateType *pChannelId;

    if (BTS7200_IsInit(PortId) != BTS7200_PORT_INIT)
    {
        return;
    }

    switch (PortId)
    {
    case BTS7200_PORT_U4100:
        pPortId = &myStateInfo.U4100;
        break;

    case BTS7200_PORT_U4101:
        pPortId = &myStateInfo.U4101;
        break;

    default:
        return;
        break;
    }

    switch (ChannelId)
    {
    case BTS7200_CHANNEL_OUT1:
        pChannelId = &(pPortId->OUT1);
        break;

    case BTS7200_CHANNEL_OUT2:
        pChannelId = &(pPortId->OUT2);
        break;

    default:
        return;
        break;
    }

    pChannelId->Level = BTS7200_CHANNEL_HIGH;

    BTS7200_InputOut(pPortId->PortId, pChannelId->ChannelId, pChannelId->Level);
    return;
}

void BTS7200_CloseChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId)
{
    struct BTS7200_PortStateType *pPortId;
    struct BTS7200_ChannelStateType *pChannelId;

    if (BTS7200_IsInit(PortId) != BTS7200_PORT_INIT)
    {
        return;
    }

    switch (PortId)
    {
    case BTS7200_PORT_U4100:
        pPortId = &myStateInfo.U4100;
        break;

    case BTS7200_PORT_U4101:
        pPortId = &myStateInfo.U4101;
        break;

    default:
        return;
        break;
    }

    switch (ChannelId)
    {
    case BTS7200_CHANNEL_OUT1:
        pChannelId = &(pPortId->OUT1);
        break;

    case BTS7200_CHANNEL_OUT2:
        pChannelId = &(pPortId->OUT2);
        break;

    default:
        return;
        break;
    }

    pChannelId->Level = BTS7200_CHANNEL_LOW;

    BTS7200_InputOut(pPortId->PortId, pChannelId->ChannelId, pChannelId->Level);
    return;
}

void BTS7200_Diagnostic()
{
    //初始化判定应该由port做
    //上次结果查验也应该由channel做
    //更新数据由channel做
    //周期50Hz
    // out1可以查验输出,channel去判断
    BTS7200_DiagnosticPort(&(myStateInfo.U4100));
    BTS7200_DiagnosticPort(&(myStateInfo.U4101));
}

static void BTS7200_InputOut(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId, enum BTS7200_LevelType Level)
{
    Dio_WriteChannel(InputPinIDMatrix[PortId][ChannelId], Level);
    return;
}

static void BTS7200_CtrlOut(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId, enum BTS7200_LevelType Level)
{
    Dio_WriteChannel(CtrlPinIDMatrix[PortId][ChannelId], Level);
    return;
}

static void BTS7200_DselOut(enum BTS7200_ChannelType ChannelId)
{
    switch (ChannelId)
    {
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

static int BTS7200_ISAdc(enum BTS7200_PortType PortId)
{
    return get_adc(IsPinIDMatrix[PortId]);
}

static int BTS7200_DohAdc(enum BTS7200_PortType PortId)
{
    return get_adc(DohPinIDMatrix[PortId]);
    //此处应该有一些基本的处理
}

static enum BTS7200_InitType BTS7200_IsInit(enum BTS7200_PortType PortId)
{
    switch (PortId)
    {
    case BTS7200_PORT_U4100:
        return myStateInfo.U4100.InitInfo;
        break;
    case BTS7200_PORT_U4101:
        return myStateInfo.U4101.InitInfo;
        break;
    default:
        break;
    }
    return 0;
}

static void BTS7200_DiagnosticHighChannel(struct BTS7200_ChannelStateType *pChannelId)
{
    if (pChannelId->ChannelId == BTS7200_CHANNEL_OUT1)
    {
    }
    else
    {
    }
    pChannelId->DiagnosticDone = BTS7200_DONE_HIGH;
}

static void BTS7200_DiagnosticLowChannel(struct BTS7200_ChannelStateType *pChannelId)
{
    pChannelId->DiagnosticDone = BTS7200_DONE_LOW;
}

static void BTS7200_DiagnosticChannel(struct BTS7200_ChannelStateType *pChannelId)
{
    // switch (pChannelId->DiagnosticDone)
    // {
    // case BTS7200_NO_DONE:
    //     if (pChannelId->Level == BTS7200_CHANNEL_HIGH)
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_HIGH;
    //     }
    //     else
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_LOW;
    //     }
    //     break;
    // case BTS7200_DONE_HIGH:
    //     if (pChannelId->Level == BTS7200_CHANNEL_HIGH)
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_HIGH;
    //     }
    //     else
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_LOW;
    //     }
    //     break;
    // case BTS7200_DONE_LOW:
    //     if (pChannelId->Level == BTS7200_CHANNEL_HIGH)
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_HIGH;
    //     }
    //     else
    //     {
    //         pChannelId->DiagnosticDone=BTS7200_DONE_LOW;
    //     }
    //     break;

    // default:
    //     break;
    // }
    if (pChannelId->Level == BTS7200_CHANNEL_HIGH)
    {
        BTS7200_DiagnosticHighChannel(pChannelId);
    }
    else
    {
        BTS7200_DiagnosticLowChannel(pChannelId);
    }
}

static void BTS7200_DiagnosticPort(struct BTS7200_PortStateType *pPortId)
{
    if (BTS7200_IsInit(pPortId->PortId) != BTS7200_PORT_INIT)
    {
        pPortId->OUT1.DiagnosticResult = BTS7200_NOT_INIT;
        pPortId->OUT2.DiagnosticResult = BTS7200_NOT_INIT;
        return;
    }
    BTS7200_DiagnosticChannel(&(pPortId->OUT1));
    BTS7200_DiagnosticChannel(&(pPortId->OUT2));
    return;
}