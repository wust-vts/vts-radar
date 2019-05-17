#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Radar.h"
#endif

#include "RadarDoc.h"
#include "RadarView.h"
#include "RWLockImpl.h"
#include <afxpriv.h>
#include <vector>
#include"QuadTree.h"
#include<fstream>
#include"BitmapEx.h"
#include"PublicMethod.h"
#include<WinSock2.h>
#include<mutex>
#include <iomanip>
#include"IPConfig.h"
#include"ShipDataService.h"
#include"login.h"
#include"Thread.h"
#include<thread>
#include<exception>
using std::vector;
using std::ifstream;
using std::mutex;
using namespace Gdiplus;
using namespace NAMESPACE_DEMTOOLS;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//聯網模式下接收數據端口,理工没用这个,用的ipconfig
#define RECEIVE_DATA_PORT 6666
#define DATA_BUFFER_SIZE 100

extern vector <SHIP> g_aisShips;	//结构体vector要定义为全局
//extern vector <ARPA_RECORD> m_arpaRecord;
//VtsRadar * m_vtsRadar_selected = NULL; //被选中的vtsRadar
extern int m_vtsRadar_selected;
extern std::mutex mtx_vtsRadar_selected;
//创建一个读写锁对象  
//CMyRWLock g_myRWLock;
extern HANDLE closeWindowEvent;
extern HANDLE closeThread_loadDemFile_event;
extern HANDLE closeThread_threadCalculateARPARecord;
extern HANDLE closeThread_drawDemBMP;
extern HANDLE rangeUpdate;

extern mutex mtx_BMPmem;
extern mutex mtx_aisShips;

//struct DemFileMessage
//{
//	MapRect box;
//	double minStepX = 0;
//	double maxStepX = 0;
//	double minStepY = 0;
//	double maxStepY = 0;
//	double stepX = 0;	//由minStepX和maxStepX决定
//	double stepY = 0;	//由minStepY和maxStepY决定
//	int invartal = 0;	//取点间隔
//	int row = 0;
//	int col = 0;
//}mapFileMessage;	//当前载入的所有cell文件所包含的综合信息(demfileData)





// CRadarView

IMPLEMENT_DYNCREATE(CRadarView, CView)

BEGIN_MESSAGE_MAP(CRadarView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CRadarView 构造/析构

CRadarView::CRadarView()
{
	// TODO:  在此处添加构造代码
	pThreadCalculateARPARecord = NULL;
	pThreadLoadDemFile = NULL;
	pThreadDEMtoBMP = NULL;
	pThreadLogin = NULL;
	pThreadReceive = NULL;
}

CRadarView::~CRadarView()
{
	GdiplusShutdown(m_gdiplusToken);
	DeleteObject(&blackBrush);
	DeleteObject(&blueBrush);
}

BOOL CRadarView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	GdiplusStartup(&m_gdiplusToken, &m_gdiplusstartupinput, NULL);

	return CView::PreCreateWindow(cs);
}

// CRadarView 绘制

void CRadarView::OnDraw(CDC* pDC)
{
	CRadarDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  在此处为本机数据添加绘制代码
	CDC dc;
	MyMemDC pMemDC(pDC);
	pMemdc = &pMemDC;
	dc.CreateCompatibleDC(&pMemDC);
	Graphics graphics(pMemDC.m_hDC);
	pMemDC.FillRect(&m_viewRect, &blackBrush);	//绘制背景色
	pMemDC.FillRgn(&m_radarRgn, &blueBrush);	//绘制雷达区域背景色
	ButtonShow(&graphics, &pMemDC);
	DrawRadar(&graphics, &pMemDC);
	//DrawMessageArea1(&graphics, &pMemDC);


	DeleteDC(dc.m_hDC);
}


// CRadarView 打印

BOOL CRadarView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CRadarView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加额外的打印前进行的初始化过程
}

void CRadarView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加打印后进行的清理过程
}


// CRadarView 诊断

#ifdef _DEBUG
void CRadarView::AssertValid() const
{
	CView::AssertValid();
}

void CRadarView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CRadarDoc* CRadarView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRadarDoc)));
	return (CRadarDoc*)m_pDocument;
}
#endif //_DEBUG


