#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "usb_adb.h"

#include "scsicmd.h"
#include "resource.h"
#include "qcnlib/QcnParser.h"
#include <msxml.h>
#include <atlstr.h>
#import "msxml6.dll" raw_interfaces_only
#include "PST.h"

#include "PortStateUI.h"

#include <ImgUnpack.h>

PSTManager::PSTManager( AFX_THREADPROC pfnThreadProc):
    mThreadProc(pfnThreadProc),
	mDevCoordinator(),
	m_WorkDev(),
	m_image(NULL)
{
    //construct update software package. get configuration about partition information.
    mAppConf.ReadConfigIni();
    //StartLogging(mAppConf.GetLogFilePath(), mAppConf.GetLogLevel(), mAppConf.GetLogTag());

    m_bWork = mAppConf.GetAutoWorkFlag();

    //if (NULL!=m_image) {
    //    delete m_image;
    //}

    m_image = new flash_image(&mAppConf);

    for (int i = 0; i < sizeof m_workdata/ sizeof m_workdata[0]; i++) {
      m_workdata[i] = NULL;
    }

    //ImgUnpack img;
    //img.UnpackDlImg(mAppConf.GetPkgDlImgPath(),mAppConf.GetAppConfIniPath());
}


BOOL PSTManager::ChangePackage(const wchar_t * dir) {
  if (mAppConf.SetPackageDir(dir))//m_PackagePath.GetBuffer(/*MAX_PATH*/)))
    		/*m_ConfigPath.GetBuffer(MAX_PATH))*/ {

        m_image->reset(FALSE);
    	m_image->ReadPackage();
        return TRUE;
    }

    return FALSE;
}

BOOL PSTManager::Initialize(CWnd *hWnd) {
  for (int i = 0; i < GetPortNum(); i++) {
    m_workdata[i] = new UsbWorkData(i, hWnd, &mAppConf,  m_image);
  }
  //mAppConf.ScanDir();
  return TRUE;
}

PSTManager::~PSTManager() {
  for (int i = 0; i < GetPortNum(); i++) {
    if (m_workdata[i] != NULL) {
       delete m_workdata[i];
       m_workdata[i] = NULL;
    }
  }
  if (m_image != NULL) {
    delete m_image;
    m_image = NULL;
  }
}

int PSTManager::GetPortNum() {
    return min(sizeof m_workdata/ sizeof m_workdata[0], mAppConf.GetUiPortTotalCount());
}

CPortStateUI* PSTManager::GetPortUI(UINT index) {
    if (index >= GetPortNum()) {
        LOGE("index(%d) is exceed port number (%d)", index, GetPortNum());
        return NULL;
    }

    return m_workdata[index]->pCtl;
}

BOOL PSTManager::IsHaveUsbWork(void) {
  UsbWorkData* workdata;
  int i= 0;
  for (; i < GetPortNum(); i++) {
    workdata = m_workdata[i];
    if (!workdata->IsIdle())
        return TRUE ;
  }
  return FALSE;
}

BOOL PSTManager::FlashDeviceDone(UsbWorkData * data) {
    if (data == NULL) {
        LOGE("error, null parameter");
        return FALSE;
    }

    data->Finish();
    if (NULL == mDevCoordinator.IsEmpty()) {
       // AfxMessageBox(L"All devices is updated!");
    }

    if (!mAppConf.GetPortDevFixedFlag()) {
        // schedule next port now, and when  app receice device remove event,
        // the current finished port is not in workdata set, for new device in the
        // same port can not bootstrap in 1 seconds, even when there are no more
        // idle device in other physical port.
        //BUT THIS PRESUME IS NEED TEST.
        sleep(10);

        //data->Clean(mDevCoordinator.IsEmpty());
        ScheduleDeviceWork();
    }
    return TRUE;
}
/*
* get usbworkdata by usb_sn, the device is in switch or in working
* or done, but not idle.
*/
UsbWorkData * PSTManager::FindUsbWorkData(wchar_t *devPath) {
    DeviceInterfaces* devIntf;

    if(!mDevCoordinator.GetDevice(devPath, &devIntf)) {
        return NULL;
    }

    // first search the before, for switch device.
    for ( int i= 0; i < GetPortNum(); i++) {
        DeviceInterfaces* item = m_workdata[i]->mActiveDevIntf;
        if (item == NULL)
            continue;

        if (item->MatchDevPath(devPath)&&
            (m_workdata[i]->GetStatus() != USB_STAT_IDLE))
            return m_workdata[i];
    }

    return NULL;
}

