#include "StdAfx.h"
#include "NicManager.h"
#include <stdio.h>
#include "log.h"
#include <Icmpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma	 comment(lib,"setupapi.lib")
//#pragma comment(lib,"icmp.lib")

NicManager::NicManager(string network):
    device_ip(""),
    segment(network)
{
    device_ip.clear();
    gateway_ip.clear();
    mAdapterName.clear();
    GetAdapter();
}

NicManager::~NicManager(void)
{
}

bool NicManager::GetAdapter()
{
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    int netCardNum = 0;
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        PIP_ADAPTER_INFO pIpAdapterInfo_Temp=pIpAdapterInfo;
        mAdapterName = pIpAdapterInfo->AdapterName;
        //PIP_ADDR_STRING current = pIpAdapterInfo->CurrentIpAddress;
        while (pIpAdapterInfo_Temp)
        {
            IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo_Temp->IpAddressList);
            IP_ADDR_STRING *pGateway = &pIpAdapterInfo_Temp->GatewayList;
            do
            {
                if (pIpAdapterInfo_Temp->Type == MIB_IF_TYPE_ETHERNET)
                    // if(device_ip.find(segment) != -1)
                    //if (strcmp(pIpAddrString->IpAddress.String, current->IpAddress.String) == 0)
                {
                    device_ip =pIpAddrString->IpAddress.String;
                    gateway_ip = pGateway->IpAddress.String;
                    delete pIpAdapterInfo;
                    return true;
                }
                pIpAddrString=pIpAddrString->Next;
                pGateway = pGateway->Next;
            } while (pIpAddrString && pGateway);
            pIpAdapterInfo_Temp = pIpAdapterInfo_Temp->Next;
        }
    }

    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }
    return false;
}

#define MAX_BUF_SIZE   300
bool Ping(const char *ip_addr) {
    HANDLE hIcmpFile = NULL;
    DWORD dwRetVal = 0;
    IPAddr ipaddr = INADDR_NONE;
    char SendData[] = "IP Monitor";
    DWORD ReplySize  = MAX_BUF_SIZE + sizeof(ICMP_ECHO_REPLY) + 8;
    char ReplyBuffer[MAX_BUF_SIZE + sizeof(ICMP_ECHO_REPLY) + 8];
    IP_OPTION_INFORMATION ipOptions;
    memset(&ipOptions, 0, sizeof(IP_OPTION_INFORMATION));
    ipOptions.Ttl = 255;
    ipOptions.Flags = NULL;

    ipaddr = inet_addr(ip_addr);
    if( ipaddr == INADDR_NONE ) {
        hostent* hp = gethostbyname(ip_addr);
        if( hp ) {
            memcpy(&ipaddr,hp->h_addr,hp->h_length);
        } else {
            return false;
        }
    }
    hIcmpFile = IcmpCreateFile();
    if( hIcmpFile == INVALID_HANDLE_VALUE ) {
        LOGE("IcmpCreateFile failed");
        return false;
    }

    dwRetVal = IcmpSendEcho( hIcmpFile,
                            ipaddr,
                            (LPVOID) SendData,
                            sizeof(SendData),
                            NULL,
                            ReplyBuffer,
                            ReplySize,
                            2000);
    IcmpCloseHandle(hIcmpFile);

    if( dwRetVal > 0 ) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        if( pEchoReply->Status != IP_SUCCESS ) {
            return false;
        }
        if( pEchoReply->Address != ipaddr ) {
            return false;
        }
        if( strcmp( SendData, (CHAR*)pEchoReply->Data ) != 0 ) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}