// CRadarView 消息处理程序
void CRadarView::ButtonCreate()
{
	int buttonHightNomal = m_viewRect.Height() / 20;
	int buttonHightSmall = m_viewRect.Height() / 30;
	//if (m_viewRect.Width() == 800 && m_viewRect.Height() == 600) //如果是800*600分辨率
	//{
	//	//左上角
	//	m_Button[BN_Range_TEXT] = CRect(1, 1, 141, 31);
	//	m_Button[BN_Range_up] = CRect(144, 1, 174, 14);
	//	m_Button[BN_Range_down] = CRect(144, 17, 174, 31);
	//	m_Button[BN_RangeRings_SWITCH] = CRect(1, 34, 141, 64);
	//	m_Button[BN_OffCentre_SWITCH] = CRect(1, 67, 101, 97);
	//	m_Button[BN_HeadingLine_SWITCH] = CRect(1, 100, 91, 130);
	//	//左下角
	//	m_Button[BN_VariableRangeMarker_1_SWITCH] = CRect(1, 469, 81, 499);
	//	m_Button[BN_VariableRangeMarker_2_SWITCH] = CRect(1, 502, 81, 532);
	//	m_Button[BN_ElectronicBearingLine_1_SWITCH] = CRect(1, 535, 81, 565);
	//	m_Button[BN_ElectronicBearingLine_2_SWITCH] = CRect(1, 568, 81, 598);
	//	m_Button[BN_ExtendedRangeMeasurement_SWITCH] = CRect(84, 535, 144, 565);
	//	m_Button[BN_TargetBearing_SWITCH] = CRect(84, 568, 144, 598);
	//	m_Button[BN_TargetRange_SWITCH] = CRect(147, 568, 207, 598);
	//	//右上角
	//	m_Button[BN_MotionMode_SWITCH] = CRect(500, 1, 540, 31);
	//	m_Button[BN_DisplayMode_SWITCH] = CRect(543, 1, 637, 31);
	//	m_Button[BN_Alarm_1_SWITCH] = CRect(514, 34, 574, 64);
	//	m_Button[BN_Alarm_2_SWITCH] = CRect(577, 34, 637, 64);
	//	m_Button[BN_ECHOStretch_SWITCH] = CRect(537, 67, 637, 97);
	//	m_Button[BN_AlarmACK_SWITCH] = CRect(567, 100, 637, 130);
	//	m_Button[BN_DayNight_SWITCH] = CRect(587, 133, 637, 163);
	//	//本船信息
	//	m_Button[BN_OwnLongitude_TEXT] = CRect(640, 1, 719, 31);
	//	m_Button[BN_OwnLatitude_TEXT] = CRect(720, 1, 798, 31);
	//	m_Button[BN_OwnHeading_TEXT] = CRect(640, 32, 719, 62);
	//	m_Button[BN_OwnCourseOverGround_TEXT] = CRect(720, 32, 798, 62);
	//	m_Button[BN_OwnSpeed_TEXT] = CRect(640, 63, 719, 93);
	//	m_Button[BN_OwnSpeedOverGround_TEXT] = CRect(720, 63, 798, 93);
	//	//
	//	m_Button[BN_VectorTime_SWITCH] = CRect(640, 96, 798, 116);
	//	m_Button[BN_TrackTime_SWITCH] = CRect(640, 117, 798, 137);
	//	m_Button[BN_CPAAlarmDistance_TEXT] = CRect(640, 140, 798, 160);
	//	m_Button[BN_TCPAAlarmTime_TEXT] = CRect(640, 161, 798, 181);
	//	//回波控制
	//	m_Button[BN_GAIN_TEXT] = CRect(640, 184, 697, 204);
	//	m_Button[BN_GAIN_BAR] = CRect(698, 184, 798, 204);
	//	m_Button[BN_Wave_TEXT] = CRect(640, 205, 697, 225);
	//	m_Button[BN_Wave_BAR] = CRect(698, 205, 798, 225);
	//	m_Button[BN_Rain_TEXT] = CRect(640, 226, 697, 246);
	//	m_Button[BN_Rain_BAR] = CRect(698, 226, 798, 246);
	//	m_Button[BN_Tune_Text] = CRect(640, 247, 697, 267);
	//	m_Button[BN_Tune_BAR] = CRect(698, 247, 798, 267);
	//	//信息窗
	//	m_Button[BN_MessageArea_1] = CRect(640, 270, 798, 428);
	//	m_Button[BN_MessageArea_2] = CRect();
	//	//右下
	//	m_Button[BN_Language_SWITCH] = CRect(587, 440, 637, 470);
	//	m_Button[BN_EchoAveraging_SWITCH] = CRect(567, 473, 637, 503);
	//	m_Button[BN_TargetRange_TEXT] = CRect(537, 506, 637, 536);
	//	m_Button[BN_TargetBearing_TEXT] = CRect(507, 537, 637, 567);
	//	m_Button[BN_TargetLongitude_TEXT] = CRect(456, 568, 546, 598);
	//	m_Button[BN_TargetLatitude_TEXT] = CRect(547, 568, 637, 598);
	//	m_Button[BN_ARPA] = CRect(640, 502, 680, 532);
	//	m_Button[BN_AIS] = CRect(681, 502, 719, 532);
	//	m_Button[BN_CourseAndMarker] = CRect(720, 502, 759, 532);
	//	m_Button[BN_Settings] = CRect(760, 502, 798, 532);
	//	m_Button[BN_StartOrStop_SWITCH] = CRect(640, 535, 798, 565);
	//	m_Button[BN_SystemTime_TEXT] = CRect(640, 568, 798, 598);
	//}
	//else
	//{
	//*******重要********下一个按钮位置取决于上一个按钮的位置，按钮初始化顺序不可改变******************//
	//左上角
	m_Button[BN_Range_TEXT] = CRect(1, 1, 1 + m_viewRect.Width() * 7 / 40, 1 + buttonHightNomal);
	//m_Button[BN_Range_up] = CRect(m_Button[BN_Range_TEXT].right + 3, 1, m_Button[BN_Range_TEXT].right + 3 + m_viewRect.Width() * 3 / 80, 1 + m_viewRect.Height() * 13 / 600);
	//m_Button[BN_Range_down] = CRect(m_Button[BN_Range_up].left, m_Button[BN_Range_up].bottom + 3, m_Button[BN_Range_up].right, 1 + buttonHightNomal);
	m_Button[BN_RangeRings_SWITCH] = CRect(1, 3 + m_Button[BN_Range_TEXT].bottom, m_Button[BN_Range_TEXT].right, m_Button[BN_Range_TEXT].bottom + 3 + buttonHightNomal);
	//m_Button[BN_OffCentre_SWITCH] = CRect(1, 3 + m_Button[BN_RangeRings_SWITCH].bottom, 1 + m_viewRect.Width() / 8, 3 + m_Button[BN_RangeRings_SWITCH].bottom + buttonHightNomal);
	//m_Button[BN_HeadingLine_SWITCH] = CRect(1, m_Button[BN_RangeRings_SWITCH].bottom + 3, 1 + m_viewRect.Width() * 9 / 80, m_Button[BN_RangeRings_SWITCH].bottom + 3 + buttonHightNomal);
	//左下角
	m_Button[BN_ElectronicBearingLine_2_SWITCH] = CRect(1, m_viewRect.bottom - 5 - buttonHightNomal, 1 + m_viewRect.Width() / 10, m_viewRect.bottom - 5);
	m_Button[BN_ElectronicBearingLine_1_SWITCH] = CRect(1, m_Button[BN_ElectronicBearingLine_2_SWITCH].top - 3 - buttonHightNomal, 1 + m_viewRect.Width() / 10, m_Button[BN_ElectronicBearingLine_2_SWITCH].top - 3);
	m_Button[BN_VariableRangeMarker_2_SWITCH] = CRect(1, m_Button[BN_ElectronicBearingLine_1_SWITCH].top - 3 - buttonHightNomal, 1 + m_viewRect.Width() / 10, m_Button[BN_ElectronicBearingLine_1_SWITCH].top - 3);
	m_Button[BN_VariableRangeMarker_1_SWITCH] = CRect(1, m_Button[BN_VariableRangeMarker_2_SWITCH].top - 3 - buttonHightNomal, 1 + m_viewRect.Width() / 10, m_Button[BN_VariableRangeMarker_2_SWITCH].top - 3);

	m_Button[BN_TargetBearing_SWITCH] = CRect(3 + m_Button[BN_ElectronicBearingLine_2_SWITCH].right, m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom - buttonHightNomal, 3 + m_Button[BN_ElectronicBearingLine_2_SWITCH].right + m_viewRect.Width() * 3 / 40, m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom);
	m_Button[BN_ExtendedRangeMeasurement_SWITCH] = CRect(m_Button[BN_TargetBearing_SWITCH].left, m_Button[BN_TargetBearing_SWITCH].top - 3 - buttonHightNomal, m_Button[BN_TargetBearing_SWITCH].right, m_Button[BN_TargetBearing_SWITCH].top - 3);
	m_Button[BN_TargetRange_SWITCH] = CRect(m_Button[BN_TargetBearing_SWITCH].right + 3, m_Button[BN_TargetBearing_SWITCH].top, m_Button[BN_TargetBearing_SWITCH].right + 3 + m_viewRect.Width() * 3 / 40, m_Button[BN_TargetBearing_SWITCH].bottom);

	//本船信息
	//m_Button[BN_OwnLatitude_TEXT] = CRect(m_viewRect.right - m_viewRect.Width() / 10 - 2, 1, m_viewRect.right - 5, 1 + buttonHightNomal);
	//m_Button[BN_OwnLongitude_TEXT] = CRect(m_viewRect.right - m_viewRect.Width() / 5, 1, m_Button[BN_OwnLatitude_TEXT].left - 1, 1 + buttonHightNomal);
	//m_Button[BN_OwnCourseOverGround_TEXT] = CRect(m_Button[BN_OwnLatitude_TEXT].left, m_Button[BN_OwnLatitude_TEXT].bottom + 1, m_Button[BN_OwnLatitude_TEXT].right, m_Button[BN_OwnLatitude_TEXT].bottom + 1 + buttonHightNomal);
	//m_Button[BN_OwnHeading_TEXT] = CRect(m_Button[BN_OwnLongitude_TEXT].left, m_Button[BN_OwnLongitude_TEXT].bottom + 1, m_Button[BN_OwnLongitude_TEXT].right, m_Button[BN_OwnLongitude_TEXT].bottom + 1 + buttonHightNomal);
	//m_Button[BN_OwnSpeedOverGround_TEXT] = CRect(m_Button[BN_OwnCourseOverGround_TEXT].left, m_Button[BN_OwnCourseOverGround_TEXT].bottom + 1, m_Button[BN_OwnCourseOverGround_TEXT].right, m_Button[BN_OwnCourseOverGround_TEXT].bottom + 1 + buttonHightNomal);
	//m_Button[BN_OwnSpeed_TEXT] = CRect(m_Button[BN_OwnHeading_TEXT].left, m_Button[BN_OwnHeading_TEXT].bottom + 1, m_Button[BN_OwnHeading_TEXT].right, m_Button[BN_OwnHeading_TEXT].bottom + 1 + buttonHightNomal);

	//vts radar
	m_Button[BN_VTS_Radar_TEXT_1] = CRect(m_viewRect.right - m_viewRect.Width() / 5, 1, m_viewRect.right - 20, 1 + buttonHightNomal);
	m_Button[BN_VTS_Radar_TEXT_2] = CRect(m_Button[BN_VTS_Radar_TEXT_1].left, m_Button[BN_VTS_Radar_TEXT_1].bottom + 1, m_Button[BN_VTS_Radar_TEXT_1].right, m_Button[BN_VTS_Radar_TEXT_1].bottom + 1 + buttonHightNomal);
	m_Button[BN_VTS_Radar_TEXT_3] = CRect(m_Button[BN_VTS_Radar_TEXT_2].left, m_Button[BN_VTS_Radar_TEXT_2].bottom + 1, m_Button[BN_VTS_Radar_TEXT_2].right, m_Button[BN_VTS_Radar_TEXT_2].bottom + 1 + buttonHightNomal);
	m_Button[BN_VTS_Radar_TEXT_4] = CRect(m_Button[BN_VTS_Radar_TEXT_3].left, m_Button[BN_VTS_Radar_TEXT_3].bottom + 1, m_Button[BN_VTS_Radar_TEXT_3].right, m_Button[BN_VTS_Radar_TEXT_3].bottom + 1 + buttonHightNomal);
	m_Button[BN_VTS_Radar_TEXT_5] = CRect(m_Button[BN_VTS_Radar_TEXT_4].left, m_Button[BN_VTS_Radar_TEXT_4].bottom + 1, m_Button[BN_VTS_Radar_TEXT_4].right, m_Button[BN_VTS_Radar_TEXT_4].bottom + 1 + buttonHightNomal);
	m_Button[BN_VTS_Radar_UP] = CRect(m_Button[BN_VTS_Radar_TEXT_1].right + 1, 1, m_viewRect.right - 5, (1 + m_Button[BN_VTS_Radar_TEXT_5].bottom) / 2);
	m_Button[BN_VTS_Radar_DOWN] = CRect(m_Button[BN_VTS_Radar_UP].left, m_Button[BN_VTS_Radar_UP].bottom + 1, m_Button[BN_VTS_Radar_UP].right, m_Button[BN_VTS_Radar_TEXT_5].bottom);

	//运动模式
	//m_Button[BN_MotionMode_SWITCH] = CRect(m_viewRect.Width() * 5 / 8, 1, m_viewRect.Width() * 5 / 8 + m_viewRect.Width() / 20, 1 + buttonHightNomal);
	//m_Button[BN_DisplayMode_SWITCH] = CRect(m_Button[BN_MotionMode_SWITCH].right + 3, 1, m_Button[BN_OwnLongitude_TEXT].left - 3, 1 + buttonHightNomal);
	//m_Button[BN_Alarm_2_SWITCH] = CRect(m_Button[BN_DisplayMode_SWITCH].right - m_viewRect.Width() * 3 / 40, m_Button[BN_DisplayMode_SWITCH].bottom + 3, m_Button[BN_DisplayMode_SWITCH].right, m_Button[BN_DisplayMode_SWITCH].bottom + 3 + buttonHightNomal);
	//m_Button[BN_Alarm_1_SWITCH] = CRect(m_Button[BN_Alarm_2_SWITCH].left - 3 - m_Button[BN_Alarm_2_SWITCH].Width(), m_Button[BN_Alarm_2_SWITCH].top, m_Button[BN_Alarm_2_SWITCH].left - 3, m_Button[BN_Alarm_2_SWITCH].bottom);
	//m_Button[BN_ECHOStretch_SWITCH] = CRect(m_Button[BN_Alarm_2_SWITCH].right - m_viewRect.Width() / 8, m_Button[BN_Alarm_2_SWITCH].bottom + 3, m_Button[BN_Alarm_2_SWITCH].right, m_Button[BN_Alarm_2_SWITCH].bottom + 3 + buttonHightNomal);
	//m_Button[BN_AlarmACK_SWITCH] = CRect(m_Button[BN_ECHOStretch_SWITCH].right - m_viewRect.Width() * 7 / 80, m_Button[BN_ECHOStretch_SWITCH].bottom + 3, m_Button[BN_ECHOStretch_SWITCH].right, m_Button[BN_ECHOStretch_SWITCH].bottom + 3 + buttonHightNomal);
	m_Button[BN_DayNight_SWITCH] = CRect(m_Button[BN_VTS_Radar_TEXT_1].left - m_viewRect.Width() / 16, 1, m_Button[BN_VTS_Radar_TEXT_1].left - 1, 1 + buttonHightNomal);

	//矢量、航迹、报警
	//m_Button[BN_VectorTime_SWITCH] = CRect(m_Button[BN_OwnSpeed_TEXT].left, m_Button[BN_OwnSpeed_TEXT].bottom + 3, m_Button[BN_OwnLatitude_TEXT].right, m_Button[BN_OwnSpeed_TEXT].bottom + 3 + buttonHightSmall);
	//m_Button[BN_TrackTime_SWITCH] = CRect(m_Button[BN_VectorTime_SWITCH].left, m_Button[BN_VectorTime_SWITCH].bottom + 3, m_Button[BN_VectorTime_SWITCH].right, m_Button[BN_VectorTime_SWITCH].bottom + 3 + buttonHightSmall);
	//m_Button[BN_CPAAlarmDistance_TEXT] = CRect(m_Button[BN_TrackTime_SWITCH].left, m_Button[BN_TrackTime_SWITCH].bottom + 3, m_Button[BN_TrackTime_SWITCH].right, m_Button[BN_TrackTime_SWITCH].bottom + 3 + buttonHightSmall);
	//m_Button[BN_TCPAAlarmTime_TEXT] = CRect(m_Button[BN_CPAAlarmDistance_TEXT].left, m_Button[BN_CPAAlarmDistance_TEXT].bottom + 3, m_Button[BN_CPAAlarmDistance_TEXT].right, m_Button[BN_CPAAlarmDistance_TEXT].bottom + 3 + buttonHightSmall);

	//回波控制
	//m_Button[BN_GAIN_BAR] = CRect(m_Button[BN_VTS_Radar_Roll_BAR].right - m_viewRect.Width() / 8, m_Button[BN_VTS_Radar_TEXT_5].bottom + 3, m_Button[BN_VTS_Radar_Roll_BAR].right, m_Button[BN_VTS_Radar_TEXT_5].bottom + 3 + buttonHightSmall);
	//m_Button[BN_GAIN_TEXT] = CRect(m_Button[BN_VTS_Radar_TEXT_5].left, m_Button[BN_GAIN_BAR].top, m_Button[BN_GAIN_BAR].left - 1, m_Button[BN_GAIN_BAR].bottom);
	//m_Button[BN_Wave_BAR] = CRect(m_Button[BN_GAIN_BAR].left, m_Button[BN_GAIN_BAR].bottom + 1, m_Button[BN_GAIN_BAR].right, m_Button[BN_GAIN_BAR].bottom + 1 + buttonHightSmall);
	//m_Button[BN_Wave_TEXT] = CRect(m_Button[BN_GAIN_TEXT].left, m_Button[BN_Wave_BAR].top, m_Button[BN_GAIN_TEXT].right, m_Button[BN_Wave_BAR].bottom);
	//m_Button[BN_Rain_BAR] = CRect(m_Button[BN_Wave_BAR].left, m_Button[BN_Wave_BAR].bottom + 1, m_Button[BN_Wave_BAR].right, m_Button[BN_Wave_BAR].bottom + 1 + buttonHightSmall);
	//m_Button[BN_Rain_TEXT] = CRect(m_Button[BN_Wave_TEXT].left, m_Button[BN_Rain_BAR].top, m_Button[BN_Wave_TEXT].right, m_Button[BN_Rain_BAR].bottom);
	//m_Button[BN_Tune_BAR] = CRect(m_Button[BN_Rain_BAR].left, m_Button[BN_Rain_BAR].bottom + 1, m_Button[BN_Rain_BAR].right, m_Button[BN_Rain_BAR].bottom + 1 + buttonHightSmall);
	//m_Button[BN_Tune_Text] = CRect(m_Button[BN_Rain_TEXT].left, m_Button[BN_Tune_BAR].top, m_Button[BN_Rain_TEXT].right, m_Button[BN_Tune_BAR].bottom);


	//增强功能、系统设置
	m_Button[BN_SystemTime_TEXT] = CRect(m_Button[BN_VTS_Radar_TEXT_1].left, m_viewRect.bottom - 5 - buttonHightNomal, m_viewRect.right - 5, m_viewRect.bottom - 5);
	//m_Button[BN_StartOrStop_SWITCH] = CRect(m_Button[BN_SystemTime_TEXT].left, m_Button[BN_SystemTime_TEXT].top - 3 - buttonHightNomal, m_Button[BN_SystemTime_TEXT].right, m_Button[BN_SystemTime_TEXT].top - 3);
	//m_Button[BN_Settings] = CRect(m_viewRect.right - m_viewRect.Width() / 20, m_Button[BN_StartOrStop_SWITCH].top - 3 - buttonHightNomal, m_Button[BN_StartOrStop_SWITCH].right, m_Button[BN_StartOrStop_SWITCH].top - 3);
	//m_Button[BN_CourseAndMarker] = CRect(m_viewRect.right - m_viewRect.Width() / 10, m_Button[BN_Settings].top, m_Button[BN_Settings].left - 1, m_Button[BN_Settings].bottom);
	m_Button[BN_AIS] = CRect(m_Button[BN_SystemTime_TEXT].left, m_Button[BN_SystemTime_TEXT].top - 1 - buttonHightNomal, m_Button[BN_SystemTime_TEXT].right, m_Button[BN_SystemTime_TEXT].top - 1);
	//m_Button[BN_ARPA] = CRect(m_Button[BN_SystemTime_TEXT].left, m_Button[BN_AIS].top, m_Button[BN_AIS].left - 1, m_Button[BN_AIS].bottom);
	m_Button[BN_AIS_SWITCH] = CRect(m_Button[BN_AIS].left, m_Button[BN_AIS].top - buttonHightNomal - 1, (m_Button[BN_AIS].left + m_Button[BN_AIS].right) / 2, m_Button[BN_AIS].top - 1);
	m_Button[BN_AIS_INFORMATION_SWITCH] = CRect(m_Button[BN_AIS_SWITCH].right + 1, m_Button[BN_AIS_SWITCH].top, m_Button[BN_AIS].right, m_Button[BN_AIS_SWITCH].bottom);
	//m_Button[BN_ARPA_VECTORTIME_SWITCH] = CRect(m_Button[BN_ARPA].left, m_Button[BN_ARPA].top - buttonHightNomal - 1, m_Button[BN_AIS].right, m_Button[BN_ARPA].top - 1);
	//m_Button[BN_ARPA_T_R_VECTOR_SWITCH] = CRect(m_Button[BN_CourseAndMarker].left, m_Button[BN_ARPA_VECTORTIME_SWITCH].top, m_Button[BN_Settings].right, m_Button[BN_ARPA_VECTORTIME_SWITCH].bottom);
	//m_Button[BN_ARPA_SWITCH] = CRect(m_Button[BN_ARPA].left, m_Button[BN_ARPA_VECTORTIME_SWITCH].top - buttonHightNomal - 1, m_Button[BN_AIS].right, m_Button[BN_ARPA_VECTORTIME_SWITCH].top - 1);
	//m_Button[BN_ARPA_RECORDTARGET_SWITCH] = CRect(m_Button[BN_CourseAndMarker].left, m_Button[BN_ARPA_SWITCH].top, m_Button[BN_Settings].right, m_Button[BN_ARPA_SWITCH].bottom);

	//m_ARPAControlBoxRect = CRect(m_Button[BN_ARPA_SWITCH].left - 1, m_Button[BN_ARPA_SWITCH].top - 1, m_Button[BN_ARPA_T_R_VECTOR_SWITCH].right + 1, m_Button[BN_ARPA_T_R_VECTOR_SWITCH].bottom + 1);
	//m_ARPAControlBoxRgn.CreateRectRgnIndirect(m_ARPAControlBoxRect);
	m_AISControlBoxRect = CRect(m_Button[BN_AIS_SWITCH].left - 1, m_Button[BN_AIS_SWITCH].top - 1, m_Button[BN_AIS_INFORMATION_SWITCH].right + 1, m_Button[BN_AIS_INFORMATION_SWITCH].bottom + 1);
	m_AISControlBoxRgn.CreateRectRgnIndirect(m_AISControlBoxRect);

	//语言、目标信息
	m_Button[BN_TargetLatitude_TEXT] = CRect(m_Button[BN_SystemTime_TEXT].left - 3 - m_viewRect.Width() * 9 / 80, m_Button[BN_SystemTime_TEXT].top, m_Button[BN_SystemTime_TEXT].left - 3, m_Button[BN_SystemTime_TEXT].bottom);
	m_Button[BN_TargetLongitude_TEXT] = CRect(m_Button[BN_TargetLatitude_TEXT].left - 1 - m_viewRect.Width() * 9 / 80, m_Button[BN_TargetLatitude_TEXT].top, m_Button[BN_TargetLatitude_TEXT].left - 1, m_Button[BN_TargetLatitude_TEXT].bottom);
	m_Button[BN_TargetBearing_TEXT] = CRect(m_Button[BN_TargetLatitude_TEXT].right - m_viewRect.Width() * 13 / 80, m_Button[BN_TargetLatitude_TEXT].top - 1 - buttonHightNomal, m_Button[BN_TargetLatitude_TEXT].right, m_Button[BN_TargetLatitude_TEXT].top - 1);
	m_Button[BN_TargetRange_TEXT] = CRect(m_Button[BN_TargetBearing_TEXT].right - m_viewRect.Width() / 8, m_Button[BN_TargetBearing_TEXT].top - 1 - buttonHightNomal, m_Button[BN_TargetBearing_TEXT].right, m_Button[BN_TargetBearing_TEXT].top - 1);
	m_Button[BN_EchoAveraging_SWITCH] = CRect(m_Button[BN_TargetRange_TEXT].right - m_viewRect.Width() * 7 / 80, m_Button[BN_TargetRange_TEXT].top - 3 - buttonHightNomal, m_Button[BN_TargetRange_TEXT].right, m_Button[BN_TargetRange_TEXT].top - 3);
	m_Button[BN_Language_SWITCH] = CRect(m_Button[BN_EchoAveraging_SWITCH].right - m_viewRect.Width() / 16, m_Button[BN_EchoAveraging_SWITCH].top - 3 - buttonHightNomal, m_Button[BN_EchoAveraging_SWITCH].right, m_Button[BN_EchoAveraging_SWITCH].top - 3);


	//信息窗
	m_Button[BN_MessageArea_1] = CRect(m_Button[BN_VTS_Radar_TEXT_5].left, m_Button[BN_VTS_Radar_TEXT_5].bottom + 3, m_Button[BN_VTS_Radar_UP].right, m_Button[BN_VTS_Radar_TEXT_5].bottom + 3 + m_viewRect.Height() * 4 / 15);
	m_Button[BN_MessageArea_2] = CRect(m_Button[BN_MessageArea_1].left, m_Button[BN_MessageArea_1].bottom + 1, m_Button[BN_MessageArea_1].right, m_Button[BN_AIS_SWITCH].top - 2);

}
void CRadarView::ButtonShow(Graphics *graphics, MyMemDC *pMemDC)
{
	SolidBrush brush_green100(Color(100, 0, 166, 0));

	SolidBrush brush_Word(Color(0, 0, 0, 0));
	SolidBrush brush_WordSelected(Color(0, 0, 0, 0));
	Pen pen_RectSelected(Color(0, 0, 0, 0), 2);
	Pen pen_RectUnSelected(Color(0, 0, 0, 0), 2);
	if (m_dayOrNight)
	{
		brush_Word.SetColor(Color(255, 255, 255, 255));
		brush_WordSelected.SetColor(Color(255, 87, 166, 74));
		pen_RectSelected.SetColor(Color(255, 87, 166, 74));
		pen_RectUnSelected.SetColor(Color(255, 100, 100, 100));
	}
	else
	{
		brush_Word.SetColor(Color(255, 42, 255, 41));
		brush_WordSelected.SetColor(Color(255, 189, 99, 188));
		pen_RectSelected.SetColor(Color(255, 78, 201, 176));
		pen_RectUnSelected.SetColor(Color(255, 42, 140, 41));
	}

	for (int i = 0; i < BUTTONNUM; i++)
	{
		if (i == BN_AIS_SWITCH || i == BN_AIS_INFORMATION_SWITCH)
		{
			if (!m_AISControlBoxDisplay)
				continue;
		}
		/*if (i == BN_ARPA_SWITCH || i == BN_ARPA_RECORDTARGET_SWITCH || i == BN_ARPA_VECTORTIME_SWITCH || i == BN_ARPA_T_R_VECTOR_SWITCH)
		{
		if (!m_ARPAControlBoxDisplay)
		continue;
		}*/
		if (m_ButtonSelect[i])
			graphics->DrawRectangle(&pen_RectSelected, m_Button[i].left, m_Button[i].top, m_Button[i].Width(), m_Button[i].Height());
		else
			graphics->DrawRectangle(&pen_RectUnSelected, m_Button[i].left, m_Button[i].top, m_Button[i].Width(), m_Button[i].Height());
	}

	int size_EN_Nomal = 18 * m_mulOf800_600;
	int size_CHN_Nomal = 20 * m_mulOf800_600;
	int size_EN_Small = 10 * m_mulOf800_600;
	int size_CHN_Small = 16 * m_mulOf800_600;
	int size_CHN_Smallest = 12 * m_mulOf800_600;
	Gdiplus::Font font_CHN_Nomal(_T("隶书"), size_CHN_Nomal, FontStyleRegular, UnitPixel);
	Gdiplus::Font font_EN_Nomal(_T("Arial"), size_EN_Nomal, FontStyleRegular, UnitPixel);
	Gdiplus::Font font_CHN_Small(_T("隶书"), size_CHN_Small, FontStyleRegular, UnitPixel);
	Gdiplus::Font font_CHN_Smallest(_T("隶书"), size_CHN_Smallest, FontStyleRegular, UnitPixel);
	Gdiplus::Font font_EN_Small(_T("Arial"), size_EN_Small, FontStyleRegular, UnitPixel);
	StringFormat stringFormat;
	stringFormat.SetLineAlignment(StringAlignmentCenter);
	CString stemp;

	int font_left_nomal = (double)m_viewRect.Width() / 160;	//文字距离按钮边框左侧距离，在800*600下为5像素
	int font_left_small = (double)m_viewRect.Width() / 400;	//文字距离按钮边框左侧距离，在800*600下为2像素
	//BN_Range_TEXT
	if (m_language)
		graphics->DrawString(L"量程", -1, &font_CHN_Nomal, PointF(m_Button[BN_Range_TEXT].left + font_left_nomal, (m_Button[BN_Range_TEXT].top + m_Button[BN_Range_TEXT].bottom) / 2), &stringFormat, &brush_Word);
	else
		graphics->DrawString(L"Range", -1, &font_EN_Nomal, PointF(m_Button[BN_Range_TEXT].left + font_left_nomal, (m_Button[BN_Range_TEXT].top + m_Button[BN_Range_TEXT].bottom) / 2), &stringFormat, &brush_Word);
	CString range("--");
	if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
		range.Format(_T("%.1f"), m_vtsRadar[m_vtsRadar_selected].getRange() / 1000);
	}
	stemp.Format(_T("%s km"), range);
	graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF((m_Button[BN_Range_TEXT].left + m_Button[BN_Range_TEXT].right) * 3 / 5, (m_Button[BN_Range_TEXT].top + m_Button[BN_Range_TEXT].bottom) / 2), &stringFormat, &brush_Word);

	//BN_Range_up
	//graphics->DrawLine(&pen_RectSelected, Point((m_Button[BN_Range_up].left + m_Button[BN_Range_up].right) / 2, m_Button[BN_Range_up].top + 3), Point(m_Button[BN_Range_up].left + 5, m_Button[BN_Range_up].bottom - 3));
	//graphics->DrawLine(&pen_RectSelected, Point((m_Button[BN_Range_up].left + m_Button[BN_Range_up].right) / 2, m_Button[BN_Range_up].top + 3), Point(m_Button[BN_Range_up].right - 5, m_Button[BN_Range_up].bottom - 3));
	//graphics->DrawLine(&pen_RectSelected, Point(m_Button[BN_Range_up].left + 5, m_Button[BN_Range_up].bottom - 3), Point(m_Button[BN_Range_up].right - 5, m_Button[BN_Range_up].bottom - 3));

	//BN_Range_down
	//graphics->DrawLine(&pen_RectSelected, Point((m_Button[BN_Range_down].left + m_Button[BN_Range_down].right) / 2, m_Button[BN_Range_down].bottom - 3), Point(m_Button[BN_Range_down].left + 5, m_Button[BN_Range_down].top + 3));
	//graphics->DrawLine(&pen_RectSelected, Point((m_Button[BN_Range_down].left + m_Button[BN_Range_down].right) / 2, m_Button[BN_Range_down].bottom - 3), Point(m_Button[BN_Range_down].right - 5, m_Button[BN_Range_down].top + 3));
	//graphics->DrawLine(&pen_RectSelected, Point(m_Button[BN_Range_down].left + 5, m_Button[BN_Range_down].top + 3), Point(m_Button[BN_Range_down].right - 5, m_Button[BN_Range_down].top + 3));

	//BN_RangeRings_SWITCH
	if (m_language)
		graphics->DrawString(L"距标", -1, &font_CHN_Nomal, PointF(m_Button[BN_RangeRings_SWITCH].left + font_left_nomal, (m_Button[BN_RangeRings_SWITCH].top + m_Button[BN_RangeRings_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	else
		graphics->DrawString(L"RR", -1, &font_EN_Nomal, PointF(m_Button[BN_RangeRings_SWITCH].left + font_left_nomal, (m_Button[BN_RangeRings_SWITCH].top + m_Button[BN_RangeRings_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_rangeRingsSwitch)
	{
		CString rangeRingsDistance("--");
		if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
			rangeRingsDistance.Format(_T("%.1f"), m_vtsRadar[m_vtsRadar_selected].getRange() / 5000);
		}
		stemp.Format(_T("%s km"), rangeRingsDistance);
		graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF((m_Button[BN_RangeRings_SWITCH].left + m_Button[BN_RangeRings_SWITCH].right) * 3 / 5, (m_Button[BN_RangeRings_SWITCH].top + m_Button[BN_RangeRings_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{

		if (m_language)
			graphics->DrawString(L"关", -1, &font_CHN_Nomal, PointF((m_Button[BN_RangeRings_SWITCH].left + m_Button[BN_RangeRings_SWITCH].right) * 3 / 5, (m_Button[BN_RangeRings_SWITCH].top + m_Button[BN_RangeRings_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Nomal, PointF((m_Button[BN_RangeRings_SWITCH].left + m_Button[BN_RangeRings_SWITCH].right) * 3 / 5, (m_Button[BN_RangeRings_SWITCH].top + m_Button[BN_RangeRings_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	double scale = 0;
	if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
		scale = m_vtsRadar[m_vtsRadar_selected].getRange() / (m_radarRect.Width() / 2) / 1000;
	}

	//BN_VariableRangeMarker_1_SWITCH
	graphics->DrawString(L"VRM1", -1, &font_EN_Small, PointF(m_Button[BN_VariableRangeMarker_1_SWITCH].left + font_left_nomal, (m_Button[BN_VariableRangeMarker_1_SWITCH].top + m_Button[BN_VariableRangeMarker_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_variableRangeMarker1Switch)
		{
			stemp.Format(_T("%.3f km"), m_VRM1Pixel*scale);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_1_SWITCH].left + m_Button[BN_VariableRangeMarker_1_SWITCH].right) * 4 / 7, (m_Button[BN_VariableRangeMarker_1_SWITCH].top + m_Button[BN_VariableRangeMarker_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}

		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF((m_Button[BN_VariableRangeMarker_1_SWITCH].left + m_Button[BN_VariableRangeMarker_1_SWITCH].right) * 3 / 5, (m_Button[BN_VariableRangeMarker_1_SWITCH].top + m_Button[BN_VariableRangeMarker_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_variableRangeMarker1Switch)
		{
			stemp.Format(_T("%.3f km"), m_VRM1Pixel*scale);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_1_SWITCH].left + m_Button[BN_VariableRangeMarker_1_SWITCH].right) * 4 / 7, (m_Button[BN_VariableRangeMarker_1_SWITCH].top + m_Button[BN_VariableRangeMarker_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_1_SWITCH].left + m_Button[BN_VariableRangeMarker_1_SWITCH].right) * 3 / 5, (m_Button[BN_VariableRangeMarker_1_SWITCH].top + m_Button[BN_VariableRangeMarker_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_VariableRangeMarker_2_SWITCH
	graphics->DrawString(L"VRM2", -1, &font_EN_Small, PointF(m_Button[BN_VariableRangeMarker_2_SWITCH].left + font_left_nomal, (m_Button[BN_VariableRangeMarker_2_SWITCH].top + m_Button[BN_VariableRangeMarker_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_variableRangeMarker2Switch)
		{
			stemp.Format(_T("%.3f km"), m_VRM2Pixel*scale);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_2_SWITCH].left + m_Button[BN_VariableRangeMarker_2_SWITCH].right) * 4 / 7, (m_Button[BN_VariableRangeMarker_2_SWITCH].top + m_Button[BN_VariableRangeMarker_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}

		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF((m_Button[BN_VariableRangeMarker_2_SWITCH].left + m_Button[BN_VariableRangeMarker_2_SWITCH].right) * 3 / 5, (m_Button[BN_VariableRangeMarker_2_SWITCH].top + m_Button[BN_VariableRangeMarker_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_variableRangeMarker2Switch)
		{
			stemp.Format(_T("%.3f km"), m_VRM2Pixel*scale);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_2_SWITCH].left + m_Button[BN_VariableRangeMarker_2_SWITCH].right) * 4 / 7, (m_Button[BN_VariableRangeMarker_2_SWITCH].top + m_Button[BN_VariableRangeMarker_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF((m_Button[BN_VariableRangeMarker_2_SWITCH].left + m_Button[BN_VariableRangeMarker_2_SWITCH].right) * 3 / 5, (m_Button[BN_VariableRangeMarker_2_SWITCH].top + m_Button[BN_VariableRangeMarker_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_ElectronicBearingLine_1_SWITCH
	graphics->DrawString(L"EBL1", -1, &font_EN_Small, PointF(m_Button[BN_ElectronicBearingLine_1_SWITCH].left + font_left_nomal, (m_Button[BN_ElectronicBearingLine_1_SWITCH].top + m_Button[BN_ElectronicBearingLine_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_electronicBearingLine1Switch)
		{
			stemp.Format(_T("%.1f"), m_electronicBearingLine1Num);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_1_SWITCH].left + m_Button[BN_ElectronicBearingLine_1_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_1_SWITCH].top + m_Button[BN_ElectronicBearingLine_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}

		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF((m_Button[BN_ElectronicBearingLine_1_SWITCH].left + m_Button[BN_ElectronicBearingLine_1_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_1_SWITCH].top + m_Button[BN_ElectronicBearingLine_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_electronicBearingLine1Switch)
		{
			stemp.Format(_T("%.1f"), m_electronicBearingLine1Num);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_1_SWITCH].left + m_Button[BN_ElectronicBearingLine_1_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_1_SWITCH].top + m_Button[BN_ElectronicBearingLine_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_1_SWITCH].left + m_Button[BN_ElectronicBearingLine_1_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_1_SWITCH].top + m_Button[BN_ElectronicBearingLine_1_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_ElectronicBearingLine_2_SWITCH
	graphics->DrawString(L"EBL2", -1, &font_EN_Small, PointF(m_Button[BN_ElectronicBearingLine_2_SWITCH].left + font_left_nomal, (m_Button[BN_ElectronicBearingLine_2_SWITCH].top + m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_electronicBearingLine2Switch)
		{
			stemp.Format(_T("%.1f"), m_electronicBearingLine2Num);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_2_SWITCH].left + m_Button[BN_ElectronicBearingLine_2_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_2_SWITCH].top + m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}

		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF((m_Button[BN_ElectronicBearingLine_2_SWITCH].left + m_Button[BN_ElectronicBearingLine_2_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_2_SWITCH].top + m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_electronicBearingLine2Switch)
		{
			stemp.Format(_T("%.1f"), m_electronicBearingLine2Num);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_2_SWITCH].left + m_Button[BN_ElectronicBearingLine_2_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_2_SWITCH].top + m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF((m_Button[BN_ElectronicBearingLine_2_SWITCH].left + m_Button[BN_ElectronicBearingLine_2_SWITCH].right) * 3 / 5, (m_Button[BN_ElectronicBearingLine_2_SWITCH].top + m_Button[BN_ElectronicBearingLine_2_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_ExtendedRangeMeasurement_SWITCH
	if (m_language)
	{
		graphics->DrawString(L"测距", -1, &font_CHN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		if (m_extendedRangeMeasurementSwitch)
			graphics->DrawString(L"开", -1, &font_CHN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left + m_Button[BN_ExtendedRangeMeasurement_SWITCH].Width() * 3 / 5, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left + m_Button[BN_ExtendedRangeMeasurement_SWITCH].Width() * 3 / 5, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		graphics->DrawString(L"ERM", -1, &font_EN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left + font_left_nomal, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		if (m_extendedRangeMeasurementSwitch)
			graphics->DrawString(L"ON", -1, &font_EN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left + m_Button[BN_ExtendedRangeMeasurement_SWITCH].Width() * 3 / 5, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF(m_Button[BN_ExtendedRangeMeasurement_SWITCH].left + m_Button[BN_ExtendedRangeMeasurement_SWITCH].Width() * 3 / 5, (m_Button[BN_ExtendedRangeMeasurement_SWITCH].top + m_Button[BN_ExtendedRangeMeasurement_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_TargetBearing_SWITCH
	graphics->DrawString(L"BRG", -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_SWITCH].left + font_left_small, (m_Button[BN_TargetBearing_SWITCH].top + m_Button[BN_TargetBearing_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_extendedRangeMeasurementSwitch)
		{
			stemp.Format(_T("%.1f"), m_targetBearingNum);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_SWITCH].left + m_Button[BN_TargetBearing_SWITCH].Width() / 2, (m_Button[BN_TargetBearing_SWITCH].top + m_Button[BN_TargetBearing_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF(m_Button[BN_TargetBearing_SWITCH].left + m_Button[BN_TargetBearing_SWITCH].Width() * 3 / 5, (m_Button[BN_TargetBearing_SWITCH].top + m_Button[BN_TargetBearing_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_extendedRangeMeasurementSwitch)
		{
			stemp.Format(_T("%.1f"), m_targetBearingNum);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_SWITCH].left + m_Button[BN_TargetBearing_SWITCH].Width() / 2, (m_Button[BN_TargetBearing_SWITCH].top + m_Button[BN_TargetBearing_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_SWITCH].left + m_Button[BN_TargetBearing_SWITCH].Width() * 3 / 5, (m_Button[BN_TargetBearing_SWITCH].top + m_Button[BN_TargetBearing_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_TargetRange_SWITCH
	graphics->DrawString(L"RNG", -1, &font_EN_Small, PointF(m_Button[BN_TargetRange_SWITCH].left + font_left_small, (m_Button[BN_TargetRange_SWITCH].top + m_Button[BN_TargetRange_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	if (m_language)
	{
		if (m_extendedRangeMeasurementSwitch)
		{
			stemp.Format(_T("%.3f"), m_targetRangeNum);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetRange_SWITCH].left + m_Button[BN_TargetRange_SWITCH].Width() / 2, (m_Button[BN_TargetRange_SWITCH].top + m_Button[BN_TargetRange_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"关", -1, &font_CHN_Small, PointF(m_Button[BN_TargetRange_SWITCH].left + m_Button[BN_TargetRange_SWITCH].Width() * 3 / 5, (m_Button[BN_TargetRange_SWITCH].top + m_Button[BN_TargetRange_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		if (m_extendedRangeMeasurementSwitch)
		{
			stemp.Format(_T("%.3f"), m_targetRangeNum);
			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetRange_SWITCH].left + m_Button[BN_TargetRange_SWITCH].Width() / 2, (m_Button[BN_TargetRange_SWITCH].top + m_Button[BN_TargetRange_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF(m_Button[BN_TargetRange_SWITCH].left + m_Button[BN_TargetRange_SWITCH].Width() * 3 / 5, (m_Button[BN_TargetRange_SWITCH].top + m_Button[BN_TargetRange_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_DayNight_SWITCH
	if (m_language)
	{
		if (m_dayOrNight)
		{
			graphics->DrawString(L"日", -1, &font_CHN_Nomal, PointF(m_Button[BN_DayNight_SWITCH].left + (m_Button[BN_DayNight_SWITCH].Width() - size_CHN_Nomal*2.5) / 2, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			graphics->DrawString(L"/夜", -1, &font_CHN_Small, PointF(m_Button[BN_DayNight_SWITCH].left + (m_Button[BN_DayNight_SWITCH].Width() - size_CHN_Nomal*2.5) / 2 + size_EN_Nomal, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
		{
			graphics->DrawString(L"日/", -1, &font_CHN_Small, PointF(m_Button[BN_DayNight_SWITCH].left + (m_Button[BN_DayNight_SWITCH].Width() - size_CHN_Nomal*2.5) / 2, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			graphics->DrawString(L"夜", -1, &font_CHN_Nomal, PointF(m_Button[BN_DayNight_SWITCH].left + (m_Button[BN_DayNight_SWITCH].Width() - size_CHN_Nomal*2.5) / 2 + size_EN_Nomal*1.35, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}

	}
	else
	{
		if (m_dayOrNight)
		{
			graphics->DrawString(L"D", -1, &font_EN_Nomal, PointF(m_Button[BN_DayNight_SWITCH].left + m_Button[BN_DayNight_SWITCH].Width() / 2 - size_EN_Small * 2, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			graphics->DrawString(L"/  N", -1, &font_EN_Small, PointF(m_Button[BN_DayNight_SWITCH].left + m_Button[BN_DayNight_SWITCH].Width() / 2 - size_EN_Small * 2 + size_EN_Nomal, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
		{
			graphics->DrawString(L"D  /", -1, &font_EN_Small, PointF(m_Button[BN_DayNight_SWITCH].left + m_Button[BN_DayNight_SWITCH].Width() / 2 - size_EN_Small * 2, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			graphics->DrawString(L"N", -1, &font_EN_Nomal, PointF(m_Button[BN_DayNight_SWITCH].left + m_Button[BN_DayNight_SWITCH].Width() / 2 - size_EN_Small * 2 + size_EN_Nomal, (m_Button[BN_DayNight_SWITCH].top + m_Button[BN_DayNight_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
	}
	//BN_MessageArea_1
	graphics->DrawString(L"-- Radar Message --", -1, &font_EN_Nomal, PointF(m_Button[BN_MessageArea_1].left + (m_Button[BN_Language_SWITCH].Width() - size_EN_Nomal*1.5) / 2, m_Button[BN_MessageArea_1].top + m_Button[BN_MessageArea_1].Height() / 7), &stringFormat, &brush_Word);
	if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
		const VtsRadar& vtsRadar = m_vtsRadar[m_vtsRadar_selected];
		stemp.Format(_T("lat:%.4f°"), vtsRadar.getLat());
		graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF(m_Button[BN_MessageArea_1].left + m_Button[BN_MessageArea_1].Width() / 5, m_Button[BN_MessageArea_1].top + m_Button[BN_MessageArea_1].Height() * 2 / 6), &stringFormat, &brush_Word);
		stemp.Format(_T("lng:%.4f°"), vtsRadar.getLng());
		graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF(m_Button[BN_MessageArea_1].left + m_Button[BN_MessageArea_1].Width() / 5, m_Button[BN_MessageArea_1].top + m_Button[BN_MessageArea_1].Height() * 3 / 6), &stringFormat, &brush_Word);
		stemp.Format(_T("range:%.0f m"), vtsRadar.getRange());
		graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF(m_Button[BN_MessageArea_1].left + m_Button[BN_MessageArea_1].Width() / 5, m_Button[BN_MessageArea_1].top + m_Button[BN_MessageArea_1].Height() * 4 / 6), &stringFormat, &brush_Word);
		stemp.Format(_T("height:%.0f m"), vtsRadar.getHeight());
		graphics->DrawString(stemp, -1, &font_EN_Nomal, PointF(m_Button[BN_MessageArea_1].left + m_Button[BN_MessageArea_1].Width() / 5, m_Button[BN_MessageArea_1].top + m_Button[BN_MessageArea_1].Height() * 5 / 6), &stringFormat, &brush_Word);
	}

	//BN_Language_SWITCH
	if (m_language)
	{
		graphics->DrawString(L"中", -1, &font_CHN_Nomal, PointF(m_Button[BN_Language_SWITCH].left + (m_Button[BN_Language_SWITCH].Width() - size_CHN_Nomal*2.5) / 2, (m_Button[BN_Language_SWITCH].top + m_Button[BN_Language_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		graphics->DrawString(L"/ En", -1, &font_EN_Small, PointF(m_Button[BN_Language_SWITCH].left + (m_Button[BN_Language_SWITCH].Width() - size_CHN_Nomal*2.5) / 2 + size_EN_Nomal*1.2, (m_Button[BN_Language_SWITCH].top + m_Button[BN_Language_SWITCH].bottom) / 2), &stringFormat, &brush_Word);

	}
	else
	{
		graphics->DrawString(L"中/", -1, &font_CHN_Small, PointF(m_Button[BN_Language_SWITCH].left + (m_Button[BN_Language_SWITCH].Width() - size_CHN_Nomal*2.5) / 2, (m_Button[BN_Language_SWITCH].top + m_Button[BN_Language_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		graphics->DrawString(L"En", -1, &font_EN_Nomal, PointF(m_Button[BN_Language_SWITCH].left + (m_Button[BN_Language_SWITCH].Width() - size_CHN_Nomal*2.5) / 2 + size_EN_Nomal*1.25, (m_Button[BN_Language_SWITCH].top + m_Button[BN_Language_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_EchoAveraging_SWITCH
	if (m_language)
	{
		graphics->DrawString(L"杂波抑制", -1, &font_CHN_Smallest, PointF(m_Button[BN_EchoAveraging_SWITCH].left + (m_Button[BN_EchoAveraging_SWITCH].Width() - size_CHN_Smallest * 5 - font_left_nomal*1.6) / 2, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		if (m_echoAveragingSwitch)
			graphics->DrawString(L"开", -1, &font_CHN_Smallest, PointF(m_Button[BN_EchoAveraging_SWITCH].right - size_CHN_Smallest*1.25 - (m_Button[BN_EchoAveraging_SWITCH].Width() - size_CHN_Smallest * 5 - font_left_nomal*1.6) / 2, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		else
			graphics->DrawString(L"关", -1, &font_CHN_Smallest, PointF(m_Button[BN_EchoAveraging_SWITCH].right - size_CHN_Smallest*1.25 - (m_Button[BN_EchoAveraging_SWITCH].Width() - size_CHN_Smallest * 5 - font_left_nomal*1.6) / 2, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}
	else
	{
		graphics->DrawString(L"EAV", -1, &font_EN_Small, PointF(m_Button[BN_EchoAveraging_SWITCH].left + (m_Button[BN_EchoAveraging_SWITCH].Width() - size_CHN_Smallest * 5 - font_left_nomal*1.6) / 2, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		if (m_echoAveragingSwitch)
			graphics->DrawString(L"ON", -1, &font_EN_Small, PointF(m_Button[BN_EchoAveraging_SWITCH].left + m_Button[BN_EchoAveraging_SWITCH].Width() * 4 / 7, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		else
			graphics->DrawString(L"OFF", -1, &font_EN_Small, PointF(m_Button[BN_EchoAveraging_SWITCH].left + m_Button[BN_EchoAveraging_SWITCH].Width() * 4 / 7, (m_Button[BN_EchoAveraging_SWITCH].top + m_Button[BN_EchoAveraging_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
	}

	//BN_TargetRange_TEXT
	stemp.Format(_T("RNG     %.3f"), m_targetRange);
	graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetRange_TEXT].left + (m_Button[BN_TargetRange_TEXT].Width() - size_EN_Small * 7) / 2, (m_Button[BN_TargetRange_TEXT].top + m_Button[BN_TargetRange_TEXT].bottom) / 2), &stringFormat, &brush_Word);

	//BN_TargetBearing_TEXT
	stemp.Format(_T("BRG  T %.1f"), m_targetBearing_T);
	graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_TEXT].left + font_left_nomal, (m_Button[BN_TargetBearing_TEXT].top + m_Button[BN_TargetBearing_TEXT].bottom) / 2), &stringFormat, &brush_Word);
	stemp.Format(_T("R %.1f"), m_targetBearing_R);
	graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetBearing_TEXT].left + m_Button[BN_TargetBearing_TEXT].Width() * 3 / 5, (m_Button[BN_TargetBearing_TEXT].top + m_Button[BN_TargetBearing_TEXT].bottom) / 2), &stringFormat, &brush_Word);

	//BN_TargetLongitude_TEXT
	if (m_targetLongitude > 0)
		stemp.Format(_T("Lon:    %.3fE"), m_targetLongitude);
	else
		stemp.Format(_T("Lon:    %.3fW"), -m_targetLongitude);
	graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetLongitude_TEXT].left + font_left_nomal, (m_Button[BN_TargetLongitude_TEXT].top + m_Button[BN_TargetLongitude_TEXT].bottom) / 2), &stringFormat, &brush_Word);

	//BN_TargetLatitude_TEXT
	if (m_targetLatitude > 0)
		stemp.Format(_T("Lat:    %.3fN"), m_targetLatitude);
	else
		stemp.Format(_T("Lat:    %.3fS"), -m_targetLatitude);
	graphics->DrawString(stemp, -1, &font_EN_Small, PointF(m_Button[BN_TargetLatitude_TEXT].left + font_left_nomal, (m_Button[BN_TargetLatitude_TEXT].top + m_Button[BN_TargetLatitude_TEXT].bottom) / 2), &stringFormat, &brush_Word);

	//BN_AIS
	graphics->DrawString(L"AIS", -1, &font_EN_Small, PointF(m_Button[BN_AIS].left + m_Button[BN_AIS].Width() / 2 - size_EN_Small, (m_Button[BN_AIS].top + m_Button[BN_AIS].bottom) / 2), &stringFormat, &brush_Word);


	//BN_AIS_SWITCH
	if (m_AISControlBoxDisplay)
	{
		if (m_language)
		{
			if (m_AISSwitch)
				graphics->DrawString(L"AIS  开", -1, &font_CHN_Smallest, PointF(m_Button[BN_AIS_SWITCH].left + (m_Button[BN_AIS_SWITCH].Width() - size_CHN_Smallest * 4) / 2, (m_Button[BN_AIS_SWITCH].top + m_Button[BN_AIS_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			else
				graphics->DrawString(L"AIS  关", -1, &font_CHN_Smallest, PointF(m_Button[BN_AIS_SWITCH].left + (m_Button[BN_AIS_SWITCH].Width() - size_CHN_Smallest * 4) / 2, (m_Button[BN_AIS_SWITCH].top + m_Button[BN_AIS_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
		{
			if (m_AISSwitch)
				graphics->DrawString(L"AIS     ON", -1, &font_EN_Small, PointF(m_Button[BN_AIS_SWITCH].left + (m_Button[BN_AIS_SWITCH].Width() - size_CHN_Nomal * 3) / 2, (m_Button[BN_AIS_SWITCH].top + m_Button[BN_AIS_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			else
				graphics->DrawString(L"AIS     OFF", -1, &font_EN_Small, PointF(m_Button[BN_AIS_SWITCH].left + (m_Button[BN_AIS_SWITCH].Width() - size_CHN_Nomal * 3) / 2, (m_Button[BN_AIS_SWITCH].top + m_Button[BN_AIS_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_AIS_INFORMATION_SWITCH
	if (m_AISControlBoxDisplay)
	{
		if (m_language)
		{
			if (m_AISInformationSwitch)
				graphics->DrawString(L"信息显示 开", -1, &font_CHN_Smallest, PointF(m_Button[BN_AIS_INFORMATION_SWITCH].left + (m_Button[BN_AIS_INFORMATION_SWITCH].Width() - size_CHN_Smallest * 6) / 2, (m_Button[BN_AIS_INFORMATION_SWITCH].top + m_Button[BN_AIS_INFORMATION_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			else
				graphics->DrawString(L"信息显示 关", -1, &font_CHN_Smallest, PointF(m_Button[BN_AIS_INFORMATION_SWITCH].left + (m_Button[BN_AIS_INFORMATION_SWITCH].Width() - size_CHN_Smallest * 6) / 2, (m_Button[BN_AIS_INFORMATION_SWITCH].top + m_Button[BN_AIS_INFORMATION_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
		else
		{
			if (m_AISInformationSwitch)
				graphics->DrawString(L"Information ON", -1, &font_EN_Small, PointF(m_Button[BN_AIS_INFORMATION_SWITCH].left + (m_Button[BN_AIS_INFORMATION_SWITCH].Width() - size_EN_Small * 8) / 2, (m_Button[BN_AIS_INFORMATION_SWITCH].top + m_Button[BN_AIS_INFORMATION_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
			else
				graphics->DrawString(L"Information OFF", -1, &font_EN_Small, PointF(m_Button[BN_AIS_INFORMATION_SWITCH].left + (m_Button[BN_AIS_INFORMATION_SWITCH].Width() - size_EN_Small * 8) / 2, (m_Button[BN_AIS_INFORMATION_SWITCH].top + m_Button[BN_AIS_INFORMATION_SWITCH].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	if (m_vtsRadar.size() <= 5){
		m_vtsRadar_first_index = 0;
	}

	//BN_VTS_Radar_TEXT_1
	if (m_vtsRadar.size() >= 1){
		std::string id = m_vtsRadar[m_vtsRadar_first_index].getId();
		int length = id.size() + 1;
		WCHAR buffer[50];
		MultiByteToWideChar(CP_ACP, 0, id.data(), id.size(), buffer, id.size());
		buffer[id.size()] = 0;
		if (m_vtsRadar_selected == m_vtsRadar_first_index){
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_1].left + (m_Button[BN_VTS_Radar_TEXT_1].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_1].top + m_Button[BN_VTS_Radar_TEXT_1].bottom) / 2), &stringFormat, &brush_WordSelected);
		}
		else{
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_1].left + (m_Button[BN_VTS_Radar_TEXT_1].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_1].top + m_Button[BN_VTS_Radar_TEXT_1].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_VTS_Radar_TEXT_2
	if (m_vtsRadar.size() >= 2){
		std::string id = m_vtsRadar[1 + m_vtsRadar_first_index].getId();
		int length = id.size() + 1;
		WCHAR buffer[50];
		MultiByteToWideChar(CP_ACP, 0, id.data(), id.size(), buffer, id.size());
		buffer[id.size()] = 0;
		if (m_vtsRadar_selected == 1 + m_vtsRadar_first_index){
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_2].left + (m_Button[BN_VTS_Radar_TEXT_2].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_2].top + m_Button[BN_VTS_Radar_TEXT_2].bottom) / 2), &stringFormat, &brush_WordSelected);
		}
		else{
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_2].left + (m_Button[BN_VTS_Radar_TEXT_2].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_2].top + m_Button[BN_VTS_Radar_TEXT_2].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_VTS_Radar_TEXT_3
	if (m_vtsRadar.size() >= 3){
		std::string id = m_vtsRadar[2 + m_vtsRadar_first_index].getId();
		int length = id.size() + 1;
		WCHAR buffer[50];
		MultiByteToWideChar(CP_ACP, 0, id.data(), id.size(), buffer, id.size());
		buffer[id.size()] = 0;
		if (m_vtsRadar_selected == 2 + m_vtsRadar_first_index){
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_3].left + (m_Button[BN_VTS_Radar_TEXT_3].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_3].top + m_Button[BN_VTS_Radar_TEXT_3].bottom) / 2), &stringFormat, &brush_WordSelected);
		}
		else{
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_3].left + (m_Button[BN_VTS_Radar_TEXT_3].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_3].top + m_Button[BN_VTS_Radar_TEXT_3].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_VTS_Radar_TEXT_4
	if (m_vtsRadar.size() >= 4){
		std::string id = m_vtsRadar[3 + m_vtsRadar_first_index].getId();
		int length = id.size() + 1;
		WCHAR buffer[50];
		MultiByteToWideChar(CP_ACP, 0, id.data(), id.size(), buffer, id.size());
		buffer[id.size()] = 0;
		if (m_vtsRadar_selected == 3 + m_vtsRadar_first_index){
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_4].left + (m_Button[BN_VTS_Radar_TEXT_4].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_4].top + m_Button[BN_VTS_Radar_TEXT_4].bottom) / 2), &stringFormat, &brush_WordSelected);
		}
		else{
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_4].left + (m_Button[BN_VTS_Radar_TEXT_4].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_4].top + m_Button[BN_VTS_Radar_TEXT_4].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_VTS_Radar_TEXT_5
	if (m_vtsRadar.size() >= 5){
		std::string id = m_vtsRadar[4 + m_vtsRadar_first_index].getId();
		int length = id.size() + 1;
		WCHAR buffer[50];
		MultiByteToWideChar(CP_ACP, 0, id.data(), id.size(), buffer, id.size());
		buffer[id.size()] = 0;
		if (m_vtsRadar_selected == 4 + m_vtsRadar_first_index){
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_5].left + (m_Button[BN_VTS_Radar_TEXT_5].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_5].top + m_Button[BN_VTS_Radar_TEXT_5].bottom) / 2), &stringFormat, &brush_WordSelected);
		}
		else{
			graphics->DrawString(buffer, -1, &font_EN_Nomal, PointF(m_Button[BN_VTS_Radar_TEXT_5].left + (m_Button[BN_VTS_Radar_TEXT_5].Width() - size_EN_Small * 8) / 2, (m_Button[BN_VTS_Radar_TEXT_5].top + m_Button[BN_VTS_Radar_TEXT_5].bottom) / 2), &stringFormat, &brush_Word);
		}
	}

	//BN_VTS_Radar_UP
	Point upTop((m_Button[BN_VTS_Radar_UP].left + m_Button[BN_VTS_Radar_UP].right) / 2, m_Button[BN_VTS_Radar_UP].top + 15);
	graphics->DrawLine(&pen_RectUnSelected, upTop, Point((m_Button[BN_VTS_Radar_UP].left + m_Button[BN_VTS_Radar_UP].right) / 2, m_Button[BN_VTS_Radar_UP].bottom - 10));
	graphics->FillEllipse(&brush_Word, Rect(upTop.X - 5, upTop.Y - 5, 10, 10));

	//BN_VTS_Radar_DOWN
	Point downDown((m_Button[BN_VTS_Radar_UP].left + m_Button[BN_VTS_Radar_UP].right) / 2, m_Button[BN_VTS_Radar_DOWN].bottom - 15);
	graphics->DrawLine(&pen_RectUnSelected, downDown, Point((m_Button[BN_VTS_Radar_UP].left + m_Button[BN_VTS_Radar_UP].right) / 2, m_Button[BN_VTS_Radar_DOWN].top + 10));
	graphics->FillEllipse(&brush_Word, Rect(downDown.X - 5, downDown.Y - 5, 10, 10));

	//BN_SystemTime_TEXT
	graphics->DrawString(m_systemTime, -1, &font_EN_Small, PointF(m_Button[BN_SystemTime_TEXT].left + m_Button[BN_SystemTime_TEXT].Width() / 2 - size_EN_Small * 5, (m_Button[BN_SystemTime_TEXT].top + m_Button[BN_SystemTime_TEXT].bottom) / 2), &stringFormat, &brush_Word);
}

bool CRadarView::InitSocket()
{
	//初始化socket资源
	WSAData wsaData;
	if (0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			MessageBox(_T("Win Sock Version error !"));
			WSACleanup();
			return false;
		}
	}
	else
	{
		MessageBox(_T("Win Sock Init Failed !"));
		return false;
	}

	m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//sock_LOGIN = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SOCKET_ERROR == m_socket)
	{
		MessageBox(_T("Create Socket Failure !"));
		//int i = WSAGetLastError();
		return false;
	}

	//IPConfig ipCon(L"IPconfig.txt");
	//ISessionNum = ipCon.SessionNumber;

	//------------设置登录服务器的ip地址和端口等-------//
	//memset((void*)&addr_sendLOGIN, 0, sizeof(addr_sendLOGIN));
	//addr_sendLOGIN.sin_family = AF_INET;
	//char c_ip[100];
	//_CStringToCharArray(ipCon.IP_ControlServer, c_ip, 100);
	//addr_sendLOGIN.sin_addr.s_addr = inet_addr(c_ip);		//注意这里，传入指定字符串ip时用该函数
	//addr_sendLOGIN.sin_port = htons(ipCon.Port_ControlServer_receive);
	//addrLen_sendLOGIN = sizeof(addr_sendLOGIN);
	//--------------------------------------------------//

	//------------设置接收服务器反馈的ip地址和端口等-------//
	//memset((void*)&addr_recLOGIN, 0, sizeof(addr_recLOGIN));
	//addr_recLOGIN.sin_family = AF_INET;
	//addr_recLOGIN.sin_addr.s_addr = htonl(INADDR_ANY);		//注意这里，传入指定字符串ip时用该函数
	//addr_recLOGIN.sin_port = htons(ipCon.Port_Radar_receive);
	//addrLen_recLOGIN = sizeof(addr_recLOGIN);
	//int result1 = bind(sock_LOGIN, (sockaddr*)&addr_recLOGIN, addrLen_recLOGIN);
	//if (SOCKET_ERROR == result1)
	//{
	//	TRACE("bind ip address and port error !");
	//	return false;
	//}
	//int imode1 = 1;
	// 0:阻塞
	//设置非阻塞模式
	//ioctlsocket(sock_LOGIN, FIONBIO, (u_long FAR*)&imode1);
	//--------------------------------------------------//

	//------------设置发送到CoachStation的心跳包的ip地址和端口等-------//
	//IPConfig IPcon;
	//memset((void*)&addr_sendHeartBeat, 0, sizeof(addr_sendHeartBeat));
	//addr_sendHeartBeat.sin_family = AF_INET;
	//char * ip_consvr1 = _CStringToCharArray(IPcon.IP_CoachStation);
	//addr_sendHeartBeat.sin_addr.s_addr = inet_addr(ip_consvr1);		//注意这里，传入指定字符串ip时用该函数
	//addr_sendHeartBeat.sin_port = htons(IPcon.Port_CoachStation_receive_heartBeat);
	//addrLen_sendHeartBeat = sizeof(addr_sendHeartBeat);
	//if (ip_consvr1 != NULL)
	//{
	//	delete ip_consvr1;
	//	ip_consvr1 = NULL;
	//}
	//--------------------------------------------------//

	sockaddr_in host;
	memset((void*)&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	//ip address
	host.sin_addr.s_addr = htonl(INADDR_ANY);
	//port
	host.sin_port = htons(RECEIVE_DATA_PORT);
	int structLen = sizeof(host);
	int result = bind(m_socket, (sockaddr*)&host, structLen);
	if (SOCKET_ERROR == result)
	{
		TRACE("bind ip address and port error !");
		return false;
	}
	int imode = 1;
	// 0:阻塞
	//设置非阻塞模式
	ioctlsocket(m_socket, FIONBIO, (u_long FAR*)&imode);
	return true;
}

int CRadarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitSocket();

	/*double x = 12637000;
	double y = 2553000;
	double lat = 0;
	double lng = 0;
	WebMercatorProjInvcal(x, y, lng, lat);*/

	// TODO:  在此添加您专用的创建代码
	memset(m_ButtonSelect, false, BUTTONNUM*sizeof(bool));

	m_viewRect = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	double _4_3 = 4.0 / 3.0;
	m_mulOf800_600 = (double)m_viewRect.Height() / 600;
	double currentRadio = (double)m_viewRect.Width() / (double)m_viewRect.Height();
	int radarViewTop = m_viewRect.Height() / 24;	//雷达图像与顶部间隔
	int radarViewLeft = (m_viewRect.Width() * 11 / 160)*(currentRadio / _4_3)*(currentRadio / _4_3);	//雷达图像与左部间隔
	m_radiusP = m_viewRect.Height() * 9 / 20;
	m_radarCenter = Point(radarViewLeft + m_radiusP, radarViewTop + m_radiusP);
	m_radarRect = CRect(radarViewLeft, radarViewTop, radarViewLeft + m_radiusP * 2, radarViewTop + m_radiusP * 2);
	CRect radarRectLarger = CRect(m_radarRect.left - 30, m_radarRect.top - 30, m_radarRect.right + 30, m_radarRect.bottom + 30);
	CRect radarRectMid = CRect(m_radarRect.left - 10, m_radarRect.top - 10, m_radarRect.right + 10, m_radarRect.bottom + 10);
	m_radarRgn.CreateEllipticRgnIndirect(&m_radarRect);
	m_radarRgn_Larger.CreateEllipticRgnIndirect(&radarRectLarger);
	m_radarRgn_Mid.CreateEllipticRgnIndirect(&radarRectMid);

	//Button初始化
	ButtonCreate();
	m_buttonRgn.CreateRectRgn(0, 0, 0, 0);
	for (int i = 0; i < BUTTONNUM; i++)
	{
		CRgn rgn;
		rgn.CreateRectRgn(m_Button[i].left, m_Button[i].top, m_Button[i].right, m_Button[i].bottom);
		m_buttonRgn.CombineRgn(&m_buttonRgn, &rgn, RGN_OR);
	}

	blackBrush.CreateSolidBrush(RGB(0, 0, 0));
	color_blue = RGB(0, 0, 64);
	color_yellow = RGB(255, 255, 0);
	blueBrush.CreateSolidBrush(color_blue);
	//m_rangeRingsDistance = m_range / m_rangeRingsNum;
	//m_perPixelNM = m_range / m_radiusP;
	//m_perPixelKM = m_perPixelNM * 1.852;
	//m_perPixelM = m_perPixelNM * 1852;
	//m_radiusM = m_radiusP*m_perPixelM;

	//timer
	SetTimer(TIMER_SYSTEMTIME, 1000, NULL);
	SetTimer(TIMER_AIS_DECODE, 100, NULL);
	SetTimer(TIMER_AIS_CHECK, 1000, NULL);
	SetTimer(TIMER_AIS_CLEAR, 1000, NULL);
	SetTimer(TIMER_REFRESH_RADAR, 1000, NULL);
	//SetTimer(TIMER_SEND_VTS_RADAR_DATA, 5000, NULL);
	//SetTimer(TIMER_GENERATE_VTS_PIC, 3000, NULL);
	InitEvent();

	/*SHIP ship;
	ship.longitude = 114.3308;
	ship.latitude = 30.6096;
	ship.length = 100;
	ship.width = 50;
	ship.heading = 270;
	ship.aisReceiveCheck = true;
	ship.aisReceiveDoubleCheck = true;
	WebMercatorProjCal(ship.longitude, ship.latitude, ship.dadiX, ship.dadiY);
	m_OtherShip.push_back(ship);*/


	AfxBeginThread(ThreadReceiveVtsRadarSetting, this);
	AfxBeginThread(ThreadGenerateVtsRadarPic, this);
	AfxBeginThread(ThreadSendVtsRadarData, this);

	//SetThreadPriority(pThreadDEMtoBMP, BELOW_NORMAL_PRIORITY_CLASS);	//pThreadDEMtoBMP线程消耗CPU比较大，调低线程优先级以提高主线程流畅度
	return 0;
}

void CRadarView::DrawRadar(Graphics *graphics, MyMemDC *pMemDC)
{
	Pen pen_Line(Color(0, 0, 0, 0), 1);
	Pen pen_white166(Color(200, 166, 166, 166), 2);	//指北线颜色
	Pen pen_red200(Color(200, 200, 0, 0), 2);		//船首线颜色
	Pen pen_blue200(Color(200, 86, 156, 214));
	Pen pen_VRM(Color(150, 86, 156, 214));	//VRM颜色
	pen_VRM.SetDashStyle(DashStyleDashDot);
	Pen pen_EBL(Color(255, 189, 99, 188));	//EBL颜色
	pen_EBL.SetDashStyle(DashStyleDashDot);
	SolidBrush brush_word(Color(0, 0, 0, 0));
	Gdiplus::Font font_EN_Nomal(_T("Arial"), 10, FontStyleRegular, UnitPixel);
	StringFormat stringFormat;
	stringFormat.SetLineAlignment(StringAlignmentCenter);
	stringFormat.SetAlignment(StringAlignmentNear);

	Region oldRegion(Rect(m_viewRect.left, m_viewRect.top, m_viewRect.Width(), m_viewRect.Height()));


	if (m_dayOrNight)
	{
		pen_Line.SetColor(Color(200, 200, 200, 200));
		brush_word.SetColor(Color(255, 255, 255, 255));
	}
	else
	{
		pen_Line.SetColor(Color(255, 42, 140, 41));
		brush_word.SetColor(Color(255, 255, 255, 255));
	}



	Region newRegion2(m_radarRgn);
	graphics->SetClip(&newRegion2);



	//船首线和指北线
	if (m_displayMode)	//如果为艏向上
	{
		graphics->TranslateTransform(m_radarCenter.X, m_radarCenter.Y);
		//graphics->RotateTransform(360 - m_MainShip.heading);
		graphics->TranslateTransform(-m_radarCenter.X, -m_radarCenter.Y);
	}

	//绘制雷达图像
	DWORD t1 = GetTickCount();
	if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
		DrawDEM(graphics, pMemDC, m_vtsRadar[m_vtsRadar_selected].radarPicWithShip);
	}
	DWORD t2 = GetTickCount();
	DWORD t = t2 - t1;

	m_COGAngle = m_headingLineAngle;	//暂定

	//电子方位线1
	if (m_electronicBearingLine1Switch)
	{
		graphics->DrawLine(&pen_EBL, Point(m_radarCenter.X, m_radarCenter.Y), Point(m_radarCenter.X + (m_radiusP + 5) * sin(m_electronicBearingLine1Angle), m_radarCenter.Y - (m_radiusP + 5)*cos(m_electronicBearingLine1Angle)));
	}
	//电子方位线2
	if (m_electronicBearingLine2Switch)
	{
		graphics->DrawLine(&pen_EBL, Point(m_radarCenter.X, m_radarCenter.Y), Point(m_radarCenter.X + (m_radiusP + 5) * sin(m_electronicBearingLine2Angle), m_radarCenter.Y - (m_radiusP + 5)*cos(m_electronicBearingLine2Angle)));
	}

	//活动距标圈1
	if (m_variableRangeMarker1Switch)
	{
		if (m_VRM1Pixel <= m_radiusP)
		{
			graphics->DrawEllipse(&pen_VRM, Rect(m_radarCenter.X - m_VRM1Pixel, m_radarCenter.Y - m_VRM1Pixel,
				m_VRM1Pixel * 2, m_VRM1Pixel * 2));
		}

	}

	//活动距标圈2
	if (m_variableRangeMarker2Switch)
	{
		if (m_VRM2Pixel <= m_radiusP)
		{
			graphics->DrawEllipse(&pen_VRM, Rect(m_radarCenter.X - m_VRM2Pixel, m_radarCenter.Y - m_VRM2Pixel,
				m_VRM2Pixel * 2, m_VRM2Pixel * 2));
		}
	}

	//测距
	if (m_extendedRangeMeasurementSwitch)
	{
		HRGN rgn = m_radarRgn;
		if (PtInRegion(rgn, m_ERM_P1.x, m_ERM_P1.y) && PtInRegion(rgn, m_ERM_P2.x, m_ERM_P2.y))
			graphics->DrawLine(&Pen(Color(255, 255, 0)), Point(m_ERM_P1.x, m_ERM_P1.y), Point(m_ERM_P2.x, m_ERM_P2.y));
	}

	//矢量时间线
	/*if (m_vectorTimeSwitch)
	{
	DrawVectorTime(graphics, pMemDC, m_MainShip, m_vectorTimeNum);
	}*/

	//测试(本船碰撞范围)
	//graphics->DrawEllipse(&Pen(Color(255, 0, 0)), Rect(m_radarCenter.X - m_MainShip.collisionRadius / m_perPixelM, m_radarCenter.Y - m_MainShip.collisionRadius / m_perPixelM, m_MainShip.collisionRadius * 2 / m_perPixelM, m_MainShip.collisionRadius * 2 / m_perPixelM));

	//绘制ais船舶
	if (m_AISSwitch&&m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
		DrawAisShips(graphics, pMemDC, m_vtsRadar[m_vtsRadar_selected], g_aisShips);
	}
	//DrawOtherShip(graphics, pMemDC);
	/*if (m_ARPASwitch)
		DrawARPARecord(graphics, pMemDC);*/

	Region newRegion1(m_radarRgn_Larger);
	graphics->SetClip(&newRegion1);



	//刻度
	int add = 18;

	for (int i = 0; i < 360; i++)
	{
		double angle = _ToRad(i);
		if (i % 5 == 0)
		{
			graphics->DrawLine(&pen_Line,
				Point(m_radarCenter.X + m_radiusP * cos(angle), m_radarCenter.Y - m_radiusP * sin(angle)),
				Point(m_radarCenter.X + (m_radiusP + 8)*cos(angle), m_radarCenter.Y - (m_radiusP + 8)*sin(angle)));
			if (i % 10 == 0)
			{
				PointF point(m_radarCenter.X + (m_radiusP + add)*cos(angle) - 10, m_radarCenter.Y - (m_radiusP + add)*sin(angle) - 5);
				CString str;
				if (i <= 90)
					str.Format(_T("%d"), 90 - i);
				else
					str.Format(_T("%d"), 450 - i);
				graphics->DrawString(str, -1, &font_EN_Nomal, point, &brush_word);
			}

		}
		else
			graphics->DrawLine(&pen_Line,
			Point(m_radarCenter.X + m_radiusP * cos(angle), m_radarCenter.Y - m_radiusP * sin(angle)),
			Point(m_radarCenter.X + (m_radiusP + 4)*cos(angle), m_radarCenter.Y - (m_radiusP + 4)*sin(angle)));
	}

	//固定距标圈
	if (m_rangeRingsSwitch)
	{
		for (int i = 1; i <= m_rangeRingsNum; i++)
		{
			CRect rect = CRect(m_radarCenter.X - (m_radiusP*i / m_rangeRingsNum), m_radarCenter.Y - (m_radiusP*i / m_rangeRingsNum),
				m_radarCenter.X + (m_radiusP*i / m_rangeRingsNum), m_radarCenter.Y + (m_radiusP*i / m_rangeRingsNum));
			graphics->DrawEllipse(&pen_Line, rect.left, rect.top, rect.Width(), rect.Height());
		}
	}


	graphics->SetClip(&oldRegion);
}


void CRadarView::DrawAisShips(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, std::vector<SHIP> aisShips){
	for (std::vector<SHIP>::iterator it = aisShips.begin(); it != aisShips.end(); it++){
		if (vtsRadar.isShipInRadar(*it)){
			DrawOneAisShip(graphics, pMemDC, vtsRadar, *it);
		}
	}
	/*for (int i = 0; i < aisShips.size(); i++){
		if (vtsRadar.isShipInRadar(aisShips[i])){
		DrawOneAisShip(graphics, pMemDC, vtsRadar, aisShips[i]);
		}
		}*/
}

void CRadarView::DrawOneAisShip(Graphics *graphics, MyMemDC *pMemDC, const VtsRadar& vtsRadar, const SHIP& ship){
	Pen pen(Color(0, 0, 0, 0));
	if (ship.aisSelected)
		pen.SetColor(Color(150, 255, 201, 14));	//橙色
	else
		pen.SetColor(Color(150, 80, 208, 211));	//蓝色
	double angle1 = _ToRad(ship.heading);	//船首
	if (angle1 >= 2 * PI)
		angle1 -= 2 * PI;
	double angle2 = angle1 + _ToRad(150);
	if (angle2 >= 2 * PI)
		angle2 -= 2 * PI;
	double angle3 = angle1 + _ToRad(210);
	if (angle3 >= 2 * PI)
		angle3 -= 2 * PI;
	double l = ship.length / vtsRadar.getScale();
	if (l < 8)
		l = 8;
	double d = 2 * l;

	double scale = vtsRadar.getRange() / (m_radarRect.Width()/2);
	Point points[5];
	GetPixelPointFromDadi(points[0].X, points[0].Y, ship.dadiX, ship.dadiY, m_radarCenter.X, m_radarCenter.Y, vtsRadar.getCenterX(), vtsRadar.getCenterY(), scale);
	points[1].X = points[0].X + (int)(l*sin(angle1));
	points[1].Y = points[0].Y - (int)(l*cos(angle1));
	points[2].X = points[0].X + (int)(l*sin(angle2));
	points[2].Y = points[0].Y - (int)(l*cos(angle2));
	points[3].X = points[0].X + (int)(l*sin(angle3));
	points[3].Y = points[0].Y - (int)(l*cos(angle3));
	points[4].X = points[0].X + (int)(d*sin(angle1));
	points[4].Y = points[0].Y - (int)(d*cos(angle1));
	graphics->DrawLine(&pen, points[0], points[4]);
	graphics->DrawLine(&pen, points[1], points[2]);
	graphics->DrawLine(&pen, points[1], points[3]);
	graphics->DrawLine(&pen, points[2], points[3]);
}


BOOL CRadarView::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	return true;
}


void CRadarView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO:  在此处添加消息处理程序代码
	// 不为绘图消息调用 CView::OnPaint()
	OnDraw(&dc);
}


void CRadarView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	HRGN radarRgn = m_radarRgn;
	if (PtInRegion(radarRgn, point.x, point.y))	//指针在雷达区域内
	{
		m_radarSelect = true;
		double pixelScale = 0; // 米/单位像素
		if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
			pixelScale = m_vtsRadar[m_vtsRadar_selected].getRange() / (m_radarRect.Width() / 2);
		}

		//电子方位线1
		if (m_electronicBearingLine1Switch)
		{
			if (m_EBL1Roam)
			{
				if (point.x >= m_radarCenter.X && point.y <m_radarCenter.Y)	//第一象限
				{
					m_electronicBearingLine1Angle = atan((double)(point.x - m_radarCenter.X) / (double)(m_radarCenter.Y - point.y));
				}
				else if (point.x > m_radarCenter.X && point.y >= m_radarCenter.Y) //第二象限
				{
					m_electronicBearingLine1Angle = PI / 2 + atan((double)(point.y - m_radarCenter.Y) / (double)(point.x - m_radarCenter.X));
				}
				else if (point.x <= m_radarCenter.X&&point.y > m_radarCenter.Y)	//第三象限
				{
					m_electronicBearingLine1Angle = PI + atan((double)(m_radarCenter.X - point.x) / (double)(point.y - m_radarCenter.Y));
				}
				else if (point.x < m_radarCenter.X&&point.y <= m_radarCenter.Y)	//第四象限
				{
					m_electronicBearingLine1Angle = PI * 3 / 2 + atan((double)(m_radarCenter.Y - point.y) / (double)(m_radarCenter.X - point.x));
				}

				m_electronicBearingLine1Num = m_electronicBearingLine1Angle>m_headingLineAngle ?
					(m_electronicBearingLine1Angle - m_headingLineAngle) : (m_headingLineAngle - m_electronicBearingLine1Angle);
				if (m_electronicBearingLine1Num > PI)
					m_electronicBearingLine1Num = 2 * PI - m_electronicBearingLine1Num;
				m_electronicBearingLine1Num = _ToAng(m_electronicBearingLine1Num);

				CRgn rgn;
				rgn.CreateRectRgnIndirect(m_Button[BN_ElectronicBearingLine_1_SWITCH]);
				rgn.CombineRgn(&rgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
		}

		//电子方位线2
		if (m_electronicBearingLine2Switch)
		{
			if (m_EBL2Roam)
			{
				if (point.x >= m_radarCenter.X && point.y <m_radarCenter.Y)	//第一象限
				{
					m_electronicBearingLine2Angle = atan((double)(point.x - m_radarCenter.X) / (double)(m_radarCenter.Y - point.y));
				}
				else if (point.x > m_radarCenter.X && point.y >= m_radarCenter.Y) //第二象限
				{
					m_electronicBearingLine2Angle = PI / 2 + atan((double)(point.y - m_radarCenter.Y) / (double)(point.x - m_radarCenter.X));
				}
				else if (point.x <= m_radarCenter.X&&point.y > m_radarCenter.Y)	//第三象限
				{
					m_electronicBearingLine2Angle = PI + atan((double)(m_radarCenter.X - point.x) / (double)(point.y - m_radarCenter.Y));
				}
				else if (point.x < m_radarCenter.X&&point.y <= m_radarCenter.Y)	//第四象限
				{
					m_electronicBearingLine2Angle = PI * 3 / 2 + atan((double)(m_radarCenter.Y - point.y) / (double)(m_radarCenter.X - point.x));
				}

				m_electronicBearingLine2Num = m_electronicBearingLine2Angle>m_headingLineAngle ?
					(m_electronicBearingLine2Angle - m_headingLineAngle) : (m_headingLineAngle - m_electronicBearingLine2Angle);
				if (m_electronicBearingLine2Num > PI)
					m_electronicBearingLine2Num = 2 * PI - m_electronicBearingLine2Num;
				m_electronicBearingLine2Num = _ToAng(m_electronicBearingLine2Num);

				CRgn rgn;
				rgn.CreateRectRgnIndirect(m_Button[BN_ElectronicBearingLine_2_SWITCH]);
				rgn.CombineRgn(&rgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
		}

		//活动距标圈1
		if (m_variableRangeMarker1Switch)
		{
			if (m_VRM1Roam)
			{
				m_VRM1Pixel = (int)sqrtf((point.x - m_radarCenter.X)*(point.x - m_radarCenter.X) + (point.y - m_radarCenter.Y)*(point.y - m_radarCenter.Y));
				m_VRM1distance = m_VRM1Pixel*m_perPixelNM;
				CRgn rgn;
				rgn.CreateRectRgnIndirect(m_Button[BN_VariableRangeMarker_1_SWITCH]);
				rgn.CombineRgn(&rgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
		}
		//活动距标圈2
		if (m_variableRangeMarker2Switch)
		{
			if (m_VRM2Roam)
			{
				m_VRM2Pixel = (int)sqrtf((point.x - m_radarCenter.X)*(point.x - m_radarCenter.X) + (point.y - m_radarCenter.Y)*(point.y - m_radarCenter.Y));
				m_VRM2distance = m_VRM2Pixel*m_perPixelNM;
				CRgn rgn;
				rgn.CreateRectRgnIndirect(m_Button[BN_VariableRangeMarker_2_SWITCH]);
				rgn.CombineRgn(&rgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
		}
		//测距
		if (m_ERM_Roam)
		{
			m_ERM_P2 = point;
			m_targetRangeNum = sqrt((m_ERM_P1.x - m_ERM_P2.x)*(m_ERM_P1.x - m_ERM_P2.x) + (m_ERM_P1.y - m_ERM_P2.y)*(m_ERM_P1.y - m_ERM_P2.y))*pixelScale;
			CPoint p1, p2;
			p1.x = 0;
			p1.y = -m_ERM_P1.y;
			p2 = m_ERM_P2 - m_ERM_P1;
			m_targetBearingNum = acos((double)(p1.x*p2.x + p1.y*p2.y) / (m_ERM_P1.y*sqrt(p2.x*p2.x + p2.y*p2.y)));
			m_targetBearingNum = _ToAng(m_targetBearingNum);
			CRgn rgn;
			rgn.CreateRectRgn(0, 0, 0, 0);
			rgn.CombineRgn(&m_buttonRgn, &m_radarRgn, RGN_OR);
			InvalidateRgn(&rgn);
		}

		//目标定位信息
		m_targetRange = sqrt((point.x - m_radarCenter.X)*(point.x - m_radarCenter.X) + (point.y - m_radarCenter.Y)*(point.y - m_radarCenter.Y))*pixelScale;
		InvalidateRect(m_Button[BN_TargetRange_TEXT]);
		CPoint p1, p2;
		p1 = CPoint(0, -1);
		p2 = point - CPoint(m_radarCenter.X, m_radarCenter.Y);
		double angleR = _ToAng(acos(-p2.y / sqrt(p2.x*p2.x + p2.y*p2.y)));
		double angleT;
		if (point.x > m_radarCenter.X)
			angleT = angleR;
		else
			angleT = 360 - angleR;
		if (m_displayMode)
		{
			m_targetBearing_R = angleR;
			m_targetBearing_T = _ToAng(m_northAngle) - angleT;
			if (m_targetBearing_T < 0)
				m_targetBearing_T += 360;
			if (m_targetBearing_T >180)
				m_targetBearing_T = 360 - m_targetBearing_T;
		}
		else
		{
			m_targetBearing_T = angleR;
			m_targetBearing_R = _ToAng(m_headingLineAngle) - angleT;
			if (m_targetBearing_R < 0)
				m_targetBearing_R += 360;
			if (m_targetBearing_R >180)
				m_targetBearing_R = 360 - m_targetBearing_R;
		}
		InvalidateRect(m_Button[BN_TargetBearing_TEXT]);

		if (m_vtsRadar_selected >= 0 && m_vtsRadar_selected < m_vtsRadar.size()){
			GetGeograFromScrPoint(point.x, point.y, m_targetLongitude, m_targetLatitude, m_radarCenter.X, m_radarCenter.Y, m_vtsRadar[m_vtsRadar_selected].getLng(), m_vtsRadar[m_vtsRadar_selected].getLat(),pixelScale);
			InvalidateRect(m_Button[BN_TargetLongitude_TEXT]);
			InvalidateRect(m_Button[BN_TargetLatitude_TEXT]);
		}

	}
	else
	{
		m_radarSelect = false;
		for (int i = 0; i < BUTTONNUM; i++)
		{

			if (PtInRect(m_Button[i], point))
			{
				m_ButtonSelect[i] = true;
				InvalidateRect(&m_Button[i]);
			}
			else
			{
				m_ButtonSelect[i] = false;
				InvalidateRect(&m_Button[i]);
			}

		}

	}

	//ARPA控制框
	/*if (m_ARPAControlBoxDisplay)
	{
	CRgn arpaControlRgn_DIFF;
	CRgn viewRgn, rgn;
	arpaControlRgn_DIFF.CreateRectRgnIndirect(m_ARPAControlBoxRect);
	rgn.CreateRectRgnIndirect(m_Button[BN_ARPA]);
	viewRgn.CreateRectRgnIndirect(m_viewRect);
	arpaControlRgn_DIFF.CombineRgn(&arpaControlRgn_DIFF, &rgn, RGN_OR);
	arpaControlRgn_DIFF.CombineRgn(&viewRgn, &arpaControlRgn_DIFF, RGN_DIFF);
	HRGN arpaControlHRgn_DIFF = arpaControlRgn_DIFF;

	if (PtInRegion(arpaControlHRgn_DIFF, point.x, point.y))
	{
	m_ARPAControlBoxDisplay = false;
	InvalidateRgn(&m_ARPAControlBoxRgn);
	}
	}*/


	//AIS控制框
	if (m_AISControlBoxDisplay)
	{
		CRgn aisControlRgn_DIFF;
		CRgn viewRgn, rgn;
		aisControlRgn_DIFF.CreateRectRgnIndirect(m_AISControlBoxRect);
		rgn.CreateRectRgnIndirect(m_Button[BN_AIS]);
		viewRgn.CreateRectRgnIndirect(m_viewRect);
		aisControlRgn_DIFF.CombineRgn(&aisControlRgn_DIFF, &rgn, RGN_OR);
		aisControlRgn_DIFF.CombineRgn(&viewRgn, &aisControlRgn_DIFF, RGN_DIFF);
		HRGN aisControlHRgn_DIFF = aisControlRgn_DIFF;

		if (PtInRegion(aisControlHRgn_DIFF, point.x, point.y))
		{
			if (m_AISControlBoxDisplay)
				m_AISControlBoxDisplay = false;
			InvalidateRgn(&m_AISControlBoxRgn);
		}
	}


	CView::OnMouseMove(nFlags, point);
}

int CRadarView::GetButtonName(CPoint point)
{
	for (int i = 0; i < BUTTONNUM; i++)
	{
		/*if (i == BN_ARPA_SWITCH || i == BN_ARPA_RECORDTARGET_SWITCH || i == BN_ARPA_VECTORTIME_SWITCH || i == BN_ARPA_T_R_VECTOR_SWITCH)
		{
		if (!m_ARPAControlBoxDisplay)
		continue;
		}*/
		if (i == BN_AIS_SWITCH || i == BN_AIS_INFORMATION_SWITCH)
		{
			if (!m_AISControlBoxDisplay)
				continue;
		}
		if (PtInRect(m_Button[i], point))
			return i;
	}
	return -1;
}

void CRadarView::OnLButtonDown(UINT nFlags, CPoint point)
{
	/*CDC *pdc = GetDC();
	COLORREF color = pdc->GetPixel(point.x, point.y);
	ReleaseDC(pdc);*/


	// TODO:  在此添加消息处理程序代码和/或调用默认值
	int buttonName = GetButtonName(point);
	if (buttonName != -1)	//鼠标点击在BUTTON内
	{
		switch (buttonName)
		{
			//case BN_Range_up:
			//{
			//	if (m_range == RANGE1)	m_range = RANGE2;
			//	else if (m_range == RANGE2)	m_range = RANGE3;
			//	else if (m_range == RANGE3)	m_range = RANGE4;
			//	else if (m_range == RANGE4)	m_range = RANGE5;
			//	else if (m_range == RANGE5)
			//	{
			//		m_range = RANGE6;
			//		m_rangeRingsNum = 6;
			//	}
			//	else if (m_range == RANGE6)	m_range = RANGE7;
			//	else if (m_range == RANGE7)	m_range = RANGE8;
			//	else if (m_range == RANGE8)	m_range = RANGE9;
			//	else if (m_range == RANGE9)	m_range = RANGE10;
			//	else if (m_range == RANGE10)m_range = RANGE10;
			//	double oldPerPixelM = m_perPixelM;
			//	m_rangeRingsDistance = m_range / m_rangeRingsNum;
			//	m_perPixelNM = m_range / m_radiusP;
			//	m_perPixelKM = m_perPixelNM*1.852;
			//	m_perPixelM = m_perPixelNM * 1852;
			//	m_radiusM = m_radiusP*m_perPixelM;
			//	m_VRM1Pixel = m_VRM1distance / m_perPixelNM;
			//	m_VRM2Pixel = m_VRM2distance / m_perPixelNM;
			//	UpdateArpaRecordPos(oldPerPixelM);//更新ARPA_RECORD位置
			//	CRgn rgn1;
			//	rgn1.CreateRectRgn(0, 0, 0, 0);
			//	rgn1.CombineRgn(&m_radarRgn, &m_buttonRgn, RGN_OR);
			//	InvalidateRgn(&rgn1);

			//	//设置量程改变信号量
			//	SetEvent(rangeUpdate);
			//	break;
			//}
			//case BN_Range_down:
			//{
			//	if (m_range == RANGE1)	m_range = RANGE1;
			//	else if (m_range == RANGE2)	m_range = RANGE1;
			//	else if (m_range == RANGE3)	m_range = RANGE2;
			//	else if (m_range == RANGE4)	m_range = RANGE3;
			//	else if (m_range == RANGE5)	m_range = RANGE4;
			//	else if (m_range == RANGE6){
			//		m_range = RANGE5;
			//		m_rangeRingsNum = 5;
			//	}
			//	else if (m_range == RANGE7)	m_range = RANGE6;
			//	else if (m_range == RANGE8)	m_range = RANGE7;
			//	else if (m_range == RANGE9)	m_range = RANGE8;
			//	else if (m_range == RANGE10)m_range = RANGE9;
			//	double oldPerPixelM = m_perPixelM;
			//	m_rangeRingsDistance = m_range / m_rangeRingsNum;
			//	m_perPixelNM = m_range / m_radiusP;
			//	m_perPixelKM = m_perPixelNM*1.852;
			//	m_perPixelM = m_perPixelNM * 1852;
			//	m_radiusM = m_radiusP*m_perPixelM;
			//	m_VRM1Pixel = m_VRM1distance / m_perPixelNM;
			//	m_VRM2Pixel = m_VRM2distance / m_perPixelNM;
			//	UpdateArpaRecordPos(oldPerPixelM);//更新ARPA_RECORD位置
			//	CRgn rgn1;
			//	rgn1.CreateRectRgn(0, 0, 0, 0);
			//	rgn1.CombineRgn(&m_radarRgn, &m_buttonRgn, RGN_OR);
			//	InvalidateRgn(&rgn1);

			//	//设置量程改变信号量
			//	SetEvent(rangeUpdate);
			//	break;
			//}
		case BN_RangeRings_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgn(m_Button[buttonName].left, m_Button[buttonName].top, m_Button[buttonName].right, m_Button[buttonName].bottom);
			rgn1.CombineRgn(&rgn1, &m_radarRgn_Mid, RGN_OR);
			m_rangeRingsSwitch = !m_rangeRingsSwitch;
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_VariableRangeMarker_1_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgnIndirect(m_Button[buttonName]);
			rgn1.CombineRgn(&rgn1, &m_radarRgn, RGN_OR);
			if (m_variableRangeMarker1Switch)
			{
				m_variableRangeMarker1Switch = false;
				m_VRM1Roam = false;
			}
			else
			{
				m_variableRangeMarker1Switch = true;
				m_VRM1Roam = true;
			}
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_VariableRangeMarker_2_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgnIndirect(m_Button[buttonName]);
			rgn1.CombineRgn(&rgn1, &m_radarRgn, RGN_OR);
			if (m_variableRangeMarker2Switch)
			{
				m_variableRangeMarker2Switch = false;
				m_VRM2Roam = false;
			}
			else
			{
				m_variableRangeMarker2Switch = true;
				m_VRM2Roam = true;
			}
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_ElectronicBearingLine_1_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgnIndirect(m_Button[buttonName]);
			rgn1.CombineRgn(&rgn1, &m_radarRgn, RGN_OR);
			if (m_electronicBearingLine1Switch)
			{
				m_electronicBearingLine1Switch = false;
				m_EBL1Roam = false;
			}
			else
			{
				m_electronicBearingLine1Switch = true;
				m_EBL1Roam = true;
			}
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_ElectronicBearingLine_2_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgnIndirect(m_Button[buttonName]);
			rgn1.CombineRgn(&rgn1, &m_radarRgn, RGN_OR);
			if (m_electronicBearingLine2Switch)
			{
				m_electronicBearingLine2Switch = false;
				m_EBL2Roam = false;
			}
			else
			{
				m_electronicBearingLine2Switch = true;
				m_EBL2Roam = true;
			}
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_ExtendedRangeMeasurement_SWITCH:
		{
			if (m_extendedRangeMeasurementSwitch)
			{
				m_extendedRangeMeasurementSwitch = false;
				m_ERMFunctionSwitch = false;
				m_targetBearingNum = 0;
				m_targetRangeNum = 0;
				m_ERM_P1 = CPoint(0, 0);
				m_ERM_P2 = CPoint(0, 0);
			}
			else
			{
				m_extendedRangeMeasurementSwitch = true;
				m_ERMFunctionSwitch = true;
			}
			CRgn rgn1;
			rgn1.CreateRectRgn(0, 0, 0, 0);
			rgn1.CombineRgn(&m_radarRgn, &m_buttonRgn, RGN_OR);
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_DayNight_SWITCH:
		{
			CRgn rgn1;
			rgn1.CreateRectRgn(0, 0, 0, 0);
			rgn1.CombineRgn(&m_radarRgn_Mid, &m_buttonRgn, RGN_OR);
			m_dayOrNight = !m_dayOrNight;
			InvalidateRgn(&rgn1);
			break;
		}
		case BN_Language_SWITCH:
		{
			m_language = !m_language;
			InvalidateRgn(&m_buttonRgn);
			break;
		}
		case BN_EchoAveraging_SWITCH:
		{
			m_echoAveragingSwitch = !m_echoAveragingSwitch;
			InvalidateRect(&m_Button[buttonName]);
			break;
		}
		case BN_AIS:
		{
			if (m_vtsRadar_selected < 0 || m_vtsRadar_selected >= m_vtsRadar.size()){
				break;
			}
			m_AISControlBoxDisplay = !m_AISControlBoxDisplay;
			InvalidateRgn(&m_AISControlBoxRgn);
			break;
		}
		case BN_AIS_SWITCH:
		{
			//if (m_ARPASwitch)
			//{
			//	//信息窗提示//
			//	SetMessageArea1Para(MA_1_ARPA_OFF);
			//	break;
			//}
			//else
			//{
			if (m_AISSwitch)
			{
				//信息窗提示//
				if (GetMessageArea1Para() == MA_1_AIS_OFF || GetMessageArea1Para() == MA_1_AIS_INFO)
					SetMessageArea1Para(MA_1_BLANK);

				m_AISSwitch = false;
				m_AISInformationSwitch = false;
			}

			else
			{
				//信息窗提示//
				if (GetMessageArea1Para() == MA_1_AIS_ON)
					SetMessageArea1Para(MA_1_BLANK);

				m_AISSwitch = true;
				m_AISInformationSwitch = true;
			}
			CRgn rgn;
			rgn.CreateRectRgn(0, 0, 0, 0);
			rgn.CombineRgn(&m_buttonRgn, &m_radarRgn, RGN_OR);
			InvalidateRgn(&rgn);
			break;
			//	}
		}
		case BN_AIS_INFORMATION_SWITCH:
		{
			if (!m_AISSwitch)
			{
				//信息窗提示//
				SetMessageArea1Para(MA_1_AIS_ON);
				break;
			}
			else
			{
				if (m_AISInformationSwitch)
				{
					m_AISInformationSwitch = false;

					//信息窗提示//
					SetMessageArea1Para(MA_1_BLANK);
				}
				else
				{
					m_AISInformationSwitch = true;

					//信息窗提示//
					SetMessageArea1Para(MA_1_AIS_INFO);
				}
				CRgn rgn;
				rgn.CreateRectRgn(0, 0, 0, 0);
				rgn.CombineRgn(&m_buttonRgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
				break;
			}
		}
		//case BN_StartOrStop_SWITCH:
		//{
		//	m_startOrStop = !m_startOrStop;
		//	//退出
		//	if (!m_startOrStop)
		//		//			EXIT = true;
		//		InvalidateRect(&m_Button[buttonName]);
		//	break;
		//}
		case BN_VTS_Radar_TEXT_1:
		{
			if (m_vtsRadar.size() > 0 + m_vtsRadar_first_index){
				mtx_vtsRadar_selected.lock();
				m_vtsRadar_selected = m_vtsRadar_first_index;
				mtx_vtsRadar_selected.unlock();
			}
			break;
		}
		case BN_VTS_Radar_TEXT_2:
		{
			if (m_vtsRadar.size() > 1 + m_vtsRadar_first_index){
				mtx_vtsRadar_selected.lock();
				m_vtsRadar_selected = 1 + m_vtsRadar_first_index;
				mtx_vtsRadar_selected.unlock();
			}
			break;
		}
		case BN_VTS_Radar_TEXT_3:
		{
			if (m_vtsRadar.size() > 2 + m_vtsRadar_first_index){
				mtx_vtsRadar_selected.lock();
				m_vtsRadar_selected = 2 + m_vtsRadar_first_index;
				mtx_vtsRadar_selected.unlock();
			}
			break;
		}
		case BN_VTS_Radar_TEXT_4:
		{
			if (m_vtsRadar.size() > 3 + m_vtsRadar_first_index){
				mtx_vtsRadar_selected.lock();
				m_vtsRadar_selected = 3 + m_vtsRadar_first_index;
				mtx_vtsRadar_selected.unlock();
			}
			break;
		}
		case BN_VTS_Radar_TEXT_5:
		{
			if (m_vtsRadar.size() > 4 + m_vtsRadar_first_index){
				mtx_vtsRadar_selected.lock();
				m_vtsRadar_selected = 4 + m_vtsRadar_first_index;
				mtx_vtsRadar_selected.unlock();
			}
			break;
		}
		case BN_VTS_Radar_UP:
		{
			if (m_vtsRadar_first_index > 0){
				m_vtsRadar_first_index--;

				CRgn rgn;
				rgn.CreateRectRgn(0, 0, 0, 0);
				rgn.CombineRgn(&m_buttonRgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
			break;
		}
		case BN_VTS_Radar_DOWN:
		{
			if (m_vtsRadar_first_index + 5 < m_vtsRadar.size()){
				m_vtsRadar_first_index++;

				CRgn rgn;
				rgn.CreateRectRgn(0, 0, 0, 0);
				rgn.CombineRgn(&m_buttonRgn, &m_radarRgn, RGN_OR);
				InvalidateRgn(&rgn);
			}
			break;
		}
		default:
			break;
		}

	}

	//ARPA控制框
	/*if (m_ARPAControlBoxDisplay)
	{
	CRgn arpaControlRgn_DIFF;
	CRgn viewRgn, rgn1;
	arpaControlRgn_DIFF.CreateRectRgnIndirect(m_ARPAControlBoxRect);
	rgn1.CreateRectRgnIndirect(m_Button[BN_ARPA]);
	viewRgn.CreateRectRgnIndirect(m_viewRect);
	arpaControlRgn_DIFF.CombineRgn(&arpaControlRgn_DIFF, &rgn1, RGN_OR);
	arpaControlRgn_DIFF.CombineRgn(&viewRgn, &arpaControlRgn_DIFF, RGN_DIFF);
	HRGN arpaControlHRgn_DIFF = arpaControlRgn_DIFF;
	if (PtInRegion(arpaControlHRgn_DIFF, point.x, point.y))
	{

	m_ARPAControlBoxDisplay = false;
	InvalidateRgn(&m_ARPAControlBoxRgn);
	}
	}*/

	//AIS控制框
	if (m_AISControlBoxDisplay)
	{
		CRgn aisControlRgn_DIFF;
		CRgn viewRgn, rgn2;
		aisControlRgn_DIFF.CreateRectRgnIndirect(m_AISControlBoxRect);
		rgn2.CreateRectRgnIndirect(m_Button[BN_AIS]);
		viewRgn.CreateRectRgnIndirect(m_viewRect);
		aisControlRgn_DIFF.CombineRgn(&aisControlRgn_DIFF, &rgn2, RGN_OR);
		aisControlRgn_DIFF.CombineRgn(&viewRgn, &aisControlRgn_DIFF, RGN_DIFF);
		HRGN aisControlHRgn_DIFF = aisControlRgn_DIFF;
		if (PtInRegion(aisControlHRgn_DIFF, point.x, point.y))
		{
			m_AISControlBoxDisplay = false;
			InvalidateRgn(&m_AISControlBoxRgn);
		}
	}

	HRGN radarRgn = m_radarRgn;
	if (PtInRegion(radarRgn, point.x, point.y))	//如果点击在雷达区域内
	{
		//电子方位线1
		if (m_EBL1Roam)
			m_EBL1Roam = false;
		//电子方位线2
		if (m_EBL2Roam)
			m_EBL2Roam = false;
		//活动距标圈1
		if (m_VRM1Roam)
			m_VRM1Roam = false;
		//活动距标圈2
		if (m_VRM2Roam)
			m_VRM2Roam = false;
		//测距
		if (m_extendedRangeMeasurementSwitch)
		{
			if (m_ERMFunctionSwitch)
			{
				bool flag = true;
				if (!m_ERM_Roam)
				{
					m_ERM_P1 = point;
					m_ERM_Roam = true;
					flag = false;
				}
				if (m_ERM_Roam&&flag)
				{
					m_ERM_Roam = false;
					m_ERMFunctionSwitch = false;
				}
			}
		}

		//AIS船舶击中测试
		if (m_AISSwitch)
		{
			/*int index = GetAISShipIndex(point);
			if (index != -1)
			{
			for (int i = 0; i < (int)m_OtherShip.size(); i++)
			{
			m_OtherShip[i].aisSelected = false;
			}
			m_OtherShip[index].aisSelected = true;
			m_aisShipSelected = &m_OtherShip[index];
			InvalidateRgn(&m_radarRgn);
			if (m_AISInformationSwitch)
			{
			SetMessageArea1Para(MA_1_AIS_INFO);
			}
			}*/

		}

		//ARPA录取目标
		//if (m_ARPASwitch)
		//{
		//	if (m_ARPARecordTargetSwitch)	//录取目标
		//	{
		//		vector<int> index;
		//		GetOtherShipIndexFromRect(index, m_arpaRecordRect);
		//		if ((int)index.size() == 0)	//目标不是船舶
		//		{
		//			//基于向vector中插入属性会更改其他属性的指针，需要在此跟新m_ArpaRecordSelected
		//			int j = 0;
		//			if (m_ArpaRecordSelected != NULL)
		//			{
		//				j = m_ArpaRecordSelected->index;
		//			}
		//			//添加arpa记录
		//			ARPA_RECORD record;
		//			record.AISship = NULL;
		//			record.longaitude = m_targetLongitude;
		//			record.latitude = m_targetLatitude;
		//			record.length_Pixel = m_arpaRecordRect.Width();
		//			//应该添加一个检测目标大小的函数，获取目标width
		//			record.length_Meter = record.length_Pixel*m_perPixelM;
		//			record.index = GetMinUnusedIndexOfARPARecord();
		//			record.TCourse = -1;
		//			record.RCourse = -1;
		//			record.collisionRadius = record.length_Meter*0.707;
		//			record.gateRect = m_arpaRecordRect;
		//			record.gateSize = m_ARPARecordCur;
		//			if (record.index == (int)m_arpaRecord.size() + 1)
		//				m_arpaRecord.push_back(record);
		//			else
		//				m_arpaRecord.insert(m_arpaRecord.begin() + record.index - 1, record);
		//			if (j != 0)
		//				m_ArpaRecordSelected = &m_arpaRecord[j - 1];
		//		}
		//		else				//目标是船舶
		//		{
		//			for (int i = 0; i < index.size(); i++)
		//			{
		//				if (m_OtherShip[index[i]].isBeArpaRecorded == false)
		//				{
		//					//基于向vector中插入属性会更改其他属性的指针，需要在此跟新m_ArpaRecordSelected
		//					int j = 0;
		//					if (m_ArpaRecordSelected != NULL)
		//					{
		//						j = m_ArpaRecordSelected->index;
		//					}
		//					//添加arpa记录
		//					ARPA_RECORD record;
		//					record.AISship = &m_OtherShip[index[i]];
		//					record.AISship->isBeArpaRecorded = true;
		//					record.index = GetMinUnusedIndexOfARPARecord();
		//					record.collisionRadius = record.AISship->collisionRadius;
		//					record.gateRect = m_arpaRecordRect;
		//					record.gateSize = m_ARPARecordCur;
		//					if (record.index == (int)m_arpaRecord.size() + 1)
		//						m_arpaRecord.push_back(record);
		//					else
		//						m_arpaRecord.insert(m_arpaRecord.begin() + record.index - 1, record);
		//					if (j != 0)
		//						m_ArpaRecordSelected = &m_arpaRecord[j - 1];
		//				}
		//			}
		//		}
		//	}
		//	else	//选中显示记录
		//	{
		//		int index = GetARPARecordIndex(point);
		//		if (index != -1)
		//		{
		//			if (m_arpaRecord[index].state != ARPA_RECORD_STATE_MISSING)
		//			{
		//				m_ArpaRecordSelected = &m_arpaRecord[index];
		//				InvalidateRgn(&m_radarRgn);

		//				SetMessageArea1Para(MA_1_ARPA_INFO);
		//			}
		//		}
		//	}
		//}
	}

	CView::OnLButtonDown(nFlags, point);
}


void CRadarView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == TIMER_SYSTEMTIME)
	{
		m_systemTime = CTime::GetCurrentTime().Format("%Y-%m-%d  %X");
		InvalidateRect(m_Button[BN_SystemTime_TEXT]);
	}

	//AIS解析事件
	if (TIMER_AIS_DECODE == nIDEvent)
	{

		//对缓冲区m_strReceived进行分割并解析，并从中删除已分割好的部分，仅留下最后的非完整片段
		/***********************************************************************************************
		注意对多语句AIS信息的解析：由于单个AIS语句（以换行符'\n'为标识）的最长字符数为82，当单个AIS信息
		包含信息多的时候，82个字符就无法囊括所有信息，所以有些AIS单个信息包含多个语句，其形式如下所示：
		!AIVDM,2,1,3,B,55P5TL01VIaAL@7WKO@mBplU@<PDhh000000001S;AJ::4A80?4i@E53,0*3E
		!AIVDM,2,2,3,B,1@0000000000000,2*55
		观察下上述语句，发现!AIVDM后面跟着的是2,1和2,2.这表明：该AIS信息包含两个语句，其中第一句中的1表示
		第一句，第二句中的第二个2表示第二句。
		根据我的观察，多条语句都是2句，且连续出现，即不会出现：出现第一句跟着是另外一条AIS信息，再跟着又是
		第二句。
		多条语句的解析看似简单：只要把第二句中的第五个字段（第五个分号和第六个分号之间数据）并到第一句中即可。
		但是试想下，这么做的前提是在分解单独AIS语句时就发现该AIS语句不是单独的一条信息，也就是说需要解析才知
		道。这样的话，只能把这项判断其是否为单独AIS信息的工作放入到解析过程中。
		具体的做法如下所述：
		1）按照'\n'标识分解出单句AIS语句；
		2）解析该句，把该句的相关信息放入到context_t结构体中
		struct aivdm_context_t {
		int part, await;
		unsigned char *field[NMEA_MAX];
		unsigned char fieldcopy[NMEA_MAX+1];
		unsigned char bits[2048];
		char shipname[AIS_SHIPNAME_MAXLEN+1];
		size_t bitlen;
		};
		解析的时候发现part（第几份）await（总份数）不等，则知道这是一条完整AIS信息的其中一句，则保留该
		结构体信息，主要是part,await,bits[2048],bitlen，解析出这几个信息后不再继续解析下去了，返回继续
		分解AIS语句。
		3）在接下来的AIS语句中发现了part == await了，说明AIS信息完整了，继续在前面保留下来的context_t结构体
		信息中添加一些信息，主要是bits[2048]中内容增加了，自然bitlen也变大了，对之进行下一步解析

		具体看下面的代码和aisdecode.cpp中的aivdmo_decode函数
		***********************************************************************************************/
		std::string aisMsg = receiveAIS();
		while (aisMsg.size() > 0)
		{
			int nIndex = aisMsg.find('\n');
			if (-1 != nIndex)
			{
				ais_t ais = { 0 };
				static struct aivdm_context_t aivdm = { 0 };

				//分割
				std::string strPacket = aisMsg.substr(0, nIndex);
				//USES_CONVERSION;
				//LPSTR str = T2A(strPacket);
				//删除
				aisMsg = aisMsg.substr(nIndex + 1);
				//m_strReceived.Delete(0, nIndex + 1);//删除时把整个AIS语句包括'\n'也删除
				//开始解析咯
				if (false == aivdmo_decode(strPacket.data(), strPacket.size(), &aivdm, &ais))
				{
					//如果失败的原因是因为该AIS信息有几份，那么这个aivdm_context_t类型的aivdm就不清空，保留本次解析
					//数据，否则则应该清空
					if (aivdm.await == aivdm.part)//非因为AIS信息被划分为几份而失败时则需清空aivdm中数据
						memset(&aivdm, 0, sizeof(aivdm));//aivdm = {0};
					continue;
				}
				//解析成功则立即清空aivdm数据
				memset(&aivdm, 0, sizeof(aivdm));
				mtx_aisShips.lock();
				g_aisShips = updateAisShipsByAisMsg(&ais, g_aisShips);
				mtx_aisShips.unlock();
			}
			else
				break;
		}
	}
	if (nIDEvent == TIMER_AIS_CHECK)
	{
		if (mtx_aisShips.try_lock()){
			g_aisShips = checkAisShipsLife(g_aisShips);
			mtx_aisShips.unlock();
		}
	}
	if (nIDEvent == TIMER_AIS_CLEAR)
	{
		if (mtx_aisShips.try_lock()){
			g_aisShips = clearDeadAisShips(g_aisShips);
			mtx_aisShips.unlock();
		}
	}

	if (nIDEvent == TIMER_REFRESH_RADAR)
	{
		Invalidate();
		//InvalidateRgn(&m_radarRgn);
	}

	if (nIDEvent == TIMER_SEND_VTS_RADAR_DATA){
		/*try{
			std::thread t(ThreadSendVtsRadarData, m_vtsRadar);
			t.detach();
			}
			catch (std::system_error &e){
			TRACE(e.what());
			}*/
	}

	CView::OnTimer(nIDEvent);
}

std::vector<SHIP> CRadarView::updateAisShipsByAisMsg(const ais_t * pAisInfo, std::vector<SHIP> aisShips)	//获取一条AIS信息
{
	CString status;
	double dGeoPoX = 0.0;
	double dGeoPoY = 0.0;
	CString strHDG, strCOG, strSOG;
	float fHeading = 0.0;//船首向 有用
	float fSOG = 0.0;//对地速度 无用
	float fSOW = 0.0;//对水航速 无用
	float fCOG = 0.0;//对地航向 无用
	float fCOW = 0.0;//对水航向 无用
	long lShipID;//添加船舶到内存中时提供的船舶ID，由YIMA SDK提供
	float fCourse = 0.0;	//有用
	float fSpeed = 0.0;	//有用

	switch (pAisInfo->type)
	{
	case 1:
	case 2:
	case 3:
	{
		//shipstatus
		switch (pAisInfo->type1.status)
		{
		case 0:
			status = "Under way using engine"; break;
		case 1:
			status = "At anchor"; break;
		case 2:
			status = "Not under command"; break;
		case 3:
			status = "Restricted manoeuverability"; break;
		case 4:
			status = "Constrained by her draught"; break;
		case 5:
			status = "Moored"; break;
		case 6:
			status = "Aground"; break;
		case 7:
			status = "Engaged in Fishing"; break;
		case 8:
			status = "Under way sailing"; break;
		case 9:
			status = "Reserved for future amendment of Navigational Status for HSC"; break;
		case 10:
			status = "Reserved for future amendment of Navigational Status for WIG"; break;
		case 11:
		case 12:
		case 13:
		case 14:
			status = "Reserved for future use"; break;
		default:
			status = "Not defined(default)";
		}

		//rot
		CString strRot;
		switch (pAisInfo->type1.turn)
		{
		case 0:
			strRot = CString("not turning"); break;
		case 127:
			strRot = CString("向右>5deg/30s"); break;
		case -127:
			strRot = CString("向左>5deg/30s"); break;
		case 128:
			strRot = CString("no turning info available"); break;
		default:
			/*float rot = 4.733*sqrt(float(abs(pAisInfo->type1.turn)));//这个公式暂时有点疑问？？？？？？？
			if(pAisInfo->type1.turn>0)
			strRot.Format("向右,%.3fdeg/min",rot);
			else
			strRot.Format("向左,%.3fdeg/min",rot);*/
			strRot.Format(_T("%d"), pAisInfo->type1.turn);
			break;
		}

		//sog
		switch (pAisInfo->type1.speed)
		{
		case 1023:
			strSOG = CString("no available SOG info"); break;
		case 1024:
			strSOG = CString(">=102.2 knots"); break;
		default:
			fSpeed = (pAisInfo->type1.speed)*0.1;
			//	strSOG.Format("%.1f knots",sog);
			break;
		}

		//accuracy
		CString poAccu;
		if (true == pAisInfo->type1.accuracy)
			poAccu = "<10m";
		else
			poAccu = ">10m";

		//longtitude & longNum:文本和纯数字
		CString strLong;
		int numLong = pAisInfo->type1.lon;
		dGeoPoX = numLong / 600000.0;
		int degLong = abs(numLong) / 600000;
		float minLong = (abs(numLong) % 600000) / 10000.0;
		switch (numLong)
		{
		case 181 * 600000:
			strLong = CString("no available longtitude info"); break;
		default:
			if (pAisInfo->type1.lon > 0)
				strLong.Format(_T("%d度%.3f分E"), abs(degLong), abs(minLong));
			else
				strLong.Format(_T("%d度%.3f分W"), abs(degLong), abs(minLong));
			break;
		}
		//Latitude & latNum
		CString strLat;
		int numLat = pAisInfo->type1.lat;
		dGeoPoY = numLat / 600000.0;
		int degLat = abs(numLat) / 600000;
		float minLat = (abs(numLat) % 600000) / 10000.0;
		switch (numLat)
		{
		case 91 * 600000:
			strLat = CString("no available latitude info"); break;
		default:
			if (numLat > 0)
				strLat.Format(_T("%d度%.3f分N"), abs(degLat), abs(minLat));
			else
				strLat.Format(_T("%d度%.3f分S"), abs(degLat), abs(minLat));
			break;
		}
		//COG
		if (3600 == pAisInfo->type1.course)
			strCOG = CString("no available COG info");
		else
			fCourse = (float)(pAisInfo->type1.course)*0.1;
		//strCOG.Format("%.1f度",(float)(pAisInfo->type1.course)*0.1);
		//HDG
		if (511 == pAisInfo->type1.heading)
			strHDG = CString("no available HDG info");
		else
			strHDG.Format(_T("%d度"), pAisInfo->type1.heading);
		break;
	}

	/************船舶静态信息***************/
	case 5:
	{
		//callsign
		//shipname
		//船舶类型
		CString strShipType;
		switch (pAisInfo->type5.shiptype)
		{
		case 0:
			strShipType = CString("Not available"); break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			strShipType = CString("Reserved for future use"); break;
		case 20:
			strShipType = CString("WIG,all ships of this type"); break;
		case 21:
			strShipType = CString("WIG,Hazardous category A"); break;
		case 22:
			strShipType = CString("WIG,Hazardous category B"); break;
		case 23:
			strShipType = CString("WIG,Hazardous category C"); break;
		case 24:
			strShipType = CString("WIG,Hazardous category D"); break;
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
			strShipType = CString("WIG,Reserved for future use"); break;
		case 30:
			strShipType = CString("Fishing"); break;
		case 31:
			strShipType = CString("Towing"); break;
		case 32:
			strShipType = CString("Towing:船长>200m或船宽>25m"); break;
		case 33:
			strShipType = CString("Dredging or underwater ops"); break;
		case 34:
			strShipType = CString("Diving ops"); break;
		case 35:
			strShipType = CString("Military ops"); break;
		case 36:
			strShipType = CString("Sailing"); break;
		case 37:
			strShipType = CString("Pleasure Craft"); break;
		case 38:
		case 39:
			strShipType = CString("Reserved"); break;
		case 40:
			strShipType = CString("HSC,all ships of this type"); break;
		case 41:
			strShipType = CString("HSC,Hazardous category A"); break;
		case 42:
			strShipType = CString("HSC,Hazardous category B"); break;
		case 43:
			strShipType = CString("HSC,Hazardous category C"); break;
		case 44:
			strShipType = CString("HSC,Hazardous category D"); break;
		case 45:
		case 46:
		case 47:
		case 48:
			strShipType = CString("HSC,Reserved for future use"); break;
		case 49:
			strShipType = CString("HSC,No additional info"); break;
		case 50:
			strShipType = CString("Pilot vessel"); break;
		case 51:
			strShipType = CString("Search and Rescue vessel"); break;
		case 52:
			strShipType = CString("Tug"); break;
		case 53:
			strShipType = CString("Port Tender"); break;
		case 54:
			strShipType = CString("Anti-pollution equipment"); break;
		case 55:
			strShipType = CString("Law Enforcement"); break;
		case 56:
		case 57:
			strShipType = CString("Spare-Local Vessel"); break;
		case 58:
			strShipType = CString("Medical Transport"); break;
		case 59:
			strShipType = CString("Ship according to RR Resolution No.18"); break;
		case 60:
			strShipType = CString("Passenger,all ships of this type"); break;
		case 61:
			strShipType = CString("Passenger,Hazardous category A"); break;
		case 62:
			strShipType = CString("Passenger,Hazardous category B"); break;
		case 63:
			strShipType = CString("Passenger,Hazardous category C"); break;
		case 64:
			strShipType = CString("Passenger,Hazardous category D"); break;
		case 65:
		case 66:
		case 67:
		case 68:
			strShipType = CString("Passenger,Reserved for future use"); break;
		case 69:
			strShipType = CString("Passenger,No additional info"); break;
		case 70:
			strShipType = CString("Cargo,all ships of this type"); break;
		case 71:
			strShipType = CString("Cargo,Hazardous category A"); break;
		case 72:
			strShipType = CString("Cargo,Hazardous category B"); break;
		case 73:
			strShipType = CString("Cargo,Hazardous category C"); break;
		case 74:
			strShipType = CString("Cargo,Hazardous category D"); break;
		case 75:
		case 76:
		case 77:
		case 78:
			strShipType = CString("Cargo,Reserved for future use"); break;
		case 79:
			strShipType = CString("Cargo,No additonal info"); break;
		case 80:
			strShipType = CString("Tanker,all ships of this type"); break;
		case 81:
			strShipType = CString("Tanker,Hazardous category A"); break;
		case 82:
			strShipType = CString("Tanker,Hazardous category B"); break;
		case 83:
			strShipType = CString("Tanker,Hazardous category C"); break;
		case 84:
			strShipType = CString("Tanker,Hazardous category D"); break;
		case 85:
		case 86:
		case 87:
		case 88:
			strShipType = CString("Tanker,Reserved for future use"); break;
		case 89:
			strShipType = CString("Tanker,No additonal info"); break;
		case 90:
			strShipType = CString("Other type,all ships of this type"); break;
		case 91:
			strShipType = CString("Other type,Hazardous category A"); break;
		case 92:
			strShipType = CString("Other type,Hazardous category B"); break;
		case 93:
			strShipType = CString("Other type,Hazardous category C"); break;
		case 94:
			strShipType = CString("Other type,Hazardous category D"); break;
		case 95:
		case 96:
		case 97:
		case 98:
			strShipType = CString("Other type,Reserved for future use"); break;
		case 99:
			strShipType = CString("Other type,No additional info"); break;
		default:
			strShipType = CString("Other type,Not available"); break;
		}
		//ship-length&width
		CString strShipLength, strShipWidth;
		int toBow = pAisInfo->type5.to_bow;
		int toStern = pAisInfo->type5.to_stern;
		int toPort = pAisInfo->type5.to_port;
		int toStarbo = pAisInfo->type5.to_starboard;
		int length = 0;
		int width = 0;
		if (0 != toBow && 0 != toStern)
		{
			length = toBow + toStern;
			if (511 == toBow || 511 == toStern)
				strShipLength.Format(_T(">=%dm"), length);
			else
				strShipLength.Format(_T("%dm"), length);

		}
		else
			strShipLength = CString("No available ship-length info");
		if (0 != toPort && 0 != toStarbo)
		{
			width = toPort + toStarbo;
			if (63 == toPort || 63 == toStarbo)
				strShipWidth.Format(_T(">=%dm"), width);
			else
				strShipWidth.Format(_T("%dm"), width);
		}
		else
			strShipWidth = CString("No available ship-Width info");
		//data-ETA
		CString strETA, strMonth, strDay, strHour, strMinute;
		int month = pAisInfo->type5.month;
		if (0 == month)
			strMonth = CString("N/A月");
		else
			strMonth.Format(_T("%d月"), month);
		int day = pAisInfo->type5.day;
		if (0 == day)
			strDay = CString("N/A月");
		else
			strDay.Format(_T("%d日"), day);
		int hour = pAisInfo->type5.hour;
		if (24 == hour)
			strHour = CString("N/A时");
		else
			strHour.Format(_T("%d时"), hour);
		int minute = pAisInfo->type5.minute;
		if (60 == minute)
			strMinute = CString("N/A分");
		else
			strMinute.Format(_T("%d分"), minute);
		strETA = strMonth + strDay + strHour + strMinute;

		//draught
		CString strDraught;
		if (pAisInfo->type5.draught == 0)
			strDraught = CString("no avaialable info");
		else
			strDraught.Format(_T("%.1fm"), (pAisInfo->type5.draught)*0.1);

		//destination
		break;
	}

	//简要位置信息
	case 18:
	{
		//sog
		switch (pAisInfo->type18.speed)
		{
		case 1023:
			strSOG = CString("no available SOG info"); break;
		case 1024:
			strSOG = CString(">=102.2 knots"); break;
		default:
			float sog = (pAisInfo->type1.speed)*0.1;
			strSOG.Format(_T("%.1f knots"), sog);
			break;
		}

		//accuracy
		CString poAccu;
		if (true == pAisInfo->type18.accuracy)
			poAccu = "<10m";
		else
			poAccu = ">10m";

		//longtitude & LongNum
		CString strLong;
		int numLong = pAisInfo->type18.lon;
		dGeoPoX = numLong / 600000.0;
		int degLong = abs(numLong) / 600000;
		float minLong = (abs(numLong) % 600000) / 10000.0;
		switch (numLong)
		{
		case 181 * 600000:
			strLong = CString("no available longtitude info"); break;
		default:
			if (pAisInfo->type18.lon > 0)
				strLong.Format(_T("%d度%.3f分E"), abs(degLong), abs(minLong));
			else
				strLong.Format(_T("%d度%.3f分W"), abs(degLong), abs(minLong));
			break;
		}

		//Latitude & LatNum
		CString strLat;
		int numLat = pAisInfo->type18.lat;
		dGeoPoX = numLat / 600000.0;
		int degLat = abs(numLat) / 600000;
		float minLat = (abs(numLat) % 600000) / 10000.0;
		switch (numLat)
		{
		case 91 * 600000:
			strLat = CString("no available latitude info"); break;
		default:
			if (numLat > 0)
				strLat.Format(_T("%d度%.3f分N"), abs(degLat), abs(minLat));
			else
				strLat.Format(_T("%d度%.3f分S"), abs(degLat), abs(minLat));
			break;
		}
		//COG
		if (3600 == pAisInfo->type18.course)
			strCOG = CString("no available COG info");
		else
			strCOG.Format(_T("%.1f度"), (float)(pAisInfo->type18.course)*0.1);

		//HDG
		if (511 == pAisInfo->type18.heading)
			strHDG = CString("no available HDG info");
		else
			strHDG.Format(_T("%d度"), pAisInfo->type18.heading);

		break;
	}

	//
	case 19:
	{
		//sog
		switch (pAisInfo->type19.speed)
		{
		case 1023:
			strSOG = CString("no available SOG info"); break;
		case 1024:
			strSOG = CString(">=102.2 knots"); break;
		default:
			float sog = (pAisInfo->type1.speed)*0.1;
			strSOG.Format(_T("%.1f knots"), sog);
			break;
		}

		//accuracy
		CString poAccu;
		if (true == pAisInfo->type19.accuracy)
			poAccu = "<10m";
		else
			poAccu = ">10m";

		//longtitude & longNum
		CString strLong;
		int numLong = pAisInfo->type19.lon;
		dGeoPoX = numLong / 600000.0;
		int degLong = abs(numLong) / 600000;
		float minLong = (abs(numLong) % 600000) / 10000.0;
		switch (numLong)
		{
		case 181 * 600000:
			strLong = CString("no available longtitude info"); break;
		default:
			if (pAisInfo->type19.lon > 0)
				strLong.Format(_T("%d度%.3f分E"), abs(degLong), abs(minLong));
			else
				strLong.Format(_T("%d度%.3f分W"), abs(degLong), abs(minLong));
			break;
		}

		//Latitude
		CString strLat;
		int numLat = pAisInfo->type19.lat;
		dGeoPoY = numLat / 600000.0;
		int degLat = abs(numLat) / 600000;
		float minLat = (abs(numLat) % 600000) / 10000.0;
		switch (numLat)
		{
		case 91 * 600000:
			strLat = CString("no available latitude info"); break;
		default:
			if (numLat > 0)
				strLat.Format(_T("%d度%.3f分N"), abs(degLat), abs(minLat));
			else
				strLat.Format(_T("%d度%.3f分S"), abs(degLat), abs(minLat));
			break;
		}

		//COG
		if (3600 == pAisInfo->type19.course)
			strCOG = CString("no available COG info");
		else
			strCOG.Format(_T("%.1f度"), (float)(pAisInfo->type19.course)*0.1);

		//HDG
		if (511 == pAisInfo->type19.heading)
			strHDG = CString("no available HDG info");
		else
			strHDG.Format(_T("%d度"), pAisInfo->type19.heading);

		//shipname

		//ship-length&width
		CString strShipLength, strShipWidth;
		int toBow = pAisInfo->type19.to_bow;
		int toStern = pAisInfo->type19.to_stern;
		int toPort = pAisInfo->type19.to_port;
		int toStarbo = pAisInfo->type19.to_starboard;
		int length = 0;
		int width = 0;
		if (0 != toBow && 0 != toStern)
		{
			length = toBow + toStern;
			if (511 == toBow || 511 == toStern)
				strShipLength.Format(_T(">=%dm"), length);
			else
				strShipLength.Format(_T("%dm"), length);

		}
		else
			strShipLength = CString("No available ship-length info");
		if (0 != toPort && 0 != toStarbo)
		{
			width = toPort + toStarbo;
			if (63 == toPort || 63 == toStarbo)
				strShipWidth.Format(_T(">=%dm"), width);
			else
				strShipWidth.Format(_T("%dm"), width);
		}
		else
			strShipWidth = CString("No available ship-Width info");

		break;
	}

	//简单船舶静态信息
	case 24:
	{
		//船舶类型
		CString strShipType;
		switch (pAisInfo->type24.shiptype)
		{
		case 0:
			strShipType = CString("Not available"); break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			strShipType = CString("Reserved for future use"); break;
		case 20:
			strShipType = CString("WIG,all ships of this type"); break;
		case 21:
			strShipType = CString("WIG,Hazardous category A"); break;
		case 22:
			strShipType = CString("WIG,Hazardous category B"); break;
		case 23:
			strShipType = CString("WIG,Hazardous category C"); break;
		case 24:
			strShipType = CString("WIG,Hazardous category D"); break;
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
			strShipType = CString("WIG,Reserved for future use"); break;
		case 30:
			strShipType = CString("Fishing"); break;
		case 31:
			strShipType = CString("Towing"); break;
		case 32:
			strShipType = CString("Towing:船长>200m或船宽>25m"); break;
		case 33:
			strShipType = CString("Dredging or underwater ops"); break;
		case 34:
			strShipType = CString("Diving ops"); break;
		case 35:
			strShipType = CString("Military ops"); break;
		case 36:
			strShipType = CString("Sailing"); break;
		case 37:
			strShipType = CString("Pleasure Craft"); break;
		case 38:
		case 39:
			strShipType = CString("Reserved"); break;
		case 40:
			strShipType = CString("HSC,all ships of this type"); break;
		case 41:
			strShipType = CString("HSC,Hazardous category A"); break;
		case 42:
			strShipType = CString("HSC,Hazardous category B"); break;
		case 43:
			strShipType = CString("HSC,Hazardous category C"); break;
		case 44:
			strShipType = CString("HSC,Hazardous category D"); break;
		case 45:
		case 46:
		case 47:
		case 48:
			strShipType = CString("HSC,Reserved for future use"); break;
		case 49:
			strShipType = CString("HSC,No additional info"); break;
		case 50:
			strShipType = CString("Pilot vessel"); break;
		case 51:
			strShipType = CString("Search and Rescue vessel"); break;
		case 52:
			strShipType = CString("Tug"); break;
		case 53:
			strShipType = CString("Port Tender"); break;
		case 54:
			strShipType = CString("Anti-pollution equipment"); break;
		case 55:
			strShipType = CString("Law Enforcement"); break;
		case 56:
		case 57:
			strShipType = CString("Spare-Local Vessel"); break;
		case 58:
			strShipType = CString("Medical Transport"); break;
		case 59:
			strShipType = CString("Ship according to RR Resolution No.18"); break;
		case 60:
			strShipType = CString("Passenger,all ships of this type"); break;
		case 61:
			strShipType = CString("Passenger,Hazardous category A"); break;
		case 62:
			strShipType = CString("Passenger,Hazardous category B"); break;
		case 63:
			strShipType = CString("Passenger,Hazardous category C"); break;
		case 64:
			strShipType = CString("Passenger,Hazardous category D"); break;
		case 65:
		case 66:
		case 67:
		case 68:
			strShipType = CString("Passenger,Reserved for future use"); break;
		case 69:
			strShipType = CString("Passenger,No additional info"); break;
		case 70:
			strShipType = CString("Cargo,all ships of this type"); break;
		case 71:
			strShipType = CString("Cargo,Hazardous category A"); break;
		case 72:
			strShipType = CString("Cargo,Hazardous category B"); break;
		case 73:
			strShipType = CString("Cargo,Hazardous category C"); break;
		case 74:
			strShipType = CString("Cargo,Hazardous category D"); break;
		case 75:
		case 76:
		case 77:
		case 78:
			strShipType = CString("Cargo,Reserved for future use"); break;
		case 79:
			strShipType = CString("Cargo,No additonal info"); break;
		case 80:
			strShipType = CString("Tanker,all ships of this type"); break;
		case 81:
			strShipType = CString("Tanker,Hazardous category A"); break;
		case 82:
			strShipType = CString("Tanker,Hazardous category B"); break;
		case 83:
			strShipType = CString("Tanker,Hazardous category C"); break;
		case 84:
			strShipType = CString("Tanker,Hazardous category D"); break;
		case 85:
		case 86:
		case 87:
		case 88:
			strShipType = CString("Tanker,Reserved for future use"); break;
		case 89:
			strShipType = CString("Tanker,No additonal info"); break;
		case 90:
			strShipType = CString("Other type,all ships of this type"); break;
		case 91:
			strShipType = CString("Other type,Hazardous category A"); break;
		case 92:
			strShipType = CString("Other type,Hazardous category B"); break;
		case 93:
			strShipType = CString("Other type,Hazardous category C"); break;
		case 94:
			strShipType = CString("Other type,Hazardous category D"); break;
		case 95:
		case 96:
		case 97:
		case 98:
			strShipType = CString("Other type,Reserved for future use"); break;
		case 99:
			strShipType = CString("Other type,No additional info"); break;
		default:
			strShipType = CString("Other type,Not available"); break;
		}
		//shipname
		//shipCallSign
		//ship-length & width
		if (!AIS_AUXILIARY_MMSI(pAisInfo->mmsi))
		{
			CString strShipLength, strShipWidth;
			int toBow = pAisInfo->type24.dim.to_bow;
			int toStern = pAisInfo->type24.dim.to_stern;
			int toPort = pAisInfo->type24.dim.to_port;
			int toStarbo = pAisInfo->type24.dim.to_starboard;
			int length = 0;
			int width = 0;
			if (0 != toBow && 0 != toStern)
			{
				length = toBow + toStern;
				if (511 == toBow || 511 == toStern)
					strShipLength.Format(_T(">=%dm"), length);
				else
					strShipLength.Format(_T("%dm"), length);

			}
			else
				strShipLength = CString("No available ship-length info");
			if (0 != toPort && 0 != toStarbo)
			{
				width = toPort + toStarbo;
				if (63 == toPort || 63 == toStarbo)
					strShipWidth.Format(_T(">=%dm"), width);
				else
					strShipWidth.Format(_T("%dm"), width);
			}
			else
				strShipWidth = CString("No available ship-Width info");
		}
		break;
	}

	default:
		break;
	}

	//继续获取其余船舶绘制参数...
	USES_CONVERSION;
	if (strHDG)
	{
		LPSTR str = T2A(strHDG);
		fHeading = (float)strtod(str, NULL);
		if (fabs(fHeading) < 1e-6)//如果船艏向不可用，则将之置为0，暂定为这样
			fHeading = 0.0;
	}
	else
		fHeading = 0.0;//如果船艏向不可用，则将之置为0
	if (strCOG)
	{
		LPSTR str = T2A(strCOG);
		fCOG = (float)strtod(str, NULL);
		if (fabs(fCOG) < 1e-6)//真航向不可用，将之置为0，，
			fCOG = 0.0;
	}
	else
		fCOG = 0.0;
	fCOW = fCOG;//AIS中没有对水航向信息，将之置为和真航向一样

	if (!strSOG.IsEmpty())
	{
		LPSTR str = T2A(strSOG);
		fSOG = (float)strtod(str, NULL);
		if (fabs(fSOG) < 1e-6)//真航速不可用，将之置为0，，
			fSOG = 0.0;
	}
	else
		fSOG = 0.0;
	fSOW = fSOG;//AIS中没有对水航速信息，将之置为和真航速一样

	if (181.0 != dGeoPoX && 91.0 != dGeoPoY)//数值不可用	
	{
		int lMMSI = pAisInfo->mmsi;
		double dadiX = 0, dadiY = 0;
		WebMercatorProjCal(dGeoPoX, dGeoPoY, dadiX, dadiY);
		vector<SHIP>::iterator i = aisShips.begin();
		while (aisShips.end() != i)
		{
			if (i->mmsi == lMMSI)	//目标船已存在,更新目标船
			{
				i->heading = fHeading;
				i->courseOverGround = fCourse;
				i->speedOverGround = fSpeed;
				i->speedThrghWather = fSpeed;	//无对水航速，暂时设为和航速一样
				i->longitude = dGeoPoX;
				i->latitude = dGeoPoY;
				i->dadiX = dadiX;
				i->dadiY = dadiY;
				i->aisCheckCount = 0;

				break;
			}
			i++;
		}
		if (aisShips.end() == i)//添加新目标船
		{
			SHIP ship;
			ship.mmsi = lMMSI;
			ship.heading = fHeading;
			ship.courseOverGround = fCourse;
			ship.speedOverGround = fSpeed;
			ship.speedThrghWather = fSpeed;	//无对水航速，暂时设为和航速一样
			ship.longitude = dGeoPoX;
			ship.latitude = dGeoPoY;
			ship.dadiX = dadiX;
			ship.dadiY = dadiY;
			ship.aisCheckCount = 0;

			//无长宽信息，暂定
			ship.length = 100;
			ship.width = 40;
			ship.collisionRadius = ship.length*0.707;
			aisShips.push_back(ship);
		}
	}
	return aisShips;
}

//北半球A、B两点距离(Km)
double CRadarView::GeoToDistance(double aLon/*a点经度*/, double aLat/*a点纬度*/, double bLon/*b点经度*/, double bLat/*b点纬度*/)
{
	double lon = aLon > bLon ? (aLon - bLon) : (bLon - aLon);
	return acos(sin(_ToRad(aLat))*sin(_ToRad(bLat)) + cos(_ToRad(aLat))*cos(_ToRad(bLat))*cos(_ToRad(lon)))*6371.004;

}

//根据船的AIS信息画一条AIS船
//void CRadarView::DrawAISShip(Graphics *graphics, MyMemDC *pMemDC, SHIP &ship)
//{
//	Pen pen(Color(0, 0, 0, 0));
//	if (ship.aisSelected)
//		pen.SetColor(Color(150, 255, 201, 14));	//橙色
//	else
//		pen.SetColor(Color(150, 80, 208, 211));	//蓝色
//	double angle1 = m_northAngle + _ToRad(ship.heading);	//船首
//	if (angle1 >= 2 * PI)
//		angle1 -= 2 * PI;
//	double angle2 = angle1 + _ToRad(150);
//	if (angle2 >= 2 * PI)
//		angle2 -= 2 * PI;
//	double angle3 = angle1 + _ToRad(210);
//	if (angle3 >= 2 * PI)
//		angle3 -= 2 * PI;
//	double l = ship.length / m_perPixelM;
//	if (l < 8)
//		l = 8;
//	double d = 2 * l;
//
//	ship.aisPoint[0].X = ship.rotateX;
//	ship.aisPoint[0].Y = ship.rotateY;
//	ship.aisPoint[1].X = ship.aisPoint[0].X + (int)(l*sin(angle1));
//	ship.aisPoint[1].Y = ship.aisPoint[0].Y - (int)(l*cos(angle1));
//	ship.aisPoint[2].X = ship.aisPoint[0].X + (int)(l*sin(angle2));
//	ship.aisPoint[2].Y = ship.aisPoint[0].Y - (int)(l*cos(angle2));
//	ship.aisPoint[3].X = ship.aisPoint[0].X + (int)(l*sin(angle3));
//	ship.aisPoint[3].Y = ship.aisPoint[0].Y - (int)(l*cos(angle3));
//	ship.aisPoint[4].X = ship.aisPoint[0].X + (int)(d*sin(angle1));
//	ship.aisPoint[4].Y = ship.aisPoint[0].Y - (int)(d*cos(angle1));
//	graphics->DrawLine(&pen, ship.aisPoint[0], ship.aisPoint[4]);
//	graphics->DrawLine(&pen, ship.aisPoint[1], ship.aisPoint[2]);
//	graphics->DrawLine(&pen, ship.aisPoint[1], ship.aisPoint[3]);
//	graphics->DrawLine(&pen, ship.aisPoint[2], ship.aisPoint[3]);
//}

void CRadarView::DrawDEM(Graphics *graphics, MyMemDC *pMemDC, CBitmapEx radarBmp)
{

	if (radarBmp.IsValid()){
		radarBmp.Save(_T("./pic/vtsRadar.bmp"));
	}
	Image img(_T("./pic/vtsRadar.bmp"));
	int radius = m_radarRect.Width() / 2;
	graphics->DrawImage(&img, m_radarCenter.X - radius, m_radarCenter.Y - radius, radius * 2, radius * 2);
}

//void CRadarView::DrawOtherShip(Graphics *graphics, MyMemDC *pMemDC)
//{
//
//	int num = m_OtherShip.size();
//	CBitmapEx bitmap;
//	bitmap.Create(m_viewRect.Width(), m_viewRect.Height());
//	bitmap.Clear();
//	for (int i = 0; i < num; i++)
//	{
//		GetScrPointFromGeo(&(m_OtherShip[i].posX), &(m_OtherShip[i].posY), m_OtherShip[i].longitude, m_OtherShip[i].latitude);
//		//	HRGN rgn = m_radarRgn;
//		/*if (PtInRegion(rgn, m_OtherShip[i].posX, m_OtherShip[i].posY))
//		{*/
//
//		DrawRadarShipOnDc(graphics, pMemDC, &m_OtherShip[i]);
//
//		DrawRadarShipOnBMP(&bitmap, &m_OtherShip[i]);
//
//
//		if (m_AISSwitch)
//			DrawAISShip(graphics, pMemDC, m_OtherShip[i]);
//
//		//	}
//
//	}
//
//	//	bitmap.FillWithKNN(_RGB(255, 255, 0));
//
//	bitmap.Save(_T("../Radar/pic/otherShip.bmp"));
//
//	//创建副本  耗时31ms
//	DWORD d1 = GetTickCount();
//	CBitmapEx bmpMem;
//	bmpMem.Create(bitmap);
//	//	mtx_otherShipBMP.lock();
//	bmpMem.Save(_T("../Radar/pic/otherShip_mem.bmp"));
//	//	mtx_otherShipBMP.unlock();
//	DWORD d2 = GetTickCount();
//	DWORD diff = d2 - d1;
//}

//void CRadarView::DrawARPARecord(Graphics *graphics, MyMemDC *pMemDC)
//{
//	Pen pen_normal(Color(34, 177, 76));
//	for (int i = 0; i < (int)m_arpaRecord.size(); i++)
//	{
//		Pen pen_body(Color(0, 0, 0));
//		SolidBrush brush_word(Color(0, 0, 0, 0));
//
//		if (m_arpaRecord[i].state == ARPA_RECORD_STATE_MISSING)
//		{
//			pen_body.SetColor(Color(104, 104, 104));
//			brush_word.SetColor(Color(104, 104, 104));
//		}
//		else
//		{
//
//			if (m_arpaRecord[i].state == ARPA_RECORD_STATE_CALCULATING)
//			{
//				pen_body.SetColor(Color(0, 255, 255));
//				brush_word.SetColor(Color(0, 255, 255));
//			}
//			else if (m_arpaRecord[i].state == ARPA_RECORD_STATE_NORMAL)
//			{
//				pen_body.SetColor(Color(34, 177, 76));
//				brush_word.SetColor(Color(34, 177, 76));
//			}
//			if (m_arpaRecord[i].passState == ARPA_RECORD_COLLISION)
//			{
//				pen_body.SetColor(Color(255, 0, 0));
//				brush_word.SetColor(Color(255, 0, 0));
//			}
//		}
//		StringFormat stringFormat;
//		stringFormat.SetLineAlignment(StringAlignmentCenter);
//
//
//		int size_EN = m_arpaRecord[i].gateRect.Width();
//		Gdiplus::Font font_EN(_T("Arial"), size_EN, FontStyleRegular, UnitPixel);
//		CString stemp;
//		stemp.Format(_T("%d"), m_arpaRecord[i].index);
//		graphics->DrawRectangle(&pen_body, Rect(m_arpaRecord[i].gateRect.left, m_arpaRecord[i].gateRect.top, m_arpaRecord[i].gateRect.Width(), m_arpaRecord[i].gateRect.Height()));
//		//	graphics->DrawString(stemp, -1, &font_EN, PointF(m_arpaRecord[i].scrX - size_EN / 2, m_arpaRecord[i].scrY), &stringFormat, &brush_word);
//		//目标碰撞范围
//		//	graphics->DrawEllipse(&Pen(Color(255, 0, 0)), Rect(m_arpaRecord[i].center.x - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].center.y - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM));
//		//碰撞时间
//		if (m_arpaRecord[i].passState == ARPA_RECORD_COLLISION)
//		{
//			int m, s;//分、秒
//			m = m_arpaRecord[i].attackTime / 60;
//			s = m_arpaRecord[i].attackTime % 60;
//			if (m == 0)
//				stemp.Format(_T("%ds"), s);
//			else
//				stemp.Format(_T("%dm%ds"), m, s);
//			m_arpaRecord[i].length_Pixel = m_arpaRecord[i].gateRect.Width();
//			int size_EN_small = m_arpaRecord[i].length_Pixel * 2 / stemp.GetLength();
//			Gdiplus::Font font_EN_small(_T("Arial"), size_EN_small, FontStyleRegular, UnitPixel);
//			graphics->DrawString(stemp, -1, &font_EN_small, PointF(m_arpaRecord[i].gateRect.left, m_arpaRecord[i].gateRect.bottom + size_EN_small), &stringFormat, &brush_word);
//		}
//
//		//if (m_arpaRecord[i].AISship != NULL)	//该记录为船只
//		//{
//
//		//	int size_EN = m_arpaRecord[i].length_Pixel;
//		//	Gdiplus::Font font_EN(_T("Arial"), size_EN, FontStyleRegular, UnitPixel);
//		//	CString stemp;
//		//	stemp.Format(_T("%d"), m_arpaRecord[i].index);
//		//	graphics->DrawRectangle(&pen_body, m_arpaRecord[i].rectBody);
//		//	graphics->DrawString(stemp, -1, &font_EN, PointF(m_arpaRecord[i].srcX - size_EN / 2, m_arpaRecord[i].srcY), &stringFormat, &brush_word);
//		//	//目标碰撞范围
//		//	graphics->DrawEllipse(&Pen(Color(255, 0, 0)), Rect(m_arpaRecord[i].center.x - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].center.y - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM));
//		//	//碰撞时间
//		//	if (m_arpaRecord[i].passState == ARPA_RECORD_COLLISION)
//		//	{
//		//		int m, s;//分、秒
//		//		m = m_arpaRecord[i].attackTime / 60;
//		//		s = m_arpaRecord[i].attackTime % 60;
//		//		if (m == 0)
//		//			stemp.Format(_T("%ds"), s);
//		//		else
//		//			stemp.Format(_T("%dm%ds"), m, s);
//		//		int size_EN_small = m_arpaRecord[i].length_Pixel * 2 / stemp.GetLength();
//		//		Gdiplus::Font font_EN_small(_T("Arial"), size_EN_small, FontStyleRegular, UnitPixel);
//		//		graphics->DrawString(stemp, -1, &font_EN_small, PointF(m_arpaRecord[i].rectBody.X, m_arpaRecord[i].rectBody.Y + m_arpaRecord[i].rectBody.Height + size_EN_small), &stringFormat, &brush_word);
//		//	}
//		//	double angle = m_northAngle + _ToRad(m_arpaRecord[i].AISship->heading);	//船首
//		//	if (angle >= 2 * PI)
//		//		angle -= 2 * PI;
//
//		//	Point p;
//		//	p.X = m_arpaRecord[i].srcX + (int)(2 * m_arpaRecord[i].length_Pixel*sin(angle));
//		//	p.Y = m_arpaRecord[i].srcY - (int)(2 * m_arpaRecord[i].length_Pixel*cos(angle));
//		//	graphics->DrawLine(&pen_body, p, Point(m_arpaRecord[i].srcX, m_arpaRecord[i].srcY));
//		//}
//		//else		//记录不是船只
//		//{
//		//	GetSrcPointFromGeo(&m_arpaRecord[i].srcX, &m_arpaRecord[i].srcY, m_arpaRecord[i].longaitude, m_arpaRecord[i].latitude);
//		//	RotatePoint(&m_arpaRecord[i].srcX, &m_arpaRecord[i].srcY, CPoint(m_MainShip.posX, m_MainShip.posY), m_northAngle);
//		//	m_arpaRecord[i].length_Pixel = m_arpaRecord[i].length_Meter / m_perPixelM;
//		//	if (m_arpaRecord[i].length_Pixel < 16)
//		//		m_arpaRecord[i].length_Pixel = 16;
//		//	m_arpaRecord[i].rectBody = Rect(m_arpaRecord[i].srcX - m_arpaRecord[i].length_Pixel / 2, m_arpaRecord[i].srcY - m_arpaRecord[i].length_Pixel / 2 + 8, m_arpaRecord[i].length_Pixel, m_arpaRecord[i].length_Pixel);
//		//	m_arpaRecord[i].center = CPoint(m_arpaRecord[i].rectBody.X + m_arpaRecord[i].rectBody.Width / 2, m_arpaRecord[i].rectBody.Y + m_arpaRecord[i].rectBody.Height / 2);
//		//	int size_EN = m_arpaRecord[i].length_Pixel;
//		//	Gdiplus::Font font_EN(_T("Arial"), size_EN, FontStyleRegular, UnitPixel);
//		//	CString stemp;
//		//	stemp.Format(_T("%d"), m_arpaRecord[i].index);
//		//	//目标碰撞范围
//		//	graphics->DrawEllipse(&Pen(Color(255, 0, 0)), Rect(m_arpaRecord[i].center.x - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].center.y - m_arpaRecord[i].collisionRadius / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM, m_arpaRecord[i].collisionRadius * 2 / m_perPixelM));
//		//	//碰撞时间
//		//	if (m_arpaRecord[i].passState == ARPA_RECORD_COLLISION)
//		//	{
//		//		int m, s;//分、秒
//		//		m = m_arpaRecord[i].attackTime / 60;
//		//		s = m_arpaRecord[i].attackTime % 60;
//		//		if (m == 0)
//		//			stemp.Format(_T("%ds"), s);
//		//		else
//		//			stemp.Format(_T("%dm%ds"), m, s);
//		//		int size_EN_small = m_arpaRecord[i].length_Pixel * 2 / stemp.GetLength();
//		//		Gdiplus::Font font_EN_small(_T("Arial"), size_EN_small, FontStyleRegular, UnitPixel);
//		//		graphics->DrawString(stemp, -1, &font_EN_small, PointF(m_arpaRecord[i].rectBody.X, m_arpaRecord[i].rectBody.Y + m_arpaRecord[i].rectBody.Height + size_EN_small), &stringFormat, &brush_word);
//		//	}
//		//	graphics->DrawRectangle(&pen_body, m_arpaRecord[i].rectBody);
//		//	graphics->DrawString(stemp, -1, &font_EN, PointF(m_arpaRecord[i].srcX - size_EN / 2, m_arpaRecord[i].rectBody.Y + m_arpaRecord[i].rectBody.Height / 2), &stringFormat, &brush_word);
//		//}
//	}
//}



void CRadarView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	HRGN radarHRGN = m_radarRgn;
	//点击在雷达区域内
	if (PtInRegion(radarHRGN, point.x, point.y))
	{
		if (m_extendedRangeMeasurementSwitch)
		{
			m_ERMFunctionSwitch = true;
			m_ERM_Roam = false;
			m_targetBearingNum = 0;
			m_targetRangeNum = 0;
			m_ERM_P1 = CPoint(0, 0);
			m_ERM_P2 = CPoint(0, 0);
			CRgn rgn1;
			rgn1.CreateRectRgn(0, 0, 0, 0);
			rgn1.CombineRgn(&m_radarRgn, &m_buttonRgn, RGN_OR);
			InvalidateRgn(&rgn1);
		}

		if (m_variableRangeMarker1Switch)
		{
			m_variableRangeMarker1Switch = false;
			m_VRM1Roam = false;
			m_VRM1Pixel = 0;
			m_VRM1distance = 0;
			CRgn rgn1;
			rgn1.CreateRectRgnIndirect(m_Button[BN_VariableRangeMarker_1_SWITCH]);
			rgn1.CombineRgn(&m_radarRgn, &rgn1, RGN_OR);
			InvalidateRgn(&rgn1);
		}
	}

	if (m_AISControlBoxDisplay)
	{
		m_AISControlBoxDisplay = false;
		InvalidateRgn(&m_AISControlBoxRgn);
	}

	if (m_ARPAControlBoxDisplay)
	{
		m_ARPAControlBoxDisplay = false;
		InvalidateRgn(&m_ARPAControlBoxRgn);
	}


	CView::OnRButtonDown(nFlags, point);
}

void CRadarView::DrawVectorTime(Graphics *graphics, MyMemDC *pMemDC, SHIP ship, double time/*单位:分钟*/)
{
	Pen pen_vectorTime(Color(150, 255, 255, 255), 2);
	pen_vectorTime.SetEndCap(LineCapRoundAnchor);
	pen_vectorTime.SetDashStyle(DashStyleDashDot);
	Point p1, p2;
	double angle1 = m_northAngle + _ToRad(ship.courseOverGround);	//船对地航向
	if (angle1 >= 2 * PI)
		angle1 -= 2 * PI;
	double l = (ship.speedOverGround*time / 60.0) / m_perPixelNM;
	p1.X = ship.posX;
	p1.Y = ship.posY;
	p2.X = p1.X + (int)(l*sin(angle1));
	p2.Y = p1.Y - (int)(l*cos(angle1));
	graphics->DrawLine(&pen_vectorTime, p1, p2);
}

void CRadarView::SetMessageArea1Para(MessageArea_1_Parameter para)
{
	m_MessageArea_1_Parameter = para;
	InvalidateRect(m_Button[BN_MessageArea_1]);
}

//void CRadarView::DrawMessageArea1(Graphics *graphics, MyMemDC *pMemDC)
//{
//	int left = m_Button[BN_MessageArea_1].left;
//	int right = m_Button[BN_MessageArea_1].right;
//	int top = m_Button[BN_MessageArea_1].top;
//	int bottom = m_Button[BN_MessageArea_1].bottom;
//	int width = m_Button[BN_MessageArea_1].Width();
//	int height = m_Button[BN_MessageArea_1].Height();
//	SolidBrush brush_word(Color(0, 0, 0));
//	SolidBrush brush_red(Color(255, 0, 0));
//	if (m_dayOrNight)
//	{
//		brush_word.SetColor(Color(255, 255, 255));
//	}
//	else
//	{
//		brush_word.SetColor(Color(42, 255, 41));
//	}
//	int size_EN_Nomal = 18 * m_mulOf800_600;
//	int size_CHN_Nomal = 20 * m_mulOf800_600;
//	int size_EN_Small = 12 * m_mulOf800_600;
//	int size_CHN_Small = 16 * m_mulOf800_600;
//	int size_CHN_Smallest = 12 * m_mulOf800_600;
//	Gdiplus::Font font_CHN_Nomal(_T("隶书"), size_CHN_Nomal, FontStyleRegular, UnitPixel);
//	Gdiplus::Font font_EN_Nomal(_T("Arial"), size_EN_Nomal, FontStyleRegular, UnitPixel);
//	Gdiplus::Font font_CHN_Small(_T("隶书"), size_CHN_Small, FontStyleRegular, UnitPixel);
//	Gdiplus::Font font_CHN_Smallest(_T("隶书"), size_CHN_Smallest, FontStyleRegular, UnitPixel);
//	Gdiplus::Font font_EN_Small(_T("Arial"), size_EN_Small, FontStyleRegular, UnitPixel);
//	StringFormat stringFormat;
//	stringFormat.SetLineAlignment(StringAlignmentCenter);
//	CString stemp;
//
//	switch (m_MessageArea_1_Parameter)
//	{
//	case MA_1_BLANK:
//		break;
//	case MA_1_EXPAND:
//	{
//		break;
//	}
//	case MA_1_ARPA_ON:
//	{
//		if (m_language)
//		{
//			graphics->DrawString(L"请打开ARPA开关！", -1, &font_CHN_Small, PointF(left, top), &brush_word);
//		}
//		else
//		{
//			graphics->DrawString(L"Please turn on the ARPA !", -1, &font_EN_Small, PointF(left, top), &brush_word);
//		}
//		break;
//	}
//	case MA_1_ARPA_OFF:
//	{
//		if (m_language)
//		{
//			graphics->DrawString(L"请关闭ARPA开关！", -1, &font_CHN_Small, PointF(left, top), &brush_word);
//		}
//		else
//		{
//			graphics->DrawString(L"Please turn off the ARPA!", -1, &font_EN_Small, PointF(left, top), &brush_word);
//		}
//		break;
//	}
//	case MA_1_ARPA_INFO:
//	{
//		ARPA_RECORD * p = &m_arpaRecord[0];
//		if (m_language)
//		{
//			int namedis = width / 15;
//			int valuedis = width / 2;
//			int topdis = height / 10;
//
//			stemp.Format(_T("----ARPA  目标%d----"), m_ArpaRecordSelected->index);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 10) / 2, top), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->bearing);
//			graphics->DrawString(L"方位", -1, &font_CHN_Small, PointF(left + namedis, top + topdis), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.3fNM"), m_ArpaRecordSelected->distance);
//			graphics->DrawString(L"距离", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 2), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 2), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM/h"), m_ArpaRecordSelected->TSpeed);
//			graphics->DrawString(L"真速度", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 3), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 3), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else if (m_ArpaRecordSelected->TCourse == -1)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->TCourse);
//			graphics->DrawString(L"真航向", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 4), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 4), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM"), m_ArpaRecordSelected->CPA);
//			graphics->DrawString(L"CPA", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 5), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 5), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fmin"), m_ArpaRecordSelected->TCPA);
//			graphics->DrawString(L"TCPA", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 6), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 6), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM/h"), m_ArpaRecordSelected->RSpeed);
//			graphics->DrawString(L"相对速度", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 7), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 7), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else if (m_ArpaRecordSelected->RCourse == -1)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->RCourse);
//			graphics->DrawString(L"相对航向", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 8), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 8), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_PASSSTEM)
//				graphics->DrawString(L"过船首", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 3) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_PASSSTERN)
//				graphics->DrawString(L"过船尾", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 3) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_COLLISION)
//				graphics->DrawString(L"有碰撞危险！", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 5) / 2, top + topdis * 9), &brush_red);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_NOCOLLISION)
//				graphics->DrawString(L"无碰撞危险", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 5) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_NOCROSSLINE)
//				graphics->DrawString(L"无相交航线", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 5) / 2, top + topdis * 9), &brush_word);
//		}
//		else
//		{
//			int namedis = width / 15;
//			int valuedis = width / 2;
//			int topdis = height / 10;
//
//			stemp.Format(_T("----ARPA  TARGET%d----"), m_ArpaRecordSelected->index);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + (width - size_CHN_Small * 10) / 2, top), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->bearing);
//			graphics->DrawString(L"Bearing", -1, &font_EN_Small, PointF(left + namedis, top + topdis), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.3fNM"), m_ArpaRecordSelected->distance);
//			graphics->DrawString(L"Distance", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 2), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 2), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM/h"), m_ArpaRecordSelected->TSpeed);
//			graphics->DrawString(L"T Speed", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 3), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 3), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else if (m_ArpaRecordSelected->TCourse == -1)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->TCourse);
//			graphics->DrawString(L"T Course", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 4), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 4), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM"), m_ArpaRecordSelected->CPA);
//			graphics->DrawString(L"CPA", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 5), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 5), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fmin"), m_ArpaRecordSelected->TCPA);
//			graphics->DrawString(L"TCPA", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 6), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 6), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1fNM/h"), m_ArpaRecordSelected->RSpeed);
//			graphics->DrawString(L"R Speed", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 7), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 7), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING)
//				stemp.Format(_T("--"));
//			else if (m_ArpaRecordSelected->RCourse == -1)
//				stemp.Format(_T("--"));
//			else
//				stemp.Format(_T("%.1f°"), m_ArpaRecordSelected->RCourse);
//			graphics->DrawString(L"R Course", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 8), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 8), &brush_word);
//
//			if (m_ArpaRecordSelected->state == ARPA_RECORD_STATE_MISSING);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_PASSSTEM)
//				graphics->DrawString(L"PASS THE STEM", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 7) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_PASSSTERN)
//				graphics->DrawString(L"PASS THE STERN", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 8) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_COLLISION)
//				graphics->DrawString(L"RISK OF COLLISION!", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 10) / 2, top + topdis * 9), &brush_red);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_NOCOLLISION)
//				graphics->DrawString(L"NO COLLISION", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 6) / 2, top + topdis * 9), &brush_word);
//			else if (m_ArpaRecordSelected->passState == ARPA_RECORD_NOCROSSLINE)
//				graphics->DrawString(L"NO CROSS LINE", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 7) / 2, top + topdis * 9), &brush_word);
//		}
//		break;
//	}
//	case MA_1_AIS_ON:
//	{
//		if (m_language)
//		{
//			graphics->DrawString(L"请打开AIS开关！", -1, &font_CHN_Small, PointF(left, top), &brush_word);
//		}
//		else
//		{
//			graphics->DrawString(L"Please turn on the AIS !", -1, &font_EN_Small, PointF(left, top), &brush_word);
//		}
//		break;
//	}
//	case MA_1_AIS_OFF:
//	{
//		if (m_language)
//		{
//			graphics->DrawString(L"请关闭AIS开关！", -1, &font_CHN_Small, PointF(left, top), &brush_word);
//		}
//		else
//		{
//			graphics->DrawString(L"Please turn off the AIS !", -1, &font_EN_Small, PointF(left, top), &brush_word);
//		}
//		break;
//	}
//	case MA_1_AIS_INFO:
//	{
//		if (m_aisShipSelected->mmsi == 0)
//		{
//			break;
//		}
//		if (m_language)
//		{
//			int namedis = width / 15;
//			int valuedis = width / 2;
//			int topdis = height / 7;
//			graphics->DrawString(L"----AIS信息----", -1, &font_CHN_Small, PointF(left + (width - size_CHN_Small * 8) / 2, top), &brush_word);
//
//			stemp.Format(_T("%d"), m_aisShipSelected->mmsi);
//			graphics->DrawString(L"MMSI", -1, &font_CHN_Small, PointF(left + namedis, top + topdis), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis), &brush_word);
//
//			stemp.Format(_T("%.1f°"), m_aisShipSelected->heading);
//			graphics->DrawString(L"船艏向", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 2), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 2), &brush_word);
//
//			stemp.Format(_T("%.1f°"), m_aisShipSelected->courseOverGround);
//			graphics->DrawString(L"对地航向", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 3), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 3), &brush_word);
//
//			stemp.Format(_T("%.1fNM"), m_aisShipSelected->speedOverGround);
//			graphics->DrawString(L"速度", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 4), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 4), &brush_word);
//
//			if (m_aisShipSelected->longitude > 0)
//				stemp.Format(_T("%.3fE"), m_aisShipSelected->longitude);
//			else
//				stemp.Format(_T("%.3fW"), -m_aisShipSelected->longitude);
//			graphics->DrawString(L"经度", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 5), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 5), &brush_word);
//
//			if (m_aisShipSelected->latitude > 0)
//				stemp.Format(_T("%.3fN"), m_aisShipSelected->latitude);
//			else
//				stemp.Format(_T("%.3fS"), -m_aisShipSelected->latitude);
//			graphics->DrawString(L"纬度", -1, &font_CHN_Small, PointF(left + namedis, top + topdis * 6), &brush_word);
//			graphics->DrawString(stemp, -1, &font_CHN_Small, PointF(left + valuedis, top + topdis * 6), &brush_word);
//		}
//		else
//		{
//			int namedis = width / 15;
//			int valuedis = width / 2;
//			int topdis = height / 7;
//			graphics->DrawString(L"----AIS  Message----", -1, &font_EN_Small, PointF(left + (width - size_EN_Small * 8) / 2, top), &brush_word);
//
//			stemp.Format(_T("%d"), m_aisShipSelected->mmsi);
//			graphics->DrawString(L"MMSI", -1, &font_EN_Small, PointF(left + namedis, top + topdis), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis), &brush_word);
//
//			stemp.Format(_T("%.1f°"), m_aisShipSelected->heading);
//			graphics->DrawString(L"Heading", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 2), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 2), &brush_word);
//
//			stemp.Format(_T("%.1f°"), m_aisShipSelected->courseOverGround);
//			graphics->DrawString(L"COG", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 3), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 3), &brush_word);
//
//			stemp.Format(_T("%.1fNM"), m_aisShipSelected->speedOverGround);
//			graphics->DrawString(L"Speed", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 4), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 4), &brush_word);
//
//			if (m_aisShipSelected->longitude > 0)
//				stemp.Format(_T("%.3fE"), m_aisShipSelected->longitude);
//			else
//				stemp.Format(_T("%.3fW"), -m_aisShipSelected->longitude);
//			graphics->DrawString(L"Longitude", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 5), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 5), &brush_word);
//
//			if (m_aisShipSelected->latitude > 0)
//				stemp.Format(_T("%.3fN"), m_aisShipSelected->latitude);
//			else
//				stemp.Format(_T("%.3fS"), -m_aisShipSelected->latitude);
//			graphics->DrawString(L"Latitude", -1, &font_EN_Small, PointF(left + namedis, top + topdis * 6), &brush_word);
//			graphics->DrawString(stemp, -1, &font_EN_Small, PointF(left + valuedis, top + topdis * 6), &brush_word);
//		}
//		break;
//	}
//	default:
//		break;
//	}
//}

//int CRadarView::GetAISShipIndex(CPoint point)
//{
//	for (int i = 0; i < (int)m_OtherShip.size(); i++)
//	{
//		CPoint p[3];
//		p[0].x = m_OtherShip[i].aisPoint[1].X;
//		p[0].y = m_OtherShip[i].aisPoint[1].Y;
//		p[1].x = m_OtherShip[i].aisPoint[2].X;
//		p[1].y = m_OtherShip[i].aisPoint[2].Y;
//		p[2].x = m_OtherShip[i].aisPoint[3].X;
//		p[2].y = m_OtherShip[i].aisPoint[3].Y;
//
//		CRgn rgn;
//		rgn.CreatePolygonRgn(p, 3, ALTERNATE);
//		if (PtInRegion(rgn, point.x, point.y))
//		{
//			rgn.DeleteObject();
//			return i;
//		}
//		rgn.DeleteObject();
//	}
//	return -1;
//}

//int CRadarView::GetARPARecordIndex(CPoint point)
//{
//	for (int i = 0; i < (int)m_arpaRecord.size(); i++)
//	{
//		if (PtInRect(m_arpaRecord[i].gateRect, point))
//		{
//			return i;
//		}
//	}
//	return -1;
//}




//void CRadarView::DrawRadarShipOnDc(Graphics *graphics, MyMemDC *pMemDC, SHIP *ship)
//{
//	CPoint p = RotatePoint(ship->posX, ship->posY, CPoint(m_MainShip.posX, m_MainShip.posY), m_northAngle);
//	ship->rotateX = p.x;
//	ship->rotateY = p.y;
//
//	SolidBrush yellowbrush(Color(255, 255, 0));
//	ship->widthPixel = ship->width / m_perPixelM;
//	ship->lengthPixel = ship->length / m_perPixelM;
//	if (ship->widthPixel < 2) ship->widthPixel = 2;
//	if (ship->lengthPixel < 8)	ship->lengthPixel = 8;
//	Rect rect(ship->posX - ship->widthPixel / 2, ship->posY - ship->lengthPixel / 2, ship->widthPixel, ship->lengthPixel);
//
//	//	graphics->SetSmoothingMode(SmoothingModeHighQuality);
//	if (m_displayMode)
//	{
//		graphics->TranslateTransform(m_radarCenter.X, m_radarCenter.Y);
//		graphics->RotateTransform(360 - m_MainShip.heading);
//		graphics->TranslateTransform(-m_radarCenter.X, -m_radarCenter.Y);
//	}
//	graphics->TranslateTransform(ship->posX, ship->posY);
//	graphics->RotateTransform(ship->heading);
//	graphics->TranslateTransform(-ship->posX, -ship->posY);
//
//
//
//	graphics->FillEllipse(&yellowbrush, rect);
//
//	graphics->ResetTransform();
//}

void CRadarView::DrawRadarShipOnBMP(CBitmapEx* bmp, SHIP* ship)
{
	if (ship->posX<0 || ship->posX>m_viewRect.Width() || ship->posY<0 || ship->posY>m_viewRect.Height())
		return;
	ship->widthPixel = ship->width / m_perPixelM;
	ship->lengthPixel = ship->length / m_perPixelM;
	if (ship->widthPixel < 2) ship->widthPixel = 2;
	if (ship->lengthPixel < 8)	ship->lengthPixel = 8;
	Rect rect(ship->posX - ship->widthPixel / 2, ship->posY - ship->lengthPixel / 2, ship->widthPixel, ship->lengthPixel);
	for (int y = rect.Y; y <= rect.Y + rect.Height; y++)
	{
		for (int x = rect.X; x <= rect.X + rect.Width; x++)
		{
			if (!isPointInEllipse(rect, x, y))
				continue;
			else
			{
				int a = x, b = y;
				RotatePoint(&a, &b, CPoint(ship->posX, ship->posY), _ToRad(ship->heading));
				if (m_displayMode)
				{
					RotatePoint(&a, &b, CPoint(m_radarCenter.X, m_radarCenter.Y), m_northAngle);
				}
				bmp->SetPixel(a, b, _RGB(255, 255, 0));
			}
		}
	}
}

//void CRadarView::GetOtherShipIndexFromRect(vector<int> &vec, CRect rect)
//{
//	for (int i = 0; i < (int)m_OtherShip.size(); i++)
//	{
//		int x = m_OtherShip[i].rotateX;
//		int y = m_OtherShip[i].rotateY;
//		if (PtInRect(rect, CPoint(x, y)))
//			vec.push_back(i);
//	}
//}

//int CRadarView::GetMinUnusedIndexOfARPARecord()
//{
//	int i = 0;
//	for (; i < (int)m_arpaRecord.size(); i++)
//	{
//		if (i + 1 < m_arpaRecord[i].index)
//		{
//			return (i + 1);
//		}
//	}
//	return (i + 1);
//}

BOOL CRadarView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (PtInRegion(m_radarRgn, pt.x, pt.y))
	{
		if (m_ARPARecordTargetSwitch)
		{
			if (zDelta > 0)
			{
				if (m_ARPARecordCur == GATE_SIZE_SMALL)
					m_ARPARecordCur = GATE_SIZE_MEDIUM;
				if (m_ARPARecordCur == GATE_SIZE_MEDIUM)
					m_ARPARecordCur = GATE_SIZE_LARGE;
			}
			else if (zDelta < 0)
			{
				if (m_ARPARecordCur == GATE_SIZE_LARGE)
					m_ARPARecordCur = GATE_SIZE_MEDIUM;
				if (m_ARPARecordCur == GATE_SIZE_MEDIUM)
					m_ARPARecordCur = GATE_SIZE_SMALL;
			}
		}
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

//void CRadarView::CalculateARPARecord()
//{
//	for (int i = 0; i < (int)m_arpaRecord.size(); i++)
//	{
//		AlphaBetaFilter(&m_arpaRecord[i]);
//		//α-β滤波器，计算产物为：
//		//1,center,srcX,srcY,其中(srcX,srcY)==center;
//		//2,DadiX,DadiY;
//		//3,speedX,speedY,单位m/s;
//		//4,gateRect
//
//		//当目标不是丢失状态时，才计算各参数+
//		if (m_arpaRecord[i].state != ARPA_RECORD_STATE_MISSING)
//		{
//			//经纬度
//			if (m_AISSwitch&&m_arpaRecord[i].AISship != NULL&&m_AISSwitch)
//			{
//				m_arpaRecord[i].longaitude = m_arpaRecord[i].AISship->longitude;
//				m_arpaRecord[i].latitude = m_arpaRecord[i].AISship->latitude;
//			}
//			else
//			{
//				WebMercatorProjInvcal(m_arpaRecord[i].DadiX, m_arpaRecord[i].DadiY, &(m_arpaRecord[i].longaitude), &(m_arpaRecord[i].latitude));
//			}
//			//距离(海里)
//			double  mainshipX, mainshipY, tarX, tarY;//本船和目标的大地坐标
//			WebMercatorProjCal(m_MainShip.longitude, m_MainShip.latitude, &mainshipX, &mainshipY);
//			tarX = m_arpaRecord[i].DadiX;
//			tarY = m_arpaRecord[i].DadiY;
//			double distance = sqrt((mainshipX - tarX)*(mainshipX - tarX) + (mainshipY - tarY)*(mainshipY - tarY));//米
//			m_arpaRecord[i].distance = distance * 0.00054;
//			//方位
//			CPoint p = RotatePoint(m_arpaRecord[i].scrX, m_arpaRecord[i].scrY, CPoint(m_MainShip.posX, m_MainShip.posY), 2 * PI - m_northAngle);
//			p = p - CPoint(m_MainShip.posX, m_MainShip.posY);
//			double angle = acos((double)-p.y / sqrt(p.x*p.x + p.y*p.y));
//			if (p.x < 0)
//				angle = 2 * PI - angle;
//			m_arpaRecord[i].bearing = _ToAng(angle);
//			//真速度(海里/小时)
//			if (m_AISSwitch&&m_arpaRecord[i].AISship != NULL&&m_AISSwitch)
//			{
//				m_arpaRecord[i].TSpeed = m_arpaRecord[i].AISship->speedOverGround;
//			}
//			else
//			{
//				m_arpaRecord[i].TSpeed = sqrt(m_arpaRecord[i].speedX*m_arpaRecord[i].speedX + m_arpaRecord[i].speedY*m_arpaRecord[i].speedY)*1.944;
//			}
//			//真航向
//			if (m_AISSwitch&&m_arpaRecord[i].AISship != NULL&&m_AISSwitch)
//			{
//				m_arpaRecord[i].TCourse = m_arpaRecord[i].AISship->courseOverGround;
//			}
//			else
//			{
//				if (m_arpaRecord[i].speedX != 0 || m_arpaRecord[i].speedX != 0)
//				{
//					double ang = _ToAng(acos(m_arpaRecord[i].speedY / sqrt(m_arpaRecord[i].speedX*m_arpaRecord[i].speedX + m_arpaRecord[i].speedY*m_arpaRecord[i].speedY)));
//					if (m_arpaRecord[i].speedX < 0)
//						ang = 360 - ang;
//					m_arpaRecord[i].TCourse = ang;
//				}
//				else
//					m_arpaRecord[i].TCourse = 0;
//
//			}
//			//相对速度		
//			PointF sp1, sp2, Rsp;	//海里/小时
//			sp1.X = m_arpaRecord[i].speedX*1.944;
//			sp1.Y = m_arpaRecord[i].speedY*1.944;
//			sp2.X = m_MainShip.speedOverGround*sin(_ToRad(m_MainShip.courseOverGround));
//			sp2.Y = m_MainShip.speedOverGround*cos(_ToRad(m_MainShip.courseOverGround));
//			Rsp = sp1 - sp2;
//			m_arpaRecord[i].RSpeed = sqrt(Rsp.X*Rsp.X + Rsp.Y*Rsp.Y);
//			//相对航向
//			if (Rsp.X != 0 || Rsp.Y != 0)
//			{
//				double ang = _ToAng(acos(Rsp.Y / sqrt(Rsp.X*Rsp.X + Rsp.Y*Rsp.Y)));
//				if (Rsp.X < 0)
//					ang = 360 - ang;
//				m_arpaRecord[i].RCourse = ang;
//			}
//			else
//			{
//				m_arpaRecord[i].RCourse = 0;
//			}
//
//
//			//经过位置
//			double r_mainship = m_MainShip.collisionRadius;
//			double r_target = m_arpaRecord[i].collisionRadius;
//			double r_all = r_mainship + r_target;
//			double crossX, crossY;
//			//计算当前是否碰撞
//			double dis_cur = sqrt((mainshipX - tarX)*(mainshipX - tarX) + (mainshipY - tarY)*(mainshipY - tarY));	//当前距离
//			if (dis_cur <= r_all)
//			{
//				m_arpaRecord[i].passState = ARPA_RECORD_COLLISION;
//				m_arpaRecord[i].attackTime = 0;
//				continue;
//			}
//			//采用相对速度的方法，假设本船静止
//			double main_SpeedX, main_SpeedY, tar_SpeedX, tar_SpeedY;			//单位m/s
//			double tarToMain_SpeedX, tarToMain_SpeedY;		//目标船相对于本船的速度
//			main_SpeedX = sp2.X*0.5144;
//			main_SpeedY = sp2.Y*0.5144;
//			tar_SpeedX = m_arpaRecord[i].speedX;
//			tar_SpeedY = m_arpaRecord[i].speedY;
//			tarToMain_SpeedX = tar_SpeedX - main_SpeedX;
//			tarToMain_SpeedY = tar_SpeedY - main_SpeedY;
//			if (tarToMain_SpeedX == 0 && tarToMain_SpeedY == 0)
//			{
//				m_arpaRecord[i].passState = ARPA_RECORD_NOCOLLISION;
//				m_arpaRecord[i].attackTime = -1;
//				continue;
//			}
//			//计算目标船是否靠近本船移动（相对速度矢量与两点连线夹角大于等于90°为远离，即不会碰撞）
//			double disVecX, disVecY;
//			disVecX = mainshipX - tarX;
//			disVecY = mainshipY - tarY;
//			double a = tarToMain_SpeedX*disVecX + tarToMain_SpeedY*disVecY;
//			if (a <= 0)
//			{
//				m_arpaRecord[i].passState = ARPA_RECORD_NOCOLLISION;
//				m_arpaRecord[i].attackTime = -1;
//				continue;
//			}
//			//求经过目标船以相对速度为斜率的直线l1
//			double A1, B1, C1;
//			if (tarToMain_SpeedX == 0)	//竖直
//			{
//				A1 = 1;
//				B1 = 0;
//				C1 = -tarX;
//			}
//			else if (tarToMain_SpeedY == 0)	//水平
//			{
//				A1 = 0;
//				B1 = 1;
//				C1 = -tarY;
//			}
//			else
//			{
//				A1 = 1;
//				B1 = -tarToMain_SpeedX / tarToMain_SpeedY;
//				C1 = -(A1*tarX + B1*tarY);
//			}
//			//求本船到l1的最短距离
//			double dis_Min = abs(A1*mainshipX + B1*mainshipY + C1) / sqrt(A1*A1 + B1*B1);
//			if (dis_Min <= r_all)	//两船相碰
//			{
//				//求碰撞时间
//				double m, n, dis_attack;
//				m = sqrt(dis_cur*dis_cur - dis_Min*dis_Min);
//				n = sqrt(r_all*r_all - dis_Min*dis_Min);
//				dis_attack = m - n;
//				m_arpaRecord[i].attackTime = dis_attack / sqrt(tarToMain_SpeedX*tarToMain_SpeedX + tarToMain_SpeedY*tarToMain_SpeedY);
//				m_arpaRecord[i].passState = ARPA_RECORD_COLLISION;
//				continue;
//			}
//			else		//两船不碰，求经过位置
//			{
//				//求经过本船以本船速度为斜率的直线l2
//				double A2, B2, C2;
//				if (tarToMain_SpeedX == 0)	//竖直
//				{
//					A2 = 1;
//					B2 = 0;
//					C2 = -mainshipX;
//				}
//				else if (tarToMain_SpeedY == 0)	//水平
//				{
//					A2 = 0;
//					B2 = 1;
//					C2 = -mainshipY;
//				}
//				else
//				{
//					A2 = 1;
//					B2 = -main_SpeedX / main_SpeedY;
//					C2 = -(A2*mainshipX + B2*mainshipY);
//				}
//				if (A1 == 0)
//				{
//					crossY = -C1 / B1;
//					crossX = -(B2*crossY + C2) / A2;
//				}
//				else if (B1 == 0)
//				{
//					crossX = -C1 / A1;
//					crossY = -(A2*crossX + C2) / B2;
//				}
//				else if (A2 == 0)
//				{
//					crossY = -C2 / B2;
//					crossX = -(B1*crossY + C1) / A1;
//				}
//				else if (B2 == 0)
//				{
//					crossX = -C2 / A2;
//					crossY = -(A1*crossX + C1) / B1;
//				}
//				else	//A1，A2，B1，B2都不为0
//				{
//					crossX = (C2*B1 - C1*B2) / (A1*B2 - A2*B1);
//					crossY = -(A1*crossX + C1) / B1;
//				}
//				double mainToCrossX, mainToCrossY;//本船到碰撞点的距离向量
//				mainToCrossX = crossX - mainshipX;
//				mainToCrossY = crossY - mainshipY;
//				double headingX, headingY;//本船首向方向向量
//				headingX = sin(_ToRad(m_MainShip.heading));
//				headingY = cos(_ToRad(m_MainShip.heading));
//				if ((headingX*mainToCrossX + headingY*mainToCrossY) >= 0)	//过船首
//				{
//					m_arpaRecord[i].passState = ARPA_RECORD_PASSSTEM;
//				}
//				else
//				{
//					m_arpaRecord[i].passState = ARPA_RECORD_PASSSTERN;
//				}
//			}
//		}
//	}
//}



//void CRadarView::DrawTest(const DemTools& demTools)
//{
//	if (demTools.m_pos == -1)
//		return;
//	int row = demTools.m_dem.at(demTools.m_pos)->quadTree.m_row;
//	int col = demTools.m_dem.at(demTools.m_pos)->quadTree.m_col;
//	float *data = demTools.m_dem.at(demTools.m_pos)->data;
//
//	CBitmapEx bitmap;
//	bitmap.Create(col, row);
//	bitmap.Clear();
//	for (int i = 0; i < row; i++)
//	{
//		for (int j = 0; j < col; j++)
//		{
//			int green = (double)data[i*col + j] / 100 * 255;
//			if (green>255) green = 255;
//			bitmap.SetPixel(j, i, RGB(0, green, 0));
//		}
//	}
//	bitmap.Save(_T("C:/Users/Administrator/Desktop/3.bmp"));
//}

//void CRadarView::DrawTest1()
//{
//	const float* data = demData_mem.pdemUseData;
//	int row = demData_mem.dem_row;
//	int col = demData_mem.dem_col;
//	CBitmapEx bitmap;
//	bitmap.Create(col, row);
//	bitmap.Clear();
//	double max = 0;
//	for (int i = 0; i < row; i++)
//	{
//		for (int j = 0; j < col; j++)
//		{
//			if (data[i*col+j]>max)
//				max = data[i*col + j];
//		}
//	}
//
//	for (int i = 0; i < row; i++)
//	{
//		for (int j = 0; j < col; j++)
//		{
//			int green = data[i*col + j] / max * 255;
//			if (green>255) green = 255;
//			bitmap.SetPixel(j, i, RGB(0, green, 0));
//		}
//	}
//	bitmap.Save(_T("C:/Users/Administrator/Desktop/4.bmp"));
//}

//动态数组，当屏幕比例处于一定范围时，采用插值法
//void CRadarView::FileDataToUseData_1(const DemMessage* demMessage)
//{
//	DWORD d1 = GetTickCount();
//	MapRect fileRect = MapRect(demMessage->quadTree.m_minX, demMessage->quadTree.m_minY, demMessage->quadTree.m_maxX, demMessage->quadTree.m_maxY);
//	MapRect radarRect = m_radarDadiRect;
//	double fileStepX = demMessage->quadTree.m_stepX;
//	double fileStepY = demMessage->quadTree.m_stepY;
//	int fileRow = demMessage->quadTree.m_row;
//	int fileCol = demMessage->quadTree.m_col;
//	const float *data = demMessage->data;
//	if (m_perPixelM < fileStepX)
//	{
//		//插值,像素间隔为m_perPiexlM
//		demData.dem_row = m_radiusP * 2;
//		demData.dem_col = m_radiusP * 2;
//		if (demData.pdemUseData != NULL)
//		{
//			free(demData.pdemUseData);
//			demData.pdemUseData = NULL;
//		}
//		if (NULL == (demData.pdemUseData = (float*)malloc(demData.dem_row*demData.dem_col*sizeof(float))))
//		{
//			MessageBox(L"memory malloc error!");
//			exit(-1);
//		}
//		memset(demData.pdemUseData, 0, demData.dem_row*demData.dem_col*sizeof(float));
//		for (int i = 0; i < demData.dem_row; i++)
//		{
//			for (int j = 0; j < demData.dem_col; j++)
//			{
//				double destX = radarRect.minX + j*m_perPixelM;
//				double destY = radarRect.maxY - i*m_perPixelM;
//				if (destX<fileRect.minX || destX>fileRect.maxX || destY<fileRect.minY || destY>fileRect.maxY)
//					continue;
//				//计算四个点的数组坐标
//				int leftUpPosX = (destX - fileRect.minX) / fileStepX;
//				int rightUpPosX = leftUpPosX + 1;
//				if (rightUpPosX > fileCol - 1) rightUpPosX = fileCol - 1;
//				int leftUpPosY = (fileRect.maxY - destY) / fileStepY;
//				int rightUpPosY = leftUpPosY;
//				int leftDownPosX = leftUpPosX;
//				int leftDownPosY = leftUpPosY + 1;
//				if (leftDownPosY > fileRow - 1) leftDownPosY = fileRow - 1;
//				int rightDownPosX = leftDownPosX;
//				int rightDownPosY = leftDownPosY;
//				//计算四个点的大地坐标和高度
//				double leftUpX = fileRect.minX + leftUpPosX*fileStepX;
//				double leftUpY = fileRect.maxY - leftUpPosY*fileStepY;
//				double leftUpH = data[leftUpPosY*fileCol + leftUpPosX];
//				double rightUpX = leftUpX + fileStepX;
//				double rightUpY = leftUpY;
//				double rightUpH = data[rightUpPosY*fileCol + rightUpPosX];
//				double leftDownX = leftUpX;
//				double leftDownY = leftUpY - fileStepY;
//				double leftDownH = data[leftDownPosY*fileCol + leftDownPosX];
//				double rightDownX = rightUpX;
//				double rightDownY = leftDownY;
//				double rightDownH = data[rightDownPosY*fileCol + rightDownPosX];
//				double destH = getLinearinterpolationOf4Point(leftUpX, leftUpY, leftUpH, rightUpX, rightUpY, rightUpH, leftDownX, leftDownY, leftDownH, rightDownX, rightDownY, rightDownH, destX, destY);
//				demData.pdemUseData[i*demData.dem_col + j] = destH;
//			}
//		}
//	}
//	else
//	{
//		//不插值，当当前量程大于3时，采用跳间隔取值法
//		int intervalX, intervalY;
//		if (m_range <= RANGE6)
//		{
//			intervalX = 1;
//			intervalY = 1;
//		}
//		else
//		{
//			intervalX = m_range / RANGE6;
//			intervalY = intervalX;
//		}
//		demData.dem_row = (radarRect.maxY - radarRect.minY) / (fileStepY*intervalY);
//		demData.dem_col = (radarRect.maxX - radarRect.minX) / (fileStepX*intervalX);
//		if (demData.pdemUseData != NULL)
//		{
//			free(demData.pdemUseData);
//			demData.pdemUseData = NULL;
//		}
//		if (NULL == (demData.pdemUseData = (float*)malloc(demData.dem_row*demData.dem_col*sizeof(float))))
//		{
//			MessageBox(L"memory malloc error!");
//			exit(-1);
//		}
//		memset(demData.pdemUseData, 0, demData.dem_row*demData.dem_col*sizeof(float));
//		int demStartX = 0, demSartY = 0, fileStartX = 0, fileStartY = 0;
//		if (radarRect.minX < fileRect.minX)
//		{
//			demStartX = (fileRect.minX - radarRect.minX) / (fileStepX*intervalX) + 1;
//			fileStartX = (radarRect.minX + demStartX*fileStepX*intervalX - fileRect.minX) / fileStepX + 0.5;
//			if (fileStartX < 0)
//				fileStartX = 0;
//		}
//		else
//		{
//			demStartX = 0;
//			fileStartX = (radarRect.minX - fileRect.minX) / fileStepX + 0.5;
//		}
//		if (radarRect.maxY > fileRect.maxY)
//		{
//			demSartY = (radarRect.maxY - fileRect.maxY) / (fileStepY*intervalY) + 1;
//			fileStartY = (fileRect.maxY - (radarRect.maxY - demSartY*fileStepY*intervalY)) / fileStepY + 0.5;
//			if (fileStartY < 0)
//				fileStartY = 0;
//		}
//		else
//		{
//			demSartY = 0;
//			fileStartY = (fileRect.maxY - radarRect.maxY) / fileStepY + 0.5;
//		}
//		for (int dsy = demSartY, fsy = fileStartY; dsy < demData.dem_row&&fsy < fileRow; dsy++, fsy += intervalY)
//		{
//			for (int dsx = demStartX, fsx = fileStartX; dsx < demData.dem_col&&fsx < fileCol; dsx++, fsx += intervalX)
//			{
//				demData.pdemUseData[dsy* demData.dem_col + dsx] = data[fsy*fileCol + fsx];
//			}
//		}
//	}
//	DWORD d2 = GetTickCount();
//	DWORD diff1 = d2 - d1;
//	mtx_demUseDataMem.lock();
//	if (demData_mem.pdemUseData != NULL)
//	{
//		free(demData_mem.pdemUseData);
//		demData_mem.pdemUseData = NULL;
//	}
//	demData_mem.dem_row = demData.dem_row;
//	demData_mem.dem_col = demData.dem_col;
//	if (NULL == (demData_mem.pdemUseData = (float*)malloc(demData_mem.dem_row*demData_mem.dem_col*sizeof(float))))
//	{
//		MessageBox(L"memory malloc error!");
//		exit(-1);
//	}
//	memcpy(demData_mem.pdemUseData, demData.pdemUseData, demData.dem_row*demData.dem_col*sizeof(float));
//	mtx_demUseDataMem.unlock();
//	DWORD d3 = GetTickCount();
//	DWORD diff2 = d3 - d2;
//}


//900*900数组，插值法
//void CRadarView::FileDataToUseData(const DemMessage* demMessage)
//{
//	DWORD d1 = GetTickCount();
//	memset(demUseData, 0.0f, DEMBMP_HEIGHT*DEMBMP_WIDTH*sizeof(float));
//	MapRect fileRect = MapRect(demMessage->quadTree.m_minX, demMessage->quadTree.m_minY, demMessage->quadTree.m_maxX, demMessage->quadTree.m_maxY);
//	MapRect radarRect = m_radarDadiRect;
//	double fileStepX = demMessage->quadTree.m_stepX;
//	double fileStepY = demMessage->quadTree.m_stepY;
//	int fileRow = demMessage->quadTree.m_row;
//	int fileCol = demMessage->quadTree.m_col;
//	const float *data = demMessage->data;
//	for (int i = 0; i < DEMBMP_HEIGHT; i++)
//	{
//		for (int j = 0; j < DEMBMP_WIDTH; j++)
//		{
//			double destX = radarRect.minX + j*m_perPixelM;
//			double destY = radarRect.maxY - i*m_perPixelM;
//			if (destX<fileRect.minX || destX>fileRect.maxX || destY<fileRect.minY || destY>fileRect.maxY)
//				continue;
//			//计算四个点的数组坐标
//			int leftUpPosX = (destX - fileRect.minX) / fileStepX;
//			int rightUpPosX = leftUpPosX + 1;
//			if (rightUpPosX >= fileCol - 1) rightUpPosX = fileCol - 1;
//			int leftUpPosY = (fileRect.maxY - destY) / fileStepY;
//			int rightUpPosY = leftUpPosY;
//			int leftDownPosX = leftUpPosX;
//			int leftDownPosY = leftUpPosY + 1;
//			if (leftDownPosY >= fileRow - 1) leftDownPosY = fileRow - 1;
//			int rightDownPosX = leftDownPosX;
//			int rightDownPosY = leftDownPosY;
//			//计算四个点的大地坐标和高度
//			double leftUpX = fileRect.minX + leftUpPosX*fileStepX;
//			double leftUpY = fileRect.maxY - leftUpPosY*fileStepY;
//			double leftUpH = data[leftUpPosY*fileCol + leftUpPosX];
//			double rightUpX = leftUpX + fileStepX;
//			double rightUpY = leftUpY;
//			double rightUpH = data[rightUpPosY*fileCol + rightUpPosX];
//			double leftDownX = leftUpX;
//			double leftDownY = leftUpY - fileStepY;
//			double leftDownH = data[leftDownPosY*fileCol + leftDownPosX];
//			double rightDownX = rightUpX;
//			double rightDownY = leftDownY;
//			double rightDownH = data[rightDownPosY*fileCol + rightDownPosX];
//			double destH = getLinearinterpolationOf4Point(leftUpX, leftUpY, leftUpH, rightUpX, rightUpY, rightUpH, leftDownX, leftDownY, leftDownH, rightDownX, rightDownY, rightDownH, destX, destY);
//			demUseData[i][j] = destH;
//		}
//	}
//	DWORD d2 = GetTickCount();
//	DWORD diff1 = d2 - d1;
//	mtx_demUseDataMem.lock();
//	memcpy(demUseData_mem, demUseData, DEMBMP_WIDTH*DEMBMP_HEIGHT*sizeof(float));
//	mtx_demUseDataMem.unlock();
//	DWORD d3 = GetTickCount();
//	DWORD diff2 = d3 - d2;
//}

//找出方框内物体的重心,并根据rect的尺寸返回其中心移动到计算重心处的sameSizeRect和合适目标尺寸大小的suitSizeRect
//FIND_GRAVITY_RESULT CRadarView::FindCenOfGravityInRect(CRect gateRect, CRect* sameSizeRect, CRect* suitSizeRect, int* x, int* y)
//{
//	//	CDC *pdc = GetDC();
//	//	mtx_otherShipBMP.lock();
//	CBitmapEx bmp;
//	bmp.Load(_T("../Radar/pic/otherShip_mem.bmp"));
//	//	mtx_otherShipBMP.unlock();
//
//	FIND_GRAVITY_RESULT result = RESULT_MISSING;
//	int minX = gateRect.right;
//	int minY = gateRect.bottom;
//	int maxX = gateRect.left;
//	int maxY = gateRect.top;
//	int targetSize = 0;
//	int sumLeft = 0;
//	int sumRight = 0;
//	int sumTop = 0;
//	int sumBottom = 0;
//	double sumX = 0, sumY = 0;
//	int rectSize = gateRect.Width()*gateRect.Height();
//	for (int i = gateRect.left; i <= gateRect.right; i++)
//	{
//		for (int j = gateRect.top; j <= gateRect.bottom; j++)
//		{
//			COLORREF color = bmp.GetPixel(i, j);
//			if (color != 0)
//			{
//				targetSize++;
//				if (i < minX) minX = i;
//				if (j < minY) minY = j;
//				if (i > maxX) maxX = i;
//				if (j > maxY) maxY = j;
//				if (i == gateRect.left) sumLeft++;
//				if (i == gateRect.right) sumRight++;
//				if (j == gateRect.top) sumTop++;
//				if (j == gateRect.bottom) sumBottom++;
//				sumX += i;
//				sumY += j;
//			}
//		}
//	}
//	if (targetSize != 0)
//	{
//		*x = sumX / targetSize;
//		*y = sumY / targetSize;
//		if (sumLeft >= (maxY - minY + 1) || sumTop >= (maxX - minX + 1) || sumRight >= (maxY - minY + 1) || sumBottom >= (maxX - minX + 1))
//		{
//			result = RESULT_LARGER;
//		}
//		else
//		{
//			result = RESULT_OK;
//		}
//		if (sameSizeRect != NULL)
//		{
//			sameSizeRect->left = *x - gateRect.Width() / 2;
//			sameSizeRect->right = *x + gateRect.Width() / 2;
//			sameSizeRect->top = *y - gateRect.Height() / 2;
//			sameSizeRect->bottom = *y + gateRect.Height() / 2;
//		}
//		if (suitSizeRect != NULL)
//		{
//			int width = abs(*x - minX);
//			if (abs(*x - maxX) > width) width = abs(*x - maxX);
//			if (abs(*y - minY) > width)	width = abs(*y - minY);
//			if (abs(*y - maxY) > width)	width = abs(*y - maxY);
//			width = width*1.2;
//			suitSizeRect->left = *x - width;
//			suitSizeRect->right = *x + width;
//			suitSizeRect->top = *y - width;
//			suitSizeRect->bottom = *y + width;
//		}
//	}
//	else //未检测到目标，默认目标在波门中心
//	{
//		*x = (gateRect.left + gateRect.right) / 2;
//		*y = (gateRect.top + gateRect.bottom) / 2;
//	}
//	//	ReleaseDC(pdc);
//	return result;
//}

//void CRadarView::AlphaBetaFilter(ARPA_RECORD* record)
//{
//	if (record->state != ARPA_RECORD_STATE_MISSING)
//	{
//		bool useAISmessage = false;
//		if (m_AISSwitch)	//如果AIS开启，使用AIS信息
//		{
//			//使用AIS信息条件：
//			//1.该ARPA_RECORD内保存有SHIP信息
//			if (record->AISship != NULL)
//			{
//				CPoint p(record->AISship->rotateX, record->AISship->rotateY);
//
//				//设置波门位置、大小
//				SetRectPosition(&(record->gateRect), p.x, p.y);
//				SetGateRectSize(&(record->gateRect), &(record->gateSize), GATE_SIZE_SMALL);
//				/*int length = (record->AISship->lengthPixel >= record->AISship->widthPixel) ? record->AISship->lengthPixel : record->AISship->widthPixel;
//				record->gateRect = CRect(p.x - length / 2, p.y - length / 2, p.x + length / 2, p.y + length / 2);
//				record->gateSize = GetGateRectSize(record->gateRect);*/
//				//设置当前速度为AIS船舶速度
//				record->speedX = record->AISship->speedOverGround*0.514*sin(_ToRad(record->AISship->courseOverGround));
//				record->speedY = record->AISship->speedOverGround*0.514*cos(_ToRad(record->AISship->courseOverGround));
//				//设置计算参数
//				double dadiX, dadiY;
//				GetGeodeFromScrPoint(p.x, p.y, &dadiX, &dadiY);
//				record->thisPosX = p.x;
//				record->thisPosY = p.y;
//				record->thisDadiX = dadiX;
//				record->thisDadiY = dadiY;
//
//				//计算下次预测点位置					
//				record->nextPredictDadiX = dadiX + record->speedX*RADAR_CYCLE;
//				record->nextPredictDadiY = dadiY + record->speedY*RADAR_CYCLE;
//				//下次预测位置转为屏幕坐标
//				GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//
//				//记录当前波门位置
//				record->center = GetCRectCenter(record->gateRect);
//				record->DadiX = dadiX;
//				record->DadiY = dadiY;
//				record->scrX = p.x;
//				record->scrY = p.y;
//
//				record->scanTimes = 10;
//				record->state = ARPA_RECORD_STATE_NORMAL;
//
//			}
//
//		}
//		else
//		{
//			/*int x = 564, y = 399;
//			double dadiX, dadiY;
//			GetGeodeFromScrPoint(x, y, &dadiX, &dadiY);
//			GetScrPointFromGeode(&x, &y, dadiX, dadiY);*/
//			if (record->scanTimes == 0)
//			{
//				//	记录目标的第一个位置，并矫正重心
//				CRect sameSizeRect;
//				FIND_GRAVITY_RESULT result = FindCenOfGravityInRect(record->gateRect, &sameSizeRect, NULL, &(record->lastPosX), &(record->lastPosY));
//				record->gateRect = sameSizeRect;
//				if (result == RESULT_MISSING)
//				{
//					record->state = ARPA_RECORD_STATE_MISSING;
//					return;
//				}
//				else if (result == RESULT_LARGER)
//				{
//					//调整波门为最大，并调整重心
//					SetGateRectSize(&(record->gateRect), &(record->gateSize), GATE_SIZE_LARGE);
//					CRect sameSizeRect2;
//					FindCenOfGravityInRect(record->gateRect, NULL, &sameSizeRect2, &(record->lastPosX), &(record->lastPosY));
//					record->gateRect = sameSizeRect2;
//					(record->scanTimes)++;
//				}
//				else if (result == RESULT_OK)
//				{
//					(record->scanTimes)++;
//				}
//				//将屏幕坐标转为大地坐标
//				GetGeodeFromScrPoint(record->lastPosX, record->lastPosY, &(record->lastDadiX), &(record->lastDadiY));
//				//记录当前波门位置
//				record->center = GetCRectCenter(record->gateRect);
//				record->scrX = record->center.x;
//				record->scrY = record->center.y;
//				GetGeodeFromScrPoint(record->scrX, record->scrY, &(record->DadiX), &(record->DadiY));
//
//			}
//			else if (record->scanTimes == 1)
//			{
//				//记录第二次扫描的坐标
//				CRect sameSizeRect;
//				FIND_GRAVITY_RESULT result = FindCenOfGravityInRect(record->gateRect, &sameSizeRect, NULL, &(record->thisPosX), &(record->thisPosY));
//				//矫正中心
//				record->gateRect = sameSizeRect;
//				if (result == RESULT_MISSING)
//				{
//					record->state = ARPA_RECORD_STATE_MISSING;
//					return;
//				}
//				else if (result == RESULT_LARGER)
//				{
//					//调整波门为最大,再次记录第二次扫描坐标
//					SetGateRectSize(&(record->gateRect), &(record->gateSize), GATE_SIZE_LARGE);
//					FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//				}
//				else if (result == RESULT_OK)
//				{
//
//				}
//				(record->scanTimes)++;
//				//将屏幕坐标转为大地坐标
//				GetGeodeFromScrPoint(record->thisPosX, record->thisPosY, &(record->thisDadiX), &(record->thisDadiY));
//				//计算预测速度
//				record->speedX = (record->thisDadiX - record->lastDadiX) / RADAR_CYCLE;
//				record->speedY = (record->thisDadiY - record->lastDadiY) / RADAR_CYCLE;
//				//计算第三次扫描预测位置
//				record->nextPredictDadiX = record->thisDadiX + record->speedX*RADAR_CYCLE;
//				record->nextPredictDadiY = record->thisDadiY + record->speedY*RADAR_CYCLE;
//				//预测位置转为屏幕坐标
//				GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//				//记录当前波门位置
//				record->center = GetCRectCenter(record->gateRect);
//				record->scrX = record->center.x;
//				record->scrY = record->center.y;
//				GetGeodeFromScrPoint(record->scrX, record->scrY, &(record->DadiX), &(record->DadiY));
//			}
//			else if (record->scanTimes == 2)
//			{
//				//第三次扫描，将波门移动到预测位置
//				SetRectPosition(&(record->gateRect), record->nextPredictPosX, record->nextPredictPosY);
//				//检测目标实际位置
//				CRect sameSizeRect1;
//				FIND_GRAVITY_RESULT result = FindCenOfGravityInRect(record->gateRect, &sameSizeRect1, NULL, &(record->thisPosX), &(record->thisPosY));
//				if (result == RESULT_MISSING)
//				{
//					//换大波门再次尝试
//					SetGateRectSize(&(record->gateRect), &(record->gateSize), GATE_SIZE_LARGE);
//					CRect sameSizeRect;
//					FIND_GRAVITY_RESULT result2 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect, NULL, &(record->thisPosX), &(record->thisPosY));
//					if (result2 == RESULT_MISSING)
//					{
//						record->state = ARPA_RECORD_STATE_MISSING;
//						return;
//					}
//					else if (result2 == RESULT_LARGER)
//					{
//						record->lossTimes++;
//						//能扫描到目标小部分，不妨放宽要求，移动波门位置重新计算该目标重心(可能会转移目标)
//						record->gateRect = sameSizeRect;
//						FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//					}
//					else if (result2 == RESULT_OK)
//					{
//					}
//					//将屏幕坐标转为大地坐标
//					GetGeodeFromScrPoint(record->thisPosX, record->thisPosY, &(record->thisDadiX), &(record->thisDadiY));
//					//计算平滑位置，平滑速度
//					double alpha = (2 * (2 * record->scanTimes - 1)) / (record->scanTimes*(record->scanTimes + 1));
//					double beta = (alpha*alpha) / (2 - alpha);
//					record->thisSmoothDadiX = record->nextPredictDadiX + alpha*(record->thisDadiX - record->nextPredictDadiX);
//					record->thisSmoothDadiY = record->nextPredictDadiY + alpha*(record->thisDadiY - record->nextPredictDadiY);
//					record->smoothSpeedX = record->speedX + beta*((record->thisDadiX - record->nextPredictDadiX) / RADAR_CYCLE);
//					record->smoothSpeedY = record->speedY + beta*((record->thisDadiY - record->nextPredictDadiY) / RADAR_CYCLE);
//					//计算下次预测位置
//					record->nextPredictDadiX = record->thisSmoothDadiX + record->smoothSpeedX*RADAR_CYCLE;
//					record->nextPredictDadiY = record->thisSmoothDadiY + record->smoothSpeedY*RADAR_CYCLE;
//					//预测位置转为屏幕坐标
//					GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//					//恢复变量便于下次扫描使用
//					record->speedX = record->smoothSpeedX;
//					record->speedY = record->smoothSpeedY;
//				}
//				else if (result == RESULT_LARGER)
//				{
//					//如果已是最大波门，不妨移动波门中心位置重新检测
//					if (record->gateSize == GATE_SIZE_LARGE)
//					{
//						record->gateRect = sameSizeRect1;
//						FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//					}
//					else  //如果不是最大波门，换为大波门重新检测
//					{
//						SetGateRectSize(&(record->gateRect), &(record->gateSize), GATE_SIZE_LARGE);
//						FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//					}
//					//将屏幕坐标转为大地坐标
//					GetGeodeFromScrPoint(record->thisPosX, record->thisPosY, &(record->thisDadiX), &(record->thisDadiY));
//					//计算平滑位置，平滑速度
//					double alpha = (2 * (2 * record->scanTimes - 1)) / (record->scanTimes*(record->scanTimes + 1));
//					double beta = (alpha*alpha) / (2 - alpha);
//					record->thisSmoothDadiX = record->nextPredictDadiX + alpha*(record->thisDadiX - record->nextPredictDadiX);
//					record->thisSmoothDadiY = record->nextPredictDadiY + alpha*(record->thisDadiY - record->nextPredictDadiY);
//					record->smoothSpeedX = record->speedX + beta*((record->thisDadiX - record->nextPredictDadiX) / RADAR_CYCLE);
//					record->smoothSpeedY = record->speedY + beta*((record->thisDadiY - record->nextPredictDadiY) / RADAR_CYCLE);
//					//计算下次预测位置
//					record->nextPredictDadiX = record->thisSmoothDadiX + record->smoothSpeedX*RADAR_CYCLE;
//					record->nextPredictDadiY = record->thisSmoothDadiY + record->smoothSpeedY*RADAR_CYCLE;
//					//预测位置转为屏幕坐标
//					GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//					//恢复变量便于下次扫描使用
//					record->speedX = record->smoothSpeedX;
//					record->speedY = record->smoothSpeedY;
//				}
//				else if (result == RESULT_OK)
//				{
//					//将屏幕坐标转为大地坐标
//					GetGeodeFromScrPoint(record->thisPosX, record->thisPosY, &(record->thisDadiX), &(record->thisDadiY));
//					//计算平滑位置，平滑速度
//					double alpha = (2 * (2 * record->scanTimes - 1)) / (record->scanTimes*(record->scanTimes + 1));
//					double beta = (alpha*alpha) / (2 - alpha);
//					record->thisSmoothDadiX = record->nextPredictDadiX + alpha*(record->thisDadiX - record->nextPredictDadiX);
//					record->thisSmoothDadiY = record->nextPredictDadiY + alpha*(record->thisDadiY - record->nextPredictDadiY);
//					record->smoothSpeedX = record->speedX + beta*((record->thisDadiX - record->nextPredictDadiX) / RADAR_CYCLE);
//					record->smoothSpeedY = record->speedY + beta*((record->thisDadiY - record->nextPredictDadiY) / RADAR_CYCLE);
//					//计算下次预测位置
//					record->nextPredictDadiX = record->thisSmoothDadiX + record->smoothSpeedX*RADAR_CYCLE;
//					record->nextPredictDadiY = record->thisSmoothDadiY + record->smoothSpeedY*RADAR_CYCLE;
//					//预测位置转为屏幕坐标
//					GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//					//恢复变量便于下次扫描使用
//					record->speedX = record->smoothSpeedX;
//					record->speedY = record->smoothSpeedY;
//				}
//				(record->scanTimes)++;
//				//记录当前波门位置
//				record->center = GetCRectCenter(record->gateRect);
//				record->scrX = record->center.x;
//				record->scrY = record->center.y;
//				GetGeodeFromScrPoint(record->scrX, record->scrY, &(record->DadiX), &(record->DadiY));
//			}
//			else if (record->scanTimes >= 3)
//			{
//				if (record->scanTimes >= 10)
//					record->scanTimes = 10;
//				//将波门移动到预测位置
//				SetRectPosition(&(record->gateRect), record->nextPredictPosX, record->nextPredictPosY);
//				//检测目标实际位置
//				CRect sameSizeRect, suitSizeRect;
//				FIND_GRAVITY_RESULT result = FindCenOfGravityInRect(record->gateRect, &sameSizeRect, &suitSizeRect, &(record->thisPosX), &(record->thisPosY));
//				if (result == RESULT_MISSING)
//				{
//					//假设当前为小波门，小->中
//					if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//					{
//						//如果当前能够转为中波门，转为中波门，并重新计算目标位置
//						CRect sameSizeRect2;
//						FIND_GRAVITY_RESULT result2 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect2, NULL, &(record->thisPosX), &(record->thisPosY));
//						if (result2 == RESULT_MISSING)
//						{
//							//假设当前为中波门，中->大
//							if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//							{
//								//如果当前能够转为大波门，转为大波门，并重新计算目标位置
//								CRect sameSizeRect3;
//								FIND_GRAVITY_RESULT result3 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect3, NULL, &(record->thisPosX), &(record->thisPosY));
//								if (result3 == RESULT_MISSING)//大波门仍无法发现目标，计算丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//								}
//								else if (result3 == RESULT_LARGER)	//大波门只能检测少许目标，将波门移动到目标重新计算位置，但增加丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//									record->gateRect = sameSizeRect3;
//									FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//								else if (result3 == RESULT_OK)
//								{
//									//大波门可以检测到目标，将丢失次数重置
//									record->lossTimes = 0;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//							}
//							else  //当前已是大波门，且未检测到目标
//							{
//								record->lossTimes++;
//								record->scanTimes = 3;
//								record->state = ARPA_RECORD_STATE_CALCULATING;
//								if (record->lossTimes > 3)
//								{
//									record->state = ARPA_RECORD_STATE_MISSING;
//									return;
//								}
//							}
//						}
//						else if (result2 == RESULT_LARGER)
//						{
//							//假设当前为中波门，中->大
//							if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//							{
//								//如果当前能够转为大波门，转为大波门，并重新计算目标位置
//								CRect sameSizeRect3;
//								FIND_GRAVITY_RESULT result3 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect3, NULL, &(record->thisPosX), &(record->thisPosY));
//								if (result3 == RESULT_MISSING)//大波门仍无法发现目标，计算丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//								}
//								else if (result3 == RESULT_LARGER)	//大波门只能检测少许目标，将波门移动到目标重新计算位置，但增加丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//									record->gateRect = sameSizeRect3;
//									FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//								else if (result3 == RESULT_OK)
//								{
//									//大波门可以检测到目标，将丢失次数重置
//									record->lossTimes = 0;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//							}
//							else  //当前已是大波门，且检测到少许目标
//							{
//
//								record->lossTimes++;
//								record->scanTimes = 3;
//								if (record->lossTimes > 3)
//								{
//									record->state = ARPA_RECORD_STATE_MISSING;
//									return;
//								}
//								record->gateRect = sameSizeRect2;
//								FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//								record->state = ARPA_RECORD_STATE_CALCULATING;
//							}
//						}
//						else if (result2 == RESULT_OK)
//						{
//							record->lossTimes = 0;
//							record->state = ARPA_RECORD_STATE_CALCULATING;
//						}
//					}
//					else  //当前已是大波门，且丢失目标
//					{
//						record->lossTimes++;
//						record->state = ARPA_RECORD_STATE_CALCULATING;
//						if (record->lossTimes > 3)
//						{
//							record->state = ARPA_RECORD_STATE_MISSING;
//							return;
//						}
//					}
//				}
//				else if (result == RESULT_LARGER)
//				{
//					//假设当前为小波门，小->中
//					if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//					{
//						//如果当前能够转为中波门，转为中波门，并重新计算目标位置
//						CRect sameSizeRect2;
//						FIND_GRAVITY_RESULT result2 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect2, NULL, &(record->thisPosX), &(record->thisPosY));
//						if (result2 == RESULT_MISSING)
//						{
//							//假设当前为中波门，中->大
//							if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//							{
//								//如果当前能够转为大波门，转为大波门，并重新计算目标位置
//								CRect sameSizeRect3;
//								FIND_GRAVITY_RESULT result3 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect3, NULL, &(record->thisPosX), &(record->thisPosY));
//								if (result3 == RESULT_MISSING)//大波门仍无法发现目标，计算丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//								}
//								else if (result3 == RESULT_LARGER)	//大波门只能检测少许目标，将波门移动到目标重新计算位置，但增加丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//									record->gateRect = sameSizeRect3;
//									FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//								else if (result3 == RESULT_OK)
//								{
//									//大波门可以检测到目标，将丢失次数重置
//									record->lossTimes = 0;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//							}
//							else  //当前已是大波门，且未检测到目标
//							{
//								record->lossTimes++;
//								record->scanTimes = 3;
//								record->state = ARPA_RECORD_STATE_CALCULATING;
//								if (record->lossTimes > 3)
//								{
//									record->state = ARPA_RECORD_STATE_MISSING;
//									return;
//								}
//							}
//						}
//						else if (result2 == RESULT_LARGER)
//						{
//							//假设当前为中波门，中->大
//							if (ChangeGateRectSize(&(record->gateRect), &(record->gateSize), 1))
//							{
//								//如果当前能够转为大波门，转为大波门，并重新计算目标位置
//								CRect sameSizeRect3;
//								FIND_GRAVITY_RESULT result3 = FindCenOfGravityInRect(record->gateRect, &sameSizeRect3, NULL, &(record->thisPosX), &(record->thisPosY));
//								if (result3 == RESULT_MISSING)//大波门仍无法发现目标，计算丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//								}
//								else if (result3 == RESULT_LARGER)	//大波门只能检测少许目标，将波门移动到目标重新计算位置，但增加丢失次数
//								{
//									record->lossTimes++;
//									record->scanTimes = 3;
//									if (record->lossTimes > 3)
//									{
//										record->state = ARPA_RECORD_STATE_MISSING;
//										return;
//									}
//									record->gateRect = sameSizeRect3;
//									FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//								else if (result3 == RESULT_OK)
//								{
//									//大波门可以检测到目标，将丢失次数重置
//									record->lossTimes = 0;
//									record->state = ARPA_RECORD_STATE_CALCULATING;
//								}
//							}
//							else  //当前已是大波门，且检测到少许目标
//							{
//
//								record->lossTimes++;
//								record->scanTimes = 3;
//								if (record->lossTimes > 3)
//								{
//									record->state = ARPA_RECORD_STATE_MISSING;
//									return;
//								}
//								record->gateRect = sameSizeRect2;
//								FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//								record->state = ARPA_RECORD_STATE_CALCULATING;
//							}
//						}
//						else if (result2 == RESULT_OK)
//						{
//							record->lossTimes = 0;
//							record->state = ARPA_RECORD_STATE_CALCULATING;
//						}
//					}
//					else  //当前已是大波门，且检测到少许目标
//					{
//						record->lossTimes++;
//						record->scanTimes = 3;
//						if (record->lossTimes > 3)
//						{
//							record->state = ARPA_RECORD_STATE_MISSING;
//							return;
//						}
//						record->gateRect = sameSizeRect;
//						FindCenOfGravityInRect(record->gateRect, NULL, NULL, &(record->thisPosX), &(record->thisPosY));
//						record->state = ARPA_RECORD_STATE_CALCULATING;
//					}
//				}
//				else if (result == RESULT_OK)
//				{
//					record->lossTimes = 0;
//					CRect newRect;
//					if (ChangeGateRectSize(record->gateRect, &newRect, -1))	//当前波门可变小
//					{
//						////若变小后的波门小于目标大小，则将波门变为目标大小
//						//if (GetGateRectArea(newRect) < GetGateRectArea(suitSizeRect))
//						//{
//						//	record->state = ARPA_RECORD_STATE_NORMAL;
//						//	record->gateRect = suitSizeRect;
//						//}
//						//else
//						//{
//						//变小后的波门大于目标大小，变小波门
//						ChangeGateRectSize(&(record->gateRect), &(record->gateSize), -1);
//
//						CRect newRect2;
//						if (ChangeGateRectSize(record->gateRect, &newRect2, -1))	//若当前波门还可变小
//						{
//							////若变小后的波门小于目标大小，则将波门变为目标大小
//							//if (GetGateRectArea(newRect2) < GetGateRectArea(suitSizeRect))
//							//{
//							//	record->state = ARPA_RECORD_STATE_NORMAL;
//							//	record->gateRect = suitSizeRect;
//							//}
//							//else
//							//{
//							//变小后的波门大于目标大小，变小波门
//							ChangeGateRectSize(&(record->gateRect), &(record->gateSize), -1);
//							//	}
//						}
//						else //当前已是小波门
//						{
//							record->state = ARPA_RECORD_STATE_NORMAL;
//							//	record->gateRect = suitSizeRect;
//						}
//						//	}
//
//					}
//					else //当前已是小波门
//					{
//						record->state = ARPA_RECORD_STATE_NORMAL;
//						//		record->gateRect = suitSizeRect;
//					}
//				}
//
//				//将屏幕坐标转为大地坐标
//				GetGeodeFromScrPoint(record->thisPosX, record->thisPosY, &(record->thisDadiX), &(record->thisDadiY));
//				//计算平滑位置，平滑速度
//				double alpha = ((double)(2 * (2 * record->scanTimes - 1))) / (record->scanTimes*(record->scanTimes + 1));
//				double beta = (alpha*alpha) / (2 - alpha);
//				record->thisSmoothDadiX = record->nextPredictDadiX + alpha*(record->thisDadiX - record->nextPredictDadiX);
//				record->thisSmoothDadiY = record->nextPredictDadiY + alpha*(record->thisDadiY - record->nextPredictDadiY);
//				record->smoothSpeedX = record->speedX + beta*((record->thisDadiX - record->nextPredictDadiX) / RADAR_CYCLE);
//				record->smoothSpeedY = record->speedY + beta*((record->thisDadiY - record->nextPredictDadiY) / RADAR_CYCLE);
//				//计算下次预测位置
//				record->nextPredictDadiX = record->thisSmoothDadiX + record->smoothSpeedX*RADAR_CYCLE;
//				record->nextPredictDadiY = record->thisSmoothDadiY + record->smoothSpeedY*RADAR_CYCLE;
//				//预测位置转为屏幕坐标
//				GetScrPointFromGeode(&(record->nextPredictPosX), &(record->nextPredictPosY), record->nextPredictDadiX, record->nextPredictDadiY);
//				//恢复变量便于下次扫描使用
//				record->speedX = record->smoothSpeedX;
//				record->speedY = record->smoothSpeedY;
//
//				(record->scanTimes)++;
//				//记录当前波门位置
//				record->center = GetCRectCenter(record->gateRect);
//				record->scrX = record->center.x;
//				record->scrY = record->center.y;
//				GetGeodeFromScrPoint(record->scrX, record->scrY, &(record->DadiX), &(record->DadiY));
//			}
//		}
//
//	}
//
//}

void CRadarView::SetGateRectSize(CRect* gateRect, GATE_SIZE * gateSize, GATE_SIZE size)
{
	CPoint center;
	center.x = (gateRect->left + gateRect->right) / 2;
	center.y = (gateRect->top + gateRect->bottom) / 2;
	switch (size)
	{
	case GATE_SIZE_SMALL:
	{
		gateRect->left = center.x - 10;
		gateRect->right = center.x + 10;
		gateRect->top = center.y - 10;
		gateRect->bottom = center.y + 10;
		if (gateSize != NULL)
			*gateSize = size;
		break;
	}
	case GATE_SIZE_MEDIUM:
	{
		gateRect->left = center.x - 13;
		gateRect->right = center.x + 13;
		gateRect->top = center.y - 13;
		gateRect->bottom = center.y + 13;
		if (gateSize != NULL)
			*gateSize = size;
		break;
	}
	case GATE_SIZE_LARGE:
	{
		gateRect->left = center.x - 13;
		gateRect->right = center.x + 13;
		gateRect->top = center.y - 13;
		gateRect->bottom = center.y + 13;
		if (gateSize != NULL)
			*gateSize = size;
		break;
	}
	default:
		break;
	}
}

GATE_SIZE CRadarView::GetGateRectSize(CRect gateRect)
{
	int width = gateRect.Width();
	if (width >= 32)
		return GATE_SIZE_LARGE;
	else if (width >= 26 && width < 32)
		return GATE_SIZE_MEDIUM;
	else
		return GATE_SIZE_SMALL;
}

int CRadarView::GetGateRectArea(CRect gateRect)
{
	return (gateRect.Width()*gateRect.Height());
}

//增大/减小波门，i>0时增加，i<0时减小，一次变一档
bool CRadarView::ChangeGateRectSize(CRect* gateRect, GATE_SIZE * gateSize, int i)
{
	GATE_SIZE size = GetGateRectSize(*gateRect);
	if (i > 0)
	{
		if (size == GATE_SIZE_SMALL)
		{
			SetGateRectSize(gateRect, gateSize, GATE_SIZE_MEDIUM);
			return true;
		}
		else if (size == GATE_SIZE_MEDIUM)
		{
			SetGateRectSize(gateRect, gateSize, GATE_SIZE_LARGE);
			return true;
		}
	}
	else if (i < 0)
	{
		if (size == GATE_SIZE_LARGE)
		{
			SetGateRectSize(gateRect, gateSize, GATE_SIZE_MEDIUM);
			return true;
		}
		else if (size == GATE_SIZE_MEDIUM)
		{
			SetGateRectSize(gateRect, gateSize, GATE_SIZE_SMALL);
			return true;
		}
	}
	return false;
}

//增大/减小波门，i>0时增加，i<0时减小，一次变一档
bool CRadarView::ChangeGateRectSize(CRect gateRect, CRect * newRect, int i)
{
	*newRect = gateRect;
	GATE_SIZE size = GetGateRectSize(gateRect);
	if (i > 0)
	{
		if (size == GATE_SIZE_SMALL)
		{
			SetGateRectSize(newRect, NULL, GATE_SIZE_MEDIUM);
			return true;
		}
		else if (size == GATE_SIZE_MEDIUM)
		{
			SetGateRectSize(newRect, NULL, GATE_SIZE_LARGE);
			return true;
		}
	}
	else if (i < 0)
	{
		if (size == GATE_SIZE_LARGE)
		{
			SetGateRectSize(newRect, NULL, GATE_SIZE_MEDIUM);
			return true;
		}
		else if (size == GATE_SIZE_MEDIUM)
		{
			SetGateRectSize(newRect, NULL, GATE_SIZE_SMALL);
			return true;
		}
	}
	return false;
}

//设置矩形的位置（不改变矩形的大小）
void CRadarView::SetRectPosition(CRect* gateRect, int x, int y)
{
	int width = gateRect->Width() / 2;
	int height = gateRect->Height() / 2;
	if (gateRect != NULL)
	{
		gateRect->left = x - width;
		gateRect->right = x + width;
		gateRect->top = y - height;
		gateRect->bottom = y + height;
	}

}

//void CRadarView::UpdateArpaRecordPos(double oldPerPixelM, bool displayModeChanged)
//{
//	for (int i = 0; i < (int)m_arpaRecord.size(); i++)
//	{
//		if (oldPerPixelM != 0)
//		{
//			int oldScrX = m_arpaRecord[i].scrX, oldScrY = m_arpaRecord[i].scrY, oldNextPredictPosX = m_arpaRecord[i].nextPredictPosX, oldNextPredictPosY = m_arpaRecord[i].nextPredictPosY;
//			m_arpaRecord[i].scrX = m_radarCenter.X + (oldScrX - m_radarCenter.X)*oldPerPixelM / m_perPixelM;
//			m_arpaRecord[i].scrY = m_radarCenter.Y + (oldScrY - m_radarCenter.Y)*oldPerPixelM / m_perPixelM;
//			m_arpaRecord[i].nextPredictPosX = m_MainShip.posX + (oldNextPredictPosX - m_MainShip.posX)*oldPerPixelM / m_perPixelM;
//			m_arpaRecord[i].nextPredictPosY = m_MainShip.posY + (oldNextPredictPosY - m_MainShip.posY)*oldPerPixelM / m_perPixelM;
//		}
//		if (displayModeChanged)
//		{
//			if (m_displayMode)
//			{
//				RotatePoint(&(m_arpaRecord[i].scrX), &(m_arpaRecord[i].scrY), CPoint(m_MainShip.posX, m_MainShip.posY), m_northAngle);
//				RotatePoint(&(m_arpaRecord[i].nextPredictPosX), &(m_arpaRecord[i].nextPredictPosY), CPoint(m_MainShip.posX, m_MainShip.posY), m_northAngle);
//			}
//			else
//			{
//				RotatePoint(&(m_arpaRecord[i].scrX), &(m_arpaRecord[i].scrY), CPoint(m_MainShip.posX, m_MainShip.posY), _ToRad(m_MainShip.heading));
//				RotatePoint(&(m_arpaRecord[i].nextPredictPosX), &(m_arpaRecord[i].nextPredictPosY), CPoint(m_MainShip.posX, m_MainShip.posY), _ToRad(m_MainShip.heading));
//			}
//		}
//		SetRectPosition(&(m_arpaRecord[i].gateRect), m_arpaRecord[i].scrX, m_arpaRecord[i].scrY);
//	}
//}

////检查当前雷达范围所覆盖的文件是否发生变化(跟新presentFileNames)
//bool CRadarView::checkFileNamesChanged()
//{
//	presentFileNames.clear();
//	m_quadTree.GetFileName(m_radarDadiRect, presentFileNames);
//	//排序
//	for (int i = 0; i < (int)presentFileNames.size() - 1; i++)
//	{
//		for (int j = 0; j < (int)presentFileNames.size() - i - 1; j++)
//		{
//			if (presentFileNames[j].Compare(presentFileNames[j + 1]) > 0)
//			{
//				CString flag = presentFileNames[j];
//				presentFileNames[j] = presentFileNames[j + 1];
//				presentFileNames[j + 1] = flag;
//			}
//		}
//	}
//	if ((int)previousFileNames.size() == 0 && (int)presentFileNames.size() == 0)
//	{
//		return false;
//	}
//	if ((int)previousFileNames.size() != (int)presentFileNames.size())
//	{
//		return true;
//	}
//	//对比
//	for (int i = 0; i < (int)previousFileNames.size() && i < (int)presentFileNames.size(); i++)
//	{
//		if (presentFileNames[i].Compare(previousFileNames[i]) != 0)
//			return true;
//	}
//	return false;
//}

//CString转char*
char* CRadarView::_CStringToCharArray(CString str)
{
	char *ptr;

#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	ptr = new char[len + 1];
	memset(ptr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ptr, len + 1, NULL, NULL);
#else
	ptr = new char[str.GetAllocLength() + 1];
	sprintf(ptr, _T("%s"), str);
#endif
	return ptr;
}

bool CRadarView::_CStringToCharArray(const CString cstr, char *pstr, int maxLen)
{
#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, cstr, -1, NULL, 0, NULL, NULL);
	if (maxLen >= (len + 1))
	{
		memset(pstr, 0, maxLen);
		WideCharToMultiByte(CP_ACP, 0, cstr, -1, pstr, len + 1, NULL, NULL);
	}
	else
		return false;
#else
	int len = cstr.GetAllocLength();
	if (maxLen >= (len + 1))
	{
		sprintf(pstr, _T("%s"), cstr);
	}
	else
		return false;
#endif
	return true;
}

bool CRadarView::PointInMapRect(const double longitude, const double latitude, const MapRect rect)
{
	if (longitude >= rect.minX&&longitude <= rect.maxX&&latitude >= rect.minY&&latitude <= rect.maxY)
		return true;
	return false;
}

bool CRadarView::GetIndexPath(CString &indexPath, MapRect &layerArea, const double longitude, const double latitude)
{
	ifstream mainIndex("INDEX/main.txt");
	if (!mainIndex.is_open())
		return false;
	char name[20], index[50];
	MapRect rect;
	while (1)
	{
		mainIndex >> name;
		mainIndex >> rect.minX >> rect.minY >> rect.maxX >> rect.maxY;
		mainIndex >> index;
		if (mainIndex.eof())
			break;
		if (PointInMapRect(longitude, latitude, rect))
		{
			indexPath = index;
			break;
		}
	}
	mainIndex.close();
	if (indexPath == "")
		return false;
	layerArea = rect;
	return true;
}

void CRadarView::InitEvent()
{
	closeThread_loadDemFile_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	closeThread_threadCalculateARPARecord = CreateEvent(NULL, FALSE, FALSE, NULL);
	closeThread_drawDemBMP = CreateEvent(NULL, FALSE, FALSE, NULL);
	rangeUpdate = CreateEvent(NULL, TRUE, FALSE, NULL);
}

//发送心跳包
//void CRadarView::SendHeartBeat()
//{
//	/*ShipDataService Shipdata;
//	CString mmsi;
//	mmsi.Format(L"%09d", m_MainShip.mmsi);
//	Shipdata.SetMMSI(mmsi);
//	const char* GPHBT = Shipdata.GetProtocol_HeartBeat(m_ip, L"RADAR", status);
//	int sendLen = sendto(sock_sendHeartBeat, GPHBT, strlen(GPHBT), 0, (sockaddr*)&addr_sendHeartBeat, addrLen_sendHeartBeat);
//	if (GPHBT != NULL)
//	{
//	delete GPHBT;
//	GPHBT = NULL;
//	}*/
//
//	byte heartBeatMsg[20];
//	heartBeatMsg[0] = 0x08;	//数据长低字节
//	heartBeatMsg[1] = 0x00; //数据长高字节
//	heartBeatMsg[2] = MSG_SESSION_HEARTBEAT;	//命令字
//	heartBeatMsg[3] = 0x01;	//序号低字节
//	heartBeatMsg[4] = 0x00;	//序号高字节
//	heartBeatMsg[5] = 0xff;	//版本号
//
//	DWORD crc = ShipDataService::CalculateCRC(heartBeatMsg, 6);
//	heartBeatMsg[6] = crc % 256;	//校验位低
//	heartBeatMsg[7] = crc / 256; //校验位高
//	int sendLen = sendto(sock_LOGIN, (char*)heartBeatMsg, 8, 0, (sockaddr*)&(addr_sendLOGIN), addrLen_sendLOGIN);
//}

bool CRadarView::_checkLocIP()
{
	int n_ipNum;
	_getLocalIPs(locIPs, &n_ipNum);
	for (int i = 0; i < n_ipNum; i++)
	{
		if (locIPs[i] == m_ip)
			return true;
	}
	return false;
}

void CRadarView::_initLocIP()
{
	IPConfig ipCon(L"IPconfig.txt");
	m_ip.Format(L"%s", ipCon.IP_Radar);
}

bool CRadarView::_getLocalIPs(std::vector<CString> &ips, int* cnt)
{
	//1.初始化wsa  
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return false;
	}
	//2.获取主机名  
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}
	//3.获取主机ip  
	HOSTENT* host = gethostbyname(hostname);
	if (host == NULL)
	{
		return false;
	}
	//4.清空容器
	ips.clear();
	//5.逐个转化为char*并拷贝返回  
	int i = 0;
	for (i = 0; host != NULL&&host->h_addr_list[i] != NULL; i++)
	{
		CString ip(inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
		ips.push_back(ip);
	}
	*cnt = i;
	return true;
}

//void CRadarView::SendSessionState(byte iSessionState)
//{
//	BYTE identity = SessionIdentity::SI_BRIDGE_RADAR;
//	BYTE inum = ISessionNum;
//	BYTE msg[20];
//	BYTE istate = iSessionState;
//	ShipDataService::GetNewHBT(msg, 20, identity, inum, istate);
//	int sendLen = sendto(sock_LOGIN, (char*)msg, 11, 0, (sockaddr*)&(addr_sendLOGIN), addrLen_sendLOGIN);
//}

std::string CRadarView::receiveAIS(){
	char buf[100];
	sockaddr_in remoteHostAddr;
	int addrLen = sizeof(remoteHostAddr);
	int getInfoLen;
	getInfoLen = recvfrom(m_socket, buf, 100, 0, (sockaddr*)&remoteHostAddr, &addrLen);
	if (getInfoLen <= 0){
		return std::string("");
	}
	buf[getInfoLen] = '\0';
	return std::string(buf);
}