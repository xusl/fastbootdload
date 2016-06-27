#include "StdAfx.h"
#include "Wifi_Connect.h"

wchar_t* CharToWchar(const char* c)  
{  
	int len = MultiByteToWideChar(CP_ACP,0,c,strlen(c),NULL,0);  
	wchar_t *m_wchar=new wchar_t[len+1];  
	MultiByteToWideChar(CP_ACP,0,c,strlen(c),m_wchar,len);  
	m_wchar[len]='\0';  
	return m_wchar;  
}  


Wifi_Connect::Wifi_Connect(void)
{
}


Wifi_Connect::~Wifi_Connect(void)
{
}


int Wifi_Connect::listenStatus()
{
	HANDLE hClient=NULL;
	DWORD dwMaxClient=2;
	DWORD dwCurVersion=0;
	DWORD dwResult=0;
	DWORD dwRetVal=0;
	int iRet=0;
	WCHAR GuidString[39]={0};
	while(1)
	{
		Sleep(5000);
		PWLAN_INTERFACE_INFO_LIST pIfList=NULL;
		PWLAN_INTERFACE_INFO pIfInfo=NULL;
		DWORD dwFlags=0;
		dwResult=WlanOpenHandle(dwMaxClient,NULL,&dwCurVersion,&hClient);
		
		if(dwResult!=ERROR_SUCCESS)
		{
			wprintf(L"WlanOpenHandle failed with error: %u\n",dwResult);
			return 1;
		}
		dwResult=WlanEnumInterfaces(hClient,NULL,&pIfList);
		if(dwResult!=ERROR_SUCCESS)
		{
			wprintf(L"WlanOpenHandle failed with error: %u\n",dwResult);
			return 1;
		}
		else
		{
			wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");
			wprintf(L"Num Entries: %lu\n",pIfList->dwNumberOfItems);
			wprintf(L"Current Index: %lu\n\n",pIfList->dwIndex);
			int i;
			for(i=0;i<(int)pIfList->dwNumberOfItems;i++)
			{
				pIfInfo=(WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[i];
				wprintf(L" Interface Index[%u]:\t%lu\n",i,i);
				iRet=StringFromGUID2(pIfInfo->InterfaceGuid,(LPOLESTR)&GuidString,sizeof(GuidString)/sizeof(*GuidString));

				if(iRet==0)
					wprintf(L"StringFromGUID2 failed\n");
				else
					wprintf(L" InterfaceGUID[%d]: %ws\n",i,GuidString);
			}
			wprintf(L" Interface Descripition[%d]: %ws",i,pIfInfo->strInterfaceDescription);
			wprintf(L"\n");
			wprintf(L"  Interface State[%d]:\t ", i);  
			switch (pIfInfo->isState) 
			{  
			case wlan_interface_state_not_ready:  
				wprintf(L"Not ready\n");  
				break;  
			case wlan_interface_state_connected:  
				wprintf(L"Connected\n");  
				break;  
			case wlan_interface_state_ad_hoc_network_formed:  
				wprintf(L"First node in a ad hoc network\n");  
				break;  
			case wlan_interface_state_disconnecting:  
				wprintf(L"Disconnecting\n");  
				break;  
			case wlan_interface_state_disconnected:  
				wprintf(L"Not connected\n");  
				break;  
			case wlan_interface_state_associating:  
				wprintf(L"Attempting to associate with a network\n");  
				break;  
			case wlan_interface_state_discovering:  
				wprintf(L"Auto configuration is discovering settings for the network\n");  
				break;  
			case wlan_interface_state_authenticating:  
				wprintf(L"In process of authenticating\n");  
				break;  
			default:  
				wprintf(L"Unknown state %ld\n", pIfInfo->isState);  
				break;  
			}
		}
	}
}
DWORD StringWToSsid(__in LPCWSTR strSsid,__out PDOT11_SSID pSsid)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    BYTE pbSsid[DOT11_SSID_MAX_LENGTH + 1] = {0};

    if (strSsid == NULL || pSsid == NULL)
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        pSsid->uSSIDLength = WideCharToMultiByte (CP_ACP,
                                                   0,
                                                   strSsid,
                                                   -1,
                                                   (LPSTR)pbSsid,
                                                   sizeof(pbSsid),
                                                   NULL,
                                                   NULL);

        pSsid->uSSIDLength--;
        memcpy(&pSsid->ucSSID, pbSsid, pSsid->uSSIDLength);
    }

    return dwRetCode;
}

