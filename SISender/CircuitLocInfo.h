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
	char circuitName[30];	// �����
	char buildingName[30];	// �Է°ǹ�
	char stair[30];			// �Է°��
	char floor[30];			// �Է���
	char room[30];			// �Է½�
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

