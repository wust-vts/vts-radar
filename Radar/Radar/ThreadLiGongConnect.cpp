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
//		//���͵�ǰSession״̬
//		pthis->ISessionState = SessionState::SS_DROPPED;
//		pthis->SendSessionState(pthis->ISessionState);
//		//���͵�¼����
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
//					TRACE("��Ϣ�ṹ����\n");
//				}
//				else if (0x02 == (BYTE)buf[6])
//				{
//					TRACE("��֧�ֵ����ID��\n");
//				}
//				else if (0x03 == (BYTE)buf[6])
//				{
//					TRACE("��֧�ֵ���ݱ�ţ�\n");
//				}
//				else if (0x04 == (BYTE)buf[6])
//				{
//					TRACE("��ͬ��ݺͱ�ŵ��豸�ѵ�¼����������ԡ�\n");
//				}
//				else if (0x05 == (BYTE)buf[6])
//				{
//					TRACE("������ѱ���ֹ��¼��\n");
//				}
//				else if (0x06 == (BYTE)buf[6])
//				{
//					TRACE("��ֹ����������\n");
//				}
//				else TRACE("��¼ʧ�ܣ�����δ֪����\n");
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
//	//���뵱ǰ�߳�˵���ѵ�¼�ɹ�
//	//���͵�ǰSession״̬
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
//			if (MSG_SESSION_CASEPREPARE == (BYTE)buf[2])	//�л�״̬Ϊ׼��
//			{
//				pthis->ISessionState = SessionState::SS_CASEPREPARE;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_CASEBEGIN == (BYTE)buf[2])		//�л�״̬Ϊ����
//			{
//				pthis->ISessionState = SessionState::SS_CASERUNNING;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_CASEEND == (BYTE)buf[2])		//�л�״̬Ϊ�����ȴ�
//			{
//				pthis->ISessionState = SessionState::SS_LOBBY;
//				pthis->SendSessionState(pthis->ISessionState);
//			}
//			else if (MSG_SESSION_LOGOUT == (BYTE)buf[2])	//ִ�йػ�����
//			{
//				pthis->ISessionState = ShipDataService::SessionState::SS_DROPPED;
//				pthis->SendSessionState(pthis->ISessionState);
//				//ִ�йػ�����
//				isLogin = FALSE;
//			}
//			else if (MSG_SESSION_RELAY == (BYTE)buf[2])	//����AIS
//			{
//				int msgLen = (int)buf[0] + (int)buf[1] * 256;
//				char str[1024];
//				//��ȡbuf��6��msgLen--3λ
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
//						//�ָ�
//						CString strPacket = strReceived.Mid(0, nIndex);
//						USES_CONVERSION;
//						LPSTR str = T2A(strPacket);
//						//ɾ��
//						strReceived.Delete(0, nIndex + 1);//ɾ��ʱ������AIS������'\n'Ҳɾ��
//						//��ʼ������
//						if (false == aivdmo_decode(str, strPacket.GetLength(), &aivdm, &ais))
//						{
//							//���ʧ�ܵ�ԭ������Ϊ��AIS��Ϣ�м��ݣ���ô���aivdm_context_t���͵�aivdm�Ͳ���գ��������ν���
//							//���ݣ�������Ӧ�����
//							if (aivdm.await == aivdm.part)//����ΪAIS��Ϣ������Ϊ���ݶ�ʧ��ʱ�������aivdm������
//								memset(&aivdm, 0, sizeof(aivdm));//aivdm = {0};
//							continue;
//						}
//						//�����ɹ����������aivdm����
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
//	//���е���һ��ʱ��˵�����������ߣ����ص�¼ǰ״̬
//	isLogin = FALSE;
//	pthis->ISessionState = SessionState::SS_DROPPED;
//	pthis->SendSessionState(pthis->ISessionState);
//	while (pthis->pThreadLogin != NULL)
//	{
//		Sleep(100);
//	}
//	if (pthis->pThreadLogin != NULL)
//	{
//		TRACE("�������ѵ��ߣ��������µ�½������¼�߳�δ�˳���\n");	
//	}
//	pthis->pThreadReceive = NULL;
//	pthis->pThreadLogin = AfxBeginThread(ThreadLogin, pthis);
//	return 0;
//}