
// SISender.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CSISenderApp:
// �� Ŭ������ ������ ���ؼ��� SISender.cpp�� �����Ͻʽÿ�.
//

class CSISenderApp : public CWinApp
{
public:
	CSISenderApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CSISenderApp theApp;