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

typedef struct  NetCardStruct
{
    DWORD    Id;         // 网卡设备号
    string   Name;     // 网卡名
    string   driver;
    string   mConnectionName;
    string   mAdapterName;
    string   mIPAddress; // IP地址
    string   mSubnetMask;// 子网掩码
    string   mGateway;// 网关
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
  //BOOL GetAdapterInfo();

private:
    int ExecuteCommand(LPSTR lpCommandLine);
  BOOL RegGetIP(const string & adapter, string& ip, string &subnetMask, string& gateway);
    BOOL RegSetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, DWORD enableDHCP);
  BOOL RegSetMultisz(HKEY hKey, LPCSTR lpValueName, CONST CHAR* lpValue);

  BOOL RegReadConnectName(const string & adapter, string& name);
BOOL RegReadAdapter(const char* driver, string &adapter);
  void EnumNetCards(list<TNetCardStruct>  *NetDeviceList);
  bool NetCardStateChange(PNetCardStruct NetCardPoint, bool Enabled);
  ULONG GetRegistryProperty(HDEVINFO DeviceInfoSet,
                                    PSP_DEVINFO_DATA DeviceInfoData,
                                    ULONG Property,
                                    LPTSTR *Buffer);
private:
    string segment;
    list<TNetCardStruct> mNicList;
    TNetCardStruct *m_pDefaultNic;
};
