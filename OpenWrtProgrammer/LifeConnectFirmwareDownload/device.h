/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-02-06  dawang.xu  init first version

=============================================================================*/
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <usb100.h>
#include <atlutil.h>
#include <setupapi.h>
#include <map>
#include "utils.h"
using namespace std;

//using std::vector;

/* Each device has a cdrom */
enum DEVICE_e {
  DEVICE_ARRIVE,
                  DEVICE_COMMAND, DEVICE_REBOOTDOWNLOAD, DEVICE_DWONLOAD,
                  DEVICE_FINISH, DEVICE_MAX};

class CDevLabel {
  public:
    CDevLabel(string &mac, string &ip_addr);
    CDevLabel(const CDevLabel & dev);
    ~CDevLabel();

    bool operator ==(const CDevLabel & ) const;
    bool Match(const CDevLabel * const &) const;
    CDevLabel & operator =(const CDevLabel & );
    string GetMac() const{ return mMac;};
    string GetIpAddr() const { return mIpAddr;};
    string GetDownloadIpAddr() const { return mDownloadIpAddr;};
    void SetMac(string mac) { mMac = mac;};
    void SetIpAddr(string ip) { mIpAddr = ip;};
    void SetDownloadIpAddr(string ip) { mDownloadIpAddr = ip;};
    void SetStatus(DEVICE_e status) {
      mStatus = status;
      TickWatchDog();
    };
    void TickWatchDog() { mStatusEnterMS = ::GetTickCount();}; /*time(NULL);*/
    BOOL CheckRemovable();
    DEVICE_e GetStatus() const { return mStatus;};
    VOID Dump(const char *tag);
    bool SetFirmware(list<char *> pFirmware);
    VOID UpdateFirmwareStatus(const char * const filename, DWORD dwTransferId);
    BOOL IsDownloadFinish();
    DWORD GetStatusEnterMS() { return mStatusEnterMS;};
    BOOL GetTransferIDs(list<DWORD> &ids);

 private:
    string             mMac;
    string             mIpAddr;
    string             mDownloadIpAddr;
    DEVICE_e           mStatus;
    DWORD              mStatusEnterMS;
    map<string, DWORD>  mFirmwareStatus;
};

class DeviceCoordinator {
  public:
    DeviceCoordinator();
    ~DeviceCoordinator();
    CDevLabel *GetValidDevice();
    CDevLabel *GetRebootDevice();
    CDevLabel *GetDevice(const char * const ipAddr, int status);
    BOOL GetDevice(const char *const ipAddr, CDevLabel** outDevIntf, BOOL byDownload);
    BOOL AddDevice(CDevLabel& dev, CDevLabel** intfs);
    BOOL RemoveDevice(CDevLabel*const & devIntf);
    BOOL SetDownloadFirmware(list<char *> firmware);
    BOOL RequestDownloadPermission(CDevLabel* const & devIntf);
    BOOL RefreshFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename, BOOL start);
    CDevLabel * EndFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename, DWORD dwTransferId);

    BOOL IsEmpty();
    BOOL Reset();
    VOID Dump(VOID);

  private:
    list<CDevLabel *>              mDevintfList;
    list<char *>                   mpFirmware;
    map<string, bool, greater<string>> mMacRecords;
    CRITICAL_SECTION               mCriticalSection;
};


#endif //__DEVICE_H__
