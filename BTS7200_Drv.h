//提供驱动芯片的接口函数
// 7200的引脚
// out
// ctrl1 ctrl2 控制脚 io
// in1 in2 输入脚 io
// DSEL 选择脚 io

// in
// IS 诊断脚  adc
// DOH1 负载检测 adc

// 可能需要的功能
// 芯片设置
// 控制输出
//     负载存在检测
//     io输出控制
//     诊断
#ifndef BTS7200_DRV_H
#define BTS7200_DRV_H

enum BTS7200_PortType
{
    BTS7200_PORT_U4100 = 0x00,
    BTS7200_PORT_U4101 = 0xFF,
};

enum BTS7200_ChannelType
{
    BTS7200_CHANNEL_OUT1 = 0x00,
    BTS7200_CHANNEL_OUT2 = 0xFF,
};

enum BTS7200_LevelType
{
    BTS7200_CHANNEL_LOW = 0x00,
    BTS7200_CHANNEL_HIGH = 0xFF,
};

enum BTS7200_InitType
{
    BTS7200_PORT_UNINIT = 0x00,
    BTS7200_PORT_INIT = 0xFF,
};

enum BTS7200_DiagnosticResultType
{
    BTS7200_NORMAL = 0x00,
    BTS7200_OVER_LOAD = 0x01,
    BTS7200_SHORT_CIRCUITED_12V = 0x02,
    BTS7200_SHORT_CIRCUITED_GND = 0x03,
};

enum BTS7200_DiagnosticDoneType
{
    BTS7200_NO_DONE=0x00,
    BTS7200_NO_DONE_HIGH=0x01,
    BTS7200_NO_DONE_LOW=0x02,
    BTS7200_DONE=0x03,
};

struct BTS7200_ChannelStateType
{
    enum BTS7200_LevelType Level;
    enum BTS7200_DiagnosticResultType DiagnosticResult;
    enum BTS7200_DiagnosticDoneType DiagnosticDone;
};

struct BTS7200_PortStateType
{
    enum BTS7200_InitType InitInfo;
    struct BTS7200_ChannelStateType OUT1;
    struct BTS7200_ChannelStateType OUT2;
};

struct BTS7200_StateInfoType
{
    struct BTS7200_PortStateType U4100;
    struct BTS7200_PortStateType U4101;
};

static struct BTS7200_StateInfoType myStateInfo;

void BTS7200_InitPort(enum BTS7200_PortType PortId,struct BTS7200_StateInfoType *State);

void BTS7200_OpenChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId,struct BTS7200_StateInfoType *State);

void BTS7200_CloseChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId,struct BTS7200_StateInfoType *State);

void BTS7200_Diagnostic(struct BTS7200_StateInfoType *State);

#endif