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

struct BTS7200_StateInfoType myStateInfo = {
    .InitInfo = BTS7200_UNINIT,
    .DselInfo = BTS7200_CHANNEL1_LOW,
    .U4100 = {

        .PortId = BTS7200_PORT_U4100,
        .OUT1 = {
            .ChannelId = BTS7200_CHANNEL_OUT1,
            .DiagnosticDone = BTS7200_NO_DONE,
            .DiagnosticResult = BTS7200_NOT_INIT,
            .Level = BTS7200_CHANNEL_LOW,
        },
        .OUT2 = {
            .ChannelId = BTS7200_CHANNEL_OUT2,
            .DiagnosticDone = BTS7200_NO_DONE,
            .DiagnosticResult = BTS7200_NOT_INIT,
            .Level = BTS7200_CHANNEL_LOW,
        },
    },
    .U4101 = {
        .PortId = BTS7200_PORT_U4101,
        .OUT1 = {
            .ChannelId = BTS7200_CHANNEL_OUT1,
            .DiagnosticDone = BTS7200_NO_DONE,
            .DiagnosticResult = BTS7200_NOT_INIT,
            .Level = BTS7200_CHANNEL_LOW,
        },
        .OUT2 = {
            .ChannelId = BTS7200_CHANNEL_OUT2,
            .DiagnosticDone = BTS7200_NO_DONE,
            .DiagnosticResult = BTS7200_NOT_INIT,
            .Level = BTS7200_CHANNEL_LOW,
        },
    }};

unsigned int InputPinIDMatrix[2][2] = {
    //   InputPinIDMatrix[BTS7200_PORT_U4100][BTS7200_CHANNEL_OUT1]
    {DIO_NUM(33, 0), DIO_NUM(33, 2)},
    {DIO_NUM(33, 3), DIO_NUM(33, 4)}};

unsigned int DselPinIDMatrix = DIO_NUM(33, 10);
//   DselPinIDMatrix

unsigned int CtrlPinIDMatrix[2][2] = {
    // CtrlPinIDMatrix[BTS7200_PORT_U4100][BTS7200_CHANNEL_OUT1]
    {DIO_NUM(33, 5), DIO_NUM(33, 6)},
    {DIO_NUM(33, 7), DIO_NUM(20, 10)}};

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

static int BTS7200_IsAdc(enum BTS7200_PortType PortId);

static int BTS7200_DohAdc(enum BTS7200_PortType PortId);

static void BTS7200_DiagnosticChannel1();

static void BTS7200_DiagnosticChannel2();

void BTS7200_Init()
{
    myStateInfo.U4100.OUT1.DiagnosticResult = BTS7200_NORMAL;
    myStateInfo.U4100.OUT1.DiagnosticDone = BTS7200_NO_DONE;

    myStateInfo.U4100.OUT2.DiagnosticResult = BTS7200_NORMAL;
    myStateInfo.U4100.OUT2.DiagnosticDone = BTS7200_NO_DONE;

    BTS7200_InputOut(myStateInfo.U4100.PortId, myStateInfo.U4100.OUT1.ChannelId, myStateInfo.U4100.OUT1.Level);
    BTS7200_InputOut(myStateInfo.U4100.PortId, myStateInfo.U4100.OUT2.ChannelId, myStateInfo.U4100.OUT2.Level);

    myStateInfo.U4101.OUT1.DiagnosticResult = BTS7200_NORMAL;
    myStateInfo.U4101.OUT1.DiagnosticDone = BTS7200_NO_DONE;

    myStateInfo.U4101.OUT2.DiagnosticResult = BTS7200_NORMAL;
    myStateInfo.U4101.OUT2.DiagnosticDone = BTS7200_NO_DONE;

    BTS7200_InputOut(myStateInfo.U4101.PortId, myStateInfo.U4101.OUT1.ChannelId, myStateInfo.U4101.OUT1.Level);
    BTS7200_InputOut(myStateInfo.U4101.PortId, myStateInfo.U4101.OUT2.ChannelId, myStateInfo.U4101.OUT2.Level);

    myStateInfo.InitInfo = BTS7200_INIT;
    return;
}

