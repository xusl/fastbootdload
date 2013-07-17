#include "stdafx.h"
#include "StringTable.h"
#include <fstream>

CStringTable& CStringTable::Instance()
{
	static CStringTable s_Instance;

	return s_Instance;
}

CStringTable::~CStringTable()
{

}

bool CStringTable::GetString(UINT id, CString& str)const
{
	const_iterator it = m_Map.find(id);
	if(it != m_Map.end())
	{
		str = it->second;
		return true;
	}

	return false;
}

CStringTable::CStringTable()
{

}

BOOL CStringTable::ParseTxtString(char* txtDataBuf)
{
	/*
	// text code
	char buf[2048] = {0};
	CFile sFile;
	if (!sFile.Open("D:\\Project\\ADSU\\bin\\Debug\\string_italy.txt"
		,CFile::modeRead | CFile::typeBinary, NULL))
	{
		return FALSE;
	}
	DWORD sLens = sFile.GetLength();
	sFile.Read(buf, sLens);
	sFile.Close();
	txtDataBuf = buf;
	*/	
			
	if (txtDataBuf != NULL)
	{
		char txtBuf[512] = {0};
		int i=0;
		UINT nID = 0; 

		while (txtDataBuf[0] != '\0')
		{			
			if (txtDataBuf[0] == '\r')
			{
				CString strTxtTemp = txtBuf;
				int iPos = strTxtTemp.Find("###");
				if (iPos > 0)
				{
					i = 0;
					CString strNum = strTxtTemp.Left(iPos);
					nID = atoi(strNum);
					CString strV = strTxtTemp.Right(strTxtTemp.GetLength()-iPos-3);
					m_Map[nID] = strV;					
					memset(txtBuf, 0, 512);
				}
			}
			
			if (txtDataBuf[0] == '\\' && txtDataBuf[1] == 'n')
			{				
				 txtBuf[i] = '\n';
				 i++;
				 txtDataBuf++;
				 txtDataBuf++;
				 continue;
			}

			txtBuf[i] = txtDataBuf[0];
			i++;
			txtDataBuf++;
		}		
		return TRUE;
	}	
	return FALSE;
}