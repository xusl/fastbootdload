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

class  NetCardStruct
{
public:
    DWORD    Id;         // 网卡设备号
    string   Name;     // 网卡名
    string   driver;
    string   mConnectionName;
    string   mAdapterName;
    string   mIPAddress; // IP地址
    string   mSubnetMask;// 子网掩码
    string   mGateway;// 网关
    BOOL     mEnableDHCP;
    bool     Disabled;     // 当前是否禁用
    bool     Changed;         // 是否更改过

    NetCardStruct() {
      Reset();
    };
    ~NetCardStruct() {
    };

    VOID Reset() {
      Id = -1;
      Name.clear();
      driver.clear();
      mConnectionName.clear();
      mAdapterName.clear();
      mIPAddress.clear();
      mSubnetMask.clear();
    };

    BOOL IsInvalid() {
      return (Id == -1);
    };
};
typedef  NetCardStruct*  PNetCardStruct;

bool Ping(const char *ip_addr);
int ResolveIpMac(const char *DestIpString, string & mac);

typedef LPVOID *PPVOID;

class NicManager
{
public:
    NicManager(string network="192.168.1");
    ~NicManager(void);
   void EnumNetCards();
    const list<NetCardStruct>* GetNicList() const { return  &mNicList;}
    NetCardStruct GetDefaultNic() const { return m_DefaultNic;};
    BOOL EnableDhcp();
    int SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask);
    BOOL UpdateIP();
    BOOL NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex);
  //BOOL GetAdapterInfo();

private:
    int ExecuteCommand(LPSTR lpCommandLine);
    BOOL RegGetIP(const string & adapter, string& ip, string &subnetMask, string& gateway,  BOOL& enableDHCP);
    BOOL RegSetIP(const string & adapter, LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, DWORD enableDHCP);
    BOOL RegSetMultisz(HKEY hKey, LPCSTR lpValueName, CONST CHAR* lpValue);
    BOOL RegReadConnectName(const string & adapter, string& name);
    BOOL RegReadAdapter(const char* driver, string &adapter);
    bool NetCardStateChange(PNetCardStruct NetCardPoint, bool Enabled);
    ULONG GetRegistryProperty(HDEVINFO DeviceInfoSet,
                                      PSP_DEVINFO_DATA DeviceInfoData,
                                      ULONG Property,
                                      LPTSTR *Buffer);
private:
    string segment;
    list<NetCardStruct> mNicList;
    NetCardStruct m_DefaultNic;
};