BOOL PSTManager::ScheduleDeviceWork() {
    BOOL flashdirect = mAppConf.GetFlashDirectFlag();
    if (!m_bWork) {
        // INFO("do not work now.");
        return FALSE;
    }

    /*
     * Get usbworkdata that in free or in switch.
     * fix_map means whether logical port (portstatui) fix map physical port.
     * this feature will be useful when multiport port download, it help operator
     * to make sure whether the device in a specific is flashed.
     */
    DeviceInterfaces* idleDev;
    UsbWorkData* workdata;
    LOGD("==========Begin ScheduleDeviceWork==============");

    for (int i=0; i < GetPortNum(); i++) {
        workdata = m_workdata[i];
        DeviceInterfaces* item = workdata->mMapDevIntf;
        if (item == NULL)
            item = workdata->mActiveDevIntf;

        if (workdata->GetStatus() == USB_STAT_WORKING)
            continue;

        while(NULL != (idleDev = mDevCoordinator.GetValidDevice())) {
            idleDev->Dump(__FUNCTION__);
            if (workdata->GetStatus() == USB_STAT_SWITCH) {
                ASSERT(item != NULL);
                if (!item->Match(idleDev))
                    continue;

                workdata->SetDevSwitchEvt(flashdirect);
                break;
            }

            if (mAppConf.GetPortDevFixedFlag() == FALSE || item == NULL || item->Match(idleDev)) {
                workdata->Start(idleDev, mThreadProc, mAppConf.GetPSTWorkTimeout(), flashdirect);
                break;
            }
        }
    }
    LOGD("==========END ScheduleDeviceWork==============");

    return TRUE;
}

BOOL PSTManager::Reset() {
    for (int i= 0; i < GetPortNum(); i++) {
        m_workdata[i]->Reset();
      }
    return TRUE;
}


BOOL PSTManager::EnumerateAdbDevice(VOID) {
    usb_handle* handle;
    vector<CDevLabel> AdbDev;
    vector<CDevLabel> FbDev;
    vector<CDevLabel>::iterator iter;
    BOOL success = FALSE;
    bool match = false;

    if (!m_bWork) {
        INFO("!!!!do not work now.");
        return FALSE;
    }

    GetDevLabelByGUID(&GUID_DEVINTERFACE_USB_DEVICE, SRV_USBCCGP, AdbDev, true);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_USB_DEVICE, SRV_WINUSB, FbDev, false);

    //GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_USBCCGP, AdbDev, true);
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_WINUSB, FbDev, false);

    find_devices(mAppConf.GetFlashDirectFlag());
    handle = usb_handle_enum_init();
    for (; handle != NULL; handle = usb_handle_next(handle)) {
        match = false;
        for (iter = AdbDev.begin(); iter != AdbDev.end();  ++ iter){
            CDevLabel adb(handle->interface_name);
            if (iter->Match(&adb)) {
            //if (iter->MatchDevPath(handle->interface_name)) {
                iter->Dump("adb interface");
                if (!mDevCoordinator.AddDevice(*iter,
                                            DEVTYPE_ADB,
                                            mAppConf.IsUseAdb(),
                                            &handle->dev_intfs))
                    continue;

                handle->dev_intfs->SetAdbHandle(handle);
                success = TRUE;
                match = true;
                break;
            }
        }
        if(match)
            continue;
        //fastboot
        for (iter = FbDev.begin(); iter != FbDev.end(); ++ iter){
            CDevLabel fb(handle->interface_name);
            if (iter->Match(&fb)) {
            //if (iter->MatchDevPath(handle->interface_name)) {
                iter->Dump("fastboot interface");
                if(!mDevCoordinator.AddDevice(*iter,
                                            DEVTYPE_FASTBOOT,
                                            mAppConf.IsUseAdb(),
                                            &handle->dev_intfs))
                    continue;
                handle->dev_intfs->SetFastbootHandle(handle);
                success = TRUE;
                usb_dev_t status = handle->dev_intfs->GetDeviceStatus();
                if ((mAppConf.GetFlashDirectFlag() && status == DEVICE_PLUGIN) ||
                    status == DEVICE_PST ||
                    status == DEVICE_CHECK) {
                    handle->dev_intfs->SetDeviceStatus(DEVICE_FLASH);
                }
                break;
            }
        }
    }

    AdbDev.clear();
    FbDev.clear();