int ResolveIpMac(const char *DestIpString, string & mac)
{
#define BUFFER_LEN  6
    DWORD dwRetVal;
    IPAddr DestIp = 0;
    IPAddr SrcIp = 0;       /* default for src ip , means INADDR_ANY IPv4 address*/
    ULONG MacAddr[2];       /* for 6-byte hardware addresses */
    ULONG PhysAddrLen = 6;  /* default to length of six bytes */
    CHAR  Buffer[BUFFER_LEN];

    //char *DestIpString = NULL;
    //char *SrcIpString = NULL;

    BYTE *bPhysAddr;
    unsigned int i;

    if (DestIpString == NULL || DestIpString[0] == '\0') {
        LOGE("invalid destination IP address.");
        return 1;
    }
    mac.clear();

    DestIp = inet_addr(DestIpString);

    memset(&MacAddr, 0xff, sizeof (MacAddr));

    //LOGE("Sending ARP request for IP address: %s", DestIpString);

    dwRetVal = SendARP(DestIp, SrcIp, &MacAddr, &PhysAddrLen);

    if (dwRetVal == NO_ERROR) {
        bPhysAddr = (BYTE *) & MacAddr;
        if (PhysAddrLen) {
            for (i = 0; i < (int) PhysAddrLen; i++) {
                memset(Buffer, 0, BUFFER_LEN);
                if (i == (PhysAddrLen - 1))
                    _snprintf(Buffer, BUFFER_LEN, "%.2X", (int) bPhysAddr[i]);
                else
                    _snprintf(Buffer, BUFFER_LEN, "%.2X:", (int) bPhysAddr[i]);
                mac += Buffer;
            }
            return 0;
        } else {
            LOGE("Warning: SendArp completed successfully, but returned length=0\n");
        }
    } else {
        const char * errstr = "";
        switch (dwRetVal) {
        case ERROR_GEN_FAILURE:
            errstr = _T(" (ERROR_GEN_FAILURE)");
            break;
        case ERROR_INVALID_PARAMETER:
            errstr = _T(" (ERROR_INVALID_PARAMETER)");
            break;
        case ERROR_INVALID_USER_BUFFER:
            errstr = _T(" (ERROR_INVALID_USER_BUFFER)");
            break;
        case ERROR_BAD_NET_NAME:
            errstr = _T(" (ERROR_GEN_FAILURE)");
            break;
        case ERROR_BUFFER_OVERFLOW:
            errstr = _T(" (ERROR_BUFFER_OVERFLOW)");
            break;
        case ERROR_NOT_FOUND:
            errstr = _T(" (ERROR_NOT_FOUND)");
            break;
        default:
            break;
        }
        LOGE("Error: SendArp failed with error: %d(%s)", dwRetVal, errstr);
    }

    return 1;
}
//-----------------------------------------------------------------
// 取得所有网卡信息
//-----------------------------------------------------------------
BOOL NicManager::GetAdapterInfo() {
    // 这里的代码适合WINDOWS2000，对于NT需要读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards
    HKEY hKey, hSubKey, hNdiIntKey;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}",
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
        return FALSE;

    DWORD dwIndex = 0;
    DWORD dwBufSize = 256;
    DWORD dwDataType;
    char szSubKey[256];
    unsigned char szData[256];

    while(RegEnumKeyEx(hKey, dwIndex++, szSubKey, &dwBufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if(RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
        {
            if(RegOpenKeyEx(hSubKey, "Ndi\\Interfaces", 0, KEY_READ, &hNdiIntKey) == ERROR_SUCCESS)
            {
                dwBufSize = 256;
                if(RegQueryValueEx(hNdiIntKey, "LowerRange", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                {
                    if(strcmp((char*)szData, "ethernet") == 0)//判断是不是以太网卡
                    {
                        dwBufSize = 256;
                        if(RegQueryValueEx(hSubKey, "DriverDesc", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                        {
                            ADAPTER_INFO *pAI = new ADAPTER_INFO;
                            pAI->strDriverDesc = (LPCTSTR)szData;
                            dwBufSize = 256;
                            if(RegQueryValueEx(hSubKey, "NetCfgInstanceID", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                            {
                                pAI->strName = (LPCTSTR)szData;
                                RegGetIP(pAI);
                            }
                            AdapterInfoVector.push_back(pAI);// 加入到容器中
                        }
                    }
                }
                RegCloseKey(hNdiIntKey);
            }
            RegCloseKey(hSubKey);
        }

        dwBufSize = 256;
    } /* end of while */

    RegCloseKey(hKey);

    return TRUE;
}


//-----------------------------------------------------------------
// 得到注册表中的IP信息
// nIndex暂时未处理
//-----------------------------------------------------------------

BOOL NicManager::RegGetIP(ADAPTER_INFO *pAI)
{
    ASSERT(pAI);

    HKEY hKey;
    string strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    strKeyName += pAI->strName;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    strKeyName.c_str(),
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
        return FALSE;

    unsigned char szData[256];
    DWORD dwDataType, dwBufSize;

    dwBufSize = 256;
    if(RegQueryValueEx(hKey, "IPAddress", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        pAI->strIP = (LPCTSTR)szData;

    dwBufSize = 256;
    if(RegQueryValueEx(hKey, "SubnetMask", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        pAI->strNetMask = (LPCTSTR)szData;

    dwBufSize = 256;
    if(RegQueryValueEx(hKey, "DefaultGateway", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        pAI->strNetGate = (LPCTSTR)szData;

    RegCloseKey(hKey);
    return TRUE;
}

BOOL NicManager::RegSetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, LPCTSTR pDnsAddress)
{
    HKEY hKey;
    CString strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    strKeyName += mAdapterName.c_str();
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    strKeyName.GetString(),
                    0,
                    KEY_WRITE,
                    &hKey) != ERROR_SUCCESS)
        return FALSE;


    char mszIPAddress[100];
    char mszNetMask[100];
    char mszNetGate[100];
    char szDnsAddr[100];

    strncpy(mszIPAddress, pIPAddress, 98);
    strncpy(mszNetMask, pNetMask, 98);
    strncpy(mszNetGate, pNetGate, 98);
    strncpy(szDnsAddr, pDnsAddress, 98);


    int nIP, nMask, nGate,nDnsAddr;
    int enableDHCP=0;


    nIP = strlen(mszIPAddress);
    nMask = strlen(mszNetMask);
    nGate = strlen(mszNetGate);
    nDnsAddr = strlen(szDnsAddr);


    *(mszIPAddress + nIP + 1) = 0x00;
    nIP += 2;


    *(mszNetMask + nMask + 1) = 0x00;
    nMask += 2;


    *(mszNetGate + nGate + 1) = 0x00;
    nGate += 2;


    *(szDnsAddr + nDnsAddr + 1) = 0x00;
    nDnsAddr += 2;


    RegSetValueEx(hKey, "IPAddress", 0, REG_MULTI_SZ, (unsigned char*)mszIPAddress, nIP);
    RegSetValueEx(hKey, "SubnetMask", 0, REG_MULTI_SZ, (unsigned char*)mszNetMask, nMask);
    RegSetValueEx(hKey, "DefaultGateway", 0, REG_MULTI_SZ, (unsigned char*)mszNetGate, nGate);
    RegSetValueEx(hKey, "NameServer", 0, REG_SZ, (unsigned char*)szDnsAddr, nDnsAddr);

    RegSetValueEx(hKey, "EnableDHCP", 0, REG_DWORD, (unsigned char*)&enableDHCP, sizeof(DWORD) );
    RegCloseKey(hKey);

    return TRUE;
}

//-----------------------------------------------------------------
// 设置注册表中DHCP
//-----------------------------------------------------------------
BOOL NicManager::RegSetDHCPIP()
{
    HKEY hKey;
    string strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    strKeyName += mAdapterName.c_str();
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    strKeyName.c_str(),
                    0,
                    KEY_WRITE,
                    &hKey) != ERROR_SUCCESS)
        return FALSE;

    int enableDHCP=1;
    char mszIPAddress[100];
    char mszNetMask[100];
    char mszNetGate[100];
    char szDnsAddr[100];
    int nIP, nMask, nGate,nDnsAddr;

    strncpy(mszIPAddress, "0.0.0.0", 98);
    strncpy(mszNetMask, "0.0.0.0", 98);
    strncpy(mszNetGate, "", 98);
    strncpy(szDnsAddr, "", 98);

    nIP = strlen(mszIPAddress);
    nMask = strlen(mszNetMask);
    nGate = strlen(mszNetGate);
    nDnsAddr=strlen(szDnsAddr);

    *(mszIPAddress + nIP + 1) = 0x00;
    nIP += 2;

    *(mszNetMask + nMask + 1) = 0x00;
    nMask += 2;

    *(mszNetGate + nGate + 1) = 0x00;
    nGate += 2;

    *(szDnsAddr + nDnsAddr + 1) = 0x00;
    nDnsAddr += 2;

    RegSetValueEx(hKey, "IPAddress", 0, REG_MULTI_SZ, (unsigned char*)mszIPAddress, nIP);
    RegSetValueEx(hKey, "SubnetMask", 0, REG_MULTI_SZ, (unsigned char*)mszNetMask, nMask);
    RegSetValueEx(hKey, "DefaultGateway", 0, REG_MULTI_SZ, (unsigned char*)mszNetGate, nGate);
    RegSetValueEx(hKey, "NameServer", 0, REG_SZ, (unsigned char*)szDnsAddr, nDnsAddr);

    int errCode = RegSetValueEx(hKey, "EnableDHCP", 0, REG_DWORD, (unsigned char*)&enableDHCP, sizeof(DWORD) );

    RegCloseKey(hKey);
    return TRUE;
}


//-----------------------------------------------------------------
// 通知IP地址的改变
//-----------------------------------------------------------------
BOOL NicManager::NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex)
{
    BOOL bResult = FALSE;
    HINSTANCE hDhcpDll;
    DHCPNOTIFYPROC pDhcpNotifyProc;
    WCHAR wcAdapterName[256];

    MultiByteToWideChar(CP_ACP, 0, lpszAdapterName, -1, wcAdapterName,256);

    if((hDhcpDll = LoadLibrary("dhcpcsvc")) == NULL)
        return FALSE;


    if((pDhcpNotifyProc = (DHCPNOTIFYPROC)GetProcAddress(hDhcpDll, "DhcpNotifyConfigChange")) != NULL)
        if((pDhcpNotifyProc)(NULL, wcAdapterName, TRUE, nIndex, NULL,NULL, 0) == ERROR_SUCCESS)
            bResult = TRUE;

    FreeLibrary(hDhcpDll);
    return bResult;
}

//-----------------------------------------------------------------
//  设置IP地址
//  如果只绑定一个IP，nIndex = 0，暂时未处理一个网卡绑定多个地址
//-----------------------------------------------------------------
BOOL NicManager::SetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate,LPCTSTR pDnsAddress)
{
    if(!RegSetIP(pIPAddress, pNetMask, pNetGate,pDnsAddress))
        return FALSE;


    //通知IP地址的改变(此方法会造成栈溢出问题，而且对于设置dhcp的立即生效没有作用，故舍弃)
    //if(!NotifyIPChange(lpszAdapterName, nIndex, pIPAddress, pNetMask))
    //  return FALSE;


    //通过禁用启用网卡实现IP立即生效
    list<TNetCardStruct> cardList;
    EnumNetCards(&cardList);
    if(!cardList.empty())
    {
        NetCardStateChange(&cardList.front(),FALSE);
        Sleep(10);
        NetCardStateChange(&cardList.front(),TRUE);
    }

    return TRUE;
}


//-----------------------------------------------------------------
//  设置DHCP IP地址
//-----------------------------------------------------------------
BOOL NicManager::SetDHCPIP()
{
    if(!RegSetDHCPIP())
        return FALSE;


    //通知IP地址的改变(此方法会造成栈溢出问题，而且对于设置dhcp的立即生效没有作用，故舍弃)
    //if(!NotifyDHCPIPChange(lpszAdapterName, nIndex))
    //  return FALSE;


    //通过禁用启用网卡实现IP立即生效
    list<TNetCardStruct> cardList;
    EnumNetCards(&cardList);
    if(!cardList.empty())
    {
        NetCardStateChange(&cardList.front(),FALSE);
        Sleep(10);
        NetCardStateChange(&cardList.front(),TRUE);
    }
    return TRUE;
}


void NicManager::EnumNetCards(list<TNetCardStruct> *NetDeviceList)
{
    string DevValue;
    PNetCardStruct NetCard;
    DWORD  Status, Problem;
    LPTSTR Buffer   = NULL;
    DWORD  BufSize  = 0;
    HDEVINFO hDevInfo   = 0;

    hDevInfo=SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if(INVALID_HANDLE_VALUE==hDevInfo)
        return;

    SP_DEVINFO_DATA  DeviceInfoData ={sizeof(SP_DEVINFO_DATA)};

    HKEY hKeyClass;
    char DeviceName[200];
    for(DWORD DeviceId=0;SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&DeviceInfoData);DeviceId++)
    {
        if (CM_Get_DevNode_Status(&Status, &Problem, DeviceInfoData.DevInst,0) != CR_SUCCESS)
            continue;
        if(GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS , &Buffer, (PULONG)&BufSize))
            DevValue = string(Buffer);

        if (strcmp(DevValue.c_str(),"Net") == 0)
        {
            DevValue = "";

            if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_ENUMERATOR_NAME , &Buffer, (PULONG)&BufSize))
                DevValue = Buffer;

            if (strcmp(DevValue.c_str(),"ROOT") != 0)
            {
                NetCard = new TNetCardStruct;
                NetCard->Id = DeviceId;
                NetCard->Name = "<Unknown Device>";
                if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DRIVER , &Buffer, (PULONG)&BufSize))
                    if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC , &Buffer, (PULONG)&BufSize))
                        NetCard->Name = Buffer;
                NetCard->Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem);
                NetCard->Changed = false;
                NetDeviceList->push_back(*NetCard);
            }
        }
    }
}

