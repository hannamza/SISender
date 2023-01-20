#pragma once

class CCriticalSectionEx
{
public:
	CCriticalSectionEx(VOID)
	{
		try{
			InitializeCriticalSection(&mSync);
		}catch(...) {
			return;
		}
		bEnter = FALSE;
	}

	~CCriticalSectionEx(VOID)
	{
		try{
			DeleteCriticalSection(&mSync);
		}catch(...) {
			return;
		}
	}

	inline VOID Enter(VOID)
	{
		//TRACE("++++++++++++++++++\n");
		try{
			EnterCriticalSection(&mSync);
		}catch(...) {
			return;
		}
		bEnter = TRUE;
	}

	inline VOID Leave(VOID)
	{
		//TRACE("-------------------\n");
		bEnter = FALSE;
		try{
			LeaveCriticalSection(&mSync);
		}catch(...) {
			return;
		}
	}

	BOOL bEnter;

private:
	CRITICAL_SECTION	mSync;
};