// ziBasic.h: interface for the CziBasic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIBASIC_H__22FC26FD_9E3F_49D2_B4BA_1BFBE4DF67F5__INCLUDED_)
#define AFX_ZIBASIC_H__22FC26FD_9E3F_49D2_B4BA_1BFBE4DF67F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../define/stdafx.h"

#ifdef Q_OS_WIN32

class CziBasic  
{
public:
	CziBasic();
	virtual ~CziBasic();
	void DealData(const PBYTE InBuff,PBYTE OutBuff,DWORD dwLen);
	DWORD ReverseDWORD(DWORD dwInData);
	WORD ReverseWORD(WORD wInData);
};

#endif // !defined(AFX_ZIBASIC_H__22FC26FD_9E3F_49D2_B4BA_1BFBE4DF67F5__INCLUDED_)

#endif