void BTS7200_OpenChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId)
{
    struct BTS7200_PortStateType *pPortId;
    struct BTS7200_ChannelStateType *pChannelId;

    if (myStateInfo.InitInfo != BTS7200_INIT)
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

    if (myStateInfo.InitInfo != BTS7200_INIT)
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
    //交替查询
    switch (myStateInfo.DselInfo)
    {
    case BTS7200_CHANNEL1_LOW:
        BTS7200_DiagnosticChannel1();
        BTS7200_DselOut(BTS7200_CHANNEL_OUT2);
        break;
    case BTS7200_CHANNEL2_HIGH:
        BTS7200_DiagnosticChannel2();
        BTS7200_DselOut(BTS7200_CHANNEL_OUT1);
        break;
    default:
        break;
    }
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

static int BTS7200_IsAdc(enum BTS7200_PortType PortId)
{
    return get_adc(IsPinIDMatrix[PortId]);
}

static int BTS7200_DohAdc(enum BTS7200_PortType PortId)
{
    return get_adc(DohPinIDMatrix[PortId]);
    //此处应该有一些基本的处理
}

static void BTS7200_DiagnosticChannel1()
{
    int DohValue = 0;
    int IsValue = 0;
    if (myStateInfo.InitInfo != BTS7200_INIT)
    {
        return;
    }

    if (myStateInfo.U4100.OUT1.Level == BTS7200_CHANNEL_LOW)
    {
        BTS7200_CtrlOut(BTS7200_PORT_U4100, BTS7200_CHANNEL_OUT1, BTS7200_CHANNEL_HIGH);
        DohValue = BTS7200_DohAdc(BTS7200_PORT_U4100);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4100);

        // if(DohValue~GND)短接地;

        // if(IsValue~fault)高温

        // if(IsValue~oloff){
        //     if(DohValue~Vs)短接电源
        //     else if(DohValue~Vs)开路
        //     else if(DohValue>Vs)逆电流
        //     else 高温
        // }
    }
    else
    {
        // BTS7200_CtrlOut(BTS7200_PORT_U4100,BTS7200_CHANNEL_OUT1,BTS7200_CHANNEL_HIGH);
        DohValue = BTS7200_DohAdc(BTS7200_PORT_U4100);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4100);
    }

    if (myStateInfo.U4101.OUT1.Level == BTS7200_CHANNEL_LOW)
    {
        BTS7200_CtrlOut(BTS7200_PORT_U4101, BTS7200_CHANNEL_OUT1, BTS7200_CHANNEL_HIGH);
        DohValue = BTS7200_DohAdc(BTS7200_PORT_U4101);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4101);
    }
    else
    {
        // BTS7200_CtrlOut(BTS7200_PORT_U4101,BTS7200_CHANNEL_OUT1,BTS7200_CHANNEL_HIGH);
        DohValue = BTS7200_DohAdc(BTS7200_PORT_U4101);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4101);
    }
}

static void BTS7200_DiagnosticChannel2()
{
    int IsValue = 0;
    if (myStateInfo.InitInfo != BTS7200_INIT)
    {
        return;
    }

    if (myStateInfo.U4100.OUT1.Level == BTS7200_CHANNEL_LOW)
    {
        BTS7200_CtrlOut(BTS7200_PORT_U4100, BTS7200_CHANNEL_OUT1, BTS7200_CHANNEL_HIGH);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4100);
    }
    else
    {
        // BTS7200_CtrlOut(BTS7200_PORT_U4100,BTS7200_CHANNEL_OUT1,BTS7200_CHANNEL_HIGH);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4100);
    }

    if (myStateInfo.U4101.OUT1.Level == BTS7200_CHANNEL_LOW)
    {
        BTS7200_CtrlOut(BTS7200_PORT_U4101, BTS7200_CHANNEL_OUT1, BTS7200_CHANNEL_HIGH);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4101);
    }
    else
    {
        // BTS7200_CtrlOut(BTS7200_PORT_U4101,BTS7200_CHANNEL_OUT1,BTS7200_CHANNEL_HIGH);
        IsValue = BTS7200_IsAdc(BTS7200_PORT_U4101);
    }
}