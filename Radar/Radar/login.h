#pragma once
#include"stdafx.h"

/**
Session系统通讯协议

Session系统概述
每一台设备通过长连接到总控服务器并进行用户登陆后，被称之为 一个Session，每个Session都有名称，身份,状态,编号，四大属性。
*/

/*
1	通用数据通讯协议
对于数据报文中数据包的规则定义如下
序号	A	B	C	D	E	F
结构	协议头	协议体	协议尾
参数名	信息长度	命令字	序号	版本号	数据	校验位
类型	WORD	BYTE	WORD	BYTE	STRING	WORD
字节数	2(0 - 1)	1(2)	2(3 - 4)	1(5)	(6 - )x	2
注：协议中每个参数的位置是按照序号的顺序排列的；字节数项中“x”表示该项参数的长度为“不定长LEN”（下同）。内存对齐方式为1字节
对于双字节word，高字节在后，低字节在前。比如信息长度为10，则发送为0x0a00

表中：
信息长度 ――包括协议头和协议尾在内的协议总长。
命令字   ――用来标识协议功能的代码。
序号     ――协议帧传输时的流水号，循环自增。
版本号   ――高位表示主版本号，低位表示次版本号。
数据     ――协议数据。
校验位   ――采用CRC - 16校验，被校验内容为除校验位外的所有部分。
*/
/*
2	登录服务器及反馈
说明：仅直接与总控服务器相连接的Session，需要发送该登陆服务器数据包，其它设备由它们的父结点设备统一同步旗下的设备状态为SS_LOBBY
数据包中命令字和协议体的定义如下：*/
//命令字：
const byte MSG_SESSION_LOGIN = 0x00;
/*协议体：
结构体	命令字	功能
ConnectSvr	0x00	请求连接服务器
ConnectSvrResp		请求连接服务器的回应
*/

//状态
enum SessionState
{
	SS_NONE = 0,	//未登陆
	SS_DROPPED,		//非正常掉线，重新登陆会进行断线重连功能	
	SS_LOBBY ,		//大厅状态，已登陆,但还未进入案例仿真
	SS_CASEPREPARE,//案例前的准备工作已就绪,例如完成了初始化
	SS_CASERUNNING,//正在进行案例仿真
	SS_MANAGING,	//正在管理  (保留类型，暂时未用),
	SS_ALL         //代表所有状态
};


//身份/名称(编号)
enum SessionIdentity
{
	SI_UNDEFINED = 0,			//未定义的身份
	SI_MANAGER,					//教练站-管理端(6)
	SI_MONITOR,					//瞭望监控端(3)
	SI_LOGIC_SERVER,				//逻辑服务器(3)
	SI_MAINSHIP_RENDERER,     //主本船渲染端(1-7)
	SI_SECONDARYSHIP_CLIENT,  //副本船渲染端(1-2)
	SI_BRIDGE_CHART,		      //驾驶台-电子海图(1-3)
	SI_BRIDGE_RADAR,		      //驾驶台-雷达模拟器(1-3)
	SI_BRIDGE_CONNING,		   //驾驶台-Conning(1-3)
	SI_DISPATCHER,				//应急调度与处置端(5)
	SI_PLAN_MONITOR,				//预案执行监控端(4)
	SI_ALL,                          //代表所有身份
};


/*
连接服务器服务器请求：内存对齐方式为1字节；

协议体中数据项的解释如下：
数据项	说明
iSessionIdentity	以什么身份类型登陆，取值参见“enum SessionIdentity”
iNumber	身份类型的设备编号
*/
typedef struct tagConnectSvr{
	byte iSessionIdentity;
	byte iNumber;
}ConnectSvr;


