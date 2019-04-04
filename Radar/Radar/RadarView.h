
// RadarView.h : CRadarView 类的接口
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

#define RADAR_CYCLE 2		//雷达扫描周期
//#define DEMBMP_WIDTH 900
//#define DEMBMP_HEIGHT 900
//#define RADAR_PIXEL_RADIUS 450
#define OTHERSHIP_MAXNUM 100
#define TIMER_SYSTEMTIME	10000
//#define TIMER_AIS_FROM_NET  10001
#define TIMER_AIS_DECODE    10002
#define TIMER_AIS_CHECK		10003		//定时检查目标船的AIS是否接收状态
#define TIMER_AIS_CLEAR	10004	//定时清空目标船，避免删除目标船后该船信息还存在
#define TIMER_REFRESH_RADAR	10005		//定时刷新图像显示区域
#define TIMER_CALCULATE_ARPARECORD 10006	//定时启动线程计算m_arpaRecord
#define TIMER_SENDHBT	10007	//心跳包
#define TIMER_CHECKIP 10008
#define TIMER_DRAW_VTS_RADAR 10009 //绘制vts雷达图像
#define TIMER_GENERATE_VTS_PIC 10010 //生成vts雷达图像，带船舶
#define TIMER_SEND_VTS_RADAR_DATA 10011//给vts设备发送雷达数据
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
#define RADAR_HIGHT 15  //雷达高度

enum ARPA_RECORD_TARGETSTATE
{
	ARPA_RECORD_STATE_NORMAL = 0,		//目标稳定（计算完毕）
	ARPA_RECORD_STATE_MISSING,		//目标丢失
	ARPA_RECORD_STATE_CALCULATING,	//目标计算中
//	ARPA_RECORD_SELECTED
};

enum ARPA_RECORD_PASSTATE
{
	ARPA_RECORD_PASSSTEM = 0,	//过船首
	ARPA_RECORD_COLLISION,		//碰撞
	ARPA_RECORD_PASSSTERN,		//过船尾
	ARPA_RECORD_NOCROSSLINE,	//无相交航线
	ARPA_RECORD_NOCOLLISION		//无碰撞
};

enum FIND_GRAVITY_RESULT
{
	RESULT_OK = 0,		//找到目标，且目标在方框内
	RESULT_LARGER,		//找到目标，但目标可能只有一部分在方框内，请求扩大搜索框
	RESULT_MISSING		//未找到目标
};

enum GATE_SIZE
{
	GATE_SIZE_SMALL = 0,
	GATE_SIZE_MEDIUM,
	GATE_SIZE_LARGE
};