#if 0
    GetDevLabelByGUID(&GUID_DEVINTERFACE_ADB, SRV_WINUSB, devicePath, false);
    for (iter = devicePath.begin();iter != devicePath.end(); ++ iter){
        LOGI("class %S %S",iter->GetParentIdPrefix(), iter->GetDevPath());
    }
#endif
    if (success)
        ScheduleDeviceWork();
    return success;
}

BOOL PSTManager::HandleComDevice(VOID) {
    vector<CDevLabel> devicePath;

    if (!m_bWork) {
        INFO("!!!!do not work now.");
        return FALSE;
    }

    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);
    vector<CDevLabel>::iterator iter;
    DeviceInterfaces* devintf;
    BOOL success = FALSE;

    for (iter = devicePath.begin(); iter != devicePath.end();++iter) {
        iter->Dump(__FUNCTION__);
        if(mDevCoordinator.AddDevice(*iter,
                                   DEVTYPE_DIAGPORT,
                                   mAppConf.IsUseAdb(),
                                   NULL))
            success = TRUE;
    }
    devicePath.clear();
    if (success)
        ScheduleDeviceWork();
    return success;
}

BOOL PSTManager::RejectCDROM(VOID){
    vector<CDevLabel> devicePath;
    vector<CDevLabel>::iterator iter;
    CSCSICmd scsi = CSCSICmd();

    if (!m_bWork) {
        INFO("!!!!do not work now.");
        return FALSE;
    }

    GetDevLabelByGUID(&GUID_DEVINTERFACE_CDROM, SRV_CDROM, devicePath, false);
    GetDevLabelByGUID(&GUID_DEVINTERFACE_DISK, SRV_DISK, devicePath, false);

    for(iter = devicePath.begin();iter != devicePath.end(); ++ iter) {
        CString path = iter->GetDevPath();
        iter->Dump( __FUNCTION__ " : " __FILE__);
        if (path.Find(_T("\\\\?\\usbstor#")) == -1) {
            //LOGI("Fix DISK %S:", path);
            continue;
        }
        path.MakeUpper();
        if (path.Find(_T("ONETOUCH")) == -1 && path.Find(_T("ALCATEL")) == -1) {
            //LOGI("USB Stor %S is not alcatel",path);
            continue;
        }

#if 0
        int  devSize = m_WorkDev.size();
        for(int j=0; j < devSize; j++) {
            if (*iter == m_WorkDev[j]) {
                LOGI("Device is have handle, %S", path);
                continue;
            }
        }
#endif
        if (mAppConf.IsUseAdb()) {
            scsi.SwitchToDebugDevice(path);
            LOGE("Swtich to debug mode");
        } else {
            scsi.SwitchToTPSTDeivce(path);
            LOGE("Swtich to TPST mode");
        }
        //m_WorkDev.push_back(*iter);
    }
    devicePath.clear();
    return TRUE;
}


BOOL PSTManager::HandleDeviceArrived(wchar_t *devPath) {
#if 0
    //ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
    UsbWorkData * data = FindUsbWorkData(devPath);
    if (data == NULL) {
        LOGD("Can not find usbworkdata for %S", devPath);
        return FALSE;
    }
    data->SetSwitchedStatus();
#endif
    return TRUE;
}