/*
连接服务器服务器的反馈：内存对齐方式为1字节；
协议体中数据项的解释如下：
数据项	说明
iStatus	0—正确，
1—消息结构错误，
2—不支持的身份ID，
3—不支持的编号，
4—相同身份和编号的设备已登录,
5—该身份已被禁止登陆,
6—禁止断线重连,
7~255—其他错误
*/
typedef struct tagConnectSvrResp{
	byte iStatus;
}ConncetSvrResp;


/*
3	同步设备状态
说明：直接与总控服务器相连接的Session如果存在旗下设备则需要负责这些设备与本机设备的状态同步，通常在某设备状态改变时发送
*/
//数据包中命令字和协议体的定义如下：
//命令字：
const byte  MSG_SESSION_STATE = 0x01;
/*协议体：
同步设备状态：内存对齐方式为1字节；
协议体中数据项的解释如下：
数据项	说明
iSessionIdentity	身份类型，取值参见“enum SessionIdentity”
iNumber	身份类型的设备编号
iSessionState	设备当前状态, 取值参见 enum SessionState
*/
typedef struct tagSyncSessionState{
	byte  iSessionIdentity;
	byte  iNumber;
	byte  iSessionState;
}SyncSessionState;


/*
4	案例准备（教练站发这个）
说明：收到该协议包, 说明各系统已经进入案例准备阶段, 进行各自的
初始化工作, 准备工作就绪后, 同步旗下及本机设备的状态为:
SS_CASEPREPARE, 当所有设备都已经进入SS_CASEPREPARE状态时, 总控服务器将发送案例正式开始的协议包通知
*/
//数据包中命令字和协议体的定义如下：
//命令字：
const byte  MSG_SESSION_CASEPREPARE = 0x02;
/*协议体：
同步设备状态：内存对齐方式为1字节；
协议体中数据项的解释如下：
数据项	说明
iCaseMode	仿真案例的模式 :
0—测试模式，
1—仿真模式，
2—录像回放模式，
3~255—其他模式

cCaseName	案例名称
cMapName	案例使用的地图名, 不能超过20字节
*/
typedef struct tagCasePrepare{
	byte  iCaseMode;
	char  cCaseName[21];
	char  cMapName[21];
}CasePrepare;


/*
5	案例启动
说明：收到该协议包, 说明各系统已经正式进入案例仿真环节, 开始接收各种实时数据, 进行各自的处理工作, 收到该包后, 每个设备需同步旗下及本机设备的状态为:
SS_CASERUNNING
*/

//数据包中命令字和协议体的定义如下：
//命令字：
const byte  MSG_SESSION_CASEBEGIN = 0x03;
//协议体：无


/*
6	案例结束
说明：收到该协议包, 各系统停止案例的仿真, 回到大厅状态, 等待新的案例的开启, 收到该包后, 每个设备需同步旗下及本机设备的状态为:
SS_LOBBY
*/

//数据包中命令字和协议体的定义如下：
//命令字：
const byte  MSG_SESSION_CASEEND = 0x04;
//协议体：无


/*
7	关闭设备
说明：收到该协议的设备，需要负责旗下子设备的关机操作，同步旗下的所有设备包括本机设备的状态为SS_NONE（未登陆），  然后执行本机的关机操作
数据包中命令字和协议体的定义如下：
*/
//命令字：
const byte  MSG_SESSION_LOGOUT = 0x05;
//协议体：无


/*
8	转发
说明：收到该协议包, 各系统停止案例的仿真, 回到大厅状态, 等待新的案例的开启, 收到该包后, 每个设备需同步旗下及本机设备的状态为:

数据包中命令字和协议体的定义如下：
*/
//命令字：
const byte  MSG_SESSION_RELAY = 0x06;
//协议体：无


/*
9	心跳包
说明：只有udp的系统需要收发该网络包，证明其在线，频率为1s.如果超过三秒钟，系统未能收到任何数据包，则认为其为掉线状态。
数据包中命令字和协议体的定义如下：
*/
//命令字：
const byte  MSG_SESSION_HEARTBEAT = 0x07;
//协议体：无