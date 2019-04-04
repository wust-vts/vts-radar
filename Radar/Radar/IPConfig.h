#pragma once;

#include"stdafx.h"

#define PORT_COACHSTATION_RECEIVE_RUDDER	8007
#define PORT_COACHSTATION_RECEIVE_HEARTBEAT	7004
#define PORT_COACHSTATION_RECEIVE_TELEGRAPH	8005
#define PORT_CONNINGSERVICE_RECEIVE_RUDDER	8007
#define PORT_CONNINGSERVICE_RECEIVE_TELEGRAPH	8005
#define PORT_CONNINGSERVICE_RECEIVE_FROM_COACHSTATION	10000
#define PORT_CONNINGSERVICE_RECEIVE_FROM_SGPANEL	10001
#define PORT_CONNINGSERVICE_RECEIVE_FROM_PSPANEL	10002
#define PORT_CONNINGSERVICE_RECEIVE_FROM_LOGPANEL	10003
#define PORT_CONNINGSERVICE_RECEIVE_HEARTBEAT		10004
#define PORT_CONNINGSERVICE_RECEIVE					10005
#define PORT_SGPANEL_RECEIVE 11000
#define PORT_SGPANEL_RECEIVE_RUDDER 11001
#define PORT_PSPANEL_RECEIVE	12000
#define PORT_PSPANEL_RECEIVE_TELEGRAPH 12001
#define PORT_LOGPANEL_RECEIVE	13000
#define PORT_ALARMPANEL_RECEIVE	14000
#define PORT_REALRUDDER_RECEIVE	10007
#define PORT_COMPASS_RECEIVE	10008
#define PORT_VI_RECEIVE			9999
#define PORT_CONTROLSERVER_RECEIVE	8777
#define PORT_RADAR_RECEIVE		7777

#define IP_COACHSTATION		"192.168.0.92"
#define IP_CONNINGSERVICE	"192.168.0.4"
#define IP_SGPANEL			"192.168.0.201"
#define IP_PSPANEL			"192.168.0.202"
#define IP_LOGPANEL			"192.168.0.203"
#define IP_ALARMPANEL		"192.168.0.204"
#define IP_REALRUDDER		"192.168.0.233"
#define IP_VI				"192.168.0.5"
#define IP_CONTROLSERVER	"192.168.0.99"
#define IP_RADAR			"192.168.0.4"

#define SESSION_NUMBER 0x03

class IPConfig
{
public:
	IPConfig();
	IPConfig(CString filePath);
	~IPConfig();

private:
	void LoadConfigFile(CString filePath);
	void LoadConfigFile();

public:
	CString IP_CoachStation;
	CString IP_ConningService;
	CString IP_SGPanel;
	CString IP_PSPanel;
	CString IP_LogPanel;
	CString IP_AlarmPanel;
	CString IP_RealRudder;	//控制台硬件IP，包括车、舵、罗经盘等
	CString IP_VirInstrument;
	CString IP_ControlServer;
	CString IP_Radar;

	u_short Port_CoachStation_receive_rudder;
	u_short Port_CoachStation_receive_telegraph;
	u_short Port_CoachStation_receive_heartBeat;
	u_short Port_ConningService_receive_rudder;
	u_short Port_ConningService_receive_telegraph;
	u_short Port_ConningService_receiveFrom_CoachStation;
	u_short Port_ConningService_receiveFrom_SGPanel;
	u_short Port_ConningService_receiveFrom_PSPanel;
	u_short Port_ConningService_receiveFrom_logPanel;
	u_short Port_ConningService_receive_heartBeat;
	u_short Port_ConningService_receive;
	u_short Port_SGPanel_receive;	//接收一般数据（除Rudder）
	u_short Port_PSPanel_receive;	//接收一般数据（除Telegraph）
	u_short Port_LogPanel_receive;
	u_short Port_AlarmPanel_receive;
	u_short Port_SGPanel_receive_rudder;	//专门用于接收Rudder
	u_short Port_PSPanel_receive_telegraph;	//专门用于接收Telegraph
	u_short Port_RealRudder_receive;		//真实舵端口
	u_short Port_Compass_receive;			//真实罗经接收数据端口
	u_short Port_VI_receive;
	u_short Port_ControlServer_receive;
	u_short Port_Radar_receive;

	byte SessionNumber;
};

