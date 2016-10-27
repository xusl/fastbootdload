#include "StdAfx.h"
#include "NicManager.h"
#include <stdio.h>
#include "log.h"
#include <Icmpapi.h>
#include <devguid.h>
#include <Wininet.h>
#include <Sensapi.h>
#include <mprapi.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma	comment(lib, "setupapi.lib")
#pragma	comment(lib, "Wininet.lib")
#pragma	comment(lib, "Sensapi.lib")
//#pragma comment(lib,"icmp.lib")

#define MAX_BUF_SIZE   300
#undef USE_NETSH

using namespace std;

NicManager::NicManager(string network):
    segment(network)
{
    mNicList.clear();
}

NicManager::~NicManager(void)
{
    mNicList.clear();
}

bool NicManager::Ping(const char *ip_addr) {
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

BOOL NicManager::CheckIpInArpTable(const char *ip, string & mac)
{
     ULONG nSize = 0;
     DWORD dwRet = 0;
     BOOL result = FALSE;

     ASSERT(ip != NULL);

    dwRet = GetIpNetTable(NULL, &nSize, TRUE);
    if (dwRet == ERROR_NO_DATA || nSize == 0) {
        LOGE("There are date in arp table, return %d, size %d", dwRet, nSize);
        return FALSE;
    }
    if (dwRet != ERROR_INSUFFICIENT_BUFFER && dwRet != NO_ERROR) {
        LOGE("GetIpNetTable occur error %d", dwRet);
        return FALSE;
    }
    PMIB_IPNETTABLE pMib = (PMIB_IPNETTABLE)malloc(sizeof(MIB_IPNETTABLE)+sizeof(MIB_IPNETROW)*nSize);
    memset(pMib, 0, sizeof(MIB_IPNETTABLE)+sizeof(MIB_IPNETROW)*nSize);
    if (pMib == NULL) {
        LOGE("NO memory");
        return FALSE;
    }

    dwRet = GetIpNetTable(pMib,&nSize,TRUE);

     if (dwRet != NO_ERROR) {
        free(pMib);
        LOGE("GetIpNetTable error, return %d, size %d", dwRet, nSize);
        return FALSE;
    }

	LOGD("Internet Address      Physical Address         Type");
    for (int i = 0; i < nSize; i++) {
		char ipaddr[20] = {0}, macaddr[20] = {0};
        char *pType = "Unknown";

		sprintf(ipaddr,"%d.%d.%d.%d",
			    ( pMib->table[i].dwAddr&0x0000ff),
			    ((pMib->table[i].dwAddr&0xff00)>>8),
                ((pMib->table[i].dwAddr&0xff0000)>>16),
                (pMib->table[i].dwAddr>>24));

		sprintf(macaddr, "%02x-%02x-%02x-%02x-%02x-%02x",
			   pMib->table[i].bPhysAddr[0],pMib->table[i].bPhysAddr[1],
			   pMib->table[i].bPhysAddr[2],pMib->table[i].bPhysAddr[3],
			   pMib->table[i].bPhysAddr[4],pMib->table[i].bPhysAddr[5]
		);
        if ( MIB_IPNET_TYPE_DYNAMIC == pMib->table[i].dwType)
            pType = "Dynamic";
        else if (MIB_IPNET_TYPE_STATIC == pMib->table[i].dwType)
            pType = "Static";
        LOGD("%-20s  %-25s  %-20s",ipaddr,macaddr, pType);
        if (strcmp(ip, ipaddr) == 0) {
            mac = macaddr;
            result = TRUE;
            break;
        }
    }

    free(pMib);
    return result;
}

int NicManager::ResolveIpMac(const char *DestIpString, string & mac)
{
#define BUFFER_LEN  6
    DWORD dwRetVal;
    IPAddr DestIp = 0;
    IPAddr SrcIp = 0;       /* default for src ip , means INADDR_ANY IPv4 address*/
    ULONG MacAddr[2];       /* for 6-byte hardware addresses */
    ULONG PhysAddrLen = 6;  /* default to length of six bytes */
    CHAR  Buffer[BUFFER_LEN];
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
        case ERROR_NETWORK_UNREACHABLE:
            errstr = _T("ERROR_NETWORK_UNREACHABLE");
            break;
        case ERROR_GEN_FAILURE:
            errstr = _T("ERROR_GEN_FAILURE");
            break;
        case ERROR_INVALID_PARAMETER:
            errstr = _T("ERROR_INVALID_PARAMETER");
            break;
        case ERROR_INVALID_USER_BUFFER:
            errstr = _T("ERROR_INVALID_USER_BUFFER");
            break;
        case ERROR_BAD_NET_NAME:
            errstr = _T("ERROR_BAD_NET_NAME");
            break;
        case ERROR_BUFFER_OVERFLOW:
            errstr = _T("ERROR_BUFFER_OVERFLOW");
            break;
        case ERROR_NOT_FOUND:
            errstr = _T("ERROR_NOT_FOUND");
            break;
        case ERROR_NO_NETWORK:
            errstr = _T("ERROR_NO_NETWORK");
            break;
        default:
            break;
        }
        LOGE("Error: SendArp failed with error: %d(%s)", dwRetVal, errstr);
    }

    return dwRetVal;
}

