#include "stdafx.h"
#include "Thread.h"
#include "RadarView.h"
#include "vtsRadar.h"
#include"PublicMethod.h"
#include<mutex>
#include<map>
#include<vector>
#include<exception>
#include<thread>

#define RECEIVE_VTS_RADAR_SETTING_PORT 8888

//std::mutex mtx_mempic;
//bool isDrawVtsStart = false;

extern std::mutex mtx_vtsRadars;
extern std::vector<SHIP> g_aisShips;

UINT ThreadReceiveVtsRadarSetting(LPVOID lpParam){
	CRadarView * pThis = (CRadarView*)lpParam;
	//��ʼ��winsocket  
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//��һ������Ϊ��λ�ֽڣ��ڶ�������Ϊ��λ�ֽ�  

	err = WSAStartup(wVersionRequested, &wsaData);//��winsock DLL����̬���ӿ��ļ������г�ʼ����Э��Winsock�İ汾֧�֣��������Ҫ����Դ��  
	if (err != 0)
	{
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//LOBYTE����ȡ��16���������λ��HIBYTE����ȡ��16��������ߣ�����ߣ��Ǹ��ֽڵ�����        
	{
		WSACleanup();
		return -1;
	}

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);//����socket��AF_INET��ʾ��Internet��ͨ�ţ�SOCK_STREAM��ʾsocket�����׽��֣���Ӧtcp��0ָ������Э��ΪTCP/IP  

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //htonl�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network long)  
	//INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ,  
	//��ʾ��ȷ����ַ,�������ַ������  
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(RECEIVE_VTS_RADAR_SETTING_PORT);//htons�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network short)  

	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//�����ص�ַ�󶨵���������socket�ϣ���ʹ�������ϱ�ʶ��socket  

	listen(sockSrv, 5);//socket������׼��������������  

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (1)
	{
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);//Ϊһ�����������ṩ����addrClient�����˷�����������Ŀͻ���IP��ַ��Ϣ�����ص���socket������������ÿͻ���������  

		char recvBuf[256];
		recv(sockConn, recvBuf, 256, 0);
		decodeVtsRadarSMsg(recvBuf, (pThis->m_vtsRadar));
		//TRACE("%s\n", recvBuf);

		//char sendBuf[1024];
		std::string msg = getVtsRadarSAMsg(pThis->m_vtsRadar);
		send(sockConn, msg.data(), strlen(msg.data()) + 1, 0);

		closesocket(sockConn);
		Sleep(1000);//1000����  
	}
	WSACleanup();
	return 0;
}

UINT ThreadSendVtsRadarData(LPVOID lpParam){
	const CRadarView * pThis = (CRadarView*)lpParam;
	while (true)
	{
		DWORD t1 = GetTickCount();
		ThreadSendVtsRadarData(pThis->m_vtsRadar);
		DWORD t2 = GetTickCount();
		DWORD t = t2 - t1;
		Sleep(3000);
	}
}

void ThreadSendVtsRadarData(std::vector<VtsRadar> vtsRadars){
	DWORD t1 = GetTickCount();
	/*for (std::vector<VtsRadar>::const_iterator it = vtsRadars.begin(); it != vtsRadars.end(); it++){
		std::thread t([it]{
			it->sendVtsRadarData();
		});
		t.detach();	
	}*/
	for (int i = 0; i < vtsRadars.size(); i++){
		const VtsRadar& vtsRadar = vtsRadars[i];
		/*std::thread t([vtsRadar]{
			try{
				vtsRadar.sendVtsRadarData();
			}
			catch (std::system_error &e){
				TRACE(e.what());
			}
		});
		t.detach();*/
		vtsRadar.sendVtsRadarData();
	}
	DWORD t2 = GetTickCount();
	DWORD t = t2 - t1;
}

UINT ThreadGenerateVtsRadarPic(LPVOID lpParam){
	CRadarView * pThis = (CRadarView*)lpParam;
	while (1){
		DWORD t1 = GetTickCount();
		if (mtx_vtsRadars.try_lock()){
			for (int i = 0; i < pThis->m_vtsRadar.size(); i++){
				pThis->m_vtsRadar[i].generateRadarWithShipPic(g_aisShips);
			}
			mtx_vtsRadars.unlock();
		}
		DWORD t2 = GetTickCount();
		DWORD t = t2 - t1;
		Sleep(2000);
	}
}