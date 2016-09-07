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
#define		BD_ADDR_LEN			12
#define 	BDADDIR				"D:\\HDT\\BDAddr"

unsigned int	i_BDAddrRangeQty;
unsigned int	i_BDAddrExceptionRangeQty;
unsigned int	i_BDAddrSingleExceptionQty;
char	us_BDAddrRange[10][7];
char	us_BDAddrSingleException[10][13];
unsigned int ui_BDAddrExceptionMin[10],ui_BDAddrExceptionMax[10];
char	BTRangeLocal[20]={0};

extern char	s_NAPUAP[10][7];
extern char 	s_Order[21];
extern char s_CommercialRef[21];
extern int iGetIMEIFromDatabase;

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

int GenSAV_NEW(char *s_IMEI, char *s_PCBASerial, char *s_PTS, char *s_PTM, char *s_HandsetIndRef,
				char *s_BenchName, DWORD dwTimeDiff, int i_Result, char *s_MAC, 
				char *s_SSID, char *s_WIFIpassword, char *s_MAC2, char *s_SSID2,char *s_memo)
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
	int i_ret;
	char s_FileName[MAX_PATH];
	DWORD LastError;
	
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
	strcpy(s_CommRef_Internal, s_CommercialRef);
	strcpy(s_PTS_Internal, s_PTS);
	strncpy(s_PTM_Internal,s_PCBASerial+4,2);
	s_PTM_Internal[2] = 0;
	//strcpy(s_PTM_Internal, s_PTM);
	strcpy(s_HandsetIndRef_Internal, s_HandsetIndRef);
	strcpy(s_OrderName_Internal, s_Order);
	strcpy(s_BenchName_Internal, s_BenchName);
	strcpy(s_MAC_Internal, s_MAC);
	strcpy(s_SSID_Internal, s_SSID);
	strcpy(s_WIFIpassword_Internal, s_WIFIpassword);
	strcpy(s_MAC_Internal2, s_MAC2);
	strcpy(s_SSID_Internal2, s_SSID2);
	
	sprintf(s_CustDate_Internal,"%02d-%02d-%4d",i_MM,i_DD,i_YY);
	sprintf(s_CustTime_Internal,"%02d:%02d:%02d",i_hh,i_mm,i_ss);
	sprintf(s_CycleTime,"%d",dwTimeDiff);

#if 0	
	if(iGetIMEIFromDatabase )
	{
		i_ret	= WM_MESPlus_AddIMEIPerso((HWND)0,s_IMEI,s_PCBASerial,s_OrderName_Internal,"V1.00","","","","","","",
	                               s_PTS_Internal,s_PTM_Internal,s_HandsetIndRef,"","",s_MAC2,s_MAC,"",s_CommercialRef,"","","","","",
	                               "","","","","","",s_SSID,s_SSID2,"","",s_WIFIpassword,
	                               "","","","","","","","","","",0,"","",dwTimeDiff,
	                               "","","","","","","","","","",s_memo);
		if(i_ret)
		{
			return -1;
		}

	}	
	if(!PathFileExists((LPCSTR)"C:\\Label\\"))
	{
		CreateDirectory("C:\\Label\\", NULL);
	}
	
	sprintf(s_FileName, "C:\\Label\\%s.sav", s_PCBASerial);
	if(!WritePrivateProfileString("SavDataInfo", "HDT_SAV_VER","V1.00", s_FileName))
	{
		LastError = GetLastError();
		sprintf(s_memo,"Write SAV file: %s fail,error code = %d",s_FileName,LastError);
		return -1;
	}	
#endif
	WritePrivateProfileString("SavDataInfo", "IMEI", s_IMEI, s_FileName);
	WritePrivateProfileString("SavDataInfo", "IMEI_2", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "ORDER_ID", s_OrderName_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "PCBA_NO", s_PCBASerial, s_FileName);
	WritePrivateProfileString("SavDataInfo", "BT_ADDRESS", s_MAC2, s_FileName);
	WritePrivateProfileString("SavDataInfo", "WIFI_ADDRESS", s_MAC, s_FileName);
	WritePrivateProfileString("SavDataInfo", "CU_REF", s_CommRef_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "PTS", s_PTS_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "PTH", s_PTM_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "THREE_DS", s_HandsetIndRef_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "SIM_LOCK_FLAG", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "IMEI_3", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "IMEI_4", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "IMEI_5", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "IMEI_6", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "MAIN_CODE_NAME", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_1", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_2", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_3", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_4", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_5", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PERSO_6", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PVK_FILE_NAME", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "SUID", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "MEID_DEC", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PESN_DEC", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "PESN_HEX", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "SW_VERSION", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "WIFI_PASSWORD", s_WIFIpassword_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "SSID_1", s_SSID_Internal, s_FileName);
	WritePrivateProfileString("SavDataInfo", "SSID_2", s_SSID_Internal2, s_FileName);
	WritePrivateProfileString("SavDataInfo", "CUST_ID", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "CUST_SN", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "CUST_PCBA_NO", "", s_FileName);
	WritePrivateProfileString("SavDataInfo", "HDT_REMARK", "", s_FileName);  

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

int GetWIFIfromDatabase(int i_Slot, char *s_PCBNO, char s_WIFI[])
{
	int i_Status=0;
	int i_result=0,i=0;

//	unsigned int i_WIFIfix;
	unsigned int i_WIFIsn;
	unsigned int i_temp;
	char s_WIFIRtn[20], s_WIFIfix[7], s_WIFIsn[9], s_WIFInum[12];

#if 0	
	i_result = WM_MESPlus_GetNumCore((HWND)i_Slot,s_PCBNO,s_Order,4,s_NAPUAP[0],s_WIFIRtn);
	if(i_result)
	{
		//b_GetWIFIEnable = 0;
		AfxMessageBox("Get WIFI MAC fail!\nPls wait for all port close and restart HDT");
		i_Status = -1;
		goto END;
	}
#endif
//	strcpy(s_BTRtn,"000E1F;15669804");// for debug
	strncpy(s_WIFIfix, s_WIFIRtn, 6);
	strncpy(s_WIFIsn, s_WIFIRtn+7, 8);
	s_WIFIfix[6]=0;
	s_WIFIsn[8]=0;

	//i_WIFIfix = atoi(s_WIFIfix);
	i_WIFIsn = atoi(s_WIFIsn);
	
	strcpy(s_WIFInum, s_WIFIfix);
	for(i=0;i<6;i++)
	{
		i_temp = i_WIFIsn/pow((double)16,(5-i));
		i_WIFIsn = i_WIFIsn-pow((double)16,(5-i))*i_temp;
		if(i_temp<10 && i_temp>=0)  s_WIFInum[i+6] = i_temp+'0';
		else if(i_temp<16 && i_temp>=10)  s_WIFInum[i+6] = i_temp-10+'A';
		else  
		{
			i_Status = -1;
			goto END;
		}
	}
	s_WIFInum[12]=0;
	strcpy(s_WIFI, s_WIFInum);

	i_Status = 0;

END:
	return i_Status ;
}

int GetBDAddr(char *p_BDAddr)
{
    int i_status=0; 
    i_status=GetBDAddrFromFile(p_BDAddr);
    if(i_status)
    {
		AfxMessageBox("error00!");
//       return i_status;
		return	-1;
    }
//    return RefreshBDAddr();
	i_status=RefreshBDAddr();
    if(i_status)
    {
		AfxMessageBox("error01!");
		return	-1;
    }
	return 0;
}

int ValidCompanyID(char *p_CompanyID)
{
    unsigned int i;
    for(i=0;i<i_BDAddrRangeQty;i++)
    {
	//	AfxMessageBox(us_BDAddrRange[i]);
	//	AfxMessageBox(p_CompanyID);
       if(strncmp(us_BDAddrRange[i],p_CompanyID,BD_ADDR_LEN/2))
           continue;
       return 0;
    }
    return -1;
}


int GetBDAddrFromFile(char *p_BDAddr)
{
    unsigned int i;
    //char us_CurrBDAddr[BD_ADDR_LEN/2+1];
    char s_CurrSn[BD_ADDR_LEN/2+1],s_companyID[7];
    char s_BDRequestFileName[MAX_PATH*2];
    char s_TempPoint[20];
    //int i_TempInt;
    unsigned int ui_MinSn,ui_MaxSn,ui_CurrSn;
    CFileFind finder;
    char s_temp[10]; 

    sprintf(s_BDRequestFileName, "%s\\Request\\BDAddr.tmp", BDADDIR);  
    if(!finder.FindFile( s_BDRequestFileName))
    {
		//WARN(FILE_LINE, "s_BDRequestFileName is: %s!", s_BDRequestFileName);
       AfxMessageBox ("No BD ADDR file found");
       return -1;
    }
    
    GetPrivateProfileString("Company ID", "CompanyID", "", s_TempPoint, 8, s_BDRequestFileName);
	
    for(i=0;i<3;i++)
    {
       if(!isxdigit(s_TempPoint[2*i]))
       {
		   //WARN(FILE_LINE, "s_TempPoint is: %s!", s_TempPoint);
		   AfxMessageBox("error1!");
           goto GETERR;
       }
       if(!isxdigit(s_TempPoint[2*i+1]))
       {
		   //WARN(FILE_LINE, "s_TempPoint is: %s!", s_TempPoint);
		   AfxMessageBox("error2!");
           goto GETERR;
       }
    }
    strncpy(s_companyID,s_TempPoint,BD_ADDR_LEN/2);
    s_companyID[BD_ADDR_LEN/2]=0;
 
    if(ValidCompanyID(s_companyID))
    {
		//WARN(FILE_LINE, "s_companyID is: %s!", s_companyID);
		AfxMessageBox("error3!");
		goto GETERR;
	}
    
	ui_MinSn = GetPrivateProfileInt("Minimal SN", "Min", -1, s_BDRequestFileName);
    ui_MaxSn = GetPrivateProfileInt("Maximal SN", "Max", -1, s_BDRequestFileName);
    ui_CurrSn = GetPrivateProfileInt("Serial Number", "SN", -1, s_BDRequestFileName); 

    if((ui_CurrSn<ui_MinSn)||(ui_CurrSn>ui_MaxSn))
    {
		//WARN(FILE_LINE, "ui_CurrSn is: %d!", ui_CurrSn);
		AfxMessageBox("error4!");
       return -2;
    }    

SNCHK:
    for(i=0;i<i_BDAddrExceptionRangeQty;i++)
    {
       if((ui_CurrSn<ui_MinSn)||(ui_CurrSn>ui_MaxSn))
       {
		   //WARN(FILE_LINE, "ui_CurrSn is: %d!", ui_CurrSn);
		   AfxMessageBox("error5!");
           return -2;
       }
       while((ui_CurrSn<=ui_BDAddrExceptionMax[i])&&(ui_CurrSn>=ui_BDAddrExceptionMin[i]))
       {
           ui_CurrSn++;
       }
    }    

    if (ui_CurrSn > 16777215)
    {
		//WARN(FILE_LINE, "ui_CurrSn is: %d!", ui_CurrSn);
        AfxMessageBox("BD file error, pls update addr!");
        return -1;   
    }
       
    sprintf(s_CurrSn, "%06x", ui_CurrSn);
    s_CurrSn[BD_ADDR_LEN/2]=0;
    for(i=0;i<3;i++)
    {
       if(!isxdigit(s_CurrSn[2*i]))
	   {
		   //WARN(FILE_LINE, "s_CurrSn is: %s!", s_CurrSn);
		   AfxMessageBox("error6!");
			goto GETERR;
	   }
       if(!isxdigit(s_CurrSn[2*i+1]))
	   {
		   //WARN(FILE_LINE, "s_CurrSn is: %s!", s_CurrSn);
		   AfxMessageBox("error7!");
			goto GETERR;
	   }
    }
       
    for(i=0;i<6;i++)
    {
       p_BDAddr[i]=toupper(s_companyID[i]);
    }
    for(i=0;i<6;i++)
    {
       p_BDAddr[i+6]=toupper(s_CurrSn[i]);
    }
    p_BDAddr[12]=0;
 
    for(i=0;i<i_BDAddrSingleExceptionQty;i++)
    {
       if(!strcmp(p_BDAddr,us_BDAddrSingleException[i]))
       {
		   //WARN(FILE_LINE, "p_BDAddr/us_BDAddrSingleException[i] is: %s  %s!", p_BDAddr,us_BDAddrSingleException[i]);
		   AfxMessageBox("error8!");
           goto SNCHK;
       }
    }
    _itoa(ui_CurrSn,s_temp,10);
    if(!WritePrivateProfileString("Serial Number", "SN", s_temp, s_BDRequestFileName))
    {
		//WARN(FILE_LINE, "s_temp is: %s!", s_temp);
		AfxMessageBox("error9!");
       return -1;
    }    

    sprintf(s_BDRequestFileName, "%s\\Delivery\\BDAddr.ini", BDADDIR);      
    if(!WritePrivateProfileString("Serial Number", "SN", s_temp, s_BDRequestFileName))
    {
		//WARN(FILE_LINE, "s_temp is: %s!", s_temp);
		AfxMessageBox("error10!");
       return -1;
    }
    return 0;
    
GETERR:
    return -1;
}


