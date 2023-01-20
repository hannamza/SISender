#pragma once

#include "TSingleton.h"

class CEncDec : public TSingleton<CEncDec>
{
public:
	CEncDec();
	~CEncDec();

public:
	void PacketEncode(BYTE* pData, int nIndex, int nCount);
	void PacketDecode(BYTE* pData, int nIndex, int nCount);

private:
	void Encrypt(BYTE* pData, int nIndex, int nCount);
};