BOOL PSTManager::HandleDeviceRemoved(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam) {
    //ASSERT(lstrlen(pDevInf->dbcc_name) > 4);
    UsbWorkData * data = FindUsbWorkData(pDevInf->dbcc_name);
    if (data == NULL) {
        LOGD("Can not find usbworkdata for %S", pDevInf->dbcc_name);
        return FALSE;
    }

    UINT stat = data->GetStatus();
    if (stat == USB_STAT_WORKING) {
        // the device is plugin off when in working, that is because some accident.
        //the accident power-off in switch is handle by the timer.
        //thread pool notify , exit
        if (data->work != NULL)
            data->work->PostThreadMessage( WM_QUIT, NULL, NULL );

        data->hWnd->KillTimer( (UINT_PTR)data);
        usb_close(data->usb);
    } else if (stat == USB_STAT_FINISH) {
        if (!mAppConf.GetPortDevFixedFlag()) {
            ERROR("We do not set m_fix_port_map, "
                  "but in device remove event we can found usb work data");
            return TRUE;
        }
    } else if (stat == USB_STAT_ERROR) {
        //usb_close(data->usb);
    } else {
        return TRUE;
    }

    mDevCoordinator.RemoveDevice(data->mActiveDevIntf);
    data->Clean(mDevCoordinator.IsEmpty());
    ScheduleDeviceWork();

    return TRUE;
}

UsbWorkData::UsbWorkData(int index, CWnd* dlg,
    AppConfig *appConf, flash_image* package) {
    hWnd = dlg;
    pCtl = new CPortStateUI;
    pCtl->Create(IDD_PORT_STATE, dlg);
    pCtl->Init(index);
    memset(mName, 0, sizeof mName);
    _snwprintf_s(mName, WORK_NAME_LEN, _T("Work Port %d"), index);
    mDevSwitchEvt = ::CreateEvent(NULL,TRUE,FALSE,mName);
    ASSERT(mDevSwitchEvt != NULL);
    mPAppConf = appConf;
    mProjectPackage = package;
    mActiveDevIntf = NULL;
    mMapDevIntf = NULL;
    Clean(TRUE);
}

UsbWorkData::~UsbWorkData() {
    DELETE_IF(pCtl);
    if (NULL != mMapDevIntf) {
        mMapDevIntf->DeleteMemory();
        delete mMapDevIntf;
    }
    ::CloseHandle(mDevSwitchEvt);
}

DWORD  UsbWorkData::WaitForDevSwitchEvt(DWORD dwMilliseconds) {
    SwitchDev(dwMilliseconds);
    return ::WaitForSingleObject(mDevSwitchEvt,dwMilliseconds);
}

DWORD  UsbWorkData::SetDevSwitchEvt(BOOL flashdirect) {
    stat = USB_STAT_WORKING;
    if (mActiveDevIntf != NULL) {
      UpdateUsbHandle(TRUE, flashdirect);
      usb_set_work(usb, TRUE);
      mActiveDevIntf->SetAttachStatus(true);
    }
    ::SetEvent(mDevSwitchEvt);
    return 0;
}

BOOL UsbWorkData::IsIdle() {
    //if (stat == USB_STAT_SWITCH || stat == USB_STAT_WORKING)
    if (stat == USB_STAT_WORKING)
        return FALSE;
    return TRUE;
}

BOOL UsbWorkData::Clean(BOOL noCleanUI) {
  usb = NULL;
  if (mActiveDevIntf != NULL) {
    mActiveDevIntf = NULL;
  }
  if (mDevSwitchEvt != NULL)
      ::ResetEvent(mDevSwitchEvt);
  stat = USB_STAT_IDLE;
  work = NULL;
  update_qcn = FALSE;
  partition_nr = 0;
  //start_time_tick = -1;
  ZeroMemory(flash_partition, sizeof(flash_partition));
  if(!noCleanUI)
      pCtl->Reset();
  return TRUE;
}

BOOL UsbWorkData::Reset(VOID) {
    LOGD("Do reset");
    if (stat == USB_STAT_WORKING) {
      if (work != NULL) //Delete, or ExitInstance
        work->PostThreadMessage( WM_QUIT, NULL, NULL );
      //usb_close(usb);
      //hWnd->KillTimer((UINT_PTR)this);
    } else if (stat == USB_STAT_SWITCH) {
      //remove_switch_device(workdata->usb_sn);
      //hWnd->KillTimer((UINT_PTR)this);
    } else if (stat == USB_STAT_FINISH) {
      ;
    } else if (stat == USB_STAT_ERROR) {
      usb_close(usb);
    } else {
      return FALSE;
    }
    start_time_tick = -1;

    Clean(TRUE);
    return TRUE;
}

