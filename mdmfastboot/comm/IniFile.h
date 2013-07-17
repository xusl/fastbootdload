// IniFile.h: interface for the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__4B52FB2C_2FF8_4A67_BE09_16F9F283C3FF__INCLUDED_)
#define AFX_INIFILE_H__4B52FB2C_2FF8_4A67_BE09_16F9F283C3FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIniFile  
{
public:
	CIniFile(CString szFileName);
	virtual ~CIniFile();

private:
	CString m_szFileName;

public:
	CString	GetString(LPCTSTR szSection, LPCTSTR szKey, LPCSTR szDefault);
	int		GetInt(LPCTSTR szSection, LPCTSTR szKey, int iDefault);
	int		GetSections(CStringArray &arrSection);
	int		GetKeys(LPCTSTR szSection, CStringArray &arrKey);
	BOOL	SetString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue);
	BOOL	SetInt(LPCTSTR szSection, LPCTSTR szKey, int iValue);

};

#endif // !defined(AFX_INIFILE_H__4B52FB2C_2FF8_4A67_BE09_16F9F283C3FF__INCLUDED_)
