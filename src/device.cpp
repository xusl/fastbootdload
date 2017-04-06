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
using namespace std;

#include "device.h"
#include "utils.h"
#include "log.h"

#pragma 	  comment(lib,"setupapi.lib")

DeviceInterfaces::DeviceInterfaces() :
    mActiveIntf(NULL),
    mAdb(NULL),
    mDiag(NULL),
    mFastboot(NULL),
    mAdbHandle(NULL),
    mFbHandle(NULL),
    mDeviceActive(DEVICE_UNKNOW),
    mAttachUiPort(false)
{
    const char *default_tag = "N/A Device";
    strncpy_s(mTag, DEV_TAG_LEN, default_tag, strlen(default_tag));
}

DeviceInterfaces::DeviceInterfaces(const DeviceInterfaces & devIntf) {
    DeviceInterfaces::operator =(devIntf);
}

CDevLabel* DeviceInterfaces::SetAdbIntf(CDevLabel& intf) {
    DELETE_IF(mAdb);
    mAdb = new CDevLabel(intf);
    return mAdb;
}

CDevLabel* DeviceInterfaces::SetDiagIntf(CDevLabel& intf) {
    DELETE_IF(mDiag);
    mDiag = new CDevLabel(intf);
    return mDiag;
}

#if 0
CPacket* DeviceInterfaces::GetPacket() {
    if (mDiag == NULL)
        return NULL;

     CPacket* m_packetDll = new CPacket();
    m_packetDll->Init(mDiag->GetComPortNum());
    return m_packetDll;

  }
#endif
CDevLabel* DeviceInterfaces::SetFastbootIntf(CDevLabel& intf) {
    DELETE_IF(mFastboot);
    mFastboot = new CDevLabel(intf);
    return mFastboot;
}

VOID DeviceInterfaces::UpdateDevTag() {
    long sn;
    long port;
    if(mAdb != NULL) {
        mAdb->GetEffectiveSnPort(&sn, &port);
    } else if(mFastboot != NULL) {
        mFastboot->GetEffectiveSnPort(&sn, &port);
    }

    if(mDiag != NULL ) {
        //wsprintf
        if (mAdb != NULL || mFastboot != NULL )
            snprintf(mTag, DEV_TAG_LEN, "COM%d (0X%X.%d)", mDiag->GetComPortNum(), sn, port);
        else
            snprintf(mTag, DEV_TAG_LEN, "COM%d", mDiag->GetComPortNum());
    } else {
        snprintf(mTag, DEV_TAG_LEN, "Device: 0X%X (%d)", sn, port);
    }
}

int DeviceInterfaces::GetDevId() {
    long sn;
    long port;

    if(mDiag != NULL)
        return mDiag->GetComPortNum();

    if(mActiveIntf != NULL) {
        mActiveIntf->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }

    if(mAdb != NULL) {
        mAdb->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }

    if(mFastboot != NULL) {
        mFastboot->GetEffectiveSnPort(&sn, &port);
        return sn+port;
    }
    return 0xdeadbeef;
}

usb_handle* DeviceInterfaces::GetUsbHandle(BOOL flashdirect) const {
    usb_dev_t status = GetDeviceStatus();
    if (GetFastbootHandle() != NULL && (DEVICE_PLUGIN == status || DEVICE_FLASH == status))
        return GetFastbootHandle();
    if (GetAdbHandle() != NULL && (DEVICE_PLUGIN == status || DEVICE_CHECK == status))
        return GetAdbHandle();
    return NULL;
}


BOOL DeviceInterfaces::SetIntf(CDevLabel& dev, TDevType type, BOOL updateActiveIntf) {
    CDevLabel* pDev = NULL;
    if (type == DEVTYPE_DIAGPORT) {
        pDev = SetDiagIntf(dev);
    } else if(type == DEVTYPE_ADB) {
        pDev = SetAdbIntf(dev);
    } else if(type == DEVTYPE_FASTBOOT) {
        pDev = SetFastbootIntf(dev);
    }
    if(pDev != NULL && updateActiveIntf)
        SetActiveIntf(pDev);
    UpdateDevTag();
    return TRUE;
}

bool DeviceInterfaces::operator ==(const DeviceInterfaces * const & devIntf) const{
    LOGE("DO DeviceInterfaces::operator ==");
    return Match(devIntf);
}

bool DeviceInterfaces::Match(const DeviceInterfaces * const & devIntf) const {
    if (this == devIntf)
        return true;

    CDevLabel* adb = devIntf->GetAdbIntf();
    CDevLabel* diag = devIntf->GetDiagIntf();
    CDevLabel* fb = devIntf->GetFastbootIntf();

    if (mAdb != NULL && (mAdb->Match(adb) || mAdb->Match(diag) ||mAdb->Match(fb)))
        return true;

    if (mDiag != NULL && (mDiag->Match(diag) || mDiag->Match(adb) ||mDiag->Match(fb)))
        return true;

    if (mFastboot != NULL && (mFastboot->Match(fb) ||mFastboot->Match(adb) || mFastboot->Match(diag)))
        return true;

    return false;
}

bool DeviceInterfaces::MatchDevPath(const wchar_t * devPath) const {
    LOGE("DO DeviceInterfaces::operator == 2");
    if ( (mAdb != NULL && (*mAdb == devPath)) ||
        (mDiag != NULL && (*mDiag == devPath)) ||
        (mFastboot != NULL && (*mFastboot == devPath)))
        return true;
    return false;
}

DeviceInterfaces & DeviceInterfaces::operator =(const DeviceInterfaces & devIntfs) {
    mActiveIntf = devIntfs.GetActiveIntf();
    mAdb = devIntfs.GetAdbIntf();
    mDiag = devIntfs.GetDiagIntf();
    mFastboot = devIntfs.GetFastbootIntf();
    mDeviceActive = devIntfs.GetDeviceStatus();
    //m_packetDll = devIntfs.GetPacket();
    mAdbHandle = devIntfs.GetAdbHandle() ;
    mFbHandle = devIntfs.GetFastbootHandle();
    strcpy(mTag, devIntfs.GetDevTag());
    mAttachUiPort = devIntfs.GetAttachStatus();
    return *this;
}

VOID DeviceInterfaces::SetDeviceStatus(usb_dev_t status) {
    mDeviceActive = status;
}

VOID DeviceInterfaces::DeleteMemory(VOID) {
    DELETE_IF(mAdb);
    DELETE_IF(mDiag);
    DELETE_IF(mFastboot);
    mActiveIntf = NULL;
    SetDeviceStatus(DEVICE_UNKNOW);
}

DeviceInterfaces::~DeviceInterfaces()
{
    mActiveIntf = NULL;
}


VOID DeviceInterfaces::Dump(const char *tag) {
    if(mFastboot != NULL)
        mFastboot->Dump("fastboot");
    if(mAdb)
        mAdb->Dump("adb");
    if(mDiag)
        mDiag->Dump("diag");
    LOGD("DeviceInterfaces::Dump END ");
}

BOOL DeviceInterfaces::Reset() {
    mActiveIntf = NULL;
    mAdbHandle = NULL;
    mFbHandle= NULL;
    SetDeviceStatus(DEVICE_UNKNOW);
    SetAttachStatus(false);
   return TRUE;
}
DeviceCoordinator::DeviceCoordinator() :
    mDevintfList()
{
    mDevintfList.clear();
}

DeviceCoordinator::~DeviceCoordinator() {
    Reset();
}

BOOL DeviceCoordinator::Reset() {
    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces* item = *it;
        item->DeleteMemory();
        delete item;
        //*it = NULL;
    }
    mDevintfList.clear();
    return TRUE;
}

DeviceInterfaces *DeviceCoordinator::GetValidDevice(int32 devMask) {
    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces *item = *it;
        usb_dev_t status = item->GetDeviceStatus();
        if (item->GetAttachStatus()) {
			LOGD("Device %d is attached.", item->GetDevTag());
            continue;
    	}
		
        if (devMask & DEVTYPE_DIAGPORT && (item->GetDiagIntf() == NULL)) {			
			LOGD("Device %d found none diag port.", item->GetDevTag());
            continue;
    	}
		
        if (devMask & DEVTYPE_ADB && (item->GetAdbIntf() == NULL)) {
			LOGD("Device %d found none adb device.", item->GetDevTag());
            continue;
    	}
		
        if (devMask & DEVTYPE_FASTBOOT && (item->GetFastbootIntf() == NULL)) {			
			LOGD("Device %d found none fastboot device.", item->GetDevTag());
            continue;
    	}
        if(status >= DEVICE_PLUGIN && status < DEVICE_REMOVED ) {
            return item;
        }
		
		LOGD("Device %d state is %d.", item->GetDevTag(), status);
    }
    LOGE("None device found");
    return NULL;
}

BOOL DeviceCoordinator::GetDevice(const wchar_t * const devPath, DeviceInterfaces** outDevIntf) {
#if 0
    list<DeviceInterfaces>::iterator it = find(mDevintfList.begin(), mDevintfList.end(), devPath);
    if (it != mDevintfList.end()) {
        *outDevIntf = *it;
        return TRUE;
    }
    return FALSE;
#endif
    if(devPath == NULL)
        return FALSE;

    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        if((*it)->MatchDevPath(devPath) && (*it)->GetDeviceStatus()) {
            *outDevIntf = *it;
            return TRUE;
        }
    }
    LOGE("Can not get device interface for  %S", devPath);
    return FALSE;
}

BOOL DeviceCoordinator::AddDevice(CDevLabel& dev,
                                 TDevType type,
                                 BOOL ignoreAttachStatus,
                                 DeviceInterfaces** intfs) {
    DeviceInterfaces* newDevIntf = NULL;
    DeviceInterfaces temp;
    temp.SetIntf(dev, type);

    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces* item = *it;
        if(temp == item) break;
    }

    if (it != mDevintfList.end()) {
        newDevIntf = *it;
        if((*it)->GetAttachStatus() && !ignoreAttachStatus) {
            LOGI("the exit device is attached, do not update the device");
            return FALSE;
        } else {
            LOGI("Update exist device");
            if (newDevIntf->GetDeviceStatus() == DEVICE_UNKNOW)
                (*it)->SetDeviceStatus(DEVICE_PLUGIN);
            (*it)->SetIntf(dev, type);
        }
    } else {
        LOGI("Create new device");
        newDevIntf = new DeviceInterfaces(temp);
        newDevIntf->SetDeviceStatus(DEVICE_PLUGIN);
        mDevintfList.push_back(newDevIntf);
    }
    if (intfs != NULL) {
        *intfs = newDevIntf;
    }
    return TRUE;
}

BOOL DeviceCoordinator::RemoveDevice(DeviceInterfaces* const & devIntf)  {
    ASSERT(devIntf != NULL);
    LOGE("RemoveDevice !");
    devIntf->Reset();
    mDevintfList.remove(devIntf);
    devIntf->DeleteMemory();
    delete devIntf;

    return TRUE;
}

BOOL DeviceCoordinator::IsEmpty() {
    return mDevintfList.empty();
}

VOID DeviceCoordinator::Dump(VOID) {
    list<DeviceInterfaces*>::iterator it;
    LOGI("DeviceCoordinator::===================================BEGIN");
    DeviceInterfaces* item ;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        item = *it;
        item->Dump("dump device");
    }
    LOGI("DeviceCoordinator::===================================END");
}

//\\?\usbstor#disk&ven_onetouch&prod_link4&rev_2.31#6&21c8898b&1&0123456789abcdef&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}
//For id , it is "6&21c8898b&1&0123456789abcdef&0".
//"6&21c8898b&1 is assigned by system, it is call parentIdPrefix, it is assigned by Windows. we use the second parent is enough.
// "0123456789abcdef" is the usb serial number in usb description, report by device,
// if "SYSTEM\\CurrentControlSet\\Control\\UsbFlags" is set, the Windows will abandon this value.
// so , device id reduces , "6&21c8898b&1&0"
//the last "0" is the port, assign by windows.
CDevLabel::CDevLabel(const wchar_t * devPath,
                     const wchar_t* parentIdPrefix,
                     bool useParentIdPrefix,
                     const wchar_t * name):
mPortNum(~1),
mEffectiveSn(~1),
mEffectivePort (~1) {
    SetServiceName(name);
    CopyDeviceDescPath(devPath, parentIdPrefix);
    mUseParentIdPrefix = useParentIdPrefix;

    memset(mDevId, 0, sizeof mDevId);
    if (devPath != NULL){
        wchar_t delimits[] = {L'#', L'#', L'#'};
        wchar_t *sne = (wchar_t *)devPath;
        wchar_t *snb;
        int len = wcslen(devPath);
        for (int i = 0; i < sizeof(delimits)/sizeof(wchar_t); i++, sne++) {
            wchar_t sep = delimits[i];
            snb = sne;
            sne = (wchar_t*)wcschr(sne, sep);
            if (sne == NULL || sne - devPath >= len) {
                LOGE("In step %d , '%c' is not found", i, sep);
                return;
            }
        }
        len = sne - snb - 1;
        wcsncpy_s(mDevId, DEV_ID_LEN, snb, len);

        mEffectiveSn = usb_host_sn(devPath);
        mEffectivePort = usb_host_sn_port(devPath);
    } else {
        LOGE("Can not get invalid device id");
    }

    memset(mMatchId, 0, sizeof mMatchId);
    if (mUseParentIdPrefix && parentIdPrefix != NULL) {
        SetMatchId(parentIdPrefix);
    } else if (wcslen(mDevId) > 0) {
        wchar_t *sne = ( wchar_t*)wcsrchr(mDevId , L'&');
        if (sne != NULL)
            wcsncpy_s(mMatchId, DEV_ID_LEN, mDevId, sne - mDevId);
        else
            LOGE("Can not find '&' in dev path.");
    }else {
        LOGE("Can not get invalid device id");
    }
}

CDevLabel::CDevLabel(const CDevLabel & dev) {
    //LOGI("Enter copy constructor");
    CDevLabel::operator=(dev);
}

CDevLabel::~CDevLabel() {
    FREE_IF(mDevPath);
    FREE_IF(mParentIdPrefix);
    mEffectiveSn = ~1;
    mEffectivePort = ~1;
}

VOID CDevLabel::Dump(const char *tag) {
    //LOGI("CDevLabel:: ID:\t%S\n\tDevice Path:%S \n\tParentIDPrefix:%S",
    //     mDevId, devPath, parentIdPrefix);
    LOG("CDevLabel (%s) :: ", tag);
    LOGD("\tID: %S", mDevId);
    LOGD("\tDevice Path: %S", mDevPath);
    LOGD("\tParentIDPrefix: %S", mParentIdPrefix);
    LOGD("\tmMatchId: %S", mMatchId);
}
void CDevLabel::CopyDeviceDescPath(const wchar_t * devPath, const wchar_t* parentIdPrefix) {
    if (devPath != NULL) {
        mDevPath = _wcsdup(devPath);

        if(mDevPath == NULL)
            LOGE("strdup failed");
    } else {
        mDevPath = NULL;
    }

    if (parentIdPrefix != NULL) {
        mParentIdPrefix = _wcsdup(parentIdPrefix);
        if(mParentIdPrefix == NULL)
            LOGE("strdup failed");
    } else {
        mParentIdPrefix = NULL;
    }
}

VOID CDevLabel::FreeBuffer() {
    FREE_IF(mDevPath);
    FREE_IF(mParentIdPrefix);
}


bool CDevLabel::SetMatchId(const wchar_t * matchId){
    if (matchId == NULL )
        return false;
    int len =wcslen(matchId);
    if (len == 0 || len >= DEV_MATCHID_LEN) {
        LOGE("matchId length exceed buffer length");
        return false;
    }
    return (0 == wcsncpy_s(mMatchId, DEV_MATCHID_LEN, matchId, len));
}

bool CDevLabel::SetDevId(const wchar_t * devId) {
    if (devId == NULL )
        return false;
    int len =wcslen(devId);
    if (len == 0 || len >= DEV_ID_LEN) {
        LOGE("devId length exceed buffer length");
        return false;
    }
    return (0 == wcsncpy_s(mDevId, DEV_ID_LEN, devId, len));
}

const wchar_t * CDevLabel::GetServiceName()  const {
    return mServiceName;
}

bool CDevLabel::SetServiceName(const wchar_t * name) {
    if (name == NULL) {
        memset(mServiceName, 0, sizeof mServiceName);
        return false;
    }
    int len = wcslen(name);
    if (len == 0 || len >= DEV_SERVICE_LEN) {
        LOGE("name length exceed buffer length");
        return false;
    }
    wcsncpy_s(mServiceName, DEV_SERVICE_LEN, name, len);
    return true;
}

bool CDevLabel::GetEffectiveSnPort(long *sn, long *port) {
    *sn = mEffectiveSn;
    *port = mEffectivePort;
    return true;
}

bool CDevLabel::SetEffectiveSnPort(long sn, long port)
{
    mEffectiveSn = sn;
    mEffectivePort = port;
    return true;
}

bool CDevLabel::SetComPort(const wchar_t *portName) {
    //LOGI("PortName %S", portName);
    //swscanf
    swscanf_s(portName, _T("COM%d"), &mPortNum);
    //LOGI("Set comport %d", mPortNum);
    return true;
}

CString CDevLabel::GetComPort() {
    CString com;
    com.Format(_T("COM%d"), mPortNum);
    return com;
}
bool CDevLabel::MatchDevPath(const wchar_t * devPath) {
    LOGE("DO CDevLabel::operator ==");
    const wchar_t *effectivePath = GetDevPath();
    if (effectivePath != NULL && devPath != NULL &&
        0 == wcscmp(effectivePath, devPath))
        return true;
    return false;
}


bool CDevLabel::Match(const CDevLabel * const & dev) const {
    const wchar_t *effectivePath = NULL;
    const wchar_t *effectivePath2 = NULL;
    if (dev == NULL)
        return false;
    if (this == dev)
        return true;

    effectivePath = GetDevPath();
    effectivePath2 = dev->GetDevPath();

    if (effectivePath != NULL && effectivePath2 != NULL &&
        0 == wcscmp(effectivePath, effectivePath2)) //stricmp
        return true;


//TPST Upgrading device path:
//\\?\usb#vid_1bbb&pid_007a#5&10cd67f3&0&3#{86e0d1e0-8089-11d0-9ce4-08003e301f73}
//Adb
//\\?\usb#vid_1bbb&pid_0196#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
//fastboot
//\\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}

//In Debug mode:
//adb interface
// parent: 6&1e805b40&1
// Devpath: \\?\usb#vid_1bbb&pid_0196#5&10cd67f3&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
//com port
//parent: (null)
// Devpath: \\?\usb#vid_1bbb&pid_0196&mi_02#6&1e805b40&1&0002#{86e0d1e0-8089-11d0-9ce4-08003e301f73}
    effectivePath = GetDevId();
    effectivePath2 = dev->GetDevId();
    if (effectivePath != NULL && effectivePath2 != NULL &&
        (0 == wcscmp(effectivePath, effectivePath2) ))
        return true;//stricmp

    if (mUseParentIdPrefix || dev->GetParentIdPrefix()) {
        effectivePath = GetMatchId();
        effectivePath2 =dev->GetMatchId();
    }

    if (effectivePath != NULL && effectivePath2 != NULL &&
        (0 == wcscmp(effectivePath, effectivePath2) ))
        return true;

    return false;
}


bool CDevLabel::operator ==(const CDevLabel & dev) const {
    return Match(&dev);
}

CDevLabel & CDevLabel::operator =(const CDevLabel & dev) {
    //FREE_IF(mDevPath);
    //FREE_IF(mParentIdPrefix);
    CopyDeviceDescPath(dev.GetDevPath(), dev.GetParentIdPrefix());
    mUseParentIdPrefix = dev.mUseParentIdPrefix;
    mEffectiveSn = dev.mEffectiveSn;
    mEffectivePort = dev.mEffectivePort;
    mPortNum = dev.GetComPortNum();
    wcscpy_s(mDevId, dev.GetDevId());
    wcscpy_s(mMatchId, dev.GetMatchId());
    wcscpy_s(mServiceName, dev.GetServiceName());
    return *this;
}


/*
  // dbcc_name:
  // \\?\USB#Vid_04e8&Pid_503b#0002F9A9828E0F06#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
  // convert to  USB\Vid_04e8&Pid_503b\0002F9A9828E0F06
  ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
  CString szDevId = pDevInf->dbcc_name+4;
  int idx = szDevId.ReverseFind(_T('#'));
  ASSERT( -1 != idx );
  szDevId.Truncate(idx);
  szDevId.Replace(_T('#'), _T('\\'));
  szDevId.MakeUpper();

  CString szClass;
  idx = szDevId.Find(_T('\\'));
  ASSERT(-1 != idx );
  szClass = szDevId.Left(idx);
  DEBUG(L"szClass %S", szClass.GetString());
*/

//5&10cd67f3&0&4 5&10cd67f3&0
long extract_serial_number(wchar_t * sn, wchar_t **ppstart , wchar_t **ppend) {
  unsigned int len;
  wchar_t *pstart, *pend;
  if (sn == NULL) {
    ERROR("Bad Parameter");
    return -1;
  }

  len = wcslen (sn);

  pstart = (wchar_t*)wcschr(sn, L'&');
  if (pstart == NULL || pstart - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  pstart++;
  pend = wcschr(pstart , L'&');
  if (pend == NULL || pend - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  if (ppstart != NULL)
    *ppstart = pstart;
  if (ppend != NULL)
    *ppend = pend;
  return wcstol(pstart , &pend, 16);
}

// \\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&4#{f72fe0d4-cbcb-407d-8814-9ed673d0dd6b}
// convert to
// 10cd67f3
long usb_host_sn(const wchar_t* dev_name, wchar_t** psn) {
    wchar_t * snb, *sne, * sn;
    int len = wcslen (dev_name); //lstrlen, lstrcmp()
    if(_wcsnicmp(L"\\\\?\\",dev_name,4) ) {
        LOGE("Not valid dev name: %S.", dev_name);
        return 0;
    }

    //strtok is not suitable;
    wchar_t delimits[] = {L'#', L'#', L'&', L'&'};
    sne = (wchar_t *)dev_name;
    for (int i = 0; i < sizeof(delimits)/sizeof(wchar_t) ; i++, sne++) {
        wchar_t sep = delimits[i];
        snb = sne;
        sne = (wchar_t*)wcschr(sne, sep);
        if (sne == NULL || sne - dev_name >= len) {
            LOGE("In step %d , %c is not found", i, sep);
            return 0;
        }
    }

    len = sne - snb;
    if (len <= 0)
        return 0;

    if (psn) {
        sn = (wchar_t*) malloc((len + 1) * sizeof(wchar_t));
        if (sn == NULL) {
            ERROR("NO memory");
        } else {
            wcsncpy(sn, snb , len);
            *(sn + len) = L'\0';
        }
        *psn = sn;
    }

    return wcstol(snb, &sne, 16);
}

long usb_host_sn_port(const wchar_t* dev_name) {
     wchar_t* begin , *end;
    if (dev_name == NULL)
        return 0;

      begin = ( wchar_t*)wcsrchr(dev_name , L'&');

        if (begin == NULL )
            return 0;

        begin ++;
          end = begin + 1;
  if (*(begin + 1) == L'#' && *(begin - 3) == L'&')
    return wcstol(begin , &end, 16);

  return 0;
}
