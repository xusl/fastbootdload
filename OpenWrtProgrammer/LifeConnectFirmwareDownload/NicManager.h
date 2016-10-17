#pragma once
#include <string>
#include<iostream>
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <vector>
#include <list>

using namespace std;

typedef int (CALLBACK* DHCPNOTIFYPROC)(LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, int);

typedef struct tagAdapterInfo
{
  string strName;// 适配器名称
  string strDriverDesc;// 适配器描述
  string strIP; // IP地址
  string strNetMask;// 子网掩码
  string strNetGate;// 网关
}ADAPTER_INFO;
typedef struct  NetCardStruct
{
    DWORD    Id;         // 网卡设备号
    string   Name;     // 网卡名
    string   deviceAddress;
    bool     Disabled;     // 当前是否禁用
    bool     Changed;         // 是否更改过
}TNetCardStruct;
typedef TNetCardStruct*  PNetCardStruct;

bool Ping(const char *ip_addr);
int ResolveIpMac(const char *DestIpString, string & mac);

typedef LPVOID *PPVOID;

class NicManager
{
public:
    NicManager(string network="192.168.1");
    ~NicManager(void);
    bool GetAdapter();
    BOOL EnableDhcp();
    int SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask);
    bool GetHostIP(string& ip, string& gw) {
        int refresh = 1;
        do {
            if (!device_ip.empty() &&(device_ip !="")&&(device_ip !="0.0.0.0")) {
                ip = device_ip;
                gw = gateway_ip;
                return true;
            }
            if (refresh >= 1) {
                if(!GetAdapter())
                    return false;
            }
        } while(refresh-- >= 0);
        return false;
    }

  BOOL NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex);
  BOOL GetAdapterInfo();
  BOOL GetConnectName(CString& name);
private:
    int ExecuteCommand(LPSTR lpCommandLine);
  BOOL RegSetDHCPIP();
  BOOL RegGetIP(ADAPTER_INFO *pAI);
    BOOL RegSetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate);
  BOOL RegSetMultisz(HKEY hKey, LPCSTR lpValueName, CONST CHAR* lpValue);

  void EnumNetCards(list<TNetCardStruct>  *NetDeviceList);
  bool NetCardStateChange(PNetCardStruct NetCardPoint, bool Enabled);
  bool GetRegistryProperty(HDEVINFO DeviceInfoSet,
                                    PSP_DEVINFO_DATA DeviceInfoData,
                                    ULONG Property,
                                    LPTSTR *Buffer,
                                    PULONG Length);
private:
    string segment;
    string device_ip;
    string gateway_ip;
    string mAdapterName;
    vector<ADAPTER_INFO*> AdapterInfoVector;
};
