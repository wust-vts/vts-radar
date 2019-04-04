
// RadarView.h : CRadarView ��Ľӿ�
//

#pragma once
#include"ShipInfo.h"
#include"QuadTree.h"
#include "aisdecode.h"
#include"ShipDataService.h"
#include <vector>
#include<list>
#include <map>
#include"DemTools.h"
#include"BitmapEx.h"
#include"login.h"
#include"RadarDoc.h"
#include"vtsRadar.h"
#include"MyMemDC.h"

#define RADAR_CYCLE 2		//�״�ɨ������
//#define DEMBMP_WIDTH 900
//#define DEMBMP_HEIGHT 900
//#define RADAR_PIXEL_RADIUS 450
#define OTHERSHIP_MAXNUM 100
#define TIMER_SYSTEMTIME	10000
//#define TIMER_AIS_FROM_NET  10001
#define TIMER_AIS_DECODE    10002
#define TIMER_AIS_CHECK		10003		//��ʱ���Ŀ�괬��AIS�Ƿ����״̬
#define TIMER_AIS_CLEAR	10004	//��ʱ���Ŀ�괬������ɾ��Ŀ�괬��ô���Ϣ������
#define TIMER_REFRESH_RADAR	10005		//��ʱˢ��ͼ����ʾ����
#define TIMER_CALCULATE_ARPARECORD 10006	//��ʱ�����̼߳���m_arpaRecord
#define TIMER_SENDHBT	10007	//������
#define TIMER_CHECKIP 10008
#define TIMER_DRAW_VTS_RADAR 10009 //����vts�״�ͼ��
#define TIMER_GENERATE_VTS_PIC 10010 //����vts�״�ͼ�񣬴�����
#define TIMER_SEND_VTS_RADAR_DATA 10011//��vts�豸�����״�����
#define BUTTONNUM	60
#define PI 3.1415927
#define RANGE1	0.125
#define RANGE2	0.25
#define RANGE3	0.5
#define RANGE4	0.75
#define RANGE5	1.5
#define RANGE6	3
#define RANGE7	6
#define RANGE8	12
#define RANGE9	24
#define RANGE10	48
#define RADAR_HIGHT 15  //�״�߶�

enum ARPA_RECORD_TARGETSTATE
{
	ARPA_RECORD_STATE_NORMAL = 0,		//Ŀ���ȶ���������ϣ�
	ARPA_RECORD_STATE_MISSING,		//Ŀ�궪ʧ
	ARPA_RECORD_STATE_CALCULATING,	//Ŀ�������
//	ARPA_RECORD_SELECTED
};

enum ARPA_RECORD_PASSTATE
{
	ARPA_RECORD_PASSSTEM = 0,	//������
	ARPA_RECORD_COLLISION,		//��ײ
	ARPA_RECORD_PASSSTERN,		//����β
	ARPA_RECORD_NOCROSSLINE,	//���ཻ����
	ARPA_RECORD_NOCOLLISION		//����ײ
};

enum FIND_GRAVITY_RESULT
{
	RESULT_OK = 0,		//�ҵ�Ŀ�꣬��Ŀ���ڷ�����
	RESULT_LARGER,		//�ҵ�Ŀ�꣬��Ŀ�����ֻ��һ�����ڷ����ڣ���������������
	RESULT_MISSING		//δ�ҵ�Ŀ��
};

enum GATE_SIZE
{
	GATE_SIZE_SMALL = 0,
	GATE_SIZE_MEDIUM,
	GATE_SIZE_LARGE
};

