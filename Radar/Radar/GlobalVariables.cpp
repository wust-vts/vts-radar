#include"stdafx.h"
#include"ShipInfo.h"
#include"vtsDEM.h"
#include"vtsRadar.h"
#include"Host.h"
#include<vector>
#include<mutex>

using namespace std;

vector <SHIP> g_aisShips;	//结构体vector要定义为全局

int m_vtsRadar_selected = -1; //被选中的VtsRadar在m_vtsRadars中的位置
mutex mtx_vtsRadar_selected; //m_vtsRadar_selected的互斥锁
mutex mtx_BMPmem;  //雷达图像的互斥锁
mutex mtx_aisShips; //g_aisShips的互斥锁
mutex mtx_vtsRadars;

BlackHost blackHost; //无效的ip端口列表

HANDLE closeWindowEvent;
HANDLE closeThread_loadDemFile_event;
HANDLE closeThread_threadCalculateARPARecord;
HANDLE closeThread_drawDemBMP;
HANDLE rangeUpdate;