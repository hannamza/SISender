
// Broker.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CBrokerApp:
// �� Ŭ������ ������ ���ؼ��� Broker.cpp�� �����Ͻʽÿ�.
//

class CBrokerApp : public CWinApp
{
public:
	CBrokerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CBrokerApp theApp;