typedef struct ARPA_RECORD{
	SHIP* AISship = NULL;
	UINT index = 0;		//������ʾ���±�
	Rect rectBody;
	Rect rectIndex;
	ARPA_RECORD_TARGETSTATE state = ARPA_RECORD_STATE_CALCULATING;

	//�Ǵ���ʱʹ������ֵ
	double longaitude = 0;
	double latitude = 0;
	CPoint center;
	int scrX = 0;					//��ǰ�������ĵ�����X
	int scrY = 0;
	double DadiX = 0;		//��ǰ�������ĵ�������X
	double DadiY = 0;		
	double length_Meter = 0;		//�����ʵ�ʳ��ȣ��ף�
	double length_Pixel = 0;		//��������س��ȣ����أ�

	//����
	double distance = 0;	//�������ľ���(��λ������)
	double bearing = 0;		//��λ
	double TSpeed = 0;		//���ٶ�
	double TCourse = 0;		//�溽��
	double RSpeed = 0;		//����ٶ�
	double RCourse = 0;		//��Ժ���
	ARPA_RECORD_PASSTATE passState;	//����������λ��
	double CPA = 0;			//�����������
	double TCPA = 0;		//��̻���ʱ��
	double collisionRadius = 0;			//��ײ�뾶���ף�
	int attackTime = 0;		//��ײʱ�䣨�룩

	//���ڦ�-���˲�����
	GATE_SIZE gateSize = GATE_SIZE_MEDIUM;	//���Ŵ�С
	int scanTimes = 0;		//�Ӽ�¼��ʼ��ɨ��Ĵ���
	int lossTimes = 0;		//������ʧĿ��Ĵ���
	int lastPosX = -1;	//�ϴμ�¼������X����Ļ���꣩
	int lastPosY = -1;
	double lastDadiX = -1;	//�ϴμ�¼�Ĵ������X
	double lastDadiY = -1;
	int thisPosX = -1;	//��μ�¼������X����Ļ���꣩
	int thisPosY = -1;
	double thisDadiX = -1;	//��μ�¼�Ĵ������X
	double thisDadiY = -1;
	int thisSmoothPosX = -1;	//��Ԥ��λ�ú�ʵ��λ�õõ���ƽ����Ļ����X
	int thisSmoothPosY = -1;
	double thisSmoothDadiX = -1;	//��Ԥ��λ�ú�ʵ��λ�õõ���ƽ���������X
	double thisSmoothDadiY = -1;
	int nextPredictPosX = -1;	//Ԥ���´ε�������X(��Ļ����)
	int nextPredictPosY = -1;
	double nextPredictDadiX = -1;	//Ԥ���´ε���Ĵ������X
	double nextPredictDadiY = -1;
	double speedX = -1;	//Ŀ���ٶ��ڴ������X�����ϵķ�������λm/s
	double speedY = -1;
	double smoothSpeedX = -1;	//Ŀ��ƽ���ٶ��ڴ������X�����ϵķ�������λm/s
	double smoothSpeedY = -1;
	CRect gateRect;
};

typedef struct Demdata
{
	float* pdemUseData = NULL;
	bool* pdem_check = NULL;
	int dem_row = 0;
	int dem_col = 0;
};

class CRadarView : public CView
{
protected: // �������л�����
	CRadarView();
	DECLARE_DYNCREATE(CRadarView)

// ����
private:
	//SOCKET sock_sendHeartBeat = -1;	//�������������׽���
	//sockaddr_in addr_sendHeartBeat;
	//int addrLen_sendHeartBeat = 0;
	CString m_ip;	//����󶨵ı���ip��ַ
	std::vector<CString> locIPs;	//�������õ�IP��ַ����
public:
	//SOCKET sock_LOGIN = -1;

	sockaddr_in addr_sendLOGIN;
	int addrLen_sendLOGIN = 0;

	sockaddr_in addr_recLOGIN;
	int addrLen_recLOGIN = 0;

	//ShipDataService::SessionState iSessionState = ShipDataService::SessionState::SS_LOBBY;
	//���ڵ�¼������
	byte ISessionState = SessionState::SS_NONE;	//�豸״̬
	byte ISessionNum = 0x01;  //�豸���

	CRadarDoc* GetDocument() const;
	MyMemDC * pMemdc;
	GdiplusStartupInput m_gdiplusstartupinput;
	ULONG_PTR m_gdiplusToken;
	SOCKET m_socket;	//�׽���
	CString m_strReceived;//���մ��ڴ��ص��ַ�

	CRect m_viewRect;	//��ͼ����
	CRgn m_radarRgn;	//�״���ͼ��Բ������
	CRgn m_radarRgn_Mid;	//�Դ�һ����״���ͼԲ��������������������Ȧ��������m_radarRgn�ڵ�����
	CRgn m_radarRgn_Larger;	//��Mid��һ����״���ͼԲ�������������̶�
	CRgn m_buttonRgn;	//��ť����
	CRect m_AISControlBoxRect;	//AIS���ƿ�����
	CRgn m_AISControlBoxRgn;	//AIS���ƿ�����
	CRect m_ARPAControlBoxRect;	//ARPA���ƿ�����
	CRgn m_ARPAControlBoxRgn;	//ARPA���ƿ�����
	CRect m_radarRect;	//�״���ͼ�ľ�������
	Point m_radarCenter;	//�״����ĵ�����
	int m_radiusP;	//�״�뾶�����أ�
	double m_radiusM;	//�״�뾶���ף�
	CBrush blackBrush;	//��ɫ������ˢ
	CBrush blueBrush;	//��ɫ������ˢ
	COLORREF color_blue;
	COLORREF color_yellow;
	CRect m_arpaRecordRect;	//APRA¼ȡʱ����������ʶ�𴬲��Ƿ��ڸ�������

	enum MessageArea_1_Parameter
	{
		MA_1_BLANK = 0,		//�������
		MA_1_EXPAND,		//չ��ͼ����ʾ
		MA_1_ARPA_ON,		//��ARPA��ʾ
		MA_1_ARPA_OFF,		//�ر�ARPA��ʾ
		MA_1_ARPA_INFO,		//��ʾ����ARPA��Ϣ
		MA_1_AIS_ON,		//��AIS��ʾ
		MA_1_AIS_OFF,		//�ر�AIS��ʾ
		MA_1_AIS_INFO,		//��ʾ����AIS��Ϣ
	};

	//SHIP m_MainShip;
	SHIP *m_aisShipSelected;	//��ѡ�е�aisĿ�괬
	SHIP m_blankShip;			//����Ϣ�Ĵ�
	ARPA_RECORD * m_ArpaRecordSelected = NULL;	//��ѡ�е�ARPAĿ��
	ARPA_RECORD m_blankArpaRecord;	//����Ϣ��ARPARECORD

	std::vector<VtsRadar> m_vtsRadar;//vtsRadar�б�
	int m_vtsRadar_first_index = 0; //��ʾ�б�����ʾ��5��vtsRadar�ӵڼ�����ʼ��ʾ��ֻ��m_vtsRadar.size()>5ʱ���ܲ�Ϊ0
	