BOOL UsbWorkData::Abort(VOID) {
    stat = USB_STAT_ERROR;
    //hWnd->KillTimer((UINT_PTR)this);
    return TRUE;
}

BOOL UsbWorkData::UpdateUsbHandle(BOOL force, BOOL flashdirect) {
    if (force || usb == NULL) {
      usb = mActiveDevIntf->GetUsbHandle(flashdirect);
      return TRUE;
    }
    return FALSE;
}

BOOL UsbWorkData::Start(DeviceInterfaces* pDevIntf, AFX_THREADPROC pfnThreadProc, UINT nElapse, BOOL flashdirect) {
    ASSERT(pDevIntf != NULL);
    mActiveDevIntf = pDevIntf;
    LOGD("Start thread to work!");

  UpdateUsbHandle(TRUE, flashdirect);

  usb_set_work(usb, TRUE);
  mActiveDevIntf->SetAttachStatus(true);
  stat = USB_STAT_WORKING;
  start_time_tick = now();
  pCtl->Reset();
  work = AfxBeginThread(pfnThreadProc, this);

  if (work != NULL) {
    INFO("Schedule work for %s with timeout %d seconds!", mActiveDevIntf->GetDevTag(), nElapse);
    work->m_bAutoDelete = TRUE;
    // hWnd->SetTimer((UINT_PTR)this, nElapse * 1000, NULL);
  } else {
    LOGE("%s : Can not begin thread!", mActiveDevIntf->GetDevTag());
    usb_set_work(usb, FALSE);
    Clean();
  }
    return TRUE;
}

BOOL UsbWorkData::Finish(VOID) {
  stat = USB_STAT_FINISH;
  if ( mMapDevIntf == NULL  &&
     mActiveDevIntf  != NULL &&
     mActiveDevIntf->GetDiagIntf() != NULL &&
     mActiveDevIntf->GetFastbootIntf() != NULL) {
    mMapDevIntf = new DeviceInterfaces();
    mMapDevIntf->SetDiagIntf(*mActiveDevIntf->GetDiagIntf());
    mMapDevIntf->SetFastbootIntf(*mActiveDevIntf->GetFastbootIntf());
  }
  usb_close(usb);
  Clean(TRUE);

  //data->work = NULL;
  //KILL work timer.
  //hWnd->KillTimer((UINT_PTR)this);
  return TRUE;
}


BOOL UsbWorkData::SwitchDev(UINT nElapse) {
  usb_close(usb);
  usb = NULL;
  work = NULL;
  stat = USB_STAT_SWITCH;
  mActiveDevIntf->SetAttachStatus(false);

  /*Set switch timeout*/
  //hWnd->SetTimer((UINT_PTR)this, nElapse * 1000, NULL);
    return TRUE;
}

BOOL UsbWorkData::SetSwitchedStatus() {
    if ( stat == USB_STAT_SWITCH) {
        LOGI("Kill switch timer");
        //hWnd->KillTimer((UINT_PTR)this);
        stat = USB_STAT_WORKING;
    } else {
        Log("device does not in switch mode");
    }
    return TRUE;
}

/*invoke in work thread*/
BOOL UsbWorkData::SetInfo(UI_INFO_TYPE info_type, PCCH msg) {
  UIInfo* info = new UIInfo;

  //if (FLASH_DONE && data->hWnd->m_fix_port_map)
  //  sleep(1);

  info->infoType = info_type;
  info->sVal = msg;
  hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)this);
  return TRUE;
}

/*invoke in UI thread.*/
BOOL UsbWorkData::SetInfo(UI_INFO_TYPE infoType, CString strInfo)
{
    pCtl->SetInfo(infoType, strInfo);
    return TRUE;
};

UINT UsbWorkData::SetProgress(int progress) {
    UIInfo* info = new UIInfo;
    info->infoType = PROGRESS_VAL;
    info->iVal = progress;
    hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                  (WPARAM)info,
                  (LPARAM)this);
    return 0;
}