//---------------------------------------------------------------------------
bool NicManager::GetRegistryProperty(HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    ULONG Property,
    PVOID Buffer,
    PULONG Length)
{
    while (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
        DeviceInfoData, Property, NULL, (BYTE *)*(TCHAR **)Buffer, *Length, Length))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            if (*(LPTSTR *)Buffer) LocalFree(*(LPTSTR *)Buffer);
            *(LPTSTR *)Buffer = (PCHAR)LocalAlloc(LPTR,*Length);
        }
        else return false;
    }
    return (*(LPTSTR *)Buffer)[0];
}


//---------------------------------------------------------------------------
//     NetCardStateChange 网卡的启用与禁用
//             NetCardPoint 是 PNetCardStruct 的指针.
//             Enabled     true = 启用     false = 禁用
//---------------------------------------------------------------------------
bool NicManager::NetCardStateChange(PNetCardStruct NetCardPoint, bool Enabled)
{
    PNetCardStruct NetCard = (PNetCardStruct)NetCardPoint;
    DWORD DeviceId = NetCard->Id;
    HDEVINFO hDevInfo = 0;
    if (INVALID_HANDLE_VALUE == (hDevInfo =
        SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT |DIGCF_ALLCLASSES)))
        return false;
    SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)};
    DWORD Status, Problem;
    if (!SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&DeviceInfoData))
        return false;

    if (CM_Get_DevNode_Status(&Status, &Problem,
        DeviceInfoData.DevInst,0) != CR_SUCCESS)
        return false;

    SP_PROPCHANGE_PARAMS PropChangeParams = {sizeof(SP_CLASSINSTALL_HEADER)};
    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    PropChangeParams.Scope = DICS_FLAG_GLOBAL;
    if (Enabled)
    {
        if (!((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem)))
        {
            NetCard->Disabled = false;
            return false;
        }
        PropChangeParams.StateChange = DICS_ENABLE;
    }
    else
    {
        if ((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem))
        {
            NetCard->Disabled = true;
            return false;
        }
        if (!((Status & DN_DISABLEABLE) && (CM_PROB_HARDWARE_DISABLED != Problem)))
            return false;
        PropChangeParams.StateChange = DICS_DISABLE;
    }

    if (!SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData,
        (SP_CLASSINSTALL_HEADER *)&PropChangeParams, sizeof(PropChangeParams)))
        return false;
    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData))
        return false;
    if (CM_Get_DevNode_Status(&Status, &Problem,
        DeviceInfoData.DevInst,0) == CR_SUCCESS)
        NetCard->Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem);
    return true;
}