	int m_radarCycle = 2;		//�״�ɨ������
	int m_delayTimes = 0;
	CRect  m_Button[BUTTONNUM];
	bool m_radarRgnLBDownBeUsed = false;	//���״������ڵ������������Ƿ�ռ�ã������湦�ܺ�APRA/AIS���ܳ�ͻ��
	double m_mulOf800_600 = 0;	//��ǰ�ֱ��������800*600�ı���
	double m_perPixelNM = 0;	//��λ���صľ��루���
	double m_perPixelKM = 0;	//��λ���صľ��루ǧ�ף�
	double m_perPixelM = 0;		//��λ���صľ��루�ף�
	bool m_radarSelect = false;	//����Ƿ�����״�������
	bool m_ButtonSelect[BUTTONNUM]; //button�Ƿ񱻵��
	bool m_dayOrNight = false;	//trueΪday
	bool m_language = true;	//trueΪ����
	//double m_range = RANGE5;		//����(����)
	//double m_rangeRingsDistance = 0;	//���ľ���
	bool m_rangeRingsSwitch = false;	//�̶����Ȧ����
	int m_rangeRingsNum = 5;	//�̶����Ȧ������
	bool m_offCentreSwitch = false;	//ƫ�Ŀ���
	bool m_headingLineSwitch = true;	//���߿���
	double m_headingLineAngle = 0;	//��������Ļ���Ϸ��ľ��Լн�(����)
	double m_northAngle = 0;		//����������Ļ���Ϸ��ľ��Լн�(����)
	double m_COGAngle = 0;		//�����Եغ�������Ļ���Ϸ��ľ��Լн�(����)(������ʱû�жԵغ�����Ϣ������Ϊ�ʹ�����һ��)
	double m_VRM1distance = 0;	//VRM1��ֵ(����)
	double m_VRM2distance = 0;	//VRM2��ֵ(����)
	int m_VRM1Pixel = 0;	//VRM1�뾶(����)
	int m_VRM2Pixel = 0;	//VRM2�뾶(����)
	bool m_variableRangeMarker1Switch = false;	//VRM1����
	bool m_variableRangeMarker2Switch = false;	//VRM2����
	bool m_VRM1Roam = false;	//VRM1����
	bool m_VRM2Roam = false;	//VRM2����
	double m_electronicBearingLine1Num = 0;	//���ӷ�λ��1��ֵ(�봬����н�)(�Ƕ�)
	double m_electronicBearingLine1Angle = 0;	//���ӷ�λ��1�н�(����Ļ���Ϸ��ľ��Լн�)(����)
	double m_electronicBearingLine2Num = 0;	//���ӷ�λ��2��ֵ(�봬����н�)(�Ƕ�)
	double m_electronicBearingLine2Angle = 200;	//���ӷ�λ��2�н�(����Ļ���Ϸ��ľ��Լн�)(����)
	bool m_electronicBearingLine1Switch = false;	//���ӷ�λ��1����
	bool m_electronicBearingLine2Switch = false;	//���ӷ�λ��2����
	bool m_EBL1Roam = false;		//���ӷ�λ��1����
	bool m_EBL2Roam = false;		//���ӷ�λ��2����
	bool m_extendedRangeMeasurementSwitch = false;	//Ŀ���࿪��
	bool m_ERMFunctionSwitch = false;		//��๦�ܿ���
	bool m_ERM_Roam = false;		//�������
	CPoint m_ERM_P1;				//������
	CPoint m_ERM_P2;				//����յ�
	double m_targetBearingNum = 0;	//��෽λ
	double m_targetRangeNum = 0;	//������
	bool m_alarm1Switch = false;	//Alarm1����
	bool m_alarm2Switch = false;	//Alarm2����
	bool m_ECHOStretchSwitch = false;	//չ����
	bool m_motionMode = true;	//trueΪ����˶�
	bool m_displayMode = true;		//trueΪ������,falseΪ������
	bool m_vectorTimeSwitch = false;	//ʸ��ʱ�俪��
	double m_vectorTimeNum = 0;		//ʸ��ʱ����ֵ
	bool m_trackTimeSwitch = false;		//����ʱ�俪��
	bool m_CPAAlarmDistanceSwitch = false;	//CPA�������뿪��
	bool m_TCPAAlarmTimeSwitch = false;	//TCPA����ʱ�俪��
	int m_gainNum = 0;		//����	0~100
	int m_waveNum = 0;		//����	0~100
	int m_rainNum = 0;		//��ѩ	0~100
	int m_tuneNum = 0;		//��г	0~100
	MessageArea_1_Parameter m_MessageArea_1_Parameter = MA_1_BLANK;	//��Ϣ��1����
	double m_targetRange = 0;	//Ŀ�����
	double m_targetBearing_T = 0;	//Ŀ��㵽Բ�ĵ���������������ļн�
	double m_targetBearing_R = 0;	//Ŀ��㵽Բ�ĵ������봬���ߵļн�
	double m_targetLongitude = 0;	//Ŀ��㾭��
	double m_targetLatitude = 0;	//Ŀ���γ��
	bool m_echoAveragingSwitch = false; //�Ӳ����ƿ���
	bool m_startOrStop = true;	//trueΪSTART
	CString m_systemTime;	//ϵͳʱ��
	bool m_ARPASwitch = false;	//APRA����
	bool m_ARPARecordTargetSwitch = false;	//ARPA¼ȡĿ�꿪��
	bool m_ARPAVectorTimeSwitch = false;	//ARPAʸ��ʱ�俪��
	double m_ARPAVectorTimeNum = 0;			//ARPAʸ��ʱ����ֵ
	bool m_ARPA_T_R_VectorSwitch = true;	//ARPA��/���ʸ���л���trueΪ��ʸ����
	bool m_ARPAControlBoxDisplay = false;	//ARPA���ƿ���ʾ
	bool m_AISSwitch = false;	//AIS����
	bool m_AISControlBoxDisplay = false;	//AIS���ƿ���ʾ
	bool m_AISInformationSwitch = false;	//AIS��Ϣ��ʾ����
	bool m_AISControlBoxRoam = false;	//�ж�����Ƿ��״ν���AIS���ƿ�
	bool m_messageArea1BeUsed = false;	//��Ϣ��1�Ƿ�ռ��
	GATE_SIZE m_ARPARecordCur = GATE_SIZE_MEDIUM;		//1��2��3�ֱ�ΪС���С���

//	std::vector<CString> previousFileNames;	//��һ���������ļ���(���ݴ���ʱʹ�õ�)
//	std::vector<CString> presentFileNames;	//��ǰ�������ļ�������ʱ�ģ����ڼ���жϣ�
//	QuadTree m_quadTree;	//�����ļ���Ϣ�ķ����Ĳ���(���������ļ�)
//	QuadTree m_fullTree;	//�������ļ���Ϣ�����Ĳ��������ڼ���BOX��
//	MapRect previousRadarBox;	//��һ���״ﷶΧ���ڵ���С�ļ�����(���ݴ���ʱʹ�õ�)
//	MapRect presentRadarBox;	//��ǰ�״ﷶΧ���ڵ���С�ļ�������ʱ�ģ����ڼ���жϣ�
	MapRect m_radarDadiRect;	//���״�뾶Ϊ�߳��ľ������򣬴������
//	MapRect m_layerArea;		//��ǰ�״������ĺ�ͼ����һ��DEM_ORIGINAL��Χ����,���ĸ���ֱ�Ϊ��С|���γ�ȣ�
//	CString m_demIndexPath;		//��ǰ�����DEM��INDEX�ļ�λ�ã����ļ����������Ĳ�����
//	float *demFileData = NULL;	//�������а����״ﷶΧ���ļ��ĸ߳���Ϣ
	NAMESPACE_DEMTOOLS::DemTools demTools;

