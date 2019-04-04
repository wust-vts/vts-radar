
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "Radar.h"
#include"PublicMethod.h"
#include "MainFrm.h"
#include"RadarView.h"
#include"login.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
END_MESSAGE_MAP()

extern bool EXIT;
// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO:  在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.style &= ~WS_BORDER;
	if (cs.hMenu != NULL)
	{
		::DestroyMenu(cs.hMenu);
		cs.hMenu = NULL;
	}
	//去掉菜单栏 
	cs.x = 0;   //改变初始位置   
	cs.y = 0;
	cs.cx = GetSystemMetrics(SM_CXSCREEN);
	cs.cy = GetSystemMetrics(SM_CYSCREEN);
	cs.style = WS_POPUP;//改变弹出风格，无标题栏

	closeWindowEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序



BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	if ((pMsg->message == WM_KEYDOWN && pMsg->wParam==VK_ESCAPE)||theApp.onClose)
	{
		
	//	SetEvent(closeWindowEvent);
	//	DWORD dwRetVal1 = WaitForSingleObject(closeThread_loadDemFile_event, INFINITE);
	////	DWORD dwRetVal2 = WaitForSingleObject(closeThread_threadCalculateARPARecord, INFINITE);
	//	SetEvent(closeWindowEvent);
	//	DWORD dwRetVal3 = WaitForSingleObject(closeThread_drawDemBMP, INFINITE);
	//	if (dwRetVal1 == WAIT_OBJECT_0&&dwRetVal3 == WAIT_OBJECT_0)
		//CRadarView* view = (CRadarView*)this->GetActiveView();
		//view->SendSessionState(SessionState::SS_DROPPED);
		exit(0);
			//PostMessage(WM_CLOSE);
			//SendMessage(WM_CLOSE);
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}



