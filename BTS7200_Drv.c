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
static void BTS7200_IoOutput()
{
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

static enum BTS7200_DiagnosticResultType BTS7200_IsDiagnostic(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId)
{
}

void BTS7200_InitPort(enum BTS7200_PortType PortId)
{
    struct BTS7200_PortStateType *pPortId;
    struct BTS7200_ChannelStateType *pChannelId;

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
    pPortId->OUT1.Level = BTS7200_CHANNEL_LOW;
    pPortId->OUT1.DiagnosticResult = BTS7200_NORMAL;
    pPortId->OUT1.DiagnosticDone = BTS7200_NO_DONE;

    pPortId->OUT2.Level = BTS7200_CHANNEL_LOW;
    pPortId->OUT2.DiagnosticResult = BTS7200_NORMAL;
    pPortId->OUT2.DiagnosticDone = BTS7200_NO_DONE;

    pPortId->InitInfo = BTS7200_PORT_INIT;

    BTS7200_IoOutput();
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

    BTS7200_IoOutput();
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

    BTS7200_IoOutput();
    return;
}

void BTS7200_Diagnostic()
{
}