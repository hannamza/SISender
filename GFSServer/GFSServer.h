
// GFSServer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CGFSServerApp:
// �� Ŭ������ ������ ���ؼ��� GFSServer.cpp�� �����Ͻʽÿ�.
//

class CGFSServerApp : public CWinApp
{
public:
	CGFSServerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CGFSServerApp theApp;