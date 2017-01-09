// ziBasic.cpp: implementation of the CziBasic class.
//
//////////////////////////////////////////////////////////////////////

#include "ziBasic.h"

#ifdef Q_OS_WIN32

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CziBasic::CziBasic()
{

}

CziBasic::~CziBasic()
{

}

void CziBasic::DealData(const PBYTE InBuff,PBYTE OutBuff,DWORD dwLen)
{
	BYTE pbBuff[16]={'\0'};
	DWORD dwTemp=dwLen/16;
	for(DWORD i=0;i<dwTemp;i++)
	{
		for(int k=0;k<16;k++)
		{
			pbBuff[k]=InBuff[k+i*16];
		}
		for(int k=0;k<16;k++)
		{
			OutBuff[k+i*16]=pbBuff[15-k];
		}
	}
}

DWORD CziBasic::ReverseDWORD(DWORD dwInData)
{
	DWORD          dwRet = 0;
	PBYTE     const     pbRet = (PBYTE)&dwRet;
	PBYTE     const     pbInData = (PBYTE)&dwInData;
	pbRet[3] = pbInData[0];
	pbRet[2] = pbInData[1];
	pbRet[1] = pbInData[2];
	pbRet[0] = pbInData[3];
	return dwRet;
}

WORD CziBasic::ReverseWORD(WORD wInData)
{
	WORD          wRet = 0;
	PBYTE     const     pbRet = (PBYTE)&wRet;
	PBYTE     const     pbInData = (PBYTE)&wInData;
	pbRet[1] = pbInData[0];
	pbRet[0] = pbInData[1];	
	return wRet;
}

#endif
