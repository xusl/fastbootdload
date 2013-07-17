// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile(CString szFileName)
{
	m_szFileName = szFileName;
}

CIniFile::~CIniFile()
{
	m_szFileName = "";
}

CString CIniFile::GetString(LPCTSTR szSection, LPCTSTR szKey, LPCSTR szDefault)
{
	CString value = "";
	char strBuf[1024] = {0};

	GetPrivateProfileString(szSection, szKey, szDefault, strBuf, 1024, m_szFileName);
	value = strBuf;

	return value;
}

int CIniFile::GetInt(LPCTSTR szSection, LPCTSTR szKey, int iDefault)
{
	return GetPrivateProfileInt(szSection, szKey, iDefault, m_szFileName);
}

BOOL CIniFile::SetString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue)
{
	return WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}

BOOL CIniFile::SetInt(LPCTSTR szSection, LPCTSTR szKey, int iValue)
{
	CString szValue;	
	szValue.Format("%d", iValue);	
	return WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}

int CIniFile::GetSections(CStringArray &arrSection)
{
	int i; 
	int iPos = 0; 
	int iMaxCount; 
	TCHAR chSectionNames[2048] = {0};
	TCHAR chSection[255] = {0};	

	GetPrivateProfileSectionNames(chSectionNames, 2048, m_szFileName); 	
	
	for (i = 0; i < 2048; i++) 
	{ 
		if (chSectionNames[i] == 0) 
			if (chSectionNames[i] == chSectionNames[i+1]) 
				break; 
	} 
	
	iMaxCount = i + 1; 
	arrSection.RemoveAll();
	
	for (i = 0; i < iMaxCount; i++) 
	{ 
		chSection[iPos++] = chSectionNames[i]; 
		if (chSectionNames[i] == 0) 
		{ 
			arrSection.Add(chSection); 
			memset(chSection, 0, 255); 
			iPos = 0; 
		} 
		
	} 
	
	return arrSection.GetSize(); 
}

int CIniFile::GetKeys(LPCTSTR szSection, CStringArray &arrKey)
{
	int i; 
	int iPos=0; 
	int iMaxCount; 
	TCHAR chKeyNames[2048]={0}; 
	TCHAR chKey[255]={0}; 
	
	GetPrivateProfileString(szSection, NULL, "", chKeyNames, 2048, m_szFileName); 	
	
	for (i = 0; i < 2048; i++) 
	{ 
		if (chKeyNames[i] == 0) 
			if (chKeyNames[i] == chKeyNames[i+1]) 
				break; 
	} 
	
	arrKey.RemoveAll();
	if (i > 0)
	{
		iMaxCount = i + 1; 
		for (i = 0; i < iMaxCount; i++) 
		{ 
			chKey[iPos++] = chKeyNames[i]; 
			if (chKeyNames[i] == 0) 
			{ 
				arrKey.Add(chKey); 
				memset(chKey, 0, 255); 
				iPos = 0; 
			} 
			
		} 
	}
	
	return arrKey.GetSize(); 
}