LPWSTR SsidToStringW(__out_ecount(count) LPWSTR   buf,__in ULONG   count,__in PDOT11_SSID pSsid)
{
    ULONG   bytes, i;

    bytes = min( count-1, pSsid->uSSIDLength);
    for( i=0; i<bytes; i++)
        mbtowc( &buf[i], (const char *)&pSsid->ucSSID[i], 1);
    buf[bytes] = '\0';

    return buf;
}
int Wifi_Connect::Get_ComPort()
{
	
	uint16 ComId[32] = {0};
    uint16 count = 0;
    int  comport = -1;
    CDeviceList devlist;
    count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
    
    if(count > 0)
    {
        comport = ComId[0];      
    }
	return comport;
}


bool Wifi_Connect::Get_WIFI_Name(WCHAR *wifi_name,string &Port_number,int iflag)
{
	int port_number=Get_ComPort();
	char temp[64];
	string MAC;
	HANDLE hClient = NULL;  
	DWORD dwMaxClient = 2;         
	DWORD dwCurVersion = 0;  
	DWORD dwResult = 0;  
	DWORD dwRetVal = 0;  
	int iRet = 0;
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;  
	PWLAN_INTERFACE_INFO pIfInfo = NULL;  
	LPCWSTR pProfileName = NULL;  
	LPWSTR pProfileXml = NULL;  
	DWORD dwFlags = 0;  
	int i,j;
	bool get_service = false;
	PWLAN_AVAILABLE_NETWORK_LIST pWLAN_AVAILABLE_NETWORK_LIST = NULL;
	PWLAN_AVAILABLE_NETWORK_LIST pWLAN_AVAILABLE_NETWORK_LIST_CHECK = NULL;
	WLAN_AVAILABLE_NETWORK wlanAN;
	
	if(port_number!=-1)
	{		
		sprintf(temp, "%d", port_number);
		string s(temp);
		Port_number="COM"+s;
		Port_number+=" ";
	}
	else
		return false;
	
	if(!get_Mac(port_number,MAC,iflag))
		return false;      

	//MAC = "0b1dad"; 
	//pProfileName = argv[1];      
	//wprintf(L"Information for profile: %ws\n\n", pProfileName);    
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);  
	if (dwResult != ERROR_SUCCESS) 
	{  
		wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
		DWORD dwMaxClient = 1; 
		WlanCloseHandle(hClient, NULL); 
		//dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
		//if (dwResult != ERROR_SUCCESS) 
			return false;  
	}  
	
	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);  
	if (dwResult != ERROR_SUCCESS)
	{  
		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);  
		WlanFreeMemory(pIfList);  
		WlanCloseHandle(hClient, NULL); 
		return false;  
	} 
	else
	{ 
		WlanScan(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,NULL, NULL, NULL); 
		Sleep(1000);
		dwResult = WlanGetAvailableNetworkList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,  
			WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES,  
			NULL, &pWLAN_AVAILABLE_NETWORK_LIST);  
		if (dwResult != ERROR_SUCCESS)  
		{              
			printf("WlanGetAvailableNetworkList failed with error: %u\n",dwResult);  
			WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST);  
			WlanFreeMemory(pIfList);  
			WlanCloseHandle(hClient, NULL); 
			return false;  
		}  

		int numberOfItems = pWLAN_AVAILABLE_NETWORK_LIST->dwNumberOfItems;  
		WCHAR strSsid[DOT11_SSID_MAX_LENGTH+1];

		WCHAR WS_MAC[10]={0};


		const char* tem=MAC.c_str();
		int len = MultiByteToWideChar(CP_ACP,0,tem,strlen(tem),NULL,0);   
		MultiByteToWideChar(CP_ACP,0,tem,strlen(tem),WS_MAC,len);  
		WS_MAC[len]='\0';  
		//CString CS_WIFI_NAME(MAC.c_str());
		
		for(j=0;j<numberOfItems;j++)
		{
			wlanAN = pWLAN_AVAILABLE_NETWORK_LIST->Network[j]; 
			if (&wlanAN != NULL)
			{
				SsidToStringW(strSsid, sizeof(strSsid)/sizeof(WCHAR),&wlanAN.dot11Ssid) ;
				//int find=_tcsstr(WS_MAC,strSsid);
				if(_tcsstr(strSsid,WS_MAC)!=NULL)
				{
					get_service = true;
					break;
				}
			}
		}
		
		if(get_service)
		{
			wcscpy(wifi_name,strSsid);
			CString CS_WIFI_NAME(strSsid);	
			if (wlanAN.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
			{
				
				WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST);
				WlanFreeMemory(pIfList);
				dwResult=WlanCloseHandle(hClient, NULL); 
				Sleep(10000);
				return true;
			}
			else
			{
				dwResult = WlanDisconnect(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,NULL);//DISCONNECT FIRST   
				if(dwResult != ERROR_SUCCESS)  
				{  
					printf("WlanDisconnect failed with error: %u\n",dwResult);  
					return false;  
				} 
				CString strHead = _T("<?xml version=\"1.0\"?>\
					 <WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\
					 <name>");
				     strHead+=CS_WIFI_NAME;
					 strHead+= _T("</name>\
					 <SSIDConfig>\
					 <SSID>\
					 <name>");
					 strHead+=CS_WIFI_NAME;
					 strHead+= _T("</name>\
					 </SSID>\
					 </SSIDConfig>\
					 <connectionType>ESS</connectionType>\
					 <connectionMode>auto</connectionMode>\
					 <MSM>\
					 <security>\
					 <authEncryption>\
					 <authentication>open</authentication>\
					 <encryption>none</encryption>\
					 <useOneX>false</useOneX>\
					 </authEncryption>\
					 </security>\
					 </MSM>\
					 </WLANProfile>");
							 
						
				WCHAR strFile[4096] = { 0 }; 
				ULONG   blenth, j;
				blenth = strHead.GetLength();
				for (j = 0; j<blenth; j++)
				{
					mbtowc(&strFile[j], (const char *)(LPCTSTR)strHead.Left(1), 1);
					strHead = strHead.Right(strHead.GetLength() - 1);
				}
				strFile[blenth] = '\0';
				WLAN_REASON_CODE Wlanreason;
				dwResult = WlanSetProfile(hClient,&pIfList->InterfaceInfo[0].InterfaceGuid,
        				0, strFile, NULL, TRUE, NULL, &Wlanreason);
				if (ERROR_SUCCESS != dwResult)
				{

					printf("wlan set profile failed %lu.\r\n", dwResult);
				}
				Sleep(10000);
				DOT11_SSID dot11Ssid = {0};
				if ((dwResult = StringWToSsid(strSsid, &dot11Ssid)) != ERROR_SUCCESS)
				{
					printf("WlanConnect success!\n");  
				}
				WlanScan(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,NULL, NULL, NULL); 
				Sleep(1000);
				dwResult = WlanGetAvailableNetworkList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,  
					WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES,  
					NULL, &pWLAN_AVAILABLE_NETWORK_LIST_CHECK); 
				if (dwResult != ERROR_SUCCESS)  
				{              
					printf("WlanGetAvailableNetworkList failed with error: %u\n",dwResult);  
					WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST); 
					WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST_CHECK); 
					WlanFreeMemory(pIfList);  
					WlanCloseHandle(hClient, NULL); 
					return false;  
				}
				for(j=0;j<numberOfItems;j++)
				{
					wlanAN = pWLAN_AVAILABLE_NETWORK_LIST_CHECK->Network[j]; 
					if (&wlanAN != NULL)
					{
						SsidToStringW(strSsid, sizeof(strSsid)/sizeof(WCHAR),&wlanAN.dot11Ssid) ;
						if(_tcsstr(strSsid,WS_MAC)!=NULL)
						{
							get_service = true;
							break;
						}
					}
				}
				if(get_service)
				{
					if (wlanAN.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
					{
						
						WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST);
						WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST_CHECK);
						WlanFreeMemory(pIfList);
						dwResult=WlanCloseHandle(hClient, NULL); 
						return true;
					}
				}
				WLAN_CONNECTION_PARAMETERS wlanConnPara; 
				wlanConnPara.wlanConnectionMode =wlan_connection_mode_profile;  
				wlanConnPara.strProfile = NULL;    
				wlanConnPara.pDot11Ssid = &wlanAN.dot11Ssid;  
				wlanConnPara.dot11BssType = dot11_BSS_type_independent; 
				wlanConnPara.pDesiredBssidList = NULL;  
				wlanConnPara.dwFlags = 0;					
				for(int k=0;k<10;k++)
				{
					dwResult=WlanConnect(hClient,&pIfList->InterfaceInfo[0].InterfaceGuid,&wlanConnPara ,NULL); 
					if (dwResult==ERROR_SUCCESS)  
					{  
						printf("WlanConnect success!\n"); 
						WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST);
						WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST_CHECK);
						WlanFreeMemory(pIfList);  
						dwResult=WlanCloseHandle(hClient, NULL); 
						return true;
					}
					Sleep(1000);
				}
				WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST); 
				WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST_CHECK);
				WlanFreeMemory(pIfList);  
				WlanCloseHandle(hClient, NULL); 
			}
		}
	}  
 
	WlanCloseHandle(hClient, NULL); 
	return false;  
}
bool Wifi_Connect::get_Mac(uint16 port,string &MAC_ADD, int iflag)
{
	bool get_mac=false;
	CSerialPort     m_SerialPort;
	//int lSize = 0;
	char	s_AT[20];
	unsigned int pCount;
	//char * LedTestOn="mac\r\n\r\n";

	char buffer_header[BUFFER_HEADER_LEN] = {0};
	byte   chr;
	unsigned int  count = 0;
	int i,iSize,j,k;
	TResult is_open;
	int temp;
	
	is_open=m_SerialPort.Open(port,115200,1,2);
	if(is_open!=EOK)
	{
		return false;
	}
	//Sleep(20000);
	
	get_mac=false;
	memset(s_AT, 0x0, 20);
	strcpy(s_AT,"mac\r\n");
	iSize = strlen(s_AT);
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}		
		if(strstr(buffer_header,"OK")!= NULL)
		{
			get_mac=true;
			break;
		}	
		Sleep(2000);
	}
	if(get_mac)
	{
		strncpy(s_WIFIAddr, buffer_header+5,12);
		s_WIFIAddr[12] = 0;
		MAC_ADD=(char*)buffer_header;		
		temp=MAC_ADD.find("OK");
		MAC_ADD=MAC_ADD.substr(temp-8,6);		
		return true;
	}
	else
	{
		if(iflag == 1)
		{
			return false;
		}
	}
	Sleep(1000);
	memset(s_AT, 0x0, 20);
	strcpy(s_AT,"baudrate\r\n");
	iSize = strlen(s_AT);	
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		if(strstr(buffer_header,"OK")!= NULL)
		{
			get_mac=true;
			break;
		}
		Sleep(1000);
	}
	if(!get_mac)
		return false;
	Sleep(1000);
	memset(s_AT, 0x0, 20);
	if(strstr(buffer_header,"1")!= NULL)
	{
		strcpy(s_AT,"baudrate 0\r\n");
	}
	else
	{
		strcpy(s_AT,"baudrate 1\r\n");
	}

	iSize = strlen(s_AT);	
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		if(strstr(buffer_header,"OK")!= NULL)
		{
			get_mac=true;
			break;
		}
		Sleep(1000);
	}
	if(!get_mac)
		return false;
	Sleep(1000);
	get_mac=false;
	memset(s_AT, 0x0, 20);
	strcpy(s_AT,"reset\r\n");
	iSize = strlen(s_AT);
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(5000);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		if(strstr(buffer_header,"Init serialcom complete")!= NULL)
		{
			get_mac=true;
			break;
		}
		Sleep(1000);
	}
	if(!get_mac)
		return false;
	Sleep(10000);
	get_mac=false;
	memset(s_AT, 0x0, 20);
	strcpy(s_AT,"mac\r\n");
	iSize = strlen(s_AT);
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		Sleep(2000);
		if(strstr(buffer_header,"OK")!= NULL)
		{
			get_mac=true;
			break;
		}			
	}
	if(get_mac)
	{
		strncpy(s_WIFIAddr, buffer_header+5,12);
		s_WIFIAddr[12] = 0;
		MAC_ADD=(char*)buffer_header;
		temp=MAC_ADD.find("OK");		
		MAC_ADD=MAC_ADD.substr(temp-8,6);
		return true;
	}
	
	return false;
}

