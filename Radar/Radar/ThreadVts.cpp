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
	//初始化winsocket  
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//第一个参数为低位字节；第二个参数为高位字节  

	err = WSAStartup(wVersionRequested, &wsaData);//对winsock DLL（动态链接库文件）进行初始化，协商Winsock的版本支持，并分配必要的资源。  
	if (err != 0)
	{
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//LOBYTE（）取得16进制数最低位；HIBYTE（）取得16进制数最高（最左边）那个字节的内容        
	{
		WSACleanup();
		return -1;
	}

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);//创建socket。AF_INET表示在Internet中通信；SOCK_STREAM表示socket是流套接字，对应tcp；0指定网络协议为TCP/IP  

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //htonl用来将主机字节顺序转换为网络字节顺序(to network long)  
	//INADDR_ANY就是指定地址为0.0.0.0的地址,  
	//表示不确定地址,或“任意地址”。”  
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(RECEIVE_VTS_RADAR_SETTING_PORT);//htons用来将主机字节顺序转换为网络字节顺序(to network short)  

	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//将本地地址绑定到所创建的socket上，以使在网络上标识该socket  

	listen(sockSrv, 5);//socket监听，准备接受连接请求。  

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	while (1)
	{
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);//为一个连接请求提供服务。addrClient包含了发出连接请求的客户机IP地址信息；返回的新socket描述服务器与该客户机的连接  

		char recvBuf[256];
		recv(sockConn, recvBuf, 256, 0);
		decodeVtsRadarSMsg(recvBuf, (pThis->m_vtsRadar));
		//TRACE("%s\n", recvBuf);

		//char sendBuf[1024];
		std::string msg = getVtsRadarSAMsg(pThis->m_vtsRadar);
		send(sockConn, msg.data(), strlen(msg.data()) + 1, 0);

		closesocket(sockConn);
		Sleep(1000);//1000毫秒  
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