BOOL NicManager::GetConnectedState() {
    DWORD flag;
    //TCHAR connectionName[256] = {0};
    BOOL result;
    DWORD dwSize = 0;
    MIB_IFTABLE *mit = NULL;
    INTERNAL_IF_OPER_STATUS operStatus = IF_OPER_STATUS_NON_OPERATIONAL;

    result = IsNetworkAlive(&flag);

    LOGE("Get network alive %d, flag 0x%x", result, flag);

    //result = InternetGetConnectedStateEx(&flag, connectionName, sizeof connectionName, NULL);
    //if (result == FALSE)
    //    return result;
    //LOGE("Get connection status 0x%x", flag);

    GetIfTable(NULL, &dwSize, true);
    mit = (MIB_IFTABLE*) new BYTE[dwSize];

    if(NO_ERROR != GetIfTable(mit, &dwSize, true))
    {
        delete []mit;
        return FALSE;
    }

    for(DWORD i = 0; i < mit->dwNumEntries; i++) {
        PMIB_IFROW pRow = mit->table+ i;
        if (m_DefaultNic.mNicDesc == (char*) pRow->bDescr ||
            0 == memcmp(m_DefaultNic.mPhysAddr, pRow->bPhysAddr, sizeof pRow->bPhysAddr)) {
            operStatus = pRow->dwOperStatus;
            LOGD("Get %s status %d", pRow->bDescr, operStatus);
        }
    }

    delete []mit;
    return operStatus == IF_OPER_STATUS_OPERATIONAL;
}

/*deprecated Now.
*/
BOOL NicManager::SwitchNic(NetCardStruct &netCard, bool Enabled) {
    DWORD dwSize = 0;
    BOOL result = FALSE;
    MIB_IFTABLE *mit = NULL;
    INTERNAL_IF_OPER_STATUS operStatus = IF_OPER_STATUS_NON_OPERATIONAL;

    GetIfTable(NULL, &dwSize, true);
    mit = (MIB_IFTABLE*) new BYTE[dwSize];

    if(NO_ERROR != GetIfTable(mit, &dwSize, true)) {
        delete []mit;
        return result;
    }

    for(DWORD i = 0; i < mit->dwNumEntries; i++) {
        PMIB_IFROW pRow = mit->table+ i;
        if (netCard.mNicDesc != (char*) pRow->bDescr &&
            memcmp(netCard.mPhysAddr, pRow->bPhysAddr, 6)) {
            continue;
        }

        mit->table[i].dwAdminStatus = Enabled ? MIB_IF_ADMIN_STATUS_UP : MIB_IF_ADMIN_STATUS_DOWN;
        if(NO_ERROR == SetIfEntry(&mit->table[i])) {
            LOGE("%s adapter succed!", Enabled ? "Enable" : "Disable");
            result = TRUE;
        } else {
            LOGE("%s adapter failed!", Enabled ? "Enable" : "Disable");
        }

        break;
    }

    delete []mit;
    return result;
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
void CIpChangeDlg::GetConnectNames()
{
    /*******************************************
    *通过mprapi库获取连接名称
    *并通过index将网卡信息和连接名称相关联
    ********************************************/
    HANDLE   hMprConfig;                    //连接信息的句柄
    DWORD   dwRet=0;                        //返回值
    PIP_INTERFACE_INFO   plfTable = NULL;   //接口信息表
    DWORD   dwBufferSize=0;                 //接口信息表空间大小

    m_AdapterInfo.csConnectName = new char [m_AdapterInfo.iCount] [256];  //申请空间存储连接名

    dwRet = MprConfigServerConnect(NULL, &hMprConfig);  //获得句柄
    dwRet = GetInterfaceInfo(NULL, &dwBufferSize);      //获得接口信息表大小

    if(dwRet == ERROR_INSUFFICIENT_BUFFER)              //获得接口信息
    {
        plfTable = (PIP_INTERFACE_INFO)HeapAlloc(GetProcessHeap(),
                                                  HEAP_ZERO_MEMORY, dwBufferSize);
        GetInterfaceInfo(plfTable, &dwBufferSize);
    }


    TCHAR   szFriendName[256];                   //接口名称
    DWORD   tchSize = sizeof(TCHAR) * 256;
    ZeroMemory(&szFriendName, tchSize);

    for (UINT i = 0; i < plfTable-> NumAdapters; i++)
    {
        IP_ADAPTER_INDEX_MAP   AdaptMap;         //接口信息
        AdaptMap = plfTable->Adapter[i];

        dwRet = MprConfigGetFriendlyName(hMprConfig, AdaptMap.Name,
            (PWCHAR)szFriendName, tchSize);      //获得连接名称unicode
        USES_CONVERSION;
        char* pName = W2A((PWCHAR)szFriendName);                           //转换为ansi

        InsertConnectName(AdaptMap.Index, pName);                          //根据Index存储名字信息
    }
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, plfTable);
}