/*bool Wifi_Connect::get_Mac(uint16 port,string &MAC_ADD)
{
	bool get_mac=false;
	CSerialPort         m_SerialPort;
	TResult is_open=m_SerialPort.Open(port,115200,1,2);
	if(is_open!=EOK)
	{
		return false;
	}
	uint32 pCount;
	char * LedTestOn="mac\r\n\r\n";
	uint8* pBuff=(uint8*)LedTestOn;
	uint32 lSize=strlen(LedTestOn);
	m_SerialPort.Send(pBuff, lSize, &pCount);
	uint8 buffer_header[BUFFER_HEADER_LEN] = {0};
	uint8   chr;
	uint32  count = 0;
	for (int i=0;i<BUFFER_HEADER_LEN;i++)
	{
		m_SerialPort.Receive(&chr, 1, &count);
		buffer_header[i] = chr;
		if(count==0)
			break;

	}
	MAC_ADD=(char*)buffer_header;
	int temp=-1;
	if((temp=MAC_ADD.find("OK"))!=-1)
	{
		get_mac=true;
		MAC_ADD=MAC_ADD.substr(temp-8,6);
		//WIFI_NAME="VF Home_"+WIFI_NAME+"_AJ";

	}
	return get_mac;
}*/

bool Wifi_Connect::get_Trace(char s_Trace[])
{
	bool get_trace=false;
	CSerialPort         m_SerialPort;
	int iSize,i,j;
	unsigned int pCount;
	char buffer_header[BUFFER_HEADER_LEN] = {0};
	byte   chr;
	unsigned int  count = 0;
	char	s_AT[20];
	char temp[64];
	int port_number=Get_ComPort();

    TResult is_open=m_SerialPort.Open(port_number,115200,1,2);
	if(is_open!=EOK)
	{
		return false;
	}
	memset(s_AT, 0x0, 20);
	strcpy(s_AT,"trace\r\n\r\n");
	iSize=strlen(s_AT);
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		Sleep(2000);
		if(strstr(buffer_header,"trace") != NULL)
		{
			get_trace=true;
			break;
		}			
	}
	if(get_trace)
	{
		strncpy(s_Trace,buffer_header+9,32);
		s_Trace[32] = 0;
	}
	return get_trace;
}

