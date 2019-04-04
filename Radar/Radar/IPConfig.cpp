#include"stdafx.h"
#include"IPConfig.h"
#include<fstream>

IPConfig::IPConfig()
{
	IP_CoachStation = IP_COACHSTATION;
	IP_ConningService = IP_CONNINGSERVICE;
	IP_SGPanel = IP_SGPANEL;
	IP_PSPanel = IP_PSPANEL;
	IP_LogPanel = IP_LOGPANEL;
	IP_AlarmPanel = IP_ALARMPANEL;
	IP_RealRudder = IP_REALRUDDER;
	IP_VirInstrument = IP_VI;
	IP_ControlServer = IP_CONTROLSERVER;
	IP_Radar = IP_RADAR;

	Port_CoachStation_receive_rudder = PORT_COACHSTATION_RECEIVE_RUDDER;
	Port_CoachStation_receive_telegraph = PORT_COACHSTATION_RECEIVE_TELEGRAPH;
	Port_CoachStation_receive_heartBeat = PORT_COACHSTATION_RECEIVE_HEARTBEAT;
	Port_ConningService_receive_rudder = PORT_CONNINGSERVICE_RECEIVE_RUDDER;
	Port_ConningService_receive_telegraph = PORT_CONNINGSERVICE_RECEIVE_TELEGRAPH;
	Port_ConningService_receiveFrom_CoachStation = PORT_CONNINGSERVICE_RECEIVE_FROM_COACHSTATION;
	Port_ConningService_receiveFrom_SGPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_SGPANEL;
	Port_ConningService_receiveFrom_PSPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_PSPANEL;
	Port_ConningService_receiveFrom_logPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_LOGPANEL;
	Port_ConningService_receive_heartBeat = PORT_CONNINGSERVICE_RECEIVE_HEARTBEAT;
	Port_ConningService_receive = PORT_CONNINGSERVICE_RECEIVE;
	Port_SGPanel_receive = PORT_SGPANEL_RECEIVE;
	Port_PSPanel_receive = PORT_PSPANEL_RECEIVE;
	Port_LogPanel_receive = PORT_LOGPANEL_RECEIVE;
	Port_AlarmPanel_receive = PORT_ALARMPANEL_RECEIVE;
	Port_SGPanel_receive_rudder = PORT_SGPANEL_RECEIVE_RUDDER;
	Port_PSPanel_receive_telegraph = PORT_PSPANEL_RECEIVE_TELEGRAPH;
	Port_RealRudder_receive = PORT_REALRUDDER_RECEIVE;
	Port_Compass_receive = PORT_COMPASS_RECEIVE;
	Port_VI_receive = PORT_VI_RECEIVE;
	Port_ControlServer_receive = PORT_CONTROLSERVER_RECEIVE;
	Port_Radar_receive = PORT_RADAR_RECEIVE;

	SessionNumber = SESSION_NUMBER;

	LoadConfigFile();
}

IPConfig::IPConfig(CString filePath)
{
	IP_CoachStation = IP_COACHSTATION;
	IP_ConningService = IP_CONNINGSERVICE;
	IP_SGPanel = IP_SGPANEL;
	IP_PSPanel = IP_PSPANEL;
	IP_LogPanel = IP_LOGPANEL;
	IP_AlarmPanel = IP_ALARMPANEL;
	IP_RealRudder = IP_REALRUDDER;
	IP_VirInstrument = IP_VI;
	IP_ControlServer = IP_CONTROLSERVER;
	IP_Radar = IP_RADAR;

	Port_CoachStation_receive_rudder = PORT_COACHSTATION_RECEIVE_RUDDER;
	Port_CoachStation_receive_telegraph = PORT_COACHSTATION_RECEIVE_TELEGRAPH;
	Port_CoachStation_receive_heartBeat = PORT_COACHSTATION_RECEIVE_HEARTBEAT;
	Port_ConningService_receive_rudder = PORT_CONNINGSERVICE_RECEIVE_RUDDER;
	Port_ConningService_receive_telegraph = PORT_CONNINGSERVICE_RECEIVE_TELEGRAPH;
	Port_ConningService_receiveFrom_CoachStation = PORT_CONNINGSERVICE_RECEIVE_FROM_COACHSTATION;
	Port_ConningService_receiveFrom_SGPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_SGPANEL;
	Port_ConningService_receiveFrom_PSPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_PSPANEL;
	Port_ConningService_receiveFrom_logPanel = PORT_CONNINGSERVICE_RECEIVE_FROM_LOGPANEL;
	Port_ConningService_receive_heartBeat = PORT_CONNINGSERVICE_RECEIVE_HEARTBEAT;
	Port_ConningService_receive = PORT_CONNINGSERVICE_RECEIVE;
	Port_SGPanel_receive = PORT_SGPANEL_RECEIVE;
	Port_PSPanel_receive = PORT_PSPANEL_RECEIVE;
	Port_LogPanel_receive = PORT_LOGPANEL_RECEIVE;
	Port_AlarmPanel_receive = PORT_ALARMPANEL_RECEIVE;
	Port_SGPanel_receive_rudder = PORT_SGPANEL_RECEIVE_RUDDER;
	Port_PSPanel_receive_telegraph = PORT_PSPANEL_RECEIVE_TELEGRAPH;
	Port_RealRudder_receive = PORT_REALRUDDER_RECEIVE;
	Port_Compass_receive = PORT_COMPASS_RECEIVE;
	Port_VI_receive = PORT_VI_RECEIVE;
	Port_ControlServer_receive = PORT_CONTROLSERVER_RECEIVE;
	Port_Radar_receive = PORT_RADAR_RECEIVE;

	SessionNumber = SESSION_NUMBER;

	LoadConfigFile(filePath);
}


