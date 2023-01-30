#include "stdafx.h"
#include "CircuitLocInfo.h"


CCircuitLocInfo::CCircuitLocInfo()
{
}


CCircuitLocInfo::~CCircuitLocInfo()
{
	m_mapCircuitLocInfo.clear();
}

BOOL CCircuitLocInfo::GetCircuitLocInfoFromCSVFile()
{
	CString strFilePath = _T("");
	strFilePath.Format(_T("%s\\data\\%s"), CCommonFunc::GetProgramDir(), CIRCUIT_LOC_INFO_CSV_NAME);

	CFile fp;
	CString strBuffer = _T("");

// 	LARGE_INTEGER startTime, endTime;
// 	QueryPerformanceCounter(&startTime);

	if (!fp.Open(strFilePath, CFile::modeRead))
	{
		return FALSE;
	}

	ULONGLONG nFileSize = fp.GetLength();
	char* cBuffer = new char[nFileSize + 1];
	memset(cBuffer, NULL, nFileSize + 1);
	fp.Read(cBuffer, nFileSize);

	fp.Close();

// 	QueryPerformanceCounter(&endTime);
// 	float nDeltaTime = 0.0;
// 	nDeltaTime = CCommonFunc::GetPreciseDeltaTime(startTime, endTime);
// 	Log::Trace("회로 위치 정보 파일 읽는데 걸린 시간 : %f", nDeltaTime);

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
	
	for (int i = 0; i < strArr.GetSize(); i++)
	{
		CString strCircuitNo = _T("");
		CString strItem = _T("");
		strRow = strArr.GetAt(i);
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
	}

	delete[] cBuffer;	

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
	char cFirstText, cSecondText;
	cFirstText = pFloor[0];
	cSecondText = pFloor[1];

	if (cFirstText == 'B')
		return FLOOR_TYPE_BASEMENT;
	else if (cFirstText == 'P')
	{
		if (cSecondText == 'I')
			return FLOOR_TYPE_PIT;
		else
			return FLOOR_TYPE_PH;
	}
	else if (cFirstText >= 0x31 && cFirstText <= 0x39)
		return FLOOR_TYPE_NORMAL;

	return FLOOR_TYPE_ETC;
}

void CCircuitLocInfo::GetLocationTxtList(CStringArray& strPathArr)
{
	CString strPath = _T("");
	strPath.Format(_T("%s\\data\\*.*"), CCommonFunc::GetProgramDir());

	CFileFind finder;
	BOOL bWorking = finder.FindFile(strPath);

	while (bWorking)
	{
		bWorking = finder.FindNextFileW();

		if (finder.IsArchived())
		{
			CString strFileName = finder.GetFileName();

			if (strFileName == _T(".") || strFileName == _T("..") || strFileName == _T("Thumbs.db"))
				continue;

			strFileName = finder.GetFileTitle();

			CString strLocationFile = CIRCUIT_LOC_INFO_TXT_NAME;
			strLocationFile = strLocationFile.Left(strLocationFile.GetLength() - 6);	//$$.Txt

			if(strLocationFile.Compare(strFileName.Left(strFileName.GetLength() - 2)) == 0)
				strPathArr.Add(strFileName);
		}
	}
}

void CCircuitLocInfo::GetCircuitLocInfoFromTxtFile(CStringArray& strPathArr)
{
	CStringArray strArr;
	for (int i = 0; i < strPathArr.GetSize(); i++)
	{
		CString strPath = _T("");
		strPath.Format(_T("%s\\data\\%s.Txt"), CCommonFunc::GetProgramDir(), strPathArr.GetAt(i));

		CFile fp;
		fp.Open(strPath, CFile::modeRead);

		ULONGLONG nFileSize = fp.GetLength();
		char* cBuffer = new char[nFileSize + 1];
		memset(cBuffer, NULL, nFileSize + 1);
		fp.Read(cBuffer, nFileSize);
		fp.Close();

		strArr.RemoveAll();

		CString strRow;
		char* cRow;
		cRow = strtok(cBuffer, "\r\n");
		while (cRow != NULL)
		{
			strRow.Format(_T("%s"), CCommonFunc::CharToWCHAR(cRow));
			strArr.Add(strRow);

			cRow = strtok(NULL, "\r\n");
		}

		for (int i = 0; i < strArr.GetSize(); i++)
		{
			CString strCircuitNo = _T("");
			CString strItem = _T("");
			strRow = strArr.GetAt(i);
			CString strTemp;
			int nCol = 0;

			AfxExtractSubString(strTemp, strRow, 0, ' ');
			strCircuitNo.Format(_T("%s%s%s%s"), strTemp.Mid(1, 2), strTemp.Mid(4, 2), strTemp.Mid(7, 1), strTemp.Mid(9, 3));	// [00-00-0-001]

			CIRCUIT_LOC_INFO cli;
			memset(cli.circuitName, NULL, 30);	// LOCATION00.Txt에는 회로이름 정보가 없음

			AfxExtractSubString(strTemp, strRow, nCol++, '.');	// 입력건물
			strTemp = strTemp.Right(strTemp.GetLength() - 14);	// [00-00-0-001] 만큼 뺀 크기
			strcpy(cli.buildingName, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

			AfxExtractSubString(strTemp, strRow, nCol++, '.');	// 입력건물 종료
			AfxExtractSubString(strTemp, strRow, nCol++, '.');	// 입력계단
			strcpy(cli.stair, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

			AfxExtractSubString(strTemp, strRow, nCol++, '.');	// 입력층
			strcpy(cli.floor, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

			AfxExtractSubString(strTemp, strRow, nCol++, '.');	// 입력실
			strcpy(cli.room, CCommonFunc::WCharToChar(strTemp.GetBuffer(0)));

			//나머지 컬럼은 필요없음

			m_mapCircuitLocInfo[strCircuitNo] = cli;
		}

		delete[] cBuffer;

	}
}
