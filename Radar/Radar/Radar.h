
// Radar.h : Radar Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CRadarApp:
// �йش����ʵ�֣������ Radar.cpp
//

class CRadarApp : public CWinApp
{
public:
	CRadarApp();
	bool onClose = false;

// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CRadarApp theApp;

