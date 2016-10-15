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
  string strName;// ����������
  string strDriverDesc;// ����������
  string strIP; // IP��ַ
  string strNetMask;// ��������
  string strNetGate;// ����
}ADAPTER_INFO;
typedef struct  NetCardStruct
{
    DWORD    Id;         // �����豸��
    string   Name;     // ������
    bool     Disabled;     // ��ǰ�Ƿ����
    bool     Changed;         // �Ƿ���Ĺ�
}TNetCardStruct;
typedef TNetCardStruct*  PNetCardStruct;

bool Ping(const char *ip_addr);
int ResolveIpMac(const char *DestIpString, string & mac);

class NicManager
{
public:
    NicManager(string network="192.168.1");
    ~NicManager(void);
    bool GetAdapter();
    BOOL EnableDhcp();
    int SetIP(LPSTR ip, LPSTR gateway, LPSTR subnetMask);
    BOOL SetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate,LPCTSTR pDnsAddress);
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
  BOOL RegSetDHCPIP();
  BOOL RegGetIP(ADAPTER_INFO *pAI);
  BOOL SetDHCPIP();
  BOOL GetAdapterInfo();
  bool GetRegistryProperty(HDEVINFO DeviceInfoSet,
                                    PSP_DEVINFO_DATA DeviceInfoData,
                                    ULONG Property,
                                    PVOID Buffer,
                                    PULONG Length);
  void EnumNetCards(list<TNetCardStruct>  *NetDeviceList);
  bool   NetCardStateChange(PNetCardStruct NetCardPoint, bool Enabled);
  BOOL GetConnectName(CString& name);

private:
    int ExecuteCommand(LPSTR lpCommandLine);
    BOOL RegSetIP(LPCTSTR pIPAddress, LPCTSTR pNetMask, LPCTSTR pNetGate, LPCTSTR pDnsAddress);
private:
    string segment;
    string device_ip;
    string gateway_ip;
    string mAdapterName;
    vector<ADAPTER_INFO*> AdapterInfoVector;
};
