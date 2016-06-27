/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-03-26  dawang.xu  add FormatBufferString for printing buffer stream
2009-03-17	dawang.xu  include <stdio.h> for multi-projects building.
2009-02-07  dawang.xu  init first version

=============================================================================*/
#include "stdafx.h"
#include "utils.h"
#include <stdio.h>
#include <direct.h>
//#include <vector>
//using std::vector;
//#include <shlwapi.h>
//#pragma   comment(lib,   "Shlwapi")
#include "math.h"
//#include <TlHelp32.h>//for function KillModemListenerThread()
//#include "log.h"

//v3.9.0
#define		HDT_MAX_SLOT		12
#define		STAT_SLOT 			HDT_MAX_SLOT+1

char * trim(char *str)
{
	if(strlen(str) == 0)
		return "";
	 int i = 0,iLength;

	iLength = strlen(str);
	 while(str[i] == '\t' || str[i] == '\r' || str[i] == '\n' || str[i] == ' ')
	 {
		  i++;
	 }
	 if(i >0)
	 {
	 	strcpy(str,str+i);
	 }

	 while(str[strlen(str) - 1] == ' ' || str[strlen(str) - 1] == '\t' || str[strlen(str) - 1] == '\r' || str[strlen(str) - 1] == '\n')
	 {
		  str[strlen(str) - 1] = 0;
	 }
	 return str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//  	Function Name:	AddSpaceToString													//
//  	Input Value:	char *s_String			target string								//
//						int i_Len				request length								//
//  	Output Value:   None															   	//
//  	Return Value:	None																//
//		Discription:	This function add space to a string in order to make a string with	//
//						requested length													//
//////////////////////////////////////////////////////////////////////////////////////////////
void AddSpaceToString(char *s_String, int i_Len)
{
	int i, k;
	i=strlen(s_String);
	for(k=i;k<i_Len;k++)
		s_String[k]=' ';
	s_String[k]='\0';
}

//////////////////////////////////////////////////////////////////////////////////////////////
//  	Function Name:	AddStarToString													//
//  	Input Value:	char *s_String			target string								//
//						int i_Len				request length								//
//  	Output Value:   None															   	//
//  	Return Value:	None																//
//		Discription:	This function add '*' to a string in order to make a string with	//
//						requested length													//
//////////////////////////////////////////////////////////////////////////////////////////////
void AddStarToString(char *s_String, int i_Len)//add by-lsh
{
	int i, k;
	i=strlen(s_String);
	for(k=i;k<i_Len;k++)
		s_String[k]='*';
	s_String[k]='\0';
}

int ValidFolderExists(char *p_FileName)//v3.9.0
{
	char s_TempDirName[MAX_PATH+1];
	int i,i_StrLen;
	strcpy(s_TempDirName, p_FileName);
	s_TempDirName[MAX_PATH-1]=0;
	i_StrLen=strlen(s_TempDirName);
	if(s_TempDirName[i_StrLen-1]==':')
		return 0;
	for(i=i_StrLen;i>=0;i--)
	{
		if(s_TempDirName[i]=='\\')
		{
			s_TempDirName[i]=0;
			break;
		}
	}
	return 0;
}

int PortToPortNum(int Port, int &PortNum)//v3.9.0
{
	CString TemStr;
	int i=0,Tem_Diag=0;

	for (i=0; i<HDT_MAX_SLOT; i++)
	{
		TemStr.Format(_T("PORT%d"), i+1);
		Tem_Diag = GetPrivateProfileInt(TemStr, _T("DIAG"), 0, _T(".\\PortId.ini"));
		if(Tem_Diag == Port)
		{
			PortNum = i+1;
			break;
		}
	}
	return 0;
}

int GaliSNfromWIFI(char *TargetCam, char *SourceCam)
{
	strcpy(TargetCam, SourceCam);
	*(TargetCam+12) = GetCaliBit(SourceCam+8, 4);
	*(TargetCam+13) = GetCaliBit(SourceCam+4, 4);
	*(TargetCam+14) = GetCaliBit(SourceCam, 4);
	*(TargetCam+15) = 0;
	return 0;
}

int GetCaliBit(char *InData, int InDataLen)
{
	int i=0, TempInt=0;
	for(i=0; i<InDataLen; i++)
	{
		TempInt =TempInt+*(InData+i);
	}
	TempInt = TempInt%10;
	TempInt = '0' + TempInt; 
	
	return TempInt;
}

int GenSAV(char *s_IMEI, char *s_CommRef, char *s_PCBASerial, char *s_PTS, char *s_PTM, char *s_HandsetIndRef,
				char *s_OrderName, int i_OrderQty, char *s_BenchName, DWORD dwTimeDiff, int i_Result, char *s_MAC, 
				char *s_SSID, char *s_WIFIpassword, char *s_MAC2, char *s_SSID2)
{
	char s_SavFmtStr[512]={0};
	char s_OrderQty[15];
	char s_IMEI_Internal[16], s_CommRef_Internal[21],s_PTS_Internal[20], s_PTM_Internal[20],
			s_HandsetIndRef_Internal[21], s_OrderName_Internal[21], s_BenchName_Internal[21],
			s_CustDate_Internal[11], s_CustTime_Internal[9],s_CycleTime[6];
	char s_SavName[MAX_PATH];
	char s_MAC_Internal[21], s_SSID_Internal[51], s_WIFIpassword_Internal[16];
	char s_MAC_Internal2[21], s_SSID_Internal2[51];
	FILE *fh_Sav;
	
	int i_MM,i_DD,i_YY;
	int i_hh,i_mm,i_ss;
	CTime time = CTime::GetCurrentTime(); 

	i_YY = time.GetYear(); 
	i_MM = time.GetMonth();
	i_DD = time.GetDay(); 
	i_hh = time.GetHour(); ///Ð¡Ê± 
	i_mm = time.GetMinute(); ///·ÖÖÓ 
	i_ss = time.GetSecond(); ///Ãë
	
	strcpy(s_IMEI_Internal, s_IMEI);
	strcpy(s_CommRef_Internal, s_CommRef);
	strcpy(s_PTS_Internal, s_PTS);
	strncpy(s_PTM_Internal,s_PCBASerial+4,2);
	s_PTM_Internal[2] = 0;
	//strcpy(s_PTM_Internal, s_PTM);
	strcpy(s_HandsetIndRef_Internal, s_HandsetIndRef);
	strcpy(s_OrderName_Internal, s_OrderName);
	strcpy(s_BenchName_Internal, s_BenchName);
	strcpy(s_MAC_Internal, s_MAC);
	strcpy(s_SSID_Internal, s_SSID);
	strcpy(s_WIFIpassword_Internal, s_WIFIpassword);
	strcpy(s_MAC_Internal2, s_MAC2);
	strcpy(s_SSID_Internal2, s_SSID2);
	
	sprintf(s_CustDate_Internal,"%02d-%02d-%4d",i_MM,i_DD,i_YY);
	sprintf(s_CustTime_Internal,"%02d:%02d:%02d",i_hh,i_mm,i_ss);
	sprintf(s_CycleTime,"%d",dwTimeDiff);
	
	AddSpaceToString(s_IMEI_Internal,15);
	AddSpaceToString(s_CommRef_Internal, 20);
	AddSpaceToString(s_PTS_Internal,5);
	AddSpaceToString(s_PTM_Internal,5);
	AddSpaceToString(s_HandsetIndRef_Internal,20);
	AddSpaceToString(s_OrderName_Internal,20);
	AddSpaceToString(s_CycleTime,5);
	
	itoa(i_OrderQty,s_OrderQty, 10);
	AddSpaceToString(s_OrderQty,10);
	AddSpaceToString(s_BenchName_Internal,20);
	AddSpaceToString(s_CustDate_Internal, 10);
	AddSpaceToString(s_CustTime_Internal, 8);
	AddSpaceToString(s_MAC_Internal,20);
	AddSpaceToString(s_SSID_Internal,50);
	AddSpaceToString(s_WIFIpassword_Internal,15);
	AddSpaceToString(s_MAC_Internal2,20);
	AddSpaceToString(s_SSID_Internal2,50);
	
	sprintf(s_SavFmtStr, "700,%s,%s,%s,%s,%s,%s,%s,%s,%d,0,01,%s,%s,%s,%s,%s,%s,%s,%s",s_IMEI_Internal,
					s_PTS_Internal, s_PTM_Internal,s_CommRef_Internal, s_HandsetIndRef_Internal,
					s_OrderName_Internal, s_OrderQty, s_BenchName_Internal,
					i_Result, s_CycleTime, s_CustDate_Internal, s_CustTime_Internal, s_MAC_Internal, s_SSID_Internal, 
					s_WIFIpassword_Internal, s_MAC_Internal2, s_SSID_Internal2);
	
	sprintf(s_SavName, "C:\\Label\\%s.sav", s_PCBASerial);
	fh_Sav=fopen(s_SavName, "w");
	if(fh_Sav==NULL)
		return -1;
	fputs(s_SavFmtStr, fh_Sav);
	fclose(fh_Sav);

	return 0;
}

int PTSTxtFileFind(char *lpPath, char *FileName)
{
//	char lpPath[MAX_PATH];
	char szFind[MAX_PATH];
	char szFile[MAX_PATH];
	CString info;
	WIN32_FIND_DATA FindFileData;
	int iFileTotal=0;

	strcpy(szFind, lpPath);
	strcat(szFind, "\\*.txt");

	HANDLE hFind=FindFirstFile((LPCSTR)szFind,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)    return -1;    
	while(TRUE)
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{
				strcpy(szFile,lpPath);
				strcat(szFile,"\\");
				strcat(szFile,(char *)FindFileData.cFileName);
				PTSTxtFileFind(szFile, FileName);
			}
		}
		else
		{	//deal with FindFileData.cFileName
			strcpy(FileName, (char *)FindFileData.cFileName);
			iFileTotal++;
		}
		if(!FindNextFile(hFind,&FindFileData))
			break;		
	}
	if(iFileTotal != 1)
	{
		FindClose(hFind);
		return -1;
	}
	else
	{	
		FindClose(hFind);
		return 0;
	}
}