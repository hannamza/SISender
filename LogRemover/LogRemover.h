
// LogRemover.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CLogRemoverApp:
// �� Ŭ������ ������ ���ؼ��� LogRemover.cpp�� �����Ͻʽÿ�.
//

class CLogRemoverApp : public CWinApp
{
public:
	CLogRemoverApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CLogRemoverApp theApp;