bool Wifi_Connect::set_Trace(char s_Trace[])
{
	bool set_trace=false;
	CSerialPort         m_SerialPort;
	int iSize,i,j;
	unsigned int pCount;
	char buffer_header[BUFFER_HEADER_LEN] = {0};
	byte   chr;
	unsigned int  count = 0;
	char	s_AT[50];
	char temp[64];
	int port_number=Get_ComPort();

    TResult is_open=m_SerialPort.Open(port_number,115200,1,2);
	if(is_open!=EOK)
	{
		return false;
	}
	memset(s_AT, 0x0, 50);
	sprintf(s_AT,"trace %s\r\n",s_Trace);
	iSize=strlen(s_AT);
	for(j = 0;j < MAX_RETRY_NUM ; j++)
	{
		m_SerialPort.Send((unsigned char *)s_AT, iSize, &pCount);
		memset(buffer_header, 0, BUFFER_HEADER_LEN);
		Sleep(100);
		for (i=0;i<BUFFER_HEADER_LEN;i++)
		{
			m_SerialPort.Receive(&chr, 1, &count);
			buffer_header[i] = chr;
			if(count==0)
				break;
	
		}
		Sleep(2000);
		if(strstr(buffer_header,"OK") != NULL)
		{
			set_trace=true;
			break;
		}			
	}
	return set_trace;
}

void Wifi_Connect::Get_WIFI_MAC(char *wifi_mac)
{	
	strcpy(wifi_mac, s_WIFIAddr);
}
