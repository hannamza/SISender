#include "stdafx.h"
#include "SM.h"
#include "MTVERIFY.h"

HANDLE CSM::shm_handle = nullptr;
SHARED_MEM* CSM::shm = nullptr;

BOOL CSM::CreateSharedMemory()
{
	BOOL bRet = FALSE;

	SECURITY_ATTRIBUTES *pSa = nullptr;
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, true, 0, false);

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = false;

	pSa = &sa;

	shm_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, SHARED_MEM_NAME);

	BOOL bSMCreated = FALSE;

	if (shm_handle == NULL)
	{
		shm_handle = CreateFileMapping(INVALID_HANDLE_VALUE, pSa, PAGE_READWRITE, 0, sizeof(SHARED_MEM), SHARED_MEM_NAME);
		if (!shm_handle)
		{
			printf("SHARED MEMORY CREATION FAILED(shm_handle)\n");
			return bRet;
		}

		bSMCreated = TRUE;
	}

	shm = (SHARED_MEM*)MapViewOfFile(shm_handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SHARED_MEM));
	if (!shm)
	{
		printf("SHARED MEMORY CREATION FAILED(shm)\n");
		return bRet;
	}
	
	if (bSMCreated)
	{
		//공유메모리 인덱스 초기화
		CSM::shm->es.SMIndex = -1;
	}

	bRet = TRUE;

	return bRet;
}

void CSM::CloseSharedMemory()
{
	if (shm_handle != nullptr)
	{
		MTVERIFY(UnmapViewOfFile(shm));
		MTVERIFY(CloseHandle(shm_handle));
	}
}

void CSM::WriteEventToSharedMemory(BYTE* pData)
{
	if (shm->es.SMIndex == EVENT_MAX_COUNT)
		shm->es.SMIndex = 0;
	else
		CSM::shm->es.SMIndex++;

	memcpy(CSM::shm->es.event[CSM::shm->es.SMIndex], pData, 15);
}

void CSM::WriteAliveCountToSharedMemory(int nProcessIndex, ULONGLONG nCount)
{
	if ((nProcessIndex < BROKER) && (nProcessIndex >= PROCESS_TOTAL_COUNT))
		return;

	CSM::shm->aliveCount[nProcessIndex] = nCount;
}

void CSM::WriteProcessRunToSharedMemory(int nProcessIndex, bool bRun)
{
	if ((nProcessIndex < BROKER) && (nProcessIndex >= PROCESS_TOTAL_COUNT))
		return;

	CSM::shm->processRun[nProcessIndex] = bRun;
}

void CSM::ReadEventBufFromSharedMemory(int nSMIndex, BYTE* pBuf, int nBufSize)
{
	//버퍼 크기가 제대로 되어 있지 않으면 처리 않함
	if (nBufSize != SI_EVENT_BUF_SIZE)
		return;

	//인덱스 값이 잘못되어 있으면 리턴
	if (nSMIndex < 0)
		return;

	//이벤트가 없으면 리턴
	if (CSM::shm->es.SMIndex < 0)
		return;

	memset(pBuf, 0, nBufSize);
	memcpy(pBuf, CSM::shm->es.event[nSMIndex], nBufSize);
}

long CSM::ReadEventIndex()
{
	return CSM::shm->es.SMIndex;
}

ULONGLONG CSM::ReadAliveCountFromSharedMemory(int nProcessIndex)
{
	if ((nProcessIndex < BROKER) && (nProcessIndex >= PROCESS_TOTAL_COUNT))
		return 0;

	return CSM::shm->aliveCount[nProcessIndex];
}

bool CSM::ReadProcessRunFromSharedMemory(int nProcessIndex)
{
	if ((nProcessIndex < BROKER) && (nProcessIndex >= PROCESS_TOTAL_COUNT))
		return false;

	return CSM::shm->processRun[nProcessIndex];
}