int RefreshBDAddr(void)
{
    unsigned int ui_CurrSn;
    char s_BDRequestFileName[MAX_PATH*2];
    char s_temp[10];
   
    sprintf(s_BDRequestFileName, "%s\\Request\\BDAddr.tmp", BDADDIR);  
    
    ui_CurrSn = GetPrivateProfileInt("Serial Number", "SN", -1, s_BDRequestFileName);
    ui_CurrSn++;
    _itoa(ui_CurrSn,s_temp,10);
    if(!WritePrivateProfileString("Serial Number", "SN", s_temp, s_BDRequestFileName))
    {
		//WARN(FILE_LINE, "s_temp is: %s!", s_temp);
		AfxMessageBox("error11!");
       return -1;
    }
    
    sprintf(s_BDRequestFileName, "%s\\Delivery\\BDAddr.ini", BDADDIR); 
    if(!WritePrivateProfileString("Serial Number", "SN", s_temp, s_BDRequestFileName))
    {
		//WARN(FILE_LINE, "s_temp is: %s!", s_temp);
		AfxMessageBox("error12!");
       return -1;
    } 

    return 0;     
}

int ValidBDAddr(char *p_BDAddr)
{
    int i;
    unsigned int i_CurrBDAddr;
    char s_company[7],s_SN[7];  
 
    if(strlen(p_BDAddr) < 12)
       return -1;
    strncpy(s_company,p_BDAddr,6);
    s_company[6] = 0;
    
	if(ValidCompanyID(s_company))
       return -2;//goto GETADDR;
    strncpy(s_SN,p_BDAddr+6,6);
    s_SN[6] = 0;
    
	sscanf(s_SN,"%x",&i_CurrBDAddr);   
    //i_CurrBDAddr = atoi(s_SN); 
    for(i=0;i<i_BDAddrExceptionRangeQty;i++)
    {
       if((i_CurrBDAddr<=ui_BDAddrExceptionMax[i])&&(i_CurrBDAddr>=ui_BDAddrExceptionMin[i]))
           return -2;//goto GETADDR;
    }       
    for(i=0;i<i_BDAddrSingleExceptionQty;i++)
    {
       if(!strcmp(us_BDAddrSingleException[i],p_BDAddr))
       {
           return -2;//goto GETADDR;
       }
    }   
    return 0;
}

