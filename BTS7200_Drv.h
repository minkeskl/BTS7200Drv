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
    BTS7200_PORT_U4101 = 0x01,
    // BTS7200_PORT_ALL = 0xFF,
};

enum BTS7200_ChannelType
{
    BTS7200_CHANNEL_OUT1 = 0x00,
    BTS7200_CHANNEL_OUT2 = 0x01,
    // BTS7200_CHANNEL_ALL = 0xFF,
};

enum BTS7200_LevelType
{
    BTS7200_CHANNEL_LOW = 0x00,
    BTS7200_CHANNEL_HIGH = 0xFF,
};

enum BTS7200_InitType
{
    BTS7200_UNINIT = 0x00,
    BTS7200_INIT = 0xFF,
};

enum BTS7200_DselType
{
    BTS7200_CHANNEL1_LOW = 0x00,
    BTS7200_CHANNEL2_HIGH = 0x01,
};

// enum BTS7200_CtrlType
// {
//     BTS7200_CTRL_LOW = 0x00,
//     BTS7200_CTRL_HIGH = 0x01,
// };

enum BTS7200_DiagnosticResultType
{
    BTS7200_NORMAL = 0x00,
    BTS7200_OVER_LOAD = 0x01,
    BTS7200_SHORT_CIRCUITED_12V = 0x02,
    BTS7200_SHORT_CIRCUITED_GND = 0x03,
    BTS7200_NOT_INIT = 0x04,
};

enum BTS7200_DiagnosticDoneType
{
    BTS7200_NO_DONE = 0x00,
    BTS7200_DONE_HIGH = 0x01,
    BTS7200_DONE_LOW = 0x02,
    //    BTS7200_DONE = 0x03,
};

struct BTS7200_ChannelStateType
{
    enum BTS7200_ChannelType ChannelId;
    enum BTS7200_LevelType Level;
    // enum BTS7200_DiagnosticResultType DiagnosticResult;
    // enum BTS7200_DiagnosticDoneType DiagnosticDone;
};

struct BTS7200_PortStateType
{
    enum BTS7200_PortType PortId;
    struct BTS7200_ChannelStateType OUT1;
    struct BTS7200_ChannelStateType OUT2;
};

struct BTS7200_StateInfoType
{
    enum BTS7200_InitType InitInfo;
    enum BTS7200_DselType DselInfo;
    struct BTS7200_PortStateType U4100;
    struct BTS7200_PortStateType U4101;
};

struct BTS7200_PortDiagnosticType
{
    enum BTS7200_DiagnosticResultType OUT1Result;
    enum BTS7200_DiagnosticResultType OUT2Result;
};

struct BTS7200_DiagnosticInfoType
{
    struct BTS7200_PortDiagnosticType U4100;
    struct BTS7200_PortDiagnosticType U4101;
};

struct BTS7200_DiagnosticInfoType DiagnosticInfo = {
    .U4100 = {

        .OUT1Result = BTS7200_NOT_INIT,
        .OUT1Result = BTS7200_NOT_INIT,
    },
    .U4101 = {
        .OUT1Result = BTS7200_NOT_INIT,
        .OUT1Result = BTS7200_NOT_INIT,

    }};

//函数名：  BTS7200_InitPort
//功能：    判断当前待插入或更新的记录在原表中是否已经存在
//输入参数：bm (表名）   待查找的 表的名字
//          zdm (字段名）在表中待查找的字段
//          zdz(字段值） 需要比较的字段的值
//返回值：
void BTS7200_Init();

//函数名：  BTS7200_OpenChannel
//功能：    判断当前待插入或更新的记录在原表中是否已经存在
//输入参数：bm (表名）   待查找的 表的名字
//          zdm (字段名）在表中待查找的字段
//          zdz(字段值） 需要比较的字段的值
//返回值：
void BTS7200_OpenChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId);

//函数名：  BTS7200_CloseChannel
//功能：    判断当前待插入或更新的记录在原表中是否已经存在
//输入参数：bm (表名）   待查找的 表的名字
//          zdm (字段名）在表中待查找的字段
//          zdz(字段值） 需要比较的字段的值
//返回值：
void BTS7200_CloseChannel(enum BTS7200_PortType PortId, enum BTS7200_ChannelType ChannelId);

//函数名：  BTS7200_Diagnostic
//功能：    判断当前待插入或更新的记录在原表中是否已经存在
//输入参数：
//返回值：
void BTS7200_Diagnostic();

#endif