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
#undef MULTI_DEVICE_FEATURE

DeviceCoordinator::DeviceCoordinator() :
    mDevintfList()
{
    mDevintfList.clear();
    SetSuperMode(FALSE);
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

    LOGD("DeviceCoordinator::Reset");
    return TRUE;
}

CDevLabel *DeviceCoordinator::GetValidDevice() {
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel *item = *it;
        if (item->GetStatus() != DEVICE_ARRIVE)
            continue;
        return item;
    }
    LOGD("There are no arrive device");
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
    LOGD("There are no rebooting device");
    return NULL;
}

CDevLabel *DeviceCoordinator::GetDevice(const char * const ipAddr, int status) {
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel *item = *it;
        if (item->GetStatus() != status)
            continue;
        switch(status) {
            case DEVICE_DWONLOAD:
            case DEVICE_FINISH:
#ifdef MULTI_DEVICE_FEATURE
                if ( item->GetDownloadIpAddr() == ipAddr) {
                    return item;
                }
#else
                return item;
#endif
                break;

            case DEVICE_ARRIVE:
            case DEVICE_COMMAND:
            case DEVICE_REBOOTDOWNLOAD:
#ifdef MULTI_DEVICE_FEATURE
                if ( item->GetIpAddr() == ipAddr) {
                    return item;
                }
#else
                return item;
#endif
                break;

            default:
                break;
        }
    }
    return NULL;
}

BOOL DeviceCoordinator::GetDevice(const char * const ipAddr, CDevLabel** outDevIntf, BOOL byDownload) {
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
#ifdef MULTI_DEVICE_FEATURE
        string ip;
        if (byDownload)
            ip = (*it)->GetDownloadIpAddr();
        else
            ip = (*it)->GetIpAddr();
        if(ip == ipAddr) {
            *outDevIntf = *it;
            return TRUE;
        }
#else
       *outDevIntf = *it;
        return TRUE;
#endif
    }
    LOGE("Can not get device interface for  %s", ipAddr);
    return FALSE;
}

BOOL DeviceCoordinator::SetDownloadFirmware(list<char *> firmware) {
    mpFirmware = firmware;
    return TRUE;
}

BOOL DeviceCoordinator::AddDevice(CDevLabel& dev,  CDevLabel** intfs) {
    CDevLabel* newDevIntf = NULL;

    LOGD("Enter DeviceCoordinator::AddDevice");

    map <string, bool, greater<string>>::iterator Rit = mMacRecords.find(dev.GetMac());
    if ( Rit != mMacRecords.end() /*&& mMacRecords[dev.GetMac()]*/) {
        LOGI("device %s have already updated.", dev.GetMac().c_str());
        return FALSE;
    }

#ifdef MULTI_DEVICE_FEATURE
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        CDevLabel* item = *it;
        if(dev == *item) break;
    }

    if (it != mDevintfList.end()) {
        newDevIntf = *it;
        LOGI("device %s (ip:%s, dlip:%s) is already added in coordinator.",
            newDevIntf->GetMac().c_str(),
            newDevIntf->GetIpAddr().c_str(),
            newDevIntf->GetDownloadIpAddr().c_str());
        return FALSE;
    } else {
        EnterCriticalSection(&mCriticalSection);
        mMacRecords.insert(pair<string, bool>(dev.GetMac(), false));
        newDevIntf = new CDevLabel(dev);
        newDevIntf->SetFirmware(mpFirmware);
        mDevintfList.push_back(newDevIntf);
        LeaveCriticalSection(&mCriticalSection);
        LOGI("add device %s (ip:%s, dlip:%s) into coordinator.",
            newDevIntf->GetMac().c_str(),
            newDevIntf->GetIpAddr().c_str(),
            newDevIntf->GetDownloadIpAddr().c_str());
    }
#else
if (IsEmpty() == FALSE) {
    LOGE("there are a device in coordinator");
    return FALSE;
}
    EnterCriticalSection(&mCriticalSection);
    mMacRecords.insert(pair<string, bool>(dev.GetMac(), false));
    newDevIntf = new CDevLabel(dev);
    newDevIntf->SetFirmware(mpFirmware);
    mDevintfList.push_back(newDevIntf);
    LeaveCriticalSection(&mCriticalSection);
    LOGI("add device %s (ip:%s, dlip:%s) into coordinator.",
        newDevIntf->GetMac().c_str(),
        newDevIntf->GetIpAddr().c_str(),
        newDevIntf->GetDownloadIpAddr().c_str());
#endif

    if (intfs != NULL) {
        *intfs = newDevIntf;
    }

    return TRUE;
}

BOOL DeviceCoordinator::RemoveDevice(CDevLabel* const & devIntf)  {
    ASSERT(devIntf != NULL);
    BOOL found = FALSE;
    list<CDevLabel*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        if (*it == devIntf) {
            found = TRUE;
            break;
        }
    }

    if (found == FALSE) {
        LOGE("Device is already removed");
        return FALSE;
    }

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
    devIntf->SetStatus(DEVICE_REBOOTDOWNLOAD);
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

BOOL DeviceCoordinator::CheckDownloadPermission() {
    CDevLabel *dev = GetRebootDevice();
    if(dev == NULL) {
        dev = GetDevice("", DEVICE_DWONLOAD);
    }
    if(dev == NULL && GetSuperMode()) {
        dev = GetValidDevice();
    }

    if (dev == NULL) {
        LOGD("There are no device can enter download.");
        return FALSE;
    }

#if 0
    list<char *>::iterator it;
    for (it = mpFirmware.begin(); it != mpFirmware.end(); ++it) {
        LOGD("Compare file '%s' <=> '%s'", *it, filename);
        if (strcmp(basename(*it), basename(filename)) == 0)
            return TRUE;
    }
#endif

    return TRUE;
}

BOOL DeviceCoordinator::RefreshFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename, BOOL start) {
    char szAddr[MAXLEN_IPv6]={0}, szServ[NI_MAXSERV] ={0};
    getnameinfo ( (LPSOCKADDR) & addr, sizeof (addr),
             szAddr, sizeof szAddr,
             szServ, sizeof szServ,
             NI_NUMERICHOST | AI_NUMERICSERV);
    if (start) {
        CDevLabel *dev = GetRebootDevice();
        if(dev == NULL && GetSuperMode()) {
            dev = GetValidDevice();
        }
        if(dev != NULL) {
            dev->SetDownloadIpAddr(szAddr);
            dev->TickWatchDog();
        }
    } else {
        CDevLabel *dev = GetDevice(szAddr, DEVICE_DWONLOAD);
        if(dev != NULL) {
            dev->TickWatchDog();
        }
    }
    return TRUE;
}

CDevLabel * DeviceCoordinator::EndFirmwareTransfer(SOCKADDR_STORAGE addr, const char * const filename, DWORD  dwTransferId) {
    char szAddr[MAXLEN_IPv6]={0}, szServ[NI_MAXSERV] ={0};
    getnameinfo ( (LPSOCKADDR) & addr, sizeof (addr),
             szAddr, sizeof szAddr,
             szServ, sizeof szServ,
             NI_NUMERICHOST | AI_NUMERICSERV );
    CDevLabel *dev = NULL;
    if(GetDevice(szAddr, &dev, true)) {
        dev->UpdateFirmwareStatus(filename, dwTransferId);
        dev->TickWatchDog();
    } else {
    LOGE("%s is not in our download manager", szAddr);
    }
    return dev;
}

CDevLabel::CDevLabel(string &mac, string &ip_addr) {
    mMac = mac;
    mIpAddr = ip_addr;
    SetStatus(DEVICE_ARRIVE);
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

bool CDevLabel::SetFirmware(list<char *> pFirmware) {
    list<char *>::iterator it;
    for (it = pFirmware.begin(); it != pFirmware.end(); ++it) {
        LOGD("ADD FILE %s", *it);
        mFirmwareStatus.insert(pair<string, DWORD>(basename(*it), INVALID_TRANSFERID));
    }
    return true;
}

VOID CDevLabel::UpdateFirmwareStatus(const char * const filename, DWORD dwTransferId) {
    assert(filename != NULL);
    LOGD("UPDAT FILE STATUS %s = %d", filename, dwTransferId);
    map <string, DWORD>::iterator it = mFirmwareStatus.find(filename);
    if ( it != mFirmwareStatus.end()) {
        mFirmwareStatus[filename] = dwTransferId;
    }
}

BOOL CDevLabel::IsDownloadFinish() {
    if (GetStatus() != DEVICE_FINISH) {
        map <string, DWORD>::iterator it = mFirmwareStatus.begin();
        for (;it != mFirmwareStatus.end(); ++it) {
            DWORD transferId = it->second;
            if(transferId == INVALID_TRANSFERID)
                return FALSE;
        }
        SetStatus(DEVICE_FINISH);
    }
    return TRUE;
}

BOOL CDevLabel::GetTransferIDs(list<DWORD> &ids) {
    map <string, DWORD>::iterator it = mFirmwareStatus.begin();
    for (;it != mFirmwareStatus.end(); ++it) {
        if(it->second != INVALID_TRANSFERID)
            ids.push_back(it->second);
    }
    return TRUE;
}

BOOL CDevLabel::CheckRemovable() {
    DWORD timeout = 100000;
    switch (GetStatus()) {
        case DEVICE_FINISH:
            timeout = 10000;
            break;

        default:
            break;
    }
    DWORD tick = ::GetTickCount();
    LOGE("Status %d, now tick %d, device timestamp %d, elapse %d", GetStatus(),
        tick, GetStatusEnterMS(), tick - GetStatusEnterMS());
    return (tick - GetStatusEnterMS() > timeout);
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
