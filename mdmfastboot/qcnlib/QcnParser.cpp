#include "StdAfx.h"
#include <string>
#include <iostream>
#include <afxwin.h>
#include <Objidl.h>
#include <comutil.h>
#include "QcnParser.h"
#include "memmgr.h"
//#include "log.h"
#include "..\utils.h"


QcnParser::QcnParser()
{
	pdata = NULL;

	//add by jie.li 2012-02-21 for LTE NV >20000
	strName1 = ""; 
	bBackup = false;
	bEfsDir = false;
	bEfsData = false;
	bProvisioning = false;
	NVdata = NULL;
	//end add
}

QcnParser::~QcnParser()
{

}

uint8* QcnParser::OpenDocument(LPCTSTR pDocName, DWORD* lens)
{
	IStorage* spRoot = NULL;
	_bstr_t bsRoot(pDocName);
	const HRESULT hr = StgOpenStorage(bsRoot, NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &spRoot);
	if (SUCCEEDED(hr))
	{
		vecEFS_Data.clear();
		vecEFS_Dir.clear();
		vecEFS_Backup.clear();

		IterateStorage(spRoot, 0, true);
		spRoot = NULL;
		*lens = m_readLens;

		MergeEFSBackup();

		return pdata;
	}
	else
	{			
		//ERROR(FILE_LINE, "Failed to open <%s>, hr = 0x%08X\n", pDocName, hr);
	}
	return NULL;
}

void QcnParser::EnumBranch(IStorage* spStorage, int indentCount, bool binDump)
{
	STATSTG stat;
	IEnumSTATSTG* spEnum=NULL;
	const HRESULT hr = spStorage->EnumElements(0, NULL, 0, &spEnum);
	
	if (SUCCEEDED(hr))
	{
		while (spEnum->Next(1, &stat, NULL) == S_OK)
		{			
			ExamineBranch(spStorage, indentCount, stat, binDump);				
			CoTaskMemFree(stat.pwcsName);			
		}
	}
	else
	{
		std::string indent(indentCount, ' ');		
		//ERROR(FILE_LINE, "%sFailed to EnumElements, hr = 0x%08X\n", indent.c_str(), hr);
	}	
}

