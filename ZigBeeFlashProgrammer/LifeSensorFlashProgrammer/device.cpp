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

#pragma 	  comment(lib,"setupapi.lib")
BOOL GetDevLabelByGUID(CONST GUID *pClsGuid, PCSTR service,
        vector<CDevLabel>& labels,  bool useParentIdPrefix)
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    if(pClsGuid == NULL || service == NULL)
        return FALSE;

    hDevInfo = SetupDiGetClassDevs(pClsGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if( INVALID_HANDLE_VALUE == hDevInfo )  {
//        LOGE("SetupDiGetClassDevs: %S", _com_error(GetLastError()).ErrorMessage());
        return FALSE;
    }

    SP_DEVICE_INTERFACE_DATA ifcData;
    ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (int i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, pClsGuid, i, &ifcData); ++ i) {
        DWORD nSize=0;
        SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};

        // Get buffer size first
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, &nSize, NULL);
        if (nSize == 0) {
            LOGE("SetupDiGetDeviceInterfaceDetail Get empty data.");
            continue;
        }

        SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;
        pDetData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(new BYTE[nSize]);
        if(NULL == pDetData) {
            LOGE("new arrary failed. No memory.");
            continue;
        }
        memset(pDetData, 0, nSize*sizeof(BYTE));
        pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if(!SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData,
                                            nSize, NULL, &devdata)) {
            LOGE("Get empty data.");
            FREE_IF(pDetData);
            continue;
        }

        HKEY hKey = NULL;
        TCHAR buffer[_MAX_PATH] = {0};
        TCHAR portName[_MAX_PATH] = {0};
        TCHAR key[MAX_PATH] = {0};
        PCCH pParentIdPrefix = NULL;
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_SERVICE,
                                              NULL, (PBYTE)buffer, sizeof(buffer), &nSize)) {
            LOGE("get SPDRP_SERVICE failed");
            FREE_IF(pDetData);
            continue;
        }
        if (_strnicmp((const char *)buffer, service, nSize/sizeof(char))) {
            //LOGE("SPDRP_SERVICE return '%S', does not match '%S'.", buffer, service);
            FREE_IF(pDetData);
            continue;
        }

        if (!SetupDiGetDeviceInstanceId(hDevInfo, &devdata, buffer, sizeof(buffer), &nSize)) {
//            LOGD("SetupDiGetDeviceInstanceId(): %S", _com_error(GetLastError()).ErrorMessage());
        } else {
            _snprintf_s(key, MAX_PATH,"SYSTEM\\CurrentControlSet\\Enum\\%s", buffer);
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                LOGD("RegOpenKeyExW error");
            } else {
                nSize = sizeof buffer;
                if (RegQueryValueEx(hKey, "ParentIdPrefix", NULL, NULL,
                                     (LPBYTE)&buffer, &nSize) != ERROR_SUCCESS) {
                    //LOGD("RegQueryValueExW error %S", buffer);
                } else {
                    pParentIdPrefix = buffer;
                }
                RegCloseKey(hKey);
            }
        }

        if(useParentIdPrefix && pParentIdPrefix == NULL) {
            FREE_IF(pDetData);
            continue;
        }

        CDevLabel devId(pDetData->DevicePath, pParentIdPrefix, useParentIdPrefix, service);

        hKey = SetupDiOpenDevRegKey(hDevInfo, &devdata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (hKey == NULL) {
            LOGE("SetupDiOpenDevRegKey %s failed",pDetData->DevicePath);
            continue;
        }

        nSize = sizeof(portName);
        if (RegQueryValueEx(hKey, "PortName", NULL, NULL,(LPBYTE)&portName,
                             &nSize) == ERROR_SUCCESS) {
            devId.SetComPort(portName);
        }

        //LOGW("DEV PATH %S, parentID %S", pDetData->DevicePath, buffer);
        labels.push_back(devId);
        RegCloseKey(hKey);
        FREE_IF(pDetData);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return TRUE;
}

DeviceInterfaces::DeviceInterfaces() :
    mActiveIntf(NULL),
    mDiag(NULL),
    mDeviceActive(DEVICE_UNKNOW),
    mAttachUiPort(false)
{
    const char *default_tag = "N/A Device";
    strncpy_s(mTag, DEV_TAG_LEN, default_tag, strlen(default_tag));
}

DeviceInterfaces::DeviceInterfaces(const DeviceInterfaces & devIntf) {
    DeviceInterfaces::operator =(devIntf);
}


CDevLabel* DeviceInterfaces::SetDiagIntf(CDevLabel& intf) {
    DELETE_IF(mDiag);
    mDiag = new CDevLabel(intf);
    return mDiag;
}



VOID DeviceInterfaces::UpdateDevTag() {
    if(mDiag != NULL ) {
             _snprintf_s(mTag, DEV_TAG_LEN, "COM%d", mDiag->GetComPortNum());
    } else {
        _snprintf_s(mTag, DEV_TAG_LEN, "Device: UNKNOWN");
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
    return 0xdeadbeef;
}

BOOL DeviceInterfaces::SetIntf(CDevLabel& dev, TDevType type, BOOL updateActiveIntf) {
    CDevLabel* pDev = NULL;
    if (type == DEVTYPE_DIAGPORT) {
        pDev = SetDiagIntf(dev);
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

    CDevLabel* diag = devIntf->GetDiagIntf();

    if (mDiag != NULL && (mDiag->Match(diag)))
        return true;

    return false;
}

bool DeviceInterfaces::MatchDevPath(const char * devPath) const {
    LOGE("DO DeviceInterfaces::operator == 2");
    if ( mDiag != NULL && (*mDiag == devPath))
        return true;
    return false;
}

DeviceInterfaces & DeviceInterfaces::operator =(const DeviceInterfaces & devIntfs) {
    mActiveIntf = devIntfs.GetActiveIntf();
    mDiag = devIntfs.GetDiagIntf();
    mDeviceActive = devIntfs.GetDeviceStatus();
    //m_packetDll = devIntfs.GetPacket();
    strcpy_s(mTag, devIntfs.GetDevTag());
    mAttachUiPort = devIntfs.GetAttachStatus();
    return *this;
}

VOID DeviceInterfaces::SetDeviceStatus(usb_dev_t status) {
    mDeviceActive = status;
}

VOID DeviceInterfaces::DeleteMemory(VOID) {
    DELETE_IF(mDiag);
    mActiveIntf = NULL;
    SetDeviceStatus(DEVICE_UNKNOW);
}

DeviceInterfaces::~DeviceInterfaces()
{
    mActiveIntf = NULL;
}


VOID DeviceInterfaces::Dump(const char *tag) {

    if(mDiag)
        mDiag->Dump("diag");
    LOGD("DeviceInterfaces::Dump END ");
}

BOOL DeviceInterfaces::Reset() {
    mActiveIntf = NULL;
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

DeviceInterfaces *DeviceCoordinator::GetValidDevice() {
    list<DeviceInterfaces*>::iterator it;
    for (it = mDevintfList.begin(); it != mDevintfList.end(); ++it) {
        DeviceInterfaces *item = *it;
        usb_dev_t status = item->GetDeviceStatus();
        if (item->GetAttachStatus())
            continue;
        if(status >= DEVICE_PLUGIN && status < DEVICE_REMOVED ) {
            return item;
        }
    }
    return NULL;
}

BOOL DeviceCoordinator::GetDevice(const char * const devPath, DeviceInterfaces** outDevIntf) {
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

BOOL DeviceCoordinator::AddDevice(CDevLabel& dev, TDevType type, DeviceInterfaces** intfs) {
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
        if((*it)->GetAttachStatus()) {
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
CDevLabel::CDevLabel(const char * devPath,
                     const char* parentIdPrefix, bool useParentIdPrefix, const char * name):
    mPortNum(~1),
    mEffectiveSn(~1),
    mEffectivePort (~1)
{
    SetServiceName(name);
    CopyDeviceDescPath(devPath, parentIdPrefix);
    mUseParentIdPrefix = useParentIdPrefix;

    memset(mDevId, 0, sizeof mDevId);
    if (devPath != NULL){
        char delimits[] = {L'#', L'#', L'#'};
        char *sne = (char *)devPath;
        char *snb;
        int len = strlen(devPath);
        for (int i = 0; i < sizeof(delimits)/sizeof(char); i++, sne++) {
            char sep = delimits[i];
            snb = sne;
            sne = (char*)strchr(sne, sep);
            if (sne == NULL || sne - devPath >= len) {
                LOGE("In step %d , '%c' is not found", i, sep);
                return;
            }
        }
        len = sne - snb - 1;
        strncpy_s(mDevId, DEV_ID_LEN, snb, len);

    mEffectiveSn = usb_host_sn(devPath);
    mEffectivePort = usb_host_sn_port(devPath);
    } else {
        LOGE("Can not get invalid device id");
    }

    memset(mMatchId, 0, sizeof mMatchId);
    if (mUseParentIdPrefix && parentIdPrefix != NULL) {
        SetMatchId(parentIdPrefix);
    } else if (strlen(mDevId) > 0) {
      char *sne = ( char*)strrchr(mDevId , L'&');
      if (sne != NULL)
          strncpy_s(mMatchId, DEV_ID_LEN, mDevId, sne - mDevId);
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
void CDevLabel::CopyDeviceDescPath(const char * devPath, const char* parentIdPrefix) {
    if (devPath != NULL) {
        mDevPath = _strdup(devPath);

        if(mDevPath == NULL)
            LOGE("strdup failed");
    } else {
        mDevPath = NULL;
    }

    if (parentIdPrefix != NULL) {
        mParentIdPrefix = _strdup(parentIdPrefix);
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


bool CDevLabel::SetMatchId(const char * matchId){
    if (matchId == NULL )
        return false;
    int len =strlen(matchId);
    if (len == 0 || len >= DEV_MATCHID_LEN) {
        LOGE("matchId length exceed buffer length");
        return false;
    }
    return (0 == strncpy_s(mMatchId, DEV_MATCHID_LEN, matchId, len));
}

bool CDevLabel::SetDevId(const char * devId) {
    if (devId == NULL )
        return false;
    int len =strlen(devId);
    if (len == 0 || len >= DEV_ID_LEN) {
        LOGE("devId length exceed buffer length");
        return false;
    }
    return (0 == strncpy_s(mDevId, DEV_ID_LEN, devId, len));
}

const char * CDevLabel::GetServiceName()  const {
    return mServiceName;
}

bool CDevLabel::SetServiceName(const char * name) {
    if (name == NULL) {
        memset(mServiceName, 0, sizeof mServiceName);
        return false;
    }
    int len = strlen(name);
    if (len == 0 || len >= DEV_SERVICE_LEN) {
        LOGE("name length exceed buffer length");
        return false;
    }
    strncpy_s(mServiceName, DEV_SERVICE_LEN, name, len);
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

bool CDevLabel::SetComPort(const char *portName) {
    //LOGI("PortName %S", portName);
    //swscanf
    sscanf_s(portName, _T("COM%d"), &mPortNum);
    //LOGI("Set comport %d", mPortNum);
    return true;
}

bool CDevLabel::MatchDevPath(const char * devPath) {
    LOGE("DO CDevLabel::operator ==");
    const char *effectivePath = GetDevPath();
    if (effectivePath != NULL && devPath != NULL &&
        0 == strcmp(effectivePath, devPath))
        return true;
    return false;
}


bool CDevLabel::Match(const CDevLabel * const & dev) const {
    const char *effectivePath = NULL;
    const char *effectivePath2 = NULL;
    if (dev == NULL)
        return false;
    if (this == dev)
        return true;

    effectivePath = GetDevPath();
    effectivePath2 = dev->GetDevPath();

    if (effectivePath != NULL && effectivePath2 != NULL &&
        0 == strcmp(effectivePath, effectivePath2)) //stricmp
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
        (0 == strcmp(effectivePath, effectivePath2) ))
        return true;//stricmp

    if (mUseParentIdPrefix || dev->GetParentIdPrefix()) {
        effectivePath = GetMatchId();
        effectivePath2 =dev->GetMatchId();
    }

    if (effectivePath != NULL && effectivePath2 != NULL &&
        (0 == strcmp(effectivePath, effectivePath2) ))
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
    strcpy_s(mDevId, dev.GetDevId());
    strcpy_s(mMatchId, dev.GetMatchId());
    strcpy_s(mServiceName, dev.GetServiceName());
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
long extract_serial_number(char * sn, char **ppstart , char **ppend) {
  unsigned int len;
  char *pstart, *pend;
  if (sn == NULL) {
    LOGE("Bad Parameter");
    return -1;
  }

  len = strlen (sn);

  pstart = (char*)strchr(sn, L'&');
  if (pstart == NULL || pstart - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  pstart++;
  pend = strchr(pstart , L'&');
  if (pend == NULL || pend - sn >= len) {
    //ERROR("Can not find first '&'.");
    return -1;
  }

  if (ppstart != NULL)
    *ppstart = pstart;
  if (ppend != NULL)
    *ppend = pend;
  return strtol(pstart , &pend, 16);
}

// \\?\usb#vid_18d1&pid_d00d#5&10cd67f3&0&4#{f72fe0d4-cbcb-407d-8814-9ed673d0dd6b}
// convert to
// 10cd67f3
long usb_host_sn(const char* dev_name, char** psn) {
    char * snb, *sne, * sn;
    int len = strlen (dev_name); //lstrlen, lstrcmp()
    if(_strnicmp("\\\\?\\",dev_name,4) ) {
        LOGE("Not valid dev name: %S.", dev_name);
        return 0;
    }

    //strtok is not suitable;
    char delimits[] = {L'#', L'#', L'&', L'&'};
    sne = (char *)dev_name;
    for (int i = 0; i < sizeof(delimits)/sizeof(char) ; i++, sne++) {
        char sep = delimits[i];
        snb = sne;
        sne = (char*)strchr(sne, sep);
        if (sne == NULL || sne - dev_name >= len) {
            LOGE("In step %d , %c is not found", i, sep);
            return 0;
        }
    }

    len = sne - snb;
    if (len <= 0)
        return 0;

    if (psn) {
        sn = (char*) malloc((len + 1) * sizeof(char));
        if (sn == NULL) {
            LOGE("NO memory");
        } else {
            strncpy(sn, snb , len);
            *(sn + len) = L'\0';
        }
        *psn = sn;
    }

    return strtol(snb, &sne, 16);
}

long usb_host_sn_port(const char* dev_name) {
     char* begin , *end;
    if (dev_name == NULL)
        return 0;

      begin = ( char*)strrchr(dev_name , L'&');

        if (begin == NULL )
            return 0;

        begin ++;
          end = begin + 1;
  if (*(begin + 1) == L'#' && *(begin - 3) == L'&')
    return strtol(begin , &end, 16);

  return 0;
}
