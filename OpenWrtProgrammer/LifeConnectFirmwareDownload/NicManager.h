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
    DWORD    Id;         // �����豸��
    string   DeviceDesc;     // ������
    string   mNicDesc;
    string   driver;
    string   mConnectionName;
    string   mAdapterName;
    string   mIPAddress; // IP��ַ
    string   mSubnetMask;// ��������
    string   mGateway;// ����
    UCHAR    mPhysAddr[MAXLEN_PHYSADDR];
    //string   mPhysAddr;
    BOOL     mEnableDHCP;
    bool     Disabled;     // ��ǰ�Ƿ����
    bool     Changed;         // �Ƿ���Ĺ�

    NetCardStruct() {
      Reset();
    };
    ~NetCardStruct() {
    };

    VOID Reset() {
      Id = -1;
      memset(mPhysAddr, 0, sizeof mPhysAddr);
      //mPhysAddr.clear();
      DeviceDesc.clear();
      mNicDesc.clear();
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

//bool Ping(const char *ip_addr);
//int ResolveIpMac(const char *DestIpString, string & mac);

typedef LPVOID *PPVOID;

class NicManager
{
public:
    NicManager(string network="192.168.1");
    ~NicManager(void);
   void EnumNetCards();
    const list<NetCardStruct>* GetNicList() const { return  &mNicList;}
    int GetNicNum() { return mNicList.size(); }
    NetCardStruct GetDefaultNic() const { return m_DefaultNic;};
    BOOL SetDefaultNic(DWORD id);
    BOOL EnableDhcp();
    int SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask);
    BOOL UpdateIP();
    BOOL NotifyIPChange(LPCTSTR lpszAdapterName, int nIndex);
    BOOL GetConnectedState();
    int ResolveIpMac(const char *DestIpString, string & mac);
    bool Ping(const char *ip_addr);

private:
    int ExecuteCommand(LPSTR lpCommandLine);

    BOOL GetNicInfo(NetCardStruct &netCard);
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
