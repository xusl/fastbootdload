#include "StdAfx.h"
#include "NicManager.h"
#include <stdio.h>
#include "log.h"
#include <Icmpapi.h>
#include <devguid.h>

#pragma comment(lib,"Iphlpapi.lib")
#pragma	 comment(lib,"setupapi.lib")
//#pragma comment(lib,"icmp.lib")

NicManager::NicManager(string network):
    device_ip(""),
    m_pDefaultNic(NULL),
    segment(network)
{
    device_ip.clear();
    gateway_ip.clear();
    mNicList.clear();
}

NicManager::~NicManager(void)
{
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
#if 0
//-----------------------------------------------------------------
// 取得所有网卡信息
//-----------------------------------------------------------------
BOOL NicManager::GetAdapterInfo() {
    // 这里的代码适合WINDOWS2000，
    //对于NT需要读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards
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
        if(RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
        {
            continue;
        }
        if(RegOpenKeyEx(hSubKey, "Ndi\\Interfaces", 0, KEY_READ, &hNdiIntKey) != ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            continue;
        }
        dwBufSize = sizeof szData;
        if(RegQueryValueEx(hNdiIntKey, "LowerRange", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        {
            if(strcmp((char*)szData, "ethernet") == 0)//判断是不是以太网卡
            {
                dwBufSize = 256;
                if(RegQueryValueEx(hSubKey, "DriverDesc", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                {
                    //ADAPTER_INFO *pAI = new ADAPTER_INFO;
                    //pAI->strDriverDesc = (LPCTSTR)szData;
                    dwBufSize = 256;
                    if(RegQueryValueEx(hSubKey, "NetCfgInstanceID", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                    {
                        //pAI->strName = (LPCTSTR)szData;
                        //RegGetIP(pAI);
                    }
                    //AdapterInfoVector.push_back(pAI);// 加入到容器中
                }
            }
        }
        RegCloseKey(hNdiIntKey);

        RegCloseKey(hSubKey);


        dwBufSize = 256;
    } /* end of while */

    RegCloseKey(hKey);

    return TRUE;
}


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

int GetInterfacesFunction()
{
    DWORD dwNumIf;
    DWORD  dwRetVal;
    if(dwRetVal = GetNumberOfInterfaces(&dwNumIf) == NO_ERROR){
        LOGE("GetNumberOfInterfaces %d", dwNumIf);
    } else {
        LOGE("GetNumberOfInterfaces() FAILED");
    }

    PIP_INTERFACE_INFO pInfo;
    ULONG ulOutBufLen = 0;
    int iReturn = 1;

    dwRetVal = GetInterfaceInfo(NULL, &ulOutBufLen);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pInfo = (IP_INTERFACE_INFO *) MALLOC(ulOutBufLen);
        if (pInfo == NULL) {
            LOGE("MALLOC FAILED");
            return 1;
        }
    }

    dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen);
    if (dwRetVal == NO_ERROR) {
        LOGE("ADAPTER NUMBER : %ld", pInfo->NumAdapters);
        for (int i = 0; i < (int) pInfo->NumAdapters; i++) {
            LOGE("ADAPTER INDEX [%d]: 0x%lx", i,
                   pInfo->Adapter[i].Index);
            LOGE("ADAPTER NAME [%d]: %ws", i,
                   pInfo->Adapter[i].Name);
        }
        iReturn = 0;
    } else if (dwRetVal == ERROR_NO_DATA) {
        LOGE("NONE ADAPTER SUPPORTSIPv4");
        iReturn = 0;
    } else {
        LOGE("GetInterfaceInfo FAILED %d", dwRetVal);
        iReturn = 1;
    }

    FREE(pInfo);
    return (iReturn);
}

bool NicManager::GetAdapter()
{
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    GetInterfacesFunction();

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
                    LOGE("Adapter %s, IP %s, gateway %s",mAdapterName.c_str(),
                        device_ip.c_str(), gateway_ip.c_str());
                        //, pIpAdapterInfo->Address  MAC ADDRESS
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
#endif

BOOL NicManager::RegReadAdapter(const char* driver, string &adapter) {
  HKEY hKey, hSubKey, hNdiIntKey;
    DWORD dwBufSize = 256;
    DWORD dwDataType;
    unsigned char szData[256] = {0};
    BOOL result = FALSE;

    ASSERT(driver != NULL);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}",
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS) {
        LOGE("RegOpenKeyEx 'System\\CurrentControlSet\\Control\\Class' failed");
        return result;
    }

    if(RegOpenKeyEx(hKey, driver, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
    {
        LOGE("RegOpenKeyEx '%s' failed", driver);
RegCloseKey(hKey);
return result;
    }
    if(RegOpenKeyEx(hSubKey, "Ndi\\Interfaces", 0, KEY_READ, &hNdiIntKey) != ERROR_SUCCESS)
    {
        LOGE("RegOpenKeyEx 'Ndi\\Interfaces' failed");
RegCloseKey(hKey);
        RegCloseKey(hSubKey);
        return result;
    }
    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hNdiIntKey, "LowerRange", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
    {
        LOGE("'LowerRange' of %s is '%s'", driver, szData);
        if(strcmp((char*)szData, "ethernet") == 0)//判断是不是以太网卡
        {
                dwBufSize = sizeof szData;
                if(RegQueryValueEx(hSubKey, "NetCfgInstanceID", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
                {
                    adapter = (char *)szData;
                    result = TRUE;
                }
        }
    } else {
        LOGE("get register key 'LowerRange' value failed");
    }
    RegCloseKey(hNdiIntKey);
    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    return result;
}

BOOL NicManager::RegGetIP(const string & adapter, string& ip, string &subnetMask, string& gateway)
{
    HKEY hKey;
    string strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";

    if (adapter.length() <=0) {
        LOGE("adapter is empty");
        return FALSE;
    }

    strKeyName += adapter;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    strKeyName.c_str(),
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
        return FALSE;

    unsigned char szData[256];
    DWORD dwDataType, dwBufSize;

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey, "IPAddress", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        ip = (LPCTSTR)szData;

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey, "SubnetMask", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        subnetMask = (LPCTSTR)szData;

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey, "DefaultGateway", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        gateway = (LPCTSTR)szData;

    RegCloseKey(hKey);
    return TRUE;
}

BOOL NicManager::RegReadConnectName(const string & adapter, string& name) {
    HKEY hKey;
    CString keyPath;
    BYTE value[MAX_PATH];
    DWORD nSize=sizeof value;
    BOOL result = FALSE;

    if (adapter.length() <=0) {
        LOGE("adapter is empty");
        return result;
    }

    keyPath.Format("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\"
                   "%s\\Connection", adapter.c_str());
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    keyPath.GetString(),
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
        return result;

    if (RegQueryValueEx(hKey, "Name", NULL, NULL, (LPBYTE)&value, &nSize) == ERROR_SUCCESS) {
        name = (char *)value;
        result = TRUE;
    } else {
        result = FALSE;
    }
    RegCloseKey(hKey);
    return result;
}

int NicManager::RegSetMultisz(HKEY hKey, LPCSTR lpValueName, CONST CHAR* lpValue) {
    int cbData;
    CHAR *pData = NULL;
    ASSERT(lpValue != NULL);
    cbData = strlen(lpValue) + 2;
    pData = (CHAR *)malloc(cbData);
    if (pData == NULL) {
        LOGE("OOPS, malloc failed");
        return 1;
    }
    strcpy(pData, lpValue);
    pData[cbData - 1] = '\0';
    RegSetValueEx(hKey, lpValueName, 0, REG_MULTI_SZ, (const BYTE *)pData, cbData);

    free(pData);
    return 0;
}

BOOL NicManager::RegSetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, DWORD enableDHCP)
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

    int result = 0;
    result += RegSetMultisz(hKey, "IPAddress", pIPAddress);
    result += RegSetMultisz(hKey, "SubnetMask", pNetMask);
    result += RegSetMultisz(hKey, "DefaultGateway", pNetGate);
    //RegSetValueEx(hKey, "NameServer", );

    result += RegSetValueEx(hKey, "EnableDHCP", 0, REG_DWORD, (unsigned char*)&enableDHCP, sizeof(DWORD) );
    RegCloseKey(hKey);

    return (result == 0);
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

// SPDRP_DRIVER value extract from register path, for PCI
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\PCI\VEN_10EC&DEV_8168&SUBSYS_3670103C&REV_06\4&3b992247&0&00E1
//key name  "Driver",  value is "{4D36E972-E325-11CE-BFC1-08002BE10318}\0008"
// OR for USB
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\Vid_05c6&Pid_9025&MI_04\6&1cf55e67&1&0004
//key "Driver", value is "{4D36E972-E325-11CE-BFC1-08002BE10318}\0041"
//
//according to "Driver" value, we can find net class path to get NIC information
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002bE10318}\0008
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002bE10318}\0041
//key "ComponentId" or "MatchingDeviceId" value is "usb\vid_05c6&pid_9025&mi_04" , "pci\ven_10ec&dev_8168&rev_06" respectively.
// get key "NetCfgInstanceID" value, this is the mAdapterName member value, for example "{9FFD1018-51D8-4A42-8167-E40813931790}".
// This is the bridger from SetDi* function & IP function.
//set/get ip address through
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\{9FFD1018-51D8-4A42-8167-E40813931790}
//get connection name from
//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Network\{4D36E972-E325-11CE-BFC1-08002BE10318}\{9FFD1018-51D8-4A42-8167-E40813931790}\Connection
//als exact "PCI\VEN_10EC&DEV_8168&SUBSYS_3670103C&REV_06\4&3B992247&0&00E1" by key "PnpInstanceID", this
//is what we get by SetDi* function.
void NicManager::EnumNetCards(list<TNetCardStruct> *NetDeviceList)
{
    NetCardStruct nic;
    DWORD  Status, Problem, code;
    LPTSTR name = NULL;
    LPTSTR driver = NULL;
    HDEVINFO hDevInfo = 0;
    BOOL result;

    //hDevInfo=SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    hDevInfo= SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

    if(INVALID_HANDLE_VALUE==hDevInfo)
        return;

    SP_DEVINFO_DATA  diData ={sizeof(SP_DEVINFO_DATA)};

    for(DWORD DeviceId=0;
        SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&diData);
        DeviceId++)
    {
        if ((code = CM_Get_DevNode_Status(&Status, &Problem, diData.DevInst,0)) != CR_SUCCESS) {
            LOGE("CM_Get_DevNode_Status return %d", code);
            continue;
        }

        if ((Status & DN_NO_SHOW_IN_DM) == DN_NO_SHOW_IN_DM) {
            LOGW("this device is not in device manager");
            continue;
        }

        if ((Status & DN_DISABLEABLE) != DN_DISABLEABLE) {
            LOGW("this device is not disableable.");
            continue;
        }

        //Net or System, etc.,
        //GetRegistryProperty(hDevInfo, &diData, SPDRP_CLASS , &Buffer);
        //"ROOT" OR "PCI"
        //GetRegistryProperty(hDevInfo, &diData, SPDRP_ENUMERATOR_NAME , &Buffer);


        if (GetRegistryProperty(hDevInfo, &diData, SPDRP_DEVICEDESC, &name) &&
            GetRegistryProperty(hDevInfo, &diData, SPDRP_DRIVER , &driver)) {
            nic.Id = DeviceId;
            nic.Name = name;
            nic.driver = driver;
            //IT IS ALWAYS ENABLED.
            nic.Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem);
            nic.Changed = false;
            result = RegReadAdapter(driver, nic.mAdapterName);
            LOGE("SPDRP_HARDWAREID %s", driver);
            if (result) {
                RegGetIP(nic.mAdapterName, nic.mIPAddress, nic.mSubnetMask, nic.mGateway);
                RegReadConnectName(nic.mAdapterName, nic.mConnectionName);

                NetDeviceList->push_back(nic);
            }
        }

        if (driver != NULL) {
            LocalFree(driver);
            driver = NULL;
        }
        if (name != NULL) {
            LocalFree(name);
            name = NULL;
        }
    }
}

//---------------------------------------------------------------------------
ULONG NicManager::GetRegistryProperty(HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    ULONG Property,
    LPTSTR *Buffer)
{
    ASSERT(Buffer != NULL);
    ULONG Length = 0;
    while (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
        DeviceInfoData, Property, NULL, (PBYTE)(*Buffer), Length, &Length))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            if (*Buffer) LocalFree(*Buffer);
            *Buffer = (LPTSTR)LocalAlloc(LPTR, Length);
        }
        else
        {
            return 0;
        }
    }
    return Length;
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
    //hDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT |DIGCF_ALLCLASSES);
    hDevInfo= SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

    if (INVALID_HANDLE_VALUE == hDevInfo)
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


#undef USE_NETSH
// Start an explorer window, directory is Tftpd32's default directory
BOOL NicManager::SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask)
{
#ifdef USE_NETSH
    CString connectionName;
    CString command;
    GetConnectName(connectionName);
    command.Format("netsh int ip set address name=\"%s\" source=static %s %s %s 1",
                   connectionName.GetString(), ip, subnetMask, gateway);
    int rc = ExecuteCommand((LPSTR)command.GetString());

    device_ip = ip;
    gateway_ip = gateway;
    return rc == 0;
#else
    if(!RegSetIP(ip, subnetMask, gateway, 0))
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

    device_ip = ip;
    gateway_ip = gateway;
    return TRUE;
#endif
} // StartExplorer

BOOL NicManager::EnableDhcp() {
#ifdef USE_NETSH
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
#else

    if(!RegSetIP("0.0.0.0","0.0.0.0", "0.0.0.0", 1))
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
    device_ip.clear();
    gateway_ip.clear();
    GetAdapter();
    return TRUE;
#endif
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
