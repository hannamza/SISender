#include "stdafx.h"
#include "ReadWriteState.h"

CReadWriteState::CReadWriteState(void)
{
}

CReadWriteState::~CReadWriteState(void)
{
}

void CReadWriteState::SetFileName(CString sFileName)
{
	m_sFileName = sFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteState::WriteValue(CString sSection, CString sKey, CString sValue)
{
	CString sPath, sFullPath;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sFullPath.Format(_T("%s\\%s"), szPath, m_sFileName);

	BOOL bResult = WritePrivateProfileString(sSection, sKey, sValue, sFullPath); 

	return bResult;
}
BOOL CReadWriteState::ReadValue(CString sSection, CString sKey, CString & sValue)
{
	CString sPath, sFullPath;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sFullPath.Format(_T("%s\\%s"), szPath, m_sFileName);

	TCHAR szValue[2048];
	DWORD nSize = GetPrivateProfileString(sSection, sKey, _T(""), szValue, 2048, sFullPath);

	if(nSize > 0)
		sValue = szValue;
	else return FALSE;

	return TRUE;
}
BOOL CReadWriteState::UpdateValue(CString sSection, CString sKey, CString & sValue)
{
	CString sFullPath;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sFullPath.Format(_T("%s\\%s"), szPath, m_sFileName);

	TCHAR szValue[2048];
	DWORD nSize = GetPrivateProfileString(sSection, sKey, _T(""), szValue, 2048, sFullPath);

	if(nSize > 0)
		sValue = szValue;
	else return FALSE;

	return TRUE;
}
BOOL CReadWriteState::WriteUpdate(CString sSection, CString sKey, CString sValue)
{
	CString sFullPath;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sFullPath.Format(_T("%s\\%s"), szPath, m_sFileName);

	BOOL bResult = WritePrivateProfileString(sSection, sKey, sValue, sFullPath); 

	return bResult;
}
BOOL CReadWriteState::WriteUpdate(CString sSection, CString sKey, int nValue)
{
	CString sValue;
	sValue.Format(_T("%d"), nValue);
	return WriteUpdate(sSection, sKey, sValue);
}
//////////////////////////////////////////////////////////////////////////////////////////////////


// WRITE /////////////////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteState::WriteState(CString sSection, CString sKey, CString sValue)
{
	return WriteValue(sSection, sKey, sValue);
}
BOOL CReadWriteState::WriteState(CString sSection, CString sKey, int nValue)
{
	CString sValue;
	sValue.Format(_T("%d"), nValue);
	return WriteValue(sSection, sKey, sValue);
}
BOOL CReadWriteState::WriteState(CString sSection, CString sKey, float fValue)
{
	CString sValue;
	sValue.Format(_T("%f"), fValue);
	return WriteValue(sSection, sKey, sValue);
}


// READ //////////////////////////////////////////////////////////////////////////////////////////
BOOL CReadWriteState::ReadState(CString sSection, CString sKey, CString & sValue)
{
	return ReadValue(sSection, sKey, sValue);
}
BOOL CReadWriteState::ReadState(CString sSection, CString sKey, int & nValue)
{
	CString sValue;
	if(ReadValue(sSection, sKey, sValue))
	{
		nValue = _ttoi(sValue.GetBuffer(0));
		sValue.ReleaseBuffer();
		return TRUE;
	}

	return FALSE;
}
BOOL CReadWriteState::ReadState(CString sSection, CString sKey, float & fValue)
{
	CString sValue;
	if(ReadValue(sSection, sKey, sValue))
	{
		fValue = (float)_ttof(sValue.GetBuffer(0));
		sValue.ReleaseBuffer();
		return TRUE;
	}

	return FALSE;
}
BOOL CReadWriteState::ReadState(CString sSection, CString sKey, double & dValue)
{
	CString sValue;
	if(ReadValue(sSection, sKey, sValue))
	{
		dValue = _ttof(sValue.GetBuffer(0));
		sValue.ReleaseBuffer();
		return TRUE;
	}

	return FALSE;
}
BOOL CReadWriteState::ReadUpdate(CString sSection, CString sKey, int & nValue)
{
	CString sValue;
	if(UpdateValue(sSection, sKey, sValue))
	{
		nValue = _ttoi(sValue.GetBuffer(0));
		sValue.ReleaseBuffer();
		return TRUE;
	}

	return FALSE;
}
BOOL CReadWriteState::ReadText(CString sSection, CString sKey, CString & sText)
{
	CString sValue;
	if(UpdateValue(sSection, sKey, sText))
	{
		return TRUE;
	}

	return FALSE;
}