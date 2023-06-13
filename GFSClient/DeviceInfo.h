#include "stdafx.h"
#include "TSingleton.h"

#define BUFF_SIZE 4096

#pragma once
class CDeviceInfo : public TSingleton<CDeviceInfo>
{
public:
	CDeviceInfo();
	~CDeviceInfo();

private:
	CMap <CString, LPCTSTR, CString, CString> m_Info;

	void GetDeviceInfo(CString strFileName);

	CWnd* m_pWnd;

public:
	void SetDeviceInfo();
	void SetMainWnd(CWnd* pWnd);

	CString GetDeviceName(CString strAddress);
};