//
void NotifyAddrChangeTest()
{
  OVERLAPPED overlap;
  DWORD ret;

  HANDLE hand = NULL;
  overlap.hEvent = WSACreateEvent();

  ret = NotifyAddrChange(&hand, &overlap);

  if (ret != NO_ERROR)
  {
    if (WSAGetLastError() != WSA_IO_PENDING)
    {
      printf("NotifyAddrChange error...%d\n", WSAGetLastError());
      return;
    }
  }

  if ( WaitForSingleObject(overlap.hEvent, INFINITE) == WAIT_OBJECT_0 )
    printf("IP Address table changed..\n");
}
#endif

BOOL NicManager::GetNicInfo(NetCardStruct &netCard) {
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    BOOL result = FALSE;
    //GetInterfacesFunction();

    if (ERROR_BUFFER_OVERFLOW == nRel) {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }

    if (ERROR_SUCCESS != nRel) {
        LOGE("GetAdaptersInfo failed, return %d", nRel);
        goto GETADAPTEROUT;
    }

    PIP_ADAPTER_INFO pInfo=pIpAdapterInfo;
    //PIP_ADDR_STRING current = pIpAdapterInfo->CurrentIpAddress;
    while (pInfo) {
        IP_ADDR_STRING *pIpAddrString =&(pInfo->IpAddressList);
        IP_ADDR_STRING *pGateway = &pInfo->GatewayList;
        //do {
        //    //if (pInfo->Type == MIB_IF_TYPE_ETHERNET)
        //    // if(device_ip.find(segment) != -1)
        if (netCard.mAdapterName == pIpAdapterInfo->AdapterName) {
            netCard.mNicDesc = pInfo->Description;
            netCard.mIPAddress =pIpAddrString->IpAddress.String;
            netCard.mGateway = pGateway->IpAddress.String;
            netCard.mEnableDHCP = pInfo->DhcpEnabled;
            memcpy(netCard.mPhysAddr, pInfo->Address, sizeof netCard.mPhysAddr);
            delete pIpAdapterInfo;
            return TRUE;
        }
        //    pIpAddrString=pIpAddrString->Next;
        //    pGateway = pGateway->Next;
        //} while (pIpAddrString && pGateway);
        pInfo = pInfo->Next;
    }

    LOGE("Can not update adapter %s information", netCard.DeviceDesc);

GETADAPTEROUT:
    if (pIpAdapterInfo) {
        delete pIpAdapterInfo;
    }
    return result;
}

BOOL NicManager::RegReadAdapter(const char* driver, string &adapter) {
    HKEY hKey, hSubKey, hNdiIntKey;
    DWORD dwBufSize = 256;
    DWORD dwDataType;
    unsigned char szData[256] = {0};
    BOOL result = FALSE;

    ASSERT(driver != NULL);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "System\\CurrentControlSet\\Control\\Class\\",//{4d36e972-e325-11ce-bfc1-08002be10318}",
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

BOOL NicManager::RegGetIP(const string & adapter, string& ip, string &subnetMask, string& gateway, BOOL& enableDHCP)
{
    HKEY hKey;
    BOOL result = TRUE;
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
                    &hKey) != ERROR_SUCCESS) {
        LOGE("OpenKey %s failed", strKeyName.c_str());
        return FALSE;
    }

    unsigned char szData[256];
    DWORD dwDataType, dwBufSize;

    DWORD value;
    dwBufSize = sizeof DWORD;
    if(RegQueryValueEx(hKey, "EnableDHCP", 0, &dwDataType, (LPBYTE)&value, &dwBufSize) == ERROR_SUCCESS)
        enableDHCP = (value != 0);
    else {
        LOGE("Query %s failed", "DefaultGateway");
        result = FALSE;
    }

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey,enableDHCP ? "DhcpIPAddress" : "IPAddress",
        0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        ip = (LPCTSTR)szData;
    else {
        LOGE("Query %s failed", "IPAddress");
        result = FALSE;
    }

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey, enableDHCP ? "DhcpSubnetMask" : "SubnetMask",
        0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        subnetMask = (LPCTSTR)szData;
    else {
        LOGE("Query %s failed", "SubnetMask");
        result = FALSE;
    }

    dwBufSize = sizeof szData;
    if(RegQueryValueEx(hKey, enableDHCP ? "DhcpDefaultGateway" : "DefaultGateway",
        0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
        gateway = (LPCTSTR)szData;
    else {
        LOGE("Query %s failed", "DefaultGateway");
        result = FALSE;
    }

    RegCloseKey(hKey);
    return result;
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

BOOL NicManager::RegSetIP(const string & adapter, LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, DWORD enableDHCP)
{
    HKEY hKey;
    CString keyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    keyName += adapter.c_str();
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    keyName.GetString(),
                    0,
                    KEY_WRITE,
                    &hKey) != ERROR_SUCCESS) {
        LOGE("Open %s failed", keyName.GetString());
        return FALSE;
    }

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
void NicManager::EnumNetCards()
{
    DWORD  Status, error, code;
    LPTSTR name = NULL;
    LPTSTR driver = NULL;
    HDEVINFO hDevInfo = 0;
    BOOL result;

    //hDevInfo=SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    hDevInfo= SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

    if(INVALID_HANDLE_VALUE==hDevInfo)
        return;

    SP_DEVINFO_DATA  diData ={sizeof(SP_DEVINFO_DATA)};
    mNicList.clear();
    m_DefaultNic.Reset();

    for(DWORD DeviceId=0;
        SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&diData);
        DeviceId++) {
        if ((code = CM_Get_DevNode_Status(&Status, &error, diData.DevInst,0)) != CR_SUCCESS) {
            LOGE("CM_Get_DevNode_Status return %d", code);
            continue;
        }

        if ((Status & DN_NO_SHOW_IN_DM) == DN_NO_SHOW_IN_DM) {
            //LOGW("this device is not in device manager");
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
            NetCardStruct nic;
            nic.Id = DeviceId;
            nic.DeviceDesc = name;
            nic.driver = driver;
            //IT IS ALWAYS ENABLED.
            nic.Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == error);
            nic.Changed = false;
            result = RegReadAdapter(driver, nic.mAdapterName);
            LOGE("SPDRP_DRIVER %s, SPDRP_DEVICEDESC %s", driver, name);
            if (result) {
                result = GetNicInfo(nic);
            }
            if (result)
            {
            //RegGetIP(nic.mAdapterName, nic.mIPAddress, nic.mSubnetMask, nic.mGateway, nic.mEnableDHCP);
                RegReadConnectName(nic.mAdapterName, nic.mConnectionName);
            }
            if (result)
                mNicList.push_back(nic);
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
    if(!mNicList.empty()) {
        m_DefaultNic = mNicList.front();
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
}

BOOL NicManager::SetDefaultNic(DWORD id) {
    list<NetCardStruct>::iterator it;
    if (id == m_DefaultNic.Id) {
        LOGD("Do not need update default NIC");
        return FALSE;
    }

    for (it = mNicList.begin(); it != mNicList.end(); ++it) {
        if (id == it->Id) {
            m_DefaultNic = *it;
            LOGD("Update default NIC %s", m_DefaultNic.DeviceDesc.c_str());
            return TRUE;
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
ULONG NicManager::GetRegistryProperty(HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA diData,
    ULONG Property,
    LPTSTR *Buffer)
{
    ASSERT(Buffer != NULL);
    ULONG Length = 0;
    while (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
        diData, Property, NULL, (PBYTE)(*Buffer), Length, &Length))
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
bool NicManager::NetCardStateChange(NetCardStruct &NetCard, bool Enabled)
{
    bool result = false;
    SP_PROPCHANGE_PARAMS params = {sizeof(SP_CLASSINSTALL_HEADER)};
    DWORD DeviceId = NetCard.Id;
    HDEVINFO hDevInfo = 0;
    //hDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT |DIGCF_ALLCLASSES);
    hDevInfo= SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

    if (INVALID_HANDLE_VALUE == hDevInfo) {
        LOGE("SetupDiGetClassDevs return invalid handle");
        return false;
    }

    SP_DEVINFO_DATA diData = {sizeof(SP_DEVINFO_DATA)};
    DWORD Status, error;
    if (!SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&diData)) {
        LOGE("SetupDiEnumDeviceInfo return ERROR");
        goto NICSTATUSCHANGEOUT;
    }

    if (CM_Get_DevNode_Status(&Status, &error, diData.DevInst,0) != CR_SUCCESS) {
        LOGE("CM_Get_DevNode_Status is return error");
        goto NICSTATUSCHANGEOUT;
    }

    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    //params.Scope = DICS_FLAG_GLOBAL;
    params.Scope = DICS_FLAG_CONFIGSPECIFIC;
    params.HwProfile = 0;
    if (Enabled) {
        if (!((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == error))) {
            NetCard.Disabled = false;
            LOGE("ERROR1, status 0X%X, error %d", Status, error);
            goto NICSTATUSCHANGEOUT;
        }
        params.StateChange = DICS_ENABLE;
    } else {
        if ((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == error)) {
            NetCard.Disabled = true;
            LOGE("ERROR2, status 0X%X, error %d", Status, error);
            goto NICSTATUSCHANGEOUT;
        }
        if (!((Status & DN_DISABLEABLE) && (CM_PROB_HARDWARE_DISABLED != error))) {
            LOGE("ERROR3, status 0x%X, error %d", Status, error);
            goto NICSTATUSCHANGEOUT;
        }
        params.StateChange = DICS_DISABLE;
    }

    if (!SetupDiSetClassInstallParams(hDevInfo, &diData, &params.ClassInstallHeader, sizeof(params))) {
        LOGE("SetupDiSetClassInstallParams error");
        goto NICSTATUSCHANGEOUT;
    }

    //if (diData.DevInst != NULL) {
    //    LOGE("DevInst is NULL");
    //}
    /*
    In Win7 x64, must use x64 build. Otherwise it works inproperly.
    */
    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &diData)) {
        LOGE("SetupDiCallClassInstaller error: 0x%X", GetLastError());
        goto NICSTATUSCHANGEOUT;
    }

    if (CM_Get_DevNode_Status(&Status, &error, diData.DevInst,0) == CR_SUCCESS)
        NetCard.Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == error);
    result = true;

NICSTATUSCHANGEOUT:
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return result;
}

BOOL NicManager::UpdateIP() {
    list<NetCardStruct>::iterator it;
    BOOL result = TRUE;

//#define NIC_IPUPDATE_TEST

#ifdef NIC_IPUPDATE_TEST
    for (it = mNicList.begin(); it != mNicList.end(); ++it) {
        LOG("Now %s, %s, %s", it->mConnectionName.c_str(),
            it->mIPAddress.c_str(), it->mGateway.c_str());
    }
    LOG("Default Now %s, %s, %s", m_DefaultNic.mConnectionName.c_str(),
            m_DefaultNic.mIPAddress.c_str(), m_DefaultNic.mGateway.c_str());
#endif

    for (it = mNicList.begin(); it != mNicList.end(); ++it) {
/*        result = result && RegGetIP(it->mAdapterName,
            it->mIPAddress,
            it->mSubnetMask,
            it->mGateway,
            it->mEnableDHCP);
*/
        result = GetNicInfo(*it) && result;
    }

/*
    result = result && RegGetIP(m_DefaultNic.mAdapterName,
        m_DefaultNic.mIPAddress,
        m_DefaultNic.mSubnetMask,
        m_DefaultNic.mGateway,
        m_DefaultNic.mEnableDHCP);
   */
    result = GetNicInfo(m_DefaultNic) && result;

#ifdef NIC_IPUPDATE_TEST
    for (it = mNicList.begin(); it != mNicList.end(); ++it) {
        LOG("Updated %s, %s, %s", it->mConnectionName.c_str(),
            it->mIPAddress.c_str(), it->mGateway.c_str());
    }
    LOG("Default Updated %s, %s, %s", m_DefaultNic.mConnectionName.c_str(),
            m_DefaultNic.mIPAddress.c_str(), m_DefaultNic.mGateway.c_str());
#endif

    return result;
}
#define DISETUP_SWITCH
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
#elif defined(DISETUP_SWITCH)
    if (m_DefaultNic.IsInvalid()) {
        LOGE("There are no valid NIC");
        return FALSE;
    }
    if(!RegSetIP(m_DefaultNic.mAdapterName, ip, subnetMask, gateway, 0)) {
        LOGE("RegSetIP failed");
        return FALSE;
    }

    //通知IP地址的改变(此方法会造成栈溢出问题，而且对于设置dhcp的立即生效没有作用，故舍弃)
    //if(!NotifyIPChange(lpszAdapterName, nIndex, pIPAddress, pNetMask))
    //  return FALSE;

    //通过禁用启用网卡实现IP立即生效
    NetCardStateChange(m_DefaultNic, FALSE);
    Sleep(100);
    NetCardStateChange(m_DefaultNic,TRUE);

    Sleep(10000);
    UpdateIP();
    return TRUE;
#else
    if (m_DefaultNic.IsInvalid()) {
        LOGE("There are no valid NIC");
        return FALSE;
    }
    SwitchNic(m_DefaultNic, FALSE);
    if(!RegSetIP(m_DefaultNic.mAdapterName, ip, subnetMask, gateway, 0)) {
        LOGE("RegSetIP failed");
        return FALSE;
    }
    SwitchNic(m_DefaultNic, TRUE);
    Sleep(8000);
    UpdateIP();
    return TRUE;
#endif
} // StartExplorer

BOOL NicManager::EnableDhcp(BOOL updateIp) {
#ifndef USE_NETSH
    if (m_DefaultNic.IsInvalid()) {
        LOGE("There are no valid NIC");
        return FALSE;
    }
    if(!RegSetIP(m_DefaultNic.mAdapterName, "0.0.0.0","0.0.0.0", "0.0.0.0", 1)) {
        LOGE("RegSetIP failed");
        return FALSE;
    }

    //通知IP地址的改变(此方法会造成栈溢出问题，而且对于设置dhcp的立即生效没有作用，故舍弃)
    //if(!NotifyDHCPIPChange(lpszAdapterName, nIndex))
    //  return FALSE;

    //通过禁用启用网卡实现IP立即生效
    NetCardStateChange(m_DefaultNic,FALSE);
    Sleep(100);
    NetCardStateChange(m_DefaultNic,TRUE);

    if (updateIp == FALSE)
        return TRUE;

    int i = 0;
    BOOL result;
    do {
        Sleep(5000);
        result = UpdateIP();
    } while(i++< 4 && !result);
    return TRUE;

#else
    CString connectionName;
    CString command;
    if (FALSE == GetConnectName(connectionName))
        return FALSE;
    command.Format("netsh int ip set addr \"%s\" dhcp ", connectionName.GetString());
    int rc = ExecuteCommand((LPSTR)command.GetString());
    //command.Format("netsh int ip set dns \"%s\" dhcp", connectionName.GetString());
    //rc += ExecuteCommand((LPSTR)command.GetString());
    GetAdapter();
    return rc == 0;
#endif
}

int NicManager::ExecuteCommand(LPSTR lpCommandLine) {
    int Rc = 0;
#if 0
    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    memset (& sInfo, 0, sizeof sInfo);
    sInfo.cb = sizeof sInfo;

    LOGE("Execute command %s", lpCommandLine);

    Rc = CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE,
                       NORMAL_PRIORITY_CLASS, NULL, NULL, &sInfo, &pInfo) ;
    CloseHandle (pInfo.hProcess);
    CloseHandle (pInfo.hThread);
#else
    //初始化shellexe信息
    SHELLEXECUTEINFO   ExeInfo;
    ZeroMemory(&ExeInfo, sizeof(SHELLEXECUTEINFO));
    ExeInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ExeInfo.lpFile = "cmd.exe";
    ExeInfo.lpParameters = lpCommandLine;
    ExeInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ExeInfo.nShow = SW_HIDE;
    ExeInfo.hwnd = NULL;
    ExeInfo.lpVerb = NULL;
    ExeInfo.lpDirectory = NULL;
    ExeInfo.hInstApp = NULL;

    //执行命令
    ShellExecuteEx(&ExeInfo);

    //等待进程结束
    WaitForSingleObject(ExeInfo.hProcess, INFINITE);

#endif
    return Rc;
}
