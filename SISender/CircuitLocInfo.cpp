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

	// 21개 컬럼
	// #RelayIndex,수신기,유닛,계통,회로,입력이름,출력이름,설비명,출력설명,설비번호,입력건물,입력종류,입력계단,입력층,입력실,출력건물,출력종류,출력계단,출력층,출력실,감시화면
	
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

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 수신기 번호
		strItem.Format(_T("%02d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 유닛 번호
		strItem.Format(_T("%02d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 계통 번호
		strItem.Format(_T("%d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 회로 번호
		strItem.Format(_T("%03d"), _wtoi(strTemp));
		strCircuitNo += strItem;

		CIRCUIT_LOC_INFO cli;

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력이름
		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 출력이름

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 설비명
		strcpy(cli.circuitName, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 출력설명
		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 설비번호

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력건물
		strcpy(cli.buildingName, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력종류

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력계단
		strcpy(cli.stair, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력층
		strcpy(cli.floor, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		AfxExtractSubString(strTemp, strRow, nCol++, ',');	// 입력실
		strcpy(cli.room, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

		//나머지 컬럼은 필요없음

		m_mapCircuitLocInfo[strCircuitNo] = cli;

// 		cItem = strtok(CCommonFunc::WCharToChar(strRow.GetBuffer(0)), ",");		//#RelayIndex
// 
// 		cItem = strtok(NULL, ",");	//수신기
// 		strItem.Format(_T("%02d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//유닛
// 		strItem.Format(_T("%02d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//계통
// 		strItem.Format(_T("%d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		cItem = strtok(NULL, ",");	//회로번호
// 		strItem.Format(_T("%03d"), atoi(cItem));
// 		strCircuitNo += strItem;
// 
// 		CIRCUIT_LOC_INFO cli;
// 		
// 		cItem = strtok(NULL, " ,");	//입력이름
// 		cItem = strtok(NULL, " ,");	//출력이름
// 		cItem = strtok(NULL, " ,");	//설비명
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