typedef struct ARPA_RECORD{
	SHIP* AISship = NULL;
	UINT index = 0;		//用于显示的下标
	Rect rectBody;
	Rect rectIndex;
	ARPA_RECORD_TARGETSTATE state = ARPA_RECORD_STATE_CALCULATING;

	//非船舶时使用下列值
	double longaitude = 0;
	double latitude = 0;
	CPoint center;
	int scrX = 0;					//当前波门中心点坐标X
	int scrY = 0;
	double DadiX = 0;		//当前波门中心点大地坐标X
	double DadiY = 0;		
	double length_Meter = 0;		//方框的实际长度（米）
	double length_Pixel = 0;		//方框的像素长度（像素）

	//公共
	double distance = 0;	//到本船的距离(单位：海里)
	double bearing = 0;		//方位
	double TSpeed = 0;		//真速度
	double TCourse = 0;		//真航向
	double RSpeed = 0;		//相对速度
	double RCourse = 0;		//相对航向
	ARPA_RECORD_PASSTATE passState;	//经过本船的位置
	double CPA = 0;			//最近会遇距离
	double TCPA = 0;		//最短会遇时间
	double collisionRadius = 0;			//碰撞半径（米）
	int attackTime = 0;		//碰撞时间（秒）

	//用于α-β滤波计算
	GATE_SIZE gateSize = GATE_SIZE_MEDIUM;	//波门大小
	int scanTimes = 0;		//从记录开始被扫描的次数
	int lossTimes = 0;		//连续丢失目标的次数
	int lastPosX = -1;	//上次记录的坐标X（屏幕坐标）
	int lastPosY = -1;
	double lastDadiX = -1;	//上次记录的大地坐标X
	double lastDadiY = -1;
	int thisPosX = -1;	//这次记录的坐标X（屏幕坐标）
	int thisPosY = -1;
	double thisDadiX = -1;	//这次记录的大地坐标X
	double thisDadiY = -1;
	int thisSmoothPosX = -1;	//由预测位置和实测位置得到的平滑屏幕坐标X
	int thisSmoothPosY = -1;
	double thisSmoothDadiX = -1;	//由预测位置和实测位置得到的平滑大地坐标X
	double thisSmoothDadiY = -1;
	int nextPredictPosX = -1;	//预测下次到达坐标X(屏幕坐标)
	int nextPredictPosY = -1;
	double nextPredictDadiX = -1;	//预测下次到达的大地坐标X
	double nextPredictDadiY = -1;
	double speedX = -1;	//目标速度在大地坐标X方向上的分量，单位m/s
	double speedY = -1;
	double smoothSpeedX = -1;	//目标平滑速度在大地坐标X方向上的分量，单位m/s
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
protected: // 仅从序列化创建
	CRadarView();
	DECLARE_DYNCREATE(CRadarView)

// 特性
private:
	//SOCKET sock_sendHeartBeat = -1;	//发送心跳包的套接字
	//sockaddr_in addr_sendHeartBeat;
	//int addrLen_sendHeartBeat = 0;
	CString m_ip;	//程序绑定的本机ip地址
	std::vector<CString> locIPs;	//本地设置的IP地址集合
public:
	//SOCKET sock_LOGIN = -1;

	sockaddr_in addr_sendLOGIN;
	int addrLen_sendLOGIN = 0;

	sockaddr_in addr_recLOGIN;
	int addrLen_recLOGIN = 0;

	//ShipDataService::SessionState iSessionState = ShipDataService::SessionState::SS_LOBBY;
	//用于登录服务器
	byte ISessionState = SessionState::SS_NONE;	//设备状态
	byte ISessionNum = 0x01;  //设备编号

	CRadarDoc* GetDocument() const;
	MyMemDC * pMemdc;
	GdiplusStartupInput m_gdiplusstartupinput;
	ULONG_PTR m_gdiplusToken;
	SOCKET m_socket;	//套接字
	CString m_strReceived;//接收串口传回的字符

	CRect m_viewRect;	//视图区域
	CRgn m_radarRgn;	//雷达视图的圆形区域
	CRgn m_radarRgn_Mid;	//稍大一点的雷达视图圆形区域，用来解决最外层线圈不包含在m_radarRgn内的问题
	CRgn m_radarRgn_Larger;	//比Mid大一点的雷达视图圆形区域，用来画刻度
	CRgn m_buttonRgn;	//按钮区域
	CRect m_AISControlBoxRect;	//AIS控制框区域
	CRgn m_AISControlBoxRgn;	//AIS控制框区域
	CRect m_ARPAControlBoxRect;	//ARPA控制框区域
	CRgn m_ARPAControlBoxRgn;	//ARPA控制框区域
	CRect m_radarRect;	//雷达视图的矩形区域
	Point m_radarCenter;	//雷达中心点坐标
	int m_radiusP;	//雷达半径（像素）
	double m_radiusM;	//雷达半径（米）
	CBrush blackBrush;	//黑色背景画刷
	CBrush blueBrush;	//蓝色背景画刷
	COLORREF color_blue;
	COLORREF color_yellow;
	CRect m_arpaRecordRect;	//APRA录取时的区域，用于识别船舶是否在该区域内

	enum MessageArea_1_Parameter
	{
		MA_1_BLANK = 0,		//清空内容
		MA_1_EXPAND,		//展宽图像显示
		MA_1_ARPA_ON,		//打开ARPA提示
		MA_1_ARPA_OFF,		//关闭ARPA提示
		MA_1_ARPA_INFO,		//显示船舶ARPA信息
		MA_1_AIS_ON,		//打开AIS提示
		MA_1_AIS_OFF,		//关闭AIS提示
		MA_1_AIS_INFO,		//显示船舶AIS信息
	};

	//SHIP m_MainShip;
	SHIP *m_aisShipSelected;	//被选中的ais目标船
	SHIP m_blankShip;			//无信息的船
	ARPA_RECORD * m_ArpaRecordSelected = NULL;	//被选中的ARPA目标
	ARPA_RECORD m_blankArpaRecord;	//无信息的ARPARECORD

	std::vector<VtsRadar> m_vtsRadar;//vtsRadar列表
	int m_vtsRadar_first_index = 0; //表示列表中显示的5个vtsRadar从第几个开始显示，只有m_vtsRadar.size()>5时才能不为0
	
	int m_radarCycle = 2;		//雷达扫描周期
	int m_delayTimes = 0;
	CRect  m_Button[BUTTONNUM];
	bool m_radarRgnLBDownBeUsed = false;	//在雷达区域内的鼠标左击功能是否被占用（避免测绘功能和APRA/AIS功能冲突）
	double m_mulOf800_600 = 0;	//当前分辨率相对于800*600的倍数
	double m_perPixelNM = 0;	//单位像素的距离（海里）
	double m_perPixelKM = 0;	//单位像素的距离（千米）
	double m_perPixelM = 0;		//单位像素的距离（米）
	bool m_radarSelect = false;	//鼠标是否进入雷达区域内
	bool m_ButtonSelect[BUTTONNUM]; //button是否被点击
	bool m_dayOrNight = false;	//true为day
	bool m_language = true;	//true为中文
	//double m_range = RANGE5;		//量程(海里)
	//double m_rangeRingsDistance = 0;	//距标的距离
	bool m_rangeRingsSwitch = false;	//固定距标圈开关
	int m_rangeRingsNum = 5;	//固定距标圈的数量
	bool m_offCentreSwitch = false;	//偏心开关
	bool m_headingLineSwitch = true;	//首线开关
	double m_headingLineAngle = 0;	//首线与屏幕正上方的绝对夹角(弧度)
	double m_northAngle = 0;		//正北向与屏幕正上方的绝对夹角(弧度)
	double m_COGAngle = 0;		//船舶对地航向与屏幕正上方的绝对夹角(弧度)(由于暂时没有对地航向信息，暂设为和船艏向一致)
	double m_VRM1distance = 0;	//VRM1数值(海里)
	double m_VRM2distance = 0;	//VRM2数值(海里)
	int m_VRM1Pixel = 0;	//VRM1半径(像素)
	int m_VRM2Pixel = 0;	//VRM2半径(像素)
	bool m_variableRangeMarker1Switch = false;	//VRM1开关
	bool m_variableRangeMarker2Switch = false;	//VRM2开关
	bool m_VRM1Roam = false;	//VRM1漫游
	bool m_VRM2Roam = false;	//VRM2漫游
	double m_electronicBearingLine1Num = 0;	//电子方位线1数值(与船艏向夹角)(角度)
	double m_electronicBearingLine1Angle = 0;	//电子方位线1夹角(与屏幕正上方的绝对夹角)(弧度)
	double m_electronicBearingLine2Num = 0;	//电子方位线2数值(与船艏向夹角)(角度)
	double m_electronicBearingLine2Angle = 200;	//电子方位线2夹角(与屏幕正上方的绝对夹角)(弧度)
	bool m_electronicBearingLine1Switch = false;	//电子方位线1开关
	bool m_electronicBearingLine2Switch = false;	//电子方位线2开关
	bool m_EBL1Roam = false;		//电子方位线1漫游
	bool m_EBL2Roam = false;		//电子方位线2漫游
	bool m_extendedRangeMeasurementSwitch = false;	//目标测距开关
	bool m_ERMFunctionSwitch = false;		//测距功能开关
	bool m_ERM_Roam = false;		//测距漫游
	CPoint m_ERM_P1;				//测距起点
	CPoint m_ERM_P2;				//测距终点
	double m_targetBearingNum = 0;	//测距方位
	double m_targetRangeNum = 0;	//测距距离
	bool m_alarm1Switch = false;	//Alarm1开关
	bool m_alarm2Switch = false;	//Alarm2开关
	bool m_ECHOStretchSwitch = false;	//展宽开关
	bool m_motionMode = true;	//true为相对运动
	bool m_displayMode = true;		//true为首向上,false为北向上
	bool m_vectorTimeSwitch = false;	//矢量时间开关
	double m_vectorTimeNum = 0;		//矢量时间数值
	bool m_trackTimeSwitch = false;		//航迹时间开关
	bool m_CPAAlarmDistanceSwitch = false;	//CPA报警距离开关
	bool m_TCPAAlarmTimeSwitch = false;	//TCPA报警时间开关
	int m_gainNum = 0;		//增益	0~100
	int m_waveNum = 0;		//海浪	0~100
	int m_rainNum = 0;		//雨雪	0~100
	int m_tuneNum = 0;		//调谐	0~100
	MessageArea_1_Parameter m_MessageArea_1_Parameter = MA_1_BLANK;	//信息窗1参数
	double m_targetRange = 0;	//目标距离
	double m_targetBearing_T = 0;	//目标点到圆心的连线与正北方向的夹角
	double m_targetBearing_R = 0;	//目标点到圆心的连线与船首线的夹角
	double m_targetLongitude = 0;	//目标点经度
	double m_targetLatitude = 0;	//目标点纬度
	bool m_echoAveragingSwitch = false; //杂波抑制开关
	bool m_startOrStop = true;	//true为START
	CString m_systemTime;	//系统时间
	bool m_ARPASwitch = false;	//APRA开关
	bool m_ARPARecordTargetSwitch = false;	//ARPA录取目标开关
	bool m_ARPAVectorTimeSwitch = false;	//ARPA矢量时间开关
	double m_ARPAVectorTimeNum = 0;			//ARPA矢量时间数值
	bool m_ARPA_T_R_VectorSwitch = true;	//ARPA真/相对矢量切换（true为真矢量）
	bool m_ARPAControlBoxDisplay = false;	//ARPA控制框显示
	bool m_AISSwitch = false;	//AIS开关
	bool m_AISControlBoxDisplay = false;	//AIS控制框显示
	bool m_AISInformationSwitch = false;	//AIS信息显示开关
	bool m_AISControlBoxRoam = false;	//判断鼠标是否首次进入AIS控制框
	bool m_messageArea1BeUsed = false;	//信息窗1是否被占用
	GATE_SIZE m_ARPARecordCur = GATE_SIZE_MEDIUM;		//1、2、3分别为小、中、大

//	std::vector<CString> previousFileNames;	//上一次索引的文件名(数据处理时使用的)
//	std::vector<CString> presentFileNames;	//当前索引的文件名（临时的，用于检测判断）
//	QuadTree m_quadTree;	//包含文件信息的非满四叉树(用于索引文件)
//	QuadTree m_fullTree;	//不包含文件信息的满四叉树（用于计算BOX）
//	MapRect previousRadarBox;	//上一次雷达范围所在的最小文件区域(数据处理时使用的)
//	MapRect presentRadarBox;	//当前雷达范围所在的最小文件区域（临时的，用于检测判断）
	MapRect m_radarDadiRect;	//以雷达半径为边长的矩形区域，大地坐标
//	MapRect m_layerArea;		//当前雷达所处的海图区域（一块DEM_ORIGINAL范围区域,其四个点分别为最小|最大经纬度）
//	CString m_demIndexPath;		//当前载入的DEM的INDEX文件位置（该文件用于生成四叉树）
//	float *demFileData = NULL;	//保存所有包含雷达范围的文件的高程信息
	NAMESPACE_DEMTOOLS::DemTools demTools;

	double scaleWidth;	//由demUseData转为图像时的伸缩比例（宽）
	double scaleHeight;	//由demUseData转为图像时的伸缩比例（高）

	CWinThread* pThreadCalculateARPARecord;
	CWinThread* pThreadLoadDemFile;
	CWinThread* pThreadDEMtoBMP;
	CWinThread* pThreadLogin;
	CWinThread* pThreadReceive;
	HANDLE m_hThreadEvent_closeCalculateARPARecord;

	enum ButtonName{
		//左上角
		BN_Range_TEXT = 0,	//量程显示区
		//BN_Range_up,		//量程增加
		//BN_Range_down,		//量程减少
		BN_RangeRings_SWITCH,		//固定距标圈
		//BN_OffCentre_SWITCH,		//偏心
		//BN_HeadingLine_SWITCH,	//船首线

		//左下角
		BN_VariableRangeMarker_1_SWITCH,	//活动距标圈1
		BN_VariableRangeMarker_2_SWITCH,	//活动距标圈2
		BN_ElectronicBearingLine_1_SWITCH,//电子方位线1
		BN_ElectronicBearingLine_2_SWITCH,//电子方位线2
		BN_ExtendedRangeMeasurement_SWITCH,//测距开关
		BN_TargetBearing_SWITCH,	//目标方位开关
		BN_TargetRange_SWITCH,		//目标距离开关

		//右上角
		//BN_MotionMode_SWITCH,	//运动方式
		//BN_DisplayMode_SWITCH,	//显示方式
		//BN_Alarm_1_SWITCH,		//警报1
		//BN_Alarm_2_SWITCH,		//警报2
		//BN_ECHOStretch_SWITCH,		//展宽
		//BN_AlarmACK_SWITCH,		//报警应答	......功能有待商榷
		BN_DayNight_SWITCH,	//日夜模式切换

		//本船信息
		//BN_OwnLongitude_TEXT,//本船经度
		//BN_OwnLatitude_TEXT,	//本船纬度
		//BN_OwnHeading_TEXT,		//本船船首
		//BN_OwnCourseOverGround_TEXT,//本船对地航向
		//BN_OwnSpeed_TEXT,		//本船航速
		//BN_OwnSpeedOverGround_TEXT,//本船对地速度
		//
		//BN_VectorTime_SWITCH,	//矢量时间
		//BN_TrackTime_SWITCH,	//航迹时间
		//BN_CPAAlarmDistance_TEXT,	//CPA报警距离
		//BN_TCPAAlarmTime_TEXT,	//TCPA报警时间

		//vts radar
		BN_VTS_Radar_TEXT_1,
		BN_VTS_Radar_TEXT_2,
		BN_VTS_Radar_TEXT_3,
		BN_VTS_Radar_TEXT_4,
		BN_VTS_Radar_TEXT_5,
		BN_VTS_Radar_UP,
		BN_VTS_Radar_DOWN,

		//回波控制
		//BN_GAIN_TEXT,			//增益文本
		//BN_GAIN_BAR,			//增益条
		//BN_Wave_TEXT,			//海浪文本
		//BN_Wave_BAR,			//海浪条
		//BN_Rain_TEXT,			//雨雪文本
		//BN_Rain_BAR,			//雨雪条
		//BN_Tune_Text,			//调谐文本
		//BN_Tune_BAR,			//调谐条
		//
		BN_MessageArea_1,		//信息窗1
		BN_MessageArea_2,		//信息窗2

		//右下角
		BN_Language_SWITCH,		//中英切换
		BN_EchoAveraging_SWITCH,	//杂波抑制
		//目标船信息
		BN_TargetRange_TEXT,	//目标距离
		BN_TargetBearing_TEXT,	//目标方位
		BN_TargetLongitude_TEXT,	//目标经度
		BN_TargetLatitude_TEXT,	//目标纬度
		//系统信息
		//BN_ARPA,				//ARPA
		//BN_ARPA_SWITCH,			//ARPA开关		由于显示设置，APRA和AIS只能打开一个
		//BN_ARPA_RECORDTARGET_SWITCH,	//ARPA记录目标开关
		//BN_ARPA_VECTORTIME_SWITCH,	//ARPA矢量时间开关
		//BN_ARPA_T_R_VECTOR_SWITCH,	//ARPA真/相对矢量切换
		BN_AIS,					//AIS
		BN_AIS_SWITCH,			//AIS开关
		BN_AIS_INFORMATION_SWITCH,	//AIS信息显示开关
		//BN_CourseAndMarker,		//航线
		//BN_Settings,			//设置
		//BN_StartOrStop_SWITCH,	//开始|暂停
		BN_SystemTime_TEXT,		//系统时间
	};

	


// 操作
public:
	void ButtonCreate();
	void ButtonShow(Graphics *graphics, MyMemDC *pMemDC);
	void DrawRadar(Graphics *graphics, MyMemDC *pMemDC);
	//void DrawOtherShip(Graphics *graphics, MyMemDC *pMemDC);	//绘制目标船
	void DrawDEM(Graphics *graphics, MyMemDC *pMemDC, CBitmapEx radarBmp);
	void DrawAisShips(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, std::vector<SHIP> aisShips);
	void DrawOneAisShip(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, const SHIP& ship);
	//void DrawAISShip(Graphics *graphics, MyMemDC *pMemDC, SHIP &ship);
	//void DrawARPARecord(Graphics *graphics, MyMemDC *pMemDC);
	void DrawVectorTime(Graphics *graphics, MyMemDC *pMemDC, SHIP ship, double time);	//绘制时间矢量线
	bool InitSocket();
	int GetButtonName(CPoint point);	//通过点击的点获取Button所在矩形从而获取按键枚举名称
	std::vector<SHIP> updateAisShipsByAisMsg(const ais_t * pAisInfo, std::vector<SHIP> aisShips);	//获取一条AIS信息
	double GeoToDistance(double aLon/*a点经度*/, double aLat/*a点纬度*/, double bLon/*b点经度*/, double bLat/*b点纬度*/);/*北半球A、B两点距离（Km）*/
	void SetMessageArea1Para(MessageArea_1_Parameter para);		//设置信息窗1参数
	MessageArea_1_Parameter GetMessageArea1Para(){ return m_MessageArea_1_Parameter; }		//获取信息窗1参数
	//void DrawMessageArea1(Graphics *graphics, MyMemDC *pMemDC);		//绘制信息窗1
	int GetAISShipIndex(CPoint point);
	//int GetARPARecordIndex(CPoint point);
	
	void DrawRadarShipOnDc(Graphics *graphics, MyMemDC *pMemDC , SHIP *ship);	//绘制雷达扫描到的目标船图像
	void DrawRadarShipOnBMP(CBitmapEx* bmp, SHIP* ship);
	void GetOtherShipIndexFromRect(std::vector<int>& vec,CRect rect);	//从rect中寻找是否存在目标船，-1为未找到
	int GetMinUnusedIndexOfARPARecord();		//从m_arpaRecord中获得一个未使用的最小的index
	void CalculateARPARecord();			//计算ARPA_RECORD数据
	void CalculateARPARecordWithOutAIS();		//根据标记的ARPA框内点的移动计算ARPA_RECORD数据
	//FIND_GRAVITY_RESULT FindCenOfGravityInRect(CRect gateRect, CRect* sameSizeRect, CRect* suitSizeRect, int* x, int* y);	//找出方框内物体的重心,并根据rect的尺寸返回其中心移动到计算重心处的sameSizeRect和合适目标尺寸大小的suitSizeRect
	void AlphaBetaFilter(ARPA_RECORD* record);	//α-β滤波算法
	void SetGateRectSize(CRect* gateRect,GATE_SIZE * gateSize, GATE_SIZE size);	//设置波门大小
	GATE_SIZE GetGateRectSize(CRect gateRect);	//得到波门大小
	int GetGateRectArea(CRect gateRect);		//得到波门面积
	bool ChangeGateRectSize(CRect* gateRect, GATE_SIZE * gateSize, int i);	//增大/减小波门，i>0时增加，i<0时减小，一次变一档
	bool ChangeGateRectSize(CRect gateRect, CRect * newRect, int i);
	void SetRectPosition(CRect* gateRect, int x, int y);	//设置矩形的位置（不改变矩形的大小）
	CPoint GetCRectCenter(CRect rect){ return CPoint((rect.left+rect.right)/2,(rect.top+rect.bottom)/2); };
	//void UpdateArpaRecordPos(double oldPerPixelM = 0, bool displayModeChanged = false);
//	bool checkFileNamesChanged();
	//void FileDataToUseData(const NAMESPACE_DEMTOOLS::DemMessage* demMessage);
	//void FileDataToUseData_1(const NAMESPACE_DEMTOOLS::DemMessage* demMessage);
	
	void DrawTest(const NAMESPACE_DEMTOOLS::DemTools& demTools);
	void DrawTest1();

	//void SendSessionState(byte iSessionState);

	std::string receiveAIS();  //获取ais信息
	
private:
	bool PointInMapRect(const double geoX, const double geoY, const MapRect rect);
	bool GetIndexPath(CString &indexPath, MapRect &layerArea, const double longitude, const double latitude);
	void InitEvent();
	void SendHeartBeat();
	bool _checkLocIP();	//检查本机IP设置是否正确
	bool _getLocalIPs(std::vector<CString> &ips, int* cnt);	//获取本机所有IP地址
	void _initLocIP();
	

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:	
	char* _CStringToCharArray(CString str);//CString转char*
	bool _CStringToCharArray(const CString cstr, char *pstr, int maxLen);
	virtual ~CRadarView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
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

#ifndef _DEBUG  // RadarView.cpp 中的调试版本
inline CRadarDoc* CRadarView::GetDocument() const
   { return reinterpret_cast<CRadarDoc*>(m_pDocument); }
#endif

