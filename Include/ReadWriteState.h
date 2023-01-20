#pragma once

class CReadWriteState
{
public:
	CReadWriteState(void);
	~CReadWriteState(void);

public:
	void SetFileName(CString sFileName);

	BOOL WriteState(CString sSection, CString sKey, CString sValue);
	BOOL WriteState(CString sSection, CString sKey, int nValue);
	BOOL WriteState(CString sSection, CString sKey, float fValue);
	BOOL WriteUpdate(CString sSection, CString sKey, CString sValue);
	BOOL WriteUpdate(CString sSection, CString sKey, int nValue);

	BOOL ReadState(CString sSection, CString sKey, CString & sValue);
	BOOL ReadState(CString sSection, CString sKey, int & nValue);
	BOOL ReadState(CString sSection, CString sKey, float & fValue);
	BOOL ReadState(CString sSection, CString sKey, double & dValue);
	BOOL ReadUpdate(CString sSection, CString sKey, int & nValue);
	BOOL ReadText(CString sSection, CString sKey, CString & sValue);

protected:
	BOOL WriteValue(CString sSection, CString sKey, CString sValue);
	BOOL ReadValue(CString sSection, CString sKey, CString & sValue);
	BOOL UpdateValue(CString sSection, CString sKey, CString & sValue);

	CString m_sFileName;
};