void QcnParser::ExamineBranch(IStorage* spStorage, int indentCount, 
							  STATSTG& stat, bool binDump)
{
	std::string indent(indentCount, ' ');
	
	switch(stat.type)
	{
	case STGTY_STORAGE:
		//add by jie.li 2012-02-21 for LTE NV>20000
		{
			int tLen1 = wcslen(stat.pwcsName);
			char* pChar1 = (char *)stat.pwcsName;
			char nameBuf1[100] = {0};
			int i = 0;
			while ( i < tLen1)
			{
				nameBuf1[i] = *pChar1;
				pChar1 = pChar1 + 2;			
				i++;
			}

			strName1 = nameBuf1;	
			TRACE("pItemName:%s\n", strName1);
			if (strName1.Compare(L"EFS_Backup") == 0)
			{
				bBackup = true;
				TRACE("akskdfkasdf\n");
			}

			if (strName1.Compare(L"EFS_Dir") == 0)
			{
				bEfsDir = true;
				bEfsData = false;
			}

			if (strName1.Compare(L"EFS_Data") == 0)
			{
				bEfsData = true;
				bEfsDir = false;
			}

			if ((strName1.Compare(L"EFS_Backup") != 0) && (strName1.Compare(L"EFS_Dir") != 0)
				&& (strName1.Compare(L"EFS_Data") != 0))
			{
				bBackup = false;
				bEfsData = false;
				bEfsDir = false;
			}

			if ((strName1.Compare(L"Provisioning_Item_Files") == 0))
			{
				bProvisioning = true;
			}
		}
		//end add
		ExamineStorage(spStorage, indentCount, stat, binDump);
		break ;
	case STGTY_STREAM:		
		//INFO(FILE_LINE, "%s str <%ls> %I64d\n", indent.c_str(), stat.pwcsName, stat.cbSize);
		int tLen = wcslen(stat.pwcsName);
		char* pChar = (char *)stat.pwcsName;
		char nameBuf[100] = {0};
		int i = 0;
		while ( i < tLen)
		{
			nameBuf[i] = *pChar;
			pChar = pChar + 2;			
			i++;
		}

		CString strName = MultiStrToWideStr(nameBuf);
		if (strName1.Compare(L"NV_ITEM_ARRAY") == 0)
		{			
			if (binDump)
			{
				DumpStreamContents(spStorage, indentCount, stat.pwcsName, stat.cbSize.LowPart);
			}			
		}	

		//add by jie.li 2012-2-17 for LTE NV>20000
		if (strName1.Compare(L"Feature_Mask") != 0 && strName1.Compare(L"File_Version") != 0 && !bProvisioning)
		{
			//read file name
			if (bEfsDir)
			{
				//TRACE("*****strName = %s\n stat.cbSize = %I64d\n", strName, stat.cbSize);
				NVdata = NULL;
				DumpStreamEfsBackupContents(spStorage, indentCount, stat.pwcsName, stat.cbSize.LowPart);
				EFS_Dir eFS_Dir;
				eFS_Dir.fileName = strName;
				eFS_Dir.dirLens = m_NVreadLens;

				EFS_Dir_Data *efsDirData = NULL;
				NEW_ARRAY(efsDirData, EFS_Dir_Data, sizeof(EFS_Dir_Data));
				if (bBackup)
				{
					efsDirData = (EFS_Dir_Data*)NVdata;
				}
				else
				{
					//modify by yanbin.wan 2013-03-18 for QCN structon change
					//int tLen = strlen(NVdata);
					int tLen = stat.cbSize.LowPart;
					int i = 0;
					while ( i < tLen)
					{
						efsDirData->stream[i] = *NVdata;
						NVdata++;			
						i++;
					}
					//efsDirData->stream = NVdata;
				}
				//EFS_Dir_Data *efsDirData = (EFS_Dir_Data*)NVdata;
				if (!efsDirData)
				{
					TRACE("DumpStreamEfsBackupContents read NVData Error.\r\n");
					return;
				}

				//modify by yanbin.wan 2013-03-18 for QCN structon change
				CString strTemp = L"";
				strTemp = efsDirData->stream;

				TRACE("strTemp1 = %s", strTemp);
				
				if (strTemp.Find(L"/nv/") >= 0)
				{

					eFS_Dir.pathFileName = strTemp;
					TRACE(" strTemp2 = %s\n", strTemp);

					vecEFS_Dir.push_back(eFS_Dir);
					delete efsDirData;
				}

				/*if (strTemp.Find("/nv/item_files/rfnv/") >= 0)
				{
					int iTest = strTemp.GetLength();
					strTemp = strTemp.Mid(20);
					strTemp = strTemp.Left(8);
					eFS_Dir.pathFileName = strTemp;
					TRACE(" strTemp2 = %s\n", strTemp);

					vecEFS_Dir.push_back(eFS_Dir);
					delete efsDirData;
				}
				else
				{
					int iLen = strTemp.GetLength();
					int iFind = strTemp.ReverseFind('/');
					strTemp = strTemp.Right(iLen-iFind-1);
					eFS_Dir.pathFileName = strTemp;
					TRACE(" strTemp2 = %s\n", strTemp);

					vecEFS_Dir.push_back(eFS_Dir);
					delete efsDirData;
				}*/
				//DumpStreamContents(spStorage, indentCount, stat.pwcsName, stat.cbSize.LowPart);
			}
			//read file contents
			if (bEfsData)
			{
				TRACE("*****strName = %s\n stat.cbSize = %I64d\n", strName, stat.cbSize);
				NVdata = NULL;
				DumpStreamEfsBackupContents(spStorage, indentCount, stat.pwcsName, stat.cbSize.LowPart);
				EFS_Data eFS_Data;
				eFS_Data.fileName = strName;
				eFS_Data.dataLens = m_NVreadLens;
				if (!NVdata)
				{
					TRACE("DumpStreamEfsBackupContents read NVData Error.\r\n");
					return;
				}
				eFS_Data.NVdata = (char*)NVdata;

				bool bFirst = false;
				/*if (bBackup && (strName == "00000000"))
				{
					bFirst = true;
				}*/

				if (!bFirst)
				{
					vecEFS_Data.push_back(eFS_Data);
				}
				//DumpStreamContents(spStorage, indentCount, stat.pwcsName, stat.cbSize.LowPart);
			}
			int icount1 = vecEFS_Dir.size();
			int icount2 = vecEFS_Data.size();
			//TRACE("icount1 = %d, icount2 = %d\n", icount1, icount2);
		}
		//end add

		break ;
	}	
}