float UsbWorkData::GetElapseSeconds() {
    if (start_time_tick == -1)
        return 0.0;
/*
     long long elapse = now() - start_time_tick;
     int int_part = elapse / MILLS_SECONDS;
     int float_part = (elapse % MILLS_SECONDS) / MICRO_SECONDS;
     float dd = ((float)(now() - start_time_tick)) / MILLS_SECONDS;
*/
     return ((float)(now() - start_time_tick)) / MILLS_SECONDS;
}
BOOL UsbWorkData::Log(const char * msg) {
    LOGI("%s::%s", GetDevTag() , msg);
    return TRUE;
}

flash_image::flash_image(AppConfig *appConfig):
  image_list(NULL),
  image_last(NULL),
  nv_buffer(NULL),
  nv_num(0),
  nv_cmd(NULL),
  mDiagDlImgSize(0),
  mFbDlImgSize(0)
{
    mAppConfig = appConfig;
    ReadPackage();
}

flash_image::~flash_image() {
  //reset(TRUE);
}


BOOL flash_image::ReadPackage() {
    CString path;
    int data_len;
    ProjectConfig projectConfig;

    mAppConfig->GetProjectConfig(projectConfig);
    const wchar_t *projectConfigFile = projectConfig.GetConfigPath().GetString();
    const wchar_t* pkg_dir = mAppConfig->GetUpdateImgPkgDir();
    read_fastboot_config(projectConfigFile, pkg_dir);
    read_diagpst_config(projectConfigFile, pkg_dir);
    //read_package_version(mAppConfig->pkg_conf_file);
    return TRUE;
}

int flash_image::read_diagpst_config(const wchar_t* config, const wchar_t* pkg_dir) {
  wchar_t partition_tbl[PARTITION_TBL_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *partition;
  size_t partition_len;
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                     NULL,
                                     NULL,
                                     partition_tbl,
                                     PARTITION_TBL_LEN,
                                     config);

  if (data_len == 0) {
    LOGW("no DIAG PST in%S .", config);
    return 0;
  }

  partition = partition_tbl;
  partition_len = wcslen(partition);

  while (partition_len > 0) {
    data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
//        path.Empty();
        path = pkg_dir;
        //path += L'\\';
        path += filename;
      AddFileBuffer(partition, path.GetBuffer(), filename);
      path.ReleaseBuffer();
    }

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);
  }

  return 0;
}

int flash_image::read_fastboot_config(const wchar_t* config, const wchar_t* pkg_dir) {
  wchar_t partition_tbl[PARTITION_TBL_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *partition;
  size_t partition_len;
  CString path;
  int data_len;

  if (config == NULL) {
    ERROR("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                     NULL,
                                     NULL,
                                     partition_tbl,
                                     PARTITION_TBL_LEN,
                                     config);

  if (data_len == 0) {
    WARN("no %S exist, load default partition table.", config);
#if 0
    wchar_t *imgs[] = {
      L"mibib", L"sbl1.mbn",
      L"sbl2", L"sbl2.mbn",
      L"rpm", L"rpm.mbn",
      L"dsp1", L"dsp1.mbn",
      L"dsp3", L"dsp3.mbn",
      L"dsp2", L"dsp2.mbn",
      L"aboot", L"appsboot.mbn",
      L"boot", L"boot-oe-msm9615.img",
      L"system", L"9615-cdp-image-9615-cdp.yaffs2",
      L"userdata", L"9615-cdp-usr-image.usrfs.yaffs2",
    };

    for (int i = 0; i < sizeof(imgs)/ sizeof(imgs[0]); i += 2) {
        //parameter push stack from right to left
        add_image(imgs[i], imgs[i+1], TRUE, config);
    }

    //set_package_dir(GetAppPath(path).GetString());
#endif
    return 0;
  }

  partition = partition_tbl;
  partition_len = wcslen(partition);

  while (partition_len > 0) {
    data_len = GetPrivateProfileString(PARTITIONTBL_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
//        path.Empty();
        path = pkg_dir;
        //path += L'\\';
        path += filename;
      add_image(partition, path.GetBuffer(), 0, config);
      path.ReleaseBuffer();
    }

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);
  }

  return 0;
}

int flash_image::add_image( wchar_t *partition, const wchar_t *lpath, BOOL write, const wchar_t* config)
{
  FlashImageInfo* img = NULL;
  if (partition == NULL || lpath == NULL) {
    ERROR("Bad parameter");
    return -1;
  }

  img = (FlashImageInfo *)calloc(1, sizeof(FlashImageInfo));
  //img = (FlashImageInfo *)malloc(sizeof(FlashImageInfo));
  if (img == NULL) ERROR("out of memory");
  //memset(img, 0, sizeof(FlashImageInfo));
  img->data = load_file(lpath, &img->size);

  if (img->data == NULL) {
    ERROR("can not load data from file %S for  partition %S", lpath, partition);
    free(img);
    return -1;
  }
  int iDl = GetPrivateProfileInt(PARTITIONTBL_DL,
									  partition,
									  1,
									  config);

  img->need_download = (1==iDl)?true:false;
  img->partition = wcsdup(partition);
  img->partition_str = WideStrToMultiStr(partition);
  img->lpath = wcsdup(lpath);

  if (image_last != NULL)
    image_last->next = img;
  else
    image_list = img;

  image_last = img;

  DEBUG("Load data from file %S for partition %S", lpath, partition);

  if (write && config != NULL) {
    WritePrivateProfileString(PARTITIONTBL_SECTION,partition,lpath,config);
  }

  return 0;
}

bool flash_image::AddFileBuffer(const wchar_t *partition, const wchar_t *lpath, const wchar_t *fileName) {
    char * part = WideStrToMultiStr(partition);
    char * fn = WideStrToMultiStr(fileName);
    //int bytes = sizeof (wchar_t) * (2+ wcslen(pkgPath)+ wcslen(filName));
    //wchar_t *lpath = (wchar_t *)malloc(bytes);
    if (part != NULL && fn != NULL && lpath != NULL){
        //memset(lpath, 0, bytes);
        FileBufStruct afBuf;
        afBuf.strFileBuf = (uint8*)load_file(lpath, &afBuf.uFileLens);
        if(afBuf.strFileBuf != NULL) {
        afBuf.strFileName = strdup(fn);
        afBuf.isDownload= true;
        strcpy((char *)(afBuf.partition + 2), part);
        m_dlFileBuffer.insert(std::pair<string,FileBufStruct>(fn,afBuf));
        LOGE("Insert diag file %s , partition %s", fn, part);
        }
    }
    DELETE_IF(part);
    DELETE_IF(fn);
    //FREE_IF(lpath);
    return true;
}

int flash_image::GetDiagDlImgSize() {
    if (mDiagDlImgSize != 0)
        return mDiagDlImgSize;

    std::map<string,FileBufStruct>::iterator it;
    for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++) {
        if(it->second.isDownload) {
            string Only_DownLoad("appsboot.mbn,tz.mbn,sbl1.mbn,rpm.mbn,appsboot_fastboot.mbn");
            string mode=it->first;
            if(Only_DownLoad.find(mode)!=-1) {
                mDiagDlImgSize+=it->second.uFileLens;
            } else {
               it->second.isDownload=false;
           }
        }
    }
    return mDiagDlImgSize;
}

int flash_image::GetFbDlImgSize() {
    if (mFbDlImgSize != 0)
        return mFbDlImgSize;
    FlashImageInfo const *image = image_enum_init();
    for(;image != NULL; image = image_enum_next(image)) {
        if (image->need_download) {
            mFbDlImgSize += image->size;
        }
    }
    return mFbDlImgSize;
}

#if 0
BOOL flash_image::set_package_dir(const wchar_t * dir) {
    if(dir == NULL || !PathFileExists(dir)) {
        ERROR("%S%S", dir == NULL ? _T("Null parameter") : dir,
                      dir == NULL ? _T("") : _T(" is not exist!"));
        return FALSE;
    }

    if (wcscmp(dir, pkg_dir) == 0) {
        INFO("Package directory is not changed.");
        return FALSE;
    }

    wcscpy(pkg_dir, dir);
    WritePrivateProfileString(PKG_SECTION, PKG_PATH, dir, mAppConfigFile.GetString());

    return TRUE;
}
#endif