int LoadFactoryBDRange(void)
{
    char s_BDRangeFileName[MAX_PATH*2];
    char s_BDAddrFileName[MAX_PATH*2];
    char s_BDAddrTmp[MAX_PATH*2];
    char s_ItemName[MAX_PATH*2];
    char s_TempPoint[20];
    char us_CurrBDAddrRange[BD_ADDR_LEN/2+1];
    char us_CurrBDAddr[BD_ADDR_LEN+1];
    int i_TempInt=0;
    int i,j;
    unsigned int ui_MinSn,ui_MaxSn,ui_CurrSn;
    int status = 0;
    char s_temp[10];
    CFileFind finder;

    sprintf(s_BDRangeFileName, "%s\\BDRange.ini", BDADDIR);  

    if(!finder.FindFile( s_BDRangeFileName))
    {
       AfxMessageBox ("No BDRange file found");
       return -1;
    } 

    i_BDAddrRangeQty = GetPrivateProfileInt("BDRANGEQTY", "QTY", -1, s_BDRangeFileName);
    for(i=0;i<i_BDAddrRangeQty;i++)
    {
       sprintf(s_ItemName, "RANGE%d", i);
       GetPrivateProfileString("BDRANGE", s_ItemName, "", s_TempPoint, 8, s_BDRangeFileName);
       for(j=0;j<3;j++)
       {
			if(!isxdigit(s_TempPoint[2*j]))
				goto	ERR;
			if(!isxdigit(s_TempPoint[2*j+1]))
				goto	ERR;
       }
       strncpy(us_BDAddrRange[i],s_TempPoint,BD_ADDR_LEN/2);
       us_BDAddrRange[i][BD_ADDR_LEN/2]=0;
    } 

    i_BDAddrExceptionRangeQty = GetPrivateProfileInt("EXCEPTIONRANGEQTY", "QTY", -1, s_BDRangeFileName);
    for(i=0;i<i_BDAddrExceptionRangeQty;i++)
    {
       sprintf(s_ItemName, "RANGE%dMIN", i);
       ui_BDAddrExceptionMin[i] = GetPrivateProfileInt("EXCEPTIONRANGE", s_ItemName, -1, s_BDRangeFileName);
       if((ui_BDAddrExceptionMin[i]>0xFFFFFF))
			goto ERR;
       sprintf(s_ItemName, "RANGE%dMAX", i);
       ui_BDAddrExceptionMax[i] = GetPrivateProfileInt("EXCEPTIONRANGE", s_ItemName, -1, s_BDRangeFileName);
       if((ui_BDAddrExceptionMax[i]>0xFFFFFF))
		   goto ERR;
    }

    i_BDAddrSingleExceptionQty = GetPrivateProfileInt("SINGLEEXCEPTIONQTY", "QTY", -1, s_BDRangeFileName);
    for(i=0;i<i_BDAddrSingleExceptionQty;i++)
    {
       sprintf(s_ItemName, "BDADDR%d", i);
       GetPrivateProfileString("SINGLEEXCEPTION", s_ItemName, "", s_TempPoint, 15, s_BDRangeFileName);
       strncpy(us_BDAddrSingleException[i],s_TempPoint,BD_ADDR_LEN);
       us_BDAddrSingleException[i][BD_ADDR_LEN]=0;
    }    

    sprintf(s_BDAddrFileName, "%s\\Delivery\\BDAddr.ini", BDADDIR);  
    if(!finder.FindFile( s_BDAddrFileName))
    {
       AfxMessageBox ("No BD ADDR file found");
       return -1;
    }
    //RET(Ini_GetInt (ih_BDRangeFile, "TAC", "TAC"))
    GetPrivateProfileString("Company ID", "CompanyID", "", s_TempPoint, 8, s_BDAddrFileName);
	strcpy(BTRangeLocal, s_TempPoint);//add by-lsh
	strcat(BTRangeLocal, "-Local");
	strcpy(s_NAPUAP[0], s_TempPoint);
    for(j=0;j<3;j++)
    {
		if(!isxdigit(s_TempPoint[2*j]))
	   		goto	ERR;
		if(!isxdigit(s_TempPoint[2*j+1]))
	   		goto	ERR;
    }
    strncpy(us_CurrBDAddrRange,s_TempPoint,BD_ADDR_LEN/2);
    us_CurrBDAddrRange[BD_ADDR_LEN/2]=0;
    if(ValidCompanyID(us_CurrBDAddrRange))
    {
       i_BDAddrRangeQty++;
       _itoa(i_BDAddrRangeQty,s_temp,10);
       WritePrivateProfileString("BDRANGEQTY", "QTY", s_temp, s_BDRangeFileName);
       sprintf(s_ItemName, "RANGE%d", i_BDAddrRangeQty);
       WritePrivateProfileString("BDRANGE", s_ItemName, s_TempPoint, s_BDRangeFileName);
    }
    ui_MinSn = GetPrivateProfileInt("Minimal SN", "Min", -1, s_BDAddrFileName);
    ui_MaxSn = GetPrivateProfileInt("Minimal SN", "Max", -1, s_BDAddrFileName);
    ui_CurrSn = GetPrivateProfileInt("Serial Number", "SN", -1, s_BDAddrFileName);
    if((ui_CurrSn<ui_MinSn)||(ui_CurrSn>ui_MaxSn))
    {
       return -2;
    } 

    sprintf(s_BDAddrTmp, "%s\\Request\\BDAddr.tmp", BDADDIR);  
    if (!CopyFile(s_BDAddrFileName,s_BDAddrTmp,FALSE)) 
    {
       AfxMessageBox("BD file error, pls restart HDT!");
       return -1;
    }    

    status=GetBDAddrFromFile(us_CurrBDAddr);
    if(status)
    {
       return status;
    }
    status=ValidBDAddr(us_CurrBDAddr);
    if(status)
    {
       return status;
    }
    return 0;
ERR:
    return -1;
}