void QcnParser::IterateStorage(IStorage* spStorage, int indentCount, bool binDump)
{
	STATSTG stat;
	std::string indent(indentCount, ' ');	
	const HRESULT hr = spStorage->Stat(&stat, STATFLAG_DEFAULT);
	if (SUCCEEDED (hr))
	{		
		//INFO(FILE_LINE, "%sstg <%ls>\n", indent.c_str(), stat.pwcsName);
		CoTaskMemFree(stat.pwcsName);
		EnumBranch(spStorage, indentCount, binDump);
	}
	else
	{		
		//INFO(FILE_LINE, "%sFailed to Stat storage, hr = 0x%08X\n", indent.c_str(), hr);
	}	
}

void QcnParser::ExamineStorage(IStorage* spStorage, int indentCount, 
							   STATSTG& stat, bool binDump)
{
	IStorage* spRoot=NULL;
	const HRESULT hr = spStorage->OpenStorage(stat.pwcsName, NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &spRoot);
	if (SUCCEEDED(hr))
	{
		IterateStorage(spRoot, indentCount + 1, binDump);
	}
	else
	{
		std::string indent(indentCount, ' ');	
		//ERROR(FILE_LINE, "%sFailed to OpenStorage <%ls>, hr = 0x%08X\n",
		//	  indent.c_str(), stat.pwcsName, hr);
	}
	spRoot = NULL;	
}

void QcnParser::DumpStreamContents(IStorage* spStorage, int indentCount,
								   LPWSTR pStreamName,DWORD readSize)
{
	IStream* spData=NULL;
	std::string indent(indentCount, ' ');
	const HRESULT hr = spStorage->OpenStream(pStreamName, NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spData);
	if (SUCCEEDED(hr))
	{		
		ULONG streamRead = 0;	
		pdata = NULL;
		NEW_ARRAY(pdata,uint8,readSize);
		if (pdata == NULL)
		{				
			//ERROR(FILE_LINE, "GlobaAlloc failed!\n");			
		}
		if (spData->Read(pdata, readSize, &streamRead) == S_OK)
		{
			if (streamRead == readSize)
			{
				m_readLens = readSize;
				return;
			}
			else
			{
				//ERROR(FILE_LINE, "Read QCN length incorrect!\n");
			}			
		}	
		return;				
	}
	else
	{		
		//ERROR(FILE_LINE, "%sFailed to OpenStream <%ls>, hr = 0x%08X\n",
		//	  indent.c_str(), pStreamName, hr);		
	}
}

//add by jie.li 2012-02-21 for LTW NV >20000
void QcnParser::DumpStreamEfsBackupContents(IStorage* spStorage, int indentCount,
											LPWSTR pStreamName,DWORD readSize)
{
	IStream* spData=NULL;
	std::string indent(indentCount, ' ');
	const HRESULT hr = spStorage->OpenStream(pStreamName, NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spData);
	if (SUCCEEDED(hr))
	{		
		ULONG streamRead = 0;	
		NVdata = NULL;
		//NEW_ARRAY(NVdata,uint8,readSize);
		NEW_ARRAY(NVdata,char,readSize);
		if (NVdata == NULL)
		{				
			//ERROR(FILE_LINE, "GlobaAlloc failed!\n");			
		}
		if (spData->Read(NVdata, readSize, &streamRead) == S_OK)
		{
			if (streamRead == readSize)
			{
				m_NVreadLens = readSize;
				return;
			}
			else
			{
				//ERROR(FILE_LINE, "Read QCN length incorrect!\n");
			}			
		}	
		return;				
	}
	else
	{		
		//ERROR(FILE_LINE, "%sFailed to OpenStream <%ls>, hr = 0x%08X\n",
			//indent.c_str(), pStreamName, hr);		
	}
}
//end add

//add by jie.li 2012-02-21 for LTE NV>20000
void QcnParser::MergeEFSBackup()
{
	int iCount = vecEFS_Dir.size();
	int iCount2 = vecEFS_Data.size();

	if (iCount == 0 || iCount2 == 0)
	{
		return;
	}

	if (iCount != iCount2)
	{
		return;
	}

	vecEFS_Backup.clear();
	EFS_Backups efsBackup;
	for (int i=0; i<iCount; i++)
	{
		if (vecEFS_Dir.at(i).fileName == vecEFS_Data.at(i).fileName)
		{
			/*for (int j=0; j<rest_of_stream; j++)
			{
				efsBackup.fileName[j] = vecEFS_Dir.at(i).pathFileName[j];
			}*/
			efsBackup.fileName = vecEFS_Dir.at(i).pathFileName;
			efsBackup.NVdata = vecEFS_Data.at(i).NVdata;
			efsBackup.dataLens = vecEFS_Data.at(i).dataLens;

			vecEFS_Backup.push_back(efsBackup);
		}
	}
}
//end add