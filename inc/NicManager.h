#pragma once
#include <string>
#include<iostream>
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <vector>
#include <list>
#include "utils.h"
//#include "settings.h"

using namespace std;

#define NIC_NETSH_TOGGLE        0
#define NIC_SETUPDI_TOGGLE      1

typedef int (CALLBACK* DHCPNOTIFYPROC)(LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, int);

class  NetCardStruct
{
public:
    DWORD    Id;         // 网卡设备号
    DWORD    dwIfIndex;
    CString   DeviceDesc;     // 网卡名
    CString   mNicDesc;
    CString   driver;
    CString   mConnectionName;
    CString   mAdapterName;
    CString   mIPAddress; // IP地址
    CString   mSubnetMask;// 子网掩码
    CString   mGateway;// 网关
    UCHAR    mPhysAddr[MAXLEN_PHYSADDR];
    //string   mPhysAddr;
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
      dwIfIndex = -1;
      memset(mPhysAddr, 0, sizeof mPhysAddr);
      //mPhysAddr.clear();
      DeviceDesc.Empty();
      mNicDesc.Empty();
      driver.Empty();
      mConnectionName.Empty();
      mAdapterName.Empty();
      mIPAddress.Empty();
      mSubnetMask.Empty();
    };

    BOOL IsInvalid() {
      return (Id == -1);
    };

    BOOL GetHostIp(string &hostIp) {
      return CStringToString(mIPAddress, hostIp);
    }

    BOOL GetGatewayIp(string &gatewayIp) {
      return CStringToString(mGateway, gatewayIp);
    }
};
//typedef  NetCardStruct*  PNetCardStruct;

//bool Ping(const char *ip_addr);
//int ResolveIpMac(const char *DestIpString, string & mac);

typedef LPVOID *PPVOID;

class NicManager
{
public:
    NicManager();
    ~NicManager(void);
  //  VOID Configuration( ConfigIni * config) ;
    void EnumNetCards();
    const list<NetCardStruct>* GetNicList() const { return  &mNicList;}
    int GetNicNum() { return (int)mNicList.size(); }
    NetCardStruct GetDefaultNic() const { return m_DefaultNic;};
    BOOL SetDefaultNic(DWORD id);
    BOOL RestoreDefaultNic();
    int SetIP( CString & ip,  CString & gateway,  CString & subnetMask, BOOL updateIp=TRUE);

    BOOL IsChangingIp() const { return m_IsChangingIp;}
    BOOL EnableDhcp(BOOL updateIp);
    BOOL UpdateIP();
    BOOL NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex);
    BOOL GetConnectedState();
    BOOL CheckIpInArpTable(const char *ip, string & mac);
    int ResolveIpMac(const char *DestIpString, string & mac);
    int FlushArp();
    bool Ping(const char *ip_addr);

private:
    BOOL GetNicInfo(NetCardStruct &netCard);
    BOOL UpdateNic();
    BOOL SwitchNic(NetCardStruct &netCard, bool Enabled);
    bool NetCardStateChange(NetCardStruct &netCard, bool Enabled);
    BOOL WaitAddrChanged();
    BOOL RegGetIP(const CString & adapter, CString& ip, CString &subnetMask, CString& gateway,  BOOL& enableDHCP);
    BOOL RegSetIP(const CString & adapter,  CString & pIPAddress,  CString & pNetMask,  CString & pNetGate, DWORD enableDHCP);
    BOOL RegSetMultisz(HKEY hKey, LPCWSTR lpValueName,  CString& lpValue);
    BOOL RegReadConnectName(const CString & adapter, CString& name);
    BOOL RegReadAdapter(LPWSTR driver, CString &adapter);
    int ExecuteCommand(LPCWSTR command, LPCWSTR parameter);

    ULONG GetRegistryProperty(HDEVINFO DeviceInfoSet,
                                      PSP_DEVINFO_DATA DeviceInfoData,
                                      ULONG Property,
                                      LPTSTR *Buffer);
private:
    BOOL                 m_IsChangingIp;
    list<NetCardStruct>  mNicList;
    NetCardStruct        m_DefaultNic;
    NetCardStruct        m_DefaultNicOrigin;
    int                  m_NicToggle;
    int                  m_Timeout;
};