	double scaleWidth;	//��demUseDataתΪͼ��ʱ��������������
	double scaleHeight;	//��demUseDataתΪͼ��ʱ�������������ߣ�

	CWinThread* pThreadCalculateARPARecord;
	CWinThread* pThreadLoadDemFile;
	CWinThread* pThreadDEMtoBMP;
	CWinThread* pThreadLogin;
	CWinThread* pThreadReceive;
	HANDLE m_hThreadEvent_closeCalculateARPARecord;

	enum ButtonName{
		//���Ͻ�
		BN_Range_TEXT = 0,	//������ʾ��
		//BN_Range_up,		//��������
		//BN_Range_down,		//���̼���
		BN_RangeRings_SWITCH,		//�̶����Ȧ
		//BN_OffCentre_SWITCH,		//ƫ��
		//BN_HeadingLine_SWITCH,	//������

		//���½�
		BN_VariableRangeMarker_1_SWITCH,	//����Ȧ1
		BN_VariableRangeMarker_2_SWITCH,	//����Ȧ2
		BN_ElectronicBearingLine_1_SWITCH,//���ӷ�λ��1
		BN_ElectronicBearingLine_2_SWITCH,//���ӷ�λ��2
		BN_ExtendedRangeMeasurement_SWITCH,//��࿪��
		BN_TargetBearing_SWITCH,	//Ŀ�귽λ����
		BN_TargetRange_SWITCH,		//Ŀ����뿪��

		//���Ͻ�
		//BN_MotionMode_SWITCH,	//�˶���ʽ
		//BN_DisplayMode_SWITCH,	//��ʾ��ʽ
		//BN_Alarm_1_SWITCH,		//����1
		//BN_Alarm_2_SWITCH,		//����2
		//BN_ECHOStretch_SWITCH,		//չ��
		//BN_AlarmACK_SWITCH,		//����Ӧ��	......�����д���ȶ
		BN_DayNight_SWITCH,	//��ҹģʽ�л�

		//������Ϣ
		//BN_OwnLongitude_TEXT,//��������
		//BN_OwnLatitude_TEXT,	//����γ��
		//BN_OwnHeading_TEXT,		//��������
		//BN_OwnCourseOverGround_TEXT,//�����Եغ���
		//BN_OwnSpeed_TEXT,		//��������
		//BN_OwnSpeedOverGround_TEXT,//�����Ե��ٶ�
		//
		//BN_VectorTime_SWITCH,	//ʸ��ʱ��
		//BN_TrackTime_SWITCH,	//����ʱ��
		//BN_CPAAlarmDistance_TEXT,	//CPA��������
		//BN_TCPAAlarmTime_TEXT,	//TCPA����ʱ��

		//vts radar
		BN_VTS_Radar_TEXT_1,
		BN_VTS_Radar_TEXT_2,
		BN_VTS_Radar_TEXT_3,
		BN_VTS_Radar_TEXT_4,
		BN_VTS_Radar_TEXT_5,
		BN_VTS_Radar_UP,
		BN_VTS_Radar_DOWN,

		//�ز�����
		//BN_GAIN_TEXT,			//�����ı�
		//BN_GAIN_BAR,			//������
		//BN_Wave_TEXT,			//�����ı�
		//BN_Wave_BAR,			//������
		//BN_Rain_TEXT,			//��ѩ�ı�
		//BN_Rain_BAR,			//��ѩ��
		//BN_Tune_Text,			//��г�ı�
		//BN_Tune_BAR,			//��г��
		//
		BN_MessageArea_1,		//��Ϣ��1
		BN_MessageArea_2,		//��Ϣ��2

		//���½�
		BN_Language_SWITCH,		//��Ӣ�л�
		BN_EchoAveraging_SWITCH,	//�Ӳ�����
		//Ŀ�괬��Ϣ
		BN_TargetRange_TEXT,	//Ŀ�����
		BN_TargetBearing_TEXT,	//Ŀ�귽λ
		BN_TargetLongitude_TEXT,	//Ŀ�꾭��
		BN_TargetLatitude_TEXT,	//Ŀ��γ��
		//ϵͳ��Ϣ
		//BN_ARPA,				//ARPA
		//BN_ARPA_SWITCH,			//ARPA����		������ʾ���ã�APRA��AISֻ�ܴ�һ��
		//BN_ARPA_RECORDTARGET_SWITCH,	//ARPA��¼Ŀ�꿪��
		//BN_ARPA_VECTORTIME_SWITCH,	//ARPAʸ��ʱ�俪��
		//BN_ARPA_T_R_VECTOR_SWITCH,	//ARPA��/���ʸ���л�
		BN_AIS,					//AIS
		BN_AIS_SWITCH,			//AIS����
		BN_AIS_INFORMATION_SWITCH,	//AIS��Ϣ��ʾ����
		//BN_CourseAndMarker,		//����
		//BN_Settings,			//����
		//BN_StartOrStop_SWITCH,	//��ʼ|��ͣ
		BN_SystemTime_TEXT,		//ϵͳʱ��
	};

	


// ����
public:
	void ButtonCreate();
	void ButtonShow(Graphics *graphics, MyMemDC *pMemDC);
	void DrawRadar(Graphics *graphics, MyMemDC *pMemDC);
	//void DrawOtherShip(Graphics *graphics, MyMemDC *pMemDC);	//����Ŀ�괬
	void DrawDEM(Graphics *graphics, MyMemDC *pMemDC, CBitmapEx radarBmp);
	void DrawAisShips(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, std::vector<SHIP> aisShips);
	void DrawOneAisShip(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, const SHIP& ship);
	//void DrawAISShip(Graphics *graphics, MyMemDC *pMemDC, SHIP &ship);
	//void DrawARPARecord(Graphics *graphics, MyMemDC *pMemDC);
	void DrawVectorTime(Graphics *graphics, MyMemDC *pMemDC, SHIP ship, double time);	//����ʱ��ʸ����
	bool InitSocket();
	int GetButtonName(CPoint point);	//ͨ������ĵ��ȡButton���ھ��δӶ���ȡ����ö������
	std::vector<SHIP> updateAisShipsByAisMsg(const ais_t * pAisInfo, std::vector<SHIP> aisShips);	//��ȡһ��AIS��Ϣ
	double GeoToDistance(double aLon/*a�㾭��*/, double aLat/*a��γ��*/, double bLon/*b�㾭��*/, double bLat/*b��γ��*/);/*������A��B������루Km��*/
	void SetMessageArea1Para(MessageArea_1_Parameter para);		//������Ϣ��1����
	MessageArea_1_Parameter GetMessageArea1Para(){ return m_MessageArea_1_Parameter; }		//��ȡ��Ϣ��1����
	//void DrawMessageArea1(Graphics *graphics, MyMemDC *pMemDC);		//������Ϣ��1
	int GetAISShipIndex(CPoint point);
	//int GetARPARecordIndex(CPoint point);
	