BOOL NicManager::GetConnectName(CString& name) {

    HKEY hKey;
    //HKEY_LOCAL_MACHINE\\,  GUID_DEVCLASS_NET
    CString keyPath;
    BYTE value[MAX_PATH];
    DWORD nSize=sizeof value;
    BOOL result = FALSE;

    if (mAdapterName.length() <=0) {
        LOGE("mAdapterName is null");
        return result;
    }

    keyPath.Format("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\"
                   "%s\\Connection", mAdapterName.c_str());
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    keyPath.GetString(),
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
        return result;

    if (RegQueryValueEx(hKey, "Name", NULL, NULL, (LPBYTE)&value, &nSize) == ERROR_SUCCESS) {
        name = value;
        result = TRUE;
    } else {
        result = FALSE;
    }
    RegCloseKey(hKey);
    return result;
}

// Start an explorer window, directory is Tftpd32's default directory
//SetIP("192.168.1.10", "255.255.255.0", "192.168.1.1");
int NicManager::SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask)
{
    CString connectionName;
    CString command;
    GetConnectName(connectionName);
    command.Format("netsh int ip set address name=\"%s\" source=static %s %s %s 1",
                   connectionName.GetString(), ip, subnetMask, gateway);
    int rc = ExecuteCommand((LPSTR)command.GetString());

    device_ip = ip;
    gateway_ip = gateway;
    return rc;
} // StartExplorer

BOOL NicManager::EnableDhcp() {
    CString connectionName;
    CString command;
    if (FALSE == GetConnectName(connectionName))
        return FALSE;

    command.Format("netsh int ip set addr \"%s\" dhcp ", connectionName.GetString());
    int rc = ExecuteCommand((LPSTR)command.GetString());
    //command.Format("netsh int ip set dns \"%s\" dhcp", connectionName.GetString());
    //rc += ExecuteCommand((LPSTR)command.GetString());

    device_ip.clear();
    gateway_ip.clear();
    GetAdapter();
    return rc == 0;
}

int NicManager::ExecuteCommand(LPSTR lpCommandLine) {
    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;
    int Rc;

    memset (& sInfo, 0, sizeof sInfo);
    sInfo.cb = sizeof sInfo;

    LOGE("Execute command %s", lpCommandLine);

    Rc = CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE,
                       NORMAL_PRIORITY_CLASS, NULL, NULL, &sInfo, &pInfo) ;
    CloseHandle (pInfo.hProcess);
    CloseHandle (pInfo.hThread);

    return Rc;
}
