#include "stdafx.h"
#include "CircuitLocInfo.h"


CCircuitLocInfo::CCircuitLocInfo()
{
}


CCircuitLocInfo::~CCircuitLocInfo()
{
	m_mapCircuitLocInfo.clear();
}

BOOL CCircuitLocInfo::GetCircuitLocInfo()
{
	CString strFilePath = _T("");
	strFilePath.Format(_T("%s\\data\\%s"), CCommonFunc::GetProgramDir(), CIRCUIT_LOC_INFO_CSV_NAME);

// 	FILE *fp;
// 	fopen_s(&fp, (CStringA)strFilePath, "r");
// 
// 	if (NULL == fp)
// 	{
// 		Log::Trace("%s File Open Error!", CCommonFunc::WCharToChar(strFilePath.GetBuffer(0)));
// 		return FALSE;
// 	}
// 
// 	fclose(fp);

	CFile fp;
	CString strBuffer = _T("");

	if (!fp.Open(strFilePath, CFile::modeRead))
	{
		return FALSE;
	}

	ULONGLONG nFileSize = fp.GetLength();
	char* cBuffer = new char[nFileSize + 1];
	memset(cBuffer, NULL, nFileSize + 1);
	fp.Read(cBuffer, nFileSize);

	CStringArray strArr;
	CString strRow;
	char* cRow;
	BOOL bHeader = TRUE;

	cRow = strtok(cBuffer, "\r\n");
	while (cRow != NULL)
	{
		if (!bHeader)
		{
			strRow.Format(_T("%s"), CCommonFunc::CharToWCHAR(cRow));
			strArr.Add(strRow);
		}
		else
		{
			bHeader = FALSE;
		}
		
		cRow = strtok(NULL, "\r\n");
	}

	// 21�� �÷�
	// #RelayIndex,���ű�,����,����,ȸ��,�Է��̸�,����̸�,�����,��¼���,�����ȣ,�Է°ǹ�,�Է�����,�Է°��,�Է���,�Է½�,��°ǹ�,�������,��°��,�����,��½�,����ȭ��
	
	char* cItem;
	for (int i = 0; i < strArr.GetSize(); i++)
	{
		CString strCircuitNo = _T("");
		CString strItem = _T("");
		strRow = strArr.GetAt(i);

		CStringArray strArrItem;
		CString strTemp;
		int nCol = 0;
		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// #RelayIndex

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ���ű� ��ȣ
		strItem.Format(_T("%02d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ���� ��ȣ
		strItem.Format(_T("%02d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ���� ��ȣ
		strItem.Format(_T("%d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ȸ�� ��ȣ
		strItem.Format(_T("%03d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		CIRCUIT_LOC_INFO cli;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է��̸�
		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ����̸�

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �����
		strcpy(cli.circuitName, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// ��¼���
		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �����ȣ

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է°ǹ�
		strcpy(cli.buildingName, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է�����

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է°��
		strcpy(cli.stair, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է���
		strcpy(cli.floor, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// �Է½�
		strcpy(cli.room, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		//������ �÷��� �ʿ����

		m_mapCircuitLocInfo[strCircuitNo] = cli;

// 		cItem = strtok(CCommonFunc::WCharToChar(strRow.GetBuffer(0)), ",");		//#RelayIndex
// 
// 		cItem = strtok(NULL, ",");	//���ű�
// 		strItem.Format(_T("%02d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//����
// 		strItem.Format(_T("%02d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//����
// 		strItem.Format(_T("%d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//ȸ�ι�ȣ
// 		strItem.Format(_T("%03d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		CIRCUIT_LOC_INFO cli;
// 		
// 		cItem = strtok(NULL, " ,");	//�Է��̸�
// 		cItem = strtok(NULL, " ,");	//����̸�
// 		cItem = strtok(NULL, " ,");	//�����
// 		strcpy(cli.circuitName, cItem);
	}

	delete[] cBuffer;
	fp.Close();

	return TRUE;
}

CString CCircuitLocInfo::GetCircuitNo(BYTE* pData)
{
	CString strCircuitNo = _T("");

	strCircuitNo += pData[SI_EVENT_BUF_FIRE_RECEIVER_1];
	strCircuitNo += pData[SI_EVENT_BUF_FIRE_RECEIVER_2];
	strCircuitNo += pData[SI_EVENT_BUF_UNIT_1];
	strCircuitNo += pData[SI_EVENT_BUF_UNIT_2];
	strCircuitNo += pData[SI_EVENT_BUF_SYSTEM];
	strCircuitNo += pData[SI_EVENT_BUF_CIRCUIT_1];
	strCircuitNo += pData[SI_EVENT_BUF_CIRCUIT_2];
	strCircuitNo += pData[SI_EVENT_BUF_CIRCUIT_3];

	return strCircuitNo;
}

int CCircuitLocInfo::CheckFloorType(char* pFloor)
{
	char cFirstText;
	cFirstText = pFloor[0];

	if (cFirstText == 'B')
		return FLOOR_TYPE_BASEMENT;
	else if (cFirstText == 'P')
		return FLOOR_TYPE_PH;

	return FLOOR_TYPE_NORMAL;
}
