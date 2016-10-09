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
#include "settings.h"
#include "device.h"
#include "log.h"

using namespace std;

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
        if (item->GetStatus() != DEVICE_ARRIVE)
            continue;
        //item->SetStatus(DEVICE_COMMAND);
        return item;
    }
    return NULL;
}

CDevLabel *DeviceCoordinator::GetRebootDevice() {
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel *item = *it;
        if (item->GetStatus() != DEVICE_REBOOTDOWNLOAD)
            continue;
        item->SetStatus(DEVICE_DWONLOAD);
        return item;
    }
    return NULL;
}

BOOL DeviceCoordinator::GetDevice(const char * const ipAddr, CDevLabel** outDevIntf) {
#if 0
    list<CDevLabel>::iterator it = find(mDevintfList.begin(), mDevintfList.end(), devPath);
    if (it != mDevintfList.end()) {
        *outDevIntf = *it;
        return TRUE;
    }
    return FALSE;
#endif
    if(ipAddr == NULL)
        return FALSE;

    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        if((*it)->GetDownloadIpAddr() == ipAddr) {
            *outDevIntf = *it;
            return TRUE;
        }
    }
    LOGE("Can not get device interface for  %S", ipAddr);
    return FALSE;
}

BOOL DeviceCoordinator::SetDownloadFirmware(list<char *> *firmware) {
    mpFirmware = firmware;
    return TRUE;
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
        newDevIntf->SetFirmware(mpFirmware);
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

BOOL DeviceCoordinator::RequestDownloadPermission(CDevLabel* const & devIntf) {
    ASSERT(devIntf != NULL);
    list<CDevLabel*>::iterator it;
    EnterCriticalSection(&mCriticalSection);
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel *item = *it;
        if (item->GetStatus() == DEVICE_REBOOTDOWNLOAD) {
            LeaveCriticalSection(&mCriticalSection);
            return FALSE;
        }
    }
    LeaveCriticalSection(&mCriticalSection);
    devIntf->SetStatus(DEVICE_REBOOTDOWNLOAD);
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


BOOL DeviceCoordinator::StartFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename) {
    char szAddr[MAXLEN_IPv6]={0}, szServ[NI_MAXSERV] ={0};
        getnameinfo ( (LPSOCKADDR) & addr, sizeof (addr),
                 szAddr, sizeof szAddr,
                 szServ, sizeof szServ,
                 NI_NUMERICHOST | AI_NUMERICSERV);
    CDevLabel *dev = GetRebootDevice();
    if(dev != NULL) {
        dev->SetDownloadIpAddr(szAddr);
    } else {
    }
    return TRUE;
}

BOOL DeviceCoordinator::EndFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename) {
    char szAddr[MAXLEN_IPv6]={0}, szServ[NI_MAXSERV] ={0};
        getnameinfo ( (LPSOCKADDR) & addr, sizeof (addr),
                 szAddr, sizeof szAddr,
                 szServ, sizeof szServ,
                 NI_NUMERICHOST | AI_NUMERICSERV );
        CDevLabel *dev = NULL;
        if(GetDevice(szAddr, &dev)) {
            dev->UpdateFirmwareStatus(filename, true);
            return dev->IsFirmwareDownload();
        } else {
        LOGE("%s is not in our download manager", szAddr);
        }
    return FALSE;
}

CDevLabel::CDevLabel(string &mac, string &ip_addr) {
    mMac = mac;
    mIpAddr = ip_addr;
    mStatus = DEVICE_ARRIVE;
    mDownloadIpAddr = "";
}

CDevLabel::CDevLabel(const CDevLabel & dev) {
    //LOGI("Enter copy constructor");
    CDevLabel::operator=(dev);
}

CDevLabel::~CDevLabel() {
    mFirmwareStatus.clear();
}

VOID CDevLabel::Dump(const char *tag) {
    LOG("CDevLabel (%s) :: ", tag);
    LOGD("\tMac: %s", mMac.c_str());
    LOGD("\tIP: %s", mIpAddr.c_str());
    LOGD("\tStatus: %d", mStatus);
}


bool CDevLabel::SetFirmware(list<char *> *pFirmware) {
    assert(pFirmware != NULL);
    list<char *>::iterator it;
    for (it = pFirmware->begin(); it != pFirmware->end(); ++it) {
        LOGD("ADD FILE %s", *it);
        mFirmwareStatus.insert(pair<string, bool>(*it, false));
    }
    return true;
}
VOID CDevLabel::UpdateFirmwareStatus(const char * const filename, bool value) {
    assert(filename != NULL);
    LOGD("UPDAT FILE STATUS %s = %d", filename, value);
    map <string, bool>::iterator it = mFirmwareStatus.find(filename);
    if ( it != mFirmwareStatus.end()) {
        mFirmwareStatus[filename] = value;
    }
}

BOOL CDevLabel::IsFirmwareDownload() {
    map <string, bool>::iterator it = mFirmwareStatus.begin();
    for (;it != mFirmwareStatus.end(); ++it) {
        if(it->second == false)
            return FALSE;
    }
    return TRUE;
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
    SetStatus(dev.GetStatus());
    return *this;
}
