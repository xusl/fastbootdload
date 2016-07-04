/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2016-01-28  shenlong.xu  init

=============================================================================*/
#include "StdAfx.h"
#include <objbase.h>
#include <initguid.h>
#include <setupapi.h>
#include <algorithm>
#include <stdio.h>
using namespace std;

#include "device.h"
#include "log.h"

DeviceCoordinator::DeviceCoordinator() :
    mDevintfList()
{
    mDevintfList.clear();
    InitializeCriticalSection(&mCriticalSection);
}

DeviceCoordinator::~DeviceCoordinator() {
    Reset();
    DeleteCriticalSection(&mCriticalSection);
}

BOOL DeviceCoordinator::Reset() {
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel* item = *it;
        delete item;
        //*it = NULL;
    }
    mDevintfList.clear();
    return TRUE;
}

CDevLabel *DeviceCoordinator::GetValidDevice() {
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel *item = *it;
        if (item->IsCommandSent())
            continue;
        item->SetCommandSent(true);
        return item;
    }
    return NULL;
}

BOOL DeviceCoordinator::GetDevice(const char * const devPath, CDevLabel** outDevIntf) {
#if 0
    list<CDevLabel>::iterator it = find(mDevintfList.begin(), mDevintfList.end(), devPath);
    if (it != mDevintfList.end()) {
        *outDevIntf = *it;
        return TRUE;
    }
    return FALSE;
#endif
    if(devPath == NULL)
        return FALSE;

    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        if((*it)->GetMac() == devPath) {
            *outDevIntf = *it;
            return TRUE;
        }
    }
    LOGE("Can not get device interface for  %S", devPath);
    return FALSE;
}

BOOL DeviceCoordinator::AddDevice(CDevLabel& dev,  CDevLabel** intfs) {
    CDevLabel* newDevIntf = NULL;

      map <string, bool, greater<string>>::iterator Rit = mMacRecords.find(dev.GetMac());
     if ( Rit != mMacRecords.end() && mMacRecords[dev.GetMac()]) {
        return FALSE;
      }

    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel* item = *it;
        if(dev == *item) break;
    }

    if (it != mDevintfList.end()) {
        newDevIntf = *it;
        //TODO::
    } else {
        LOGI("Create new device");
    EnterCriticalSection(&mCriticalSection);
        mMacRecords.insert(pair<string, bool>(dev.GetMac(), false));
        newDevIntf = new CDevLabel(dev);
        mDevintfList.push_back(newDevIntf);
    LeaveCriticalSection(&mCriticalSection);
    }
    if (intfs != NULL) {
        *intfs = newDevIntf;
    }
    return TRUE;
}

BOOL DeviceCoordinator::RemoveDevice(CDevLabel* const & devIntf)  {
    ASSERT(devIntf != NULL);
    LOGE("RemoveDevice !");
    EnterCriticalSection(&mCriticalSection);
    mDevintfList.remove(devIntf);
    mMacRecords[devIntf->GetMac()] = true;
    delete devIntf;
    LeaveCriticalSection(&mCriticalSection);
    return TRUE;
}

BOOL DeviceCoordinator::IsEmpty() {
    return mDevintfList.empty();
}

VOID DeviceCoordinator::Dump(VOID) {
    list<CDevLabel*>::iterator it;
    LOGI("DeviceCoordinator::===================================BEGIN");
    CDevLabel* item ;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        item = *it;
        item->Dump("dump device");
    }
    LOGI("DeviceCoordinator::===================================END");
}

CDevLabel::CDevLabel(string &mac, string &ip_addr) {
    mMac = mac;
    mIpAddr = ip_addr;
    mCommandSent = false;
}

CDevLabel::CDevLabel(const CDevLabel & dev) {
    //LOGI("Enter copy constructor");
    CDevLabel::operator=(dev);
}

CDevLabel::~CDevLabel() {

}

VOID CDevLabel::Dump(const char *tag) {
    LOG("CDevLabel (%s) :: ", tag);
    LOGD("\tMac: %s", mMac.c_str());
    LOGD("\tIP: %s", mIpAddr.c_str());
    LOGD("\tStatus: %d", mCommandSent);
}


bool CDevLabel::Match(const CDevLabel * const & dev) const {
    if (dev == NULL)
        return false;
    if (this == dev)
        return true;
    return GetMac() == dev->GetMac();
}


bool CDevLabel::operator ==(const CDevLabel & dev) const {
    return Match(&dev);
}

CDevLabel & CDevLabel::operator =(const CDevLabel & dev) {
    SetMac(dev.GetMac());
    SetIpAddr(dev.GetIpAddr());
    SetCommandSent(dev.IsCommandSent());
    return *this;
}

