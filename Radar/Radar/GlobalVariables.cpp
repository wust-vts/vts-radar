#include"stdafx.h"
#include"ShipInfo.h"
#include"vtsDEM.h"
#include"vtsRadar.h"
#include"Host.h"
#include<vector>
#include<mutex>

using namespace std;

vector <SHIP> g_aisShips;	//�ṹ��vectorҪ����Ϊȫ��

int m_vtsRadar_selected = -1; //��ѡ�е�VtsRadar��m_vtsRadars�е�λ��
mutex mtx_vtsRadar_selected; //m_vtsRadar_selected�Ļ�����
mutex mtx_BMPmem;  //�״�ͼ��Ļ�����
mutex mtx_aisShips; //g_aisShips�Ļ�����
mutex mtx_vtsRadars;

BlackHost blackHost; //��Ч��ip�˿��б�

HANDLE closeWindowEvent;
HANDLE closeThread_loadDemFile_event;
HANDLE closeThread_threadCalculateARPARecord;
HANDLE closeThread_drawDemBMP;
HANDLE rangeUpdate;