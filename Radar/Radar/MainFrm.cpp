
// MainFrm.cpp : CMainFrame ���ʵ��
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
// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	// TODO:  �ڴ���ӳ�Ա��ʼ������
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.style &= ~WS_BORDER;
	if (cs.hMenu != NULL)
	{
		::DestroyMenu(cs.hMenu);
		cs.hMenu = NULL;
	}
	//ȥ���˵��� 
	cs.x = 0;   //�ı��ʼλ��   
	cs.y = 0;
	cs.cx = GetSystemMetrics(SM_CXSCREEN);
	cs.cy = GetSystemMetrics(SM_CYSCREEN);
	cs.style = WS_POPUP;//�ı䵯������ޱ�����

	closeWindowEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	return TRUE;
}

// CMainFrame ���

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


// CMainFrame ��Ϣ�������



BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  �ڴ����ר�ô����/����û���
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