IPConfig::~IPConfig()
{
}

void IPConfig::LoadConfigFile()
{
	LoadConfigFile(_T("IPconfig.txt"));
}

void IPConfig::LoadConfigFile(CString filePath)
{
	std::ifstream inFile;
	inFile.open(filePath);
	if (!inFile.is_open())
		return;

	while (!inFile.eof())
	{
		char head[100], mid[10], tail[50];
		inFile >> head >> mid >> tail;
		if (strcmp(head, "IP_CoachStation") == 0)
			IP_CoachStation = tail;
		else if (strcmp(head, "IP_ConningService") == 0)
			IP_ConningService = tail;
		else if (strcmp(head, "IP_SGPanel") == 0)
			IP_SGPanel = tail;
		else if (strcmp(head, "IP_PSPanel") == 0)
			IP_PSPanel = tail;
		else if (strcmp(head, "IP_LogPanel") == 0)
			IP_LogPanel = tail;
		else if (strcmp(head, "IP_AlarmPanel") == 0)
			IP_AlarmPanel = tail;
		else if (strcmp(head, "IP_RealRudder") == 0)
			IP_RealRudder = tail;
		else if (strcmp(head, "IP_VI") == 0)
			IP_VirInstrument = tail;
		else if (strcmp(head, "IP_ControlServer") == 0)
			IP_ControlServer = tail;
		else if (strcmp(head, "IP_Radar") == 0)
			IP_Radar = tail;
		else if (strcmp(head, "Port_CoachStation_receive_rudder") == 0)
			Port_CoachStation_receive_rudder = atoi(tail);
		else if (strcmp(head, "Port_CoachStation_receive_telegraph") == 0)
			Port_CoachStation_receive_telegraph = atoi(tail);
		else if (strcmp(head, "Port_CoachStation_receive_heartBeat") == 0)
			Port_CoachStation_receive_heartBeat = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receive_rudder") == 0)
			Port_ConningService_receive_rudder = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receive_telegraph") == 0)
			Port_ConningService_receive_telegraph = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receiveFrom_CoachStation") == 0)
			Port_ConningService_receiveFrom_CoachStation = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receiveFrom_SGPanel") == 0)
			Port_ConningService_receiveFrom_SGPanel = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receiveFrom_PSPanel") == 0)
			Port_ConningService_receiveFrom_PSPanel = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receiveFrom_logPanel") == 0)
			Port_ConningService_receiveFrom_logPanel = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receive_heartBeat") == 0)
			Port_ConningService_receive_heartBeat = atoi(tail);
		else if (strcmp(head, "Port_ConningService_receive") == 0)
			Port_ConningService_receive = atoi(tail);
		else if (strcmp(head, "Port_SGPanel_receive") == 0)
			Port_SGPanel_receive = atoi(tail);
		else if (strcmp(head, "Port_PSPanel_receive") == 0)
			Port_PSPanel_receive = atoi(tail);
		else if (strcmp(head, "Port_LogPanel_receive") == 0)
			Port_LogPanel_receive = atoi(tail);
		else if (strcmp(head, "Port_AlarmPanel_receive") == 0)
			Port_AlarmPanel_receive = atoi(tail);
		else if (strcmp(head, "Port_SGPanel_receive_Rudder") == 0)
			Port_SGPanel_receive_rudder = atoi(tail);
		else if (strcmp(head, "Port_PSPanel_receive_Telegraph") == 0)
			Port_PSPanel_receive_telegraph = atoi(tail);
		else if (strcmp(head, "Port_RealRudder_receive") == 0)
			Port_RealRudder_receive = atoi(tail);
		else if (strcmp(head, "Port_Compass_receive") == 0)
			Port_Compass_receive = atoi(tail);
		else if (strcmp(head, "Port_VI_receive") == 0)
			Port_VI_receive = atoi(tail);
		else if (strcmp(head, "Port_ControlServer_receive") == 0)
			Port_ControlServer_receive = atoi(tail);
		else if (strcmp(head, "Port_Radar_receive") == 0)
			Port_Radar_receive = atoi(tail);
		else if (strcmp(head, "SessionNumber") == 0)
			SessionNumber = atoi(tail);
	}


	inFile.close();
}