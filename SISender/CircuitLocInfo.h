#pragma once
#include "TSingleton.h"

#define CIRCUIT_LOC_INFO_CSV_NAME	L"RVRELAYINFO.csv"
#define CIRCUIT_LOC_INFO_TXT_NAME	L"LOCATION$$.Txt"

enum {
	FLOOR_TYPE_BASEMENT,
	FLOOR_TYPE_NORMAL,
	FLOOR_TYPE_PH,
	FLOOR_TYPE_PIT,
	FLOOR_TYPE_ETC
};

typedef struct 
{
	char circuitName[30];	// 설비명
	char buildingName[30];	// 입력건물
	char stair[30];			// 입력계단
	char floor[30];			// 입력층
	char room[30];			// 입력실
}CIRCUIT_LOC_INFO;

class CCircuitLocInfo : public TSingleton<CCircuitLocInfo>
{
public:
	CCircuitLocInfo();
	~CCircuitLocInfo();

	std::map<CString, CIRCUIT_LOC_INFO> m_mapCircuitLocInfo;

	BOOL GetCircuitLocInfoFromCSVFile();
	CString GetCircuitNo(BYTE* pData);
	int CheckFloorType(char* pFloor);
	void GetLocationTxtList(CStringArray& strPathArr);
	void GetCircuitLocInfoFromTxtFile(CStringArray& strPathArr);
};

