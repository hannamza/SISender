#pragma once

#define SHARED_MEM_NAME	L"SIPROJECT_SHARED_MEM"

#pragma pack(push, 1)

typedef struct 
{
	long SMIndex;
	BYTE event[EVENT_MAX_COUNT][SI_EVENT_BUF_SIZE];
}EVENT_STRUCT;

typedef struct  
{
	bool processRun[PROCESS_TOTAL_COUNT];
 	ULONGLONG aliveCount[PROCESS_TOTAL_COUNT];
	EVENT_STRUCT es;
}SHARED_MEM;

class AFX_EXT_CLASS CSM
{
public:
	static HANDLE shm_handle;
	static SHARED_MEM* shm;

	static BOOL CreateSharedMemory();
	static void CloseSharedMemory();
	static void WriteEventToSharedMemory(BYTE* pData);
	static void WriteAliveCountToSharedMemory(int nProcessIndex, ULONGLONG nCount);
	static void WriteProcessRunToSharedMemory(int nProcessIndex, bool bRun);
	static void ReadEventBufFromSharedMemory(int nSMIndex, BYTE* pBuf, int nBufSize);
	static long ReadEventIndex();
	static ULONGLONG ReadAliveCountFromSharedMemory(int nProcessIndex);
	static bool ReadProcessRunFromSharedMemory(int nProcessIndex);
};

#pragma pack(pop, 1)