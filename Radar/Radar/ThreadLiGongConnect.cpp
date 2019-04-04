#include"stdafx.h"
#include"RadarView.h"

BOOL isLogin = FALSE;

//UINT ThreadLogin(LPVOID lpParam)
//{
//	CRadarView* pthis = (CRadarView*)lpParam;
//	BYTE identity = SessionIdentity::SI_BRIDGE_RADAR;
//	BYTE inum = pthis->ISessionNum;
//	
//	while (!isLogin)
//	{
//		//发送当前Session状态
//		pthis->ISessionState = SessionState::SS_DROPPED;
//		pthis->SendSessionState(pthis->ISessionState);
//		//发送登录请求
//		BYTE msg[20];
//		ShipDataService::GetNewHBT(msg, 20, identity, inum);
//		int sendLen = sendto(pthis->sock_LOGIN, (char*)msg, 20, 0, (sockaddr*)&(pthis->addr_sendLOGIN), pthis->addrLen_sendLOGIN);
//
//		Sleep(1000);
//
//		unsigned char buf[100];
//		sockaddr_in remoteHostAddr;
//		int addrLen = sizeof(remoteHostAddr);
//		int getInfoLen = -1;
//
//		for (int i = 0; i < 1000; i++)
//		{
//			getInfoLen = recvfrom(pthis->sock_LOGIN, (char*)buf, 100, 0, (sockaddr*)&remoteHostAddr, &addrLen);
//			if (getInfoLen == 9)
//				break;
//		}
//
//		if (getInfoLen == 9 && ShipDataService::CRCcheck((BYTE*)buf, getInfoLen) 
//			&& MSG_SESSION_LOGIN == (BYTE)buf[2])
//		{ 
//			if (0x00 == (BYTE)buf[6])
//			{
//				while (pthis->pThreadReceive != NULL)
//				{
//					Sleep(100);
//				}
//				isLogin = TRUE;
//				pthis->pThreadReceive = AfxBeginThread(ThreadReceive, pthis);
//				break;
//			}
//			else
//			{
//				if (0x01 == (BYTE)buf[6])
//				{
//					TRACE("消息结构错误！\n");
//				}
//				else if (0x02 == (BYTE)buf[6])
//				{
//					TRACE("不支持的身份ID！\n");
//				}
//				else if (0x03 == (BYTE)buf[6])
//				{
//					TRACE("不支持的身份编号！\n");
//				}
//				else if (0x04 == (BYTE)buf[6])
//				{
//					TRACE("相同身份和编号的设备已登录，请继续尝试。\n");
//				}
//				else if (0x05 == (BYTE)buf[6])
//				{
//					TRACE("该身份已被禁止登录！\n");
//				}
//				else if (0x06 == (BYTE)buf[6])
//				{
//					TRACE("禁止断线重连！\n");
//				}
//				else TRACE("登录失败，其它未知错误！\n");
//			}
//		}	
//	}
//	pthis->pThreadLogin = NULL;
//	return 0;
//}


//UINT ThreadReceive(LPVOID lpParam)
//{
//	CRadarView* pthis = (CRadarView*)lpParam;
//	DWORD timeBegin = GetTickCount();
//	DWORD timeEnd = GetTickCount();
//	//进入当前线程说明已登录成功
//	//发送当前Session状态
//	pthis->ISessionState = SessionState::SS_LOBBY;
//	pthis->SendSessionState(pthis->ISessionState);
//	while (isLogin)
//	{
//		timeEnd = GetTickCount();
//		unsigned char buf[1024];
//		ZeroMemory(buf, 1024);
//		sockaddr_in remoteHostAddr;
//		int addrLen = sizeof(remoteHostAddr);
//		int getInfoLen = -1;
//		for (int i = 0; i < 1000; i++)
//		{
//			getInfoLen = recvfrom(pthis->sock_LOGIN, (char*)buf, 1024, 0, (sockaddr*)&remoteHostAddr, &addrLen);
//			if (getInfoLen > 0) break;
//		}
//		
//		if (getInfoLen > 0 && ShipDataService::CRCcheck((BYTE*)buf, getInfoLen))
//		{
//			timeBegin = GetTickCount();
//			timeEnd = GetTickCount();
//			if (MSG_SESSION_CASEPREPARE == (BYTE)buf[2])	//切换状态为准备
//			{
//				pthis->ISessionState = SessionState::SS_CASEPREPARE;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_CASEBEGIN == (BYTE)buf[2])		//切换状态为运行
//			{
//				pthis->ISessionState = SessionState::SS_CASERUNNING;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_CASEEND == (BYTE)buf[2])		//切换状态为大厅等待
//			{
//				pthis->ISessionState = SessionState::SS_LOBBY;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_LOGOUT == (BYTE)buf[2])	//执行关机操作
//			{
//				pthis->ISessionState = ShipDataService::SessionState::SS_DROPPED;
//				pthis->SendSessionState(pthis->ISessionState);
//				//执行关机操作
//				isLogin = FALSE;
//			}
//			else if (MSG_SESSION_RELAY == (BYTE)buf[2])	//解析AIS
//			{
//				int msgLen = (int)buf[0] + (int)buf[1] * 256;
//				char str[1024];
//				//截取buf的6至msgLen--3位
//				int i = 6, j = 0;
//				for (; i <= msgLen - 3; i++, j++)
//				{
//					str[j] = buf[i];
//				}
//				str[j] = '\0';
//				CString strReceived = CString(str);
//
//				while (!strReceived.IsEmpty())
//				{
//					int nIndex = strReceived.Find('\n');
//					if (-1 != nIndex)
//					{
//						ais_t ais = { 0 };
//						static struct aivdm_context_t aivdm = { 0 };
//
//						//分割
//						CString strPacket = strReceived.Mid(0, nIndex);
//						USES_CONVERSION;
//						LPSTR str = T2A(strPacket);
//						//删除
//						strReceived.Delete(0, nIndex + 1);//删除时把整个AIS语句包括'\n'也删除
//						//开始解析咯
//						if (false == aivdmo_decode(str, strPacket.GetLength(), &aivdm, &ais))
//						{
//							//如果失败的原因是因为该AIS信息有几份，那么这个aivdm_context_t类型的aivdm就不清空，保留本次解析
//							//数据，否则则应该清空
//							if (aivdm.await == aivdm.part)//非因为AIS信息被划分为几份而失败时则需清空aivdm中数据
//								memset(&aivdm, 0, sizeof(aivdm));//aivdm = {0};
//							continue;
//						}
//						//解析成功则立即清空aivdm数据
//						memset(&aivdm, 0, sizeof(aivdm));
//						pthis->GetAisMessage(&ais);
//					}
//					else
//						break;
//				}
//
//			}
//		}
//		else
//		{
//			if (abs((int)(timeBegin - timeEnd))>10000)
//				isLogin = FALSE;
//		}
//		Sleep(0);
//	}
//	//运行到这一步时，说明服务器掉线，返回登录前状态
//	isLogin = FALSE;
//	pthis->ISessionState = SessionState::SS_DROPPED;
//	pthis->SendSessionState(pthis->ISessionState);
//	while (pthis->pThreadLogin != NULL)
//	{
//		Sleep(100);
//	}
//	if (pthis->pThreadLogin != NULL)
//	{
//		TRACE("服务器已掉线，返回重新登陆，但登录线程未退出！\n");	
//	}
//	pthis->pThreadReceive = NULL;
//	pthis->pThreadLogin = AfxBeginThread(ThreadLogin, pthis);
//	return 0;
//}