const FlashImageInfo* flash_image::get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize) {
  FlashImageInfo* img;

  // ASSERT( ppdata == NULL || psize == NULL );

  for (img = image_list; img; img = img->next) {
    if (wcscmp(partition, img->partition) == 0) {
      if(ppdata != NULL)
        *ppdata = img->data;
      if(psize != NULL)
        *psize = img->size;
      return img;
    }
  }
  return NULL;
}

const FlashImageInfo* flash_image::image_enum_init (void) {
    return image_list;
}

const FlashImageInfo* flash_image::image_enum_next (const FlashImageInfo* img) {
    if (img == NULL)
        return NULL;

    return img->next;
}

BOOL flash_image::qcn_cmds_enum_init (char *cmd) {
  if (cmd == NULL)
    cmd = "nv write";

  if (nv_cmd == NULL || strcmp(nv_cmd, cmd) != 0)  {
     FREE_IF(nv_cmd);
     nv_cmd = strdup(cmd);
     if(nv_cmd == NULL) {
      ERROR("OUT OF MEMORY");
      return FALSE;
     }

     if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
  }

    if (nv_buffer == NULL)
    {
        QcnParser chQcn;

        if (chQcn.OpenDocument(mAppConfig->GetPkgQcnPath()) == FALSE)
        {
        return FALSE;
        }
        return chQcn.GetNVWriteCommands(nv_cmd, &nv_buffer, &nv_num);
        // chQcn.PutNVWriteCommands(nvBuf, dwLens);
    }

    return TRUE;
}

const char* flash_image::qcn_cmds_enum_next (unsigned int index) {
  if (nv_buffer == NULL) {
    ERROR("No nv cmds buffer, may be should call qcn_cmds_enum_init first!");
    return NULL;
  }

  if (index >= nv_num) {
    ERROR("index (%d) is exceed nv number (%d)!", index, nv_num);
    return NULL;
  }

  return *(nv_buffer + index);
}

BOOL flash_image::set_download_flag(CString strPartitionName, bool bDownload) {
	BOOL bRet = 0;
	FlashImageInfo* img = image_list;
	for(;img != NULL; ) {
		if (0 == wcscmp(img->partition, strPartitionName.GetBuffer()))
		{
            ProjectConfig projectConfig;
            mAppConfig->GetProjectConfig(projectConfig);
            const wchar_t *projectConfigFile = projectConfig.GetConfigPath().GetString();
			img->need_download = bDownload;
            WritePrivateProfileString(PARTITIONTBL_DL,
                              strPartitionName.GetString(),
                              bDownload?L"1":L"0",
                              projectConfigFile);
			bRet = 1;
			break;
		}
		img = img->next;
	}
	return bRet;
}

BOOL flash_image::reset(BOOL free_only) {
    FlashImageInfo *img;
    for (img = image_list; img; img = image_list) {
      image_list = img->next;
      if (img->partition != NULL) {
        free(img->partition);
        img->partition = NULL;
      }

      if (img->partition_str != NULL) {
        delete img->partition_str;
        img->partition_str = NULL;
      }

      if (img->lpath != NULL) {
        free(img->lpath);
        img->lpath = NULL;
      }

      if (img->data != NULL) {
        free(img->data);
        img->data = NULL;
      }

      free(img);
      img = NULL;
    }

    image_list = NULL;
    image_last = NULL;

    if (!free_only) {
      if (nv_buffer != NULL) {
      QcnParser::PutNVWriteCommands(nv_buffer, nv_num);
      nv_buffer = NULL;
      nv_num = 0;
      }
      FREE_IF (nv_cmd);
    }

  std::map<string,FileBufStruct>::iterator it;
    for (it = m_dlFileBuffer.begin(); it != m_dlFileBuffer.end(); it++)
    {
        FREE_IF(it->second.strFileBuf);
         FREE_IF(it->second.strFileName);
    }

  mDiagDlImgSize = 0;
  mFbDlImgSize = 0;
	return TRUE;
}

