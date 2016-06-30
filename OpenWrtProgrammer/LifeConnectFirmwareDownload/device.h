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
    void SetMac(string mac) { mMac = mac;};
    void SetIpAddr(string ip) { mIpAddr = ip;};
    void SetCommandSent(bool send) { mCommandSent = send;};
    bool IsCommandSent() const { return mCommandSent;};
    VOID Dump(const char *tag);

 private:
    string mMac;
    string mIpAddr;
    bool   mCommandSent;
};

class DeviceCoordinator {
  public:
    DeviceCoordinator();
    ~DeviceCoordinator();
    CDevLabel *GetValidDevice();
    BOOL GetDevice(const char *const mac, CDevLabel** outDevIntf);
    BOOL AddDevice(CDevLabel& dev, CDevLabel** intfs);
    BOOL RemoveDevice(CDevLabel*const & devIntf);
    BOOL IsEmpty();
    BOOL Reset();
    VOID Dump(VOID);

  private:
    list<CDevLabel *>  mDevintfList;
    map<string, bool, greater<string>> mMacRecords;
    CRITICAL_SECTION   mCriticalSection;
};


#endif //__DEVICE_H__