	void DrawRadarShipOnDc(Graphics *graphics, MyMemDC *pMemDC , SHIP *ship);	//�����״�ɨ�赽��Ŀ�괬ͼ��
	void DrawRadarShipOnBMP(CBitmapEx* bmp, SHIP* ship);
	void GetOtherShipIndexFromRect(std::vector<int>& vec,CRect rect);	//��rect��Ѱ���Ƿ����Ŀ�괬��-1Ϊδ�ҵ�
	int GetMinUnusedIndexOfARPARecord();		//��m_arpaRecord�л��һ��δʹ�õ���С��index
	void CalculateARPARecord();			//����ARPA_RECORD����
	void CalculateARPARecordWithOutAIS();		//���ݱ�ǵ�ARPA���ڵ���ƶ�����ARPA_RECORD����
	//FIND_GRAVITY_RESULT FindCenOfGravityInRect(CRect gateRect, CRect* sameSizeRect, CRect* suitSizeRect, int* x, int* y);	//�ҳ����������������,������rect�ĳߴ緵���������ƶ����������Ĵ���sameSizeRect�ͺ���Ŀ��ߴ��С��suitSizeRect
	void AlphaBetaFilter(ARPA_RECORD* record);	//��-���˲��㷨
	void SetGateRectSize(CRect* gateRect,GATE_SIZE * gateSize, GATE_SIZE size);	//���ò��Ŵ�С
	GATE_SIZE GetGateRectSize(CRect gateRect);	//�õ����Ŵ�С
	int GetGateRectArea(CRect gateRect);		//�õ��������
	bool ChangeGateRectSize(CRect* gateRect, GATE_SIZE * gateSize, int i);	//����/��С���ţ�i>0ʱ���ӣ�i<0ʱ��С��һ�α�һ��
	bool ChangeGateRectSize(CRect gateRect, CRect * newRect, int i);
	void SetRectPosition(CRect* gateRect, int x, int y);	//���þ��ε�λ�ã����ı���εĴ�С��
	CPoint GetCRectCenter(CRect rect){ return CPoint((rect.left+rect.right)/2,(rect.top+rect.bottom)/2); };
	//void UpdateArpaRecordPos(double oldPerPixelM = 0, bool displayModeChanged = false);
//	bool checkFileNamesChanged();
	//void FileDataToUseData(const NAMESPACE_DEMTOOLS::DemMessage* demMessage);
	//void FileDataToUseData_1(const NAMESPACE_DEMTOOLS::DemMessage* demMessage);
	
	void DrawTest(const NAMESPACE_DEMTOOLS::DemTools& demTools);
	void DrawTest1();

	//void SendSessionState(byte iSessionState);

	std::string receiveAIS();  //��ȡais��Ϣ
	
private:
	bool PointInMapRect(const double geoX, const double geoY, const MapRect rect);
	bool GetIndexPath(CString &indexPath, MapRect &layerArea, const double longitude, const double latitude);
	void InitEvent();
	void SendHeartBeat();
	bool _checkLocIP();	//��鱾��IP�����Ƿ���ȷ
	bool _getLocalIPs(std::vector<CString> &ips, int* cnt);	//��ȡ��������IP��ַ
	void _initLocIP();
	

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:	
	char* _CStringToCharArray(CString str);//CStringתchar*
	bool _CStringToCharArray(const CString cstr, char *pstr, int maxLen);
	virtual ~CRadarView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#ifndef _DEBUG  // RadarView.cpp �еĵ��԰汾
inline CRadarDoc* CRadarView::GetDocument() const
   { return reinterpret_cast<CRadarDoc*>(m_pDocument); }
#endif

