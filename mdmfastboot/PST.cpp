#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "usb_adb.h"
#include "scsicmd.h"
#include "resource.h"
#include <atlstr.h>
#include "PST.h"
#include "PortStateUI.h"
#include <ImgUnpack.h>
#include "fastbootflash.h"
#include "adbhost.h"
#include "usb_vendors.h"
#include "DiagPST.h"
#include "AdbPST.h"
#include "telnet.h"
#include <sstream>

PSTManager::PSTManager( AFX_THREADPROC pfnThreadProc):
    mThreadProc(pfnThreadProc),
	mDevCoordinator(),
	mHttpServer((PVOID)this, &HttpServerGetFileCB, &HttpServerMessageCB),
	m_WorkDev(),
	m_image(NULL),
	m_bInit(FALSE),
	m_GridHeight(0),
    m_GridWidth(0)
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
    mScheduleEvt = CreateEvent(NULL,TRUE,FALSE,_T("PSTManagerSchedule"));


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

BOOL PSTManager::Initialize(CWnd *hWnd, BOOL showPort) {
  int row, column;
  int portsNum = GetPortNum();
  row = GetPortRows();
  column = portsNum / row;

  if (row * column < portsNum) {
    row > column ? column++ :  row++;
  }

  for (UINT i = 0; i < GetPortNum(); i++) {
    m_workdata[i] = new UsbWorkData(i, hWnd, &mAppConf,  m_image);
    if (!showPort)
        m_workdata[i]->ShowSubWindow(showPort);
  }
  RECT rect = GetPortRect(0);
  m_GridHeight = (rect.bottom - rect.top) * row + VERTICAL_GAP * (row - 1);
  m_GridWidth = (rect.right - rect.left) * column + HORIZONAL_GAP * (column - 1);

  //hWnd->Invalidate();
  //mAppConf.ScanDir();
  m_bInit = TRUE;

  mNicManager.EnumNetCards();
  return TRUE;
}

PSTManager::~PSTManager() {
  for (UINT i = 0; i < GetPortNum(); i++) {
    if (m_workdata[i] != NULL) {
       delete m_workdata[i];
       m_workdata[i] = NULL;
    }
  }
  if (m_image != NULL) {
    delete m_image;
    m_image = NULL;
  }
     if (mScheduleEvt!= NULL) {
      ::CloseHandle(mScheduleEvt);
      mScheduleEvt = NULL;
    }
}

BOOL PSTManager::SetPortDialogs(int x, int y) {
    return SetPortDialogs(x, y,GetPortGridWidth(), GetPortGridHeight());
}

BOOL PSTManager::SetPortDialogs(int x, int y,  int w, int h)
{
  //int size = sizeof(m_workdata) / sizeof(m_workdata[0]);
  int r, c, pw, ph;
  CPortStateUI*  port;
  int R_NUM, C_NUM;
  int portsNum = GetPortNum();

  R_NUM = GetPortRows();// mAppConf.GetUiPortRowCount();
  C_NUM = portsNum / R_NUM;

  if (R_NUM * C_NUM < portsNum) {
    R_NUM > C_NUM ? C_NUM ++ :  R_NUM++;
  }
  if (w > m_GridWidth)
      x += (w - m_GridWidth)/C_NUM / 2;
  if (h > m_GridHeight)
      y += (y - m_GridHeight)/R_NUM / 2;

  pw = w / C_NUM;
  ph = h / R_NUM;

  for (r = 0; r < R_NUM; r++) {
    for (c = 0; c < C_NUM; c++) {
      if (r * C_NUM + c >= portsNum) break;

      port = GetPortUI(r * C_NUM + c);// m_workdata[r * C_NUM + c]->pCtl;
      if (port == NULL) continue;
      port->SetWindowPos(0,
                         x + c * pw,
                         y + r * ph,
                         pw,
                         ph,
                         SWP_NOSENDCHANGING | SWP_NOSIZE);
    }
  }

  return true;
}

VOID PSTManager::SetWork(BOOL work, BOOL schedule) {
    m_bWork = work;
    if (!work || !schedule)
        return;

  if (mAppConf.GetPlatformType() == PLATFORM_CPE) {
      AfxBeginThread(CMiniHttpDownloadServer::StartHttpServer, (LPVOID)&mHttpServer);
      AfxBeginThread(RunTelnetServer, this);
   } else if(mAppConf.IsUseAdbShell()) {
        RejectCDROM();
        HandleComDevice(FALSE);
        EnumerateAdbDevice();
   } else {
        RejectCDROM();
        if(!EnumerateAdbDevice(FALSE))
            HandleComDevice();
    }
}

UINT PSTManager::GetPortNum() {
    return min(sizeof m_workdata/ sizeof m_workdata[0], mAppConf.GetUiPortTotalCount());
}

UINT PSTManager::GetPortRows() {
    int rows = 1, columns = 1;
    int num = GetPortNum();
    for (; rows * columns < num; ) {
        if (rows > columns)
            columns++;
        else
            rows++;
    }
    return rows;
}

RECT PSTManager::GetPortRect(UINT index) {
    RECT rect = {0};
    CPortStateUI * port = GetPortUI(index);
    if (port != NULL)
        port->GetClientRect(&rect);
    return rect;
}

UsbWorkData *PSTManager::GetWorkData(UINT index) {
    if (index >= GetPortNum()) {
        LOGE("index(%d) is exceed port number (%d)", index, GetPortNum());
        return NULL;
    }

    return m_workdata[index];
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
  UINT i= 0;
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
    for ( UINT i= 0; i < GetPortNum(); i++) {
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
    DeviceInterfaces* idleDev;
    UsbWorkData* workdata;
    BOOL flashdirect = mAppConf.GetFlashDirectFlag();
    if (!m_bWork) {
        // INFO("do not work now.");
        return FALSE;
    }

    LOGD("==========Begin ScheduleDeviceWork==============");
    if (mAppConf.GetPlatformType() == PLATFORM_CPE) {
//       ::SetEvent(mScheduleEvt);
         workdata = m_workdata[0];
        if (workdata->GetStatus() == USB_STAT_SWITCH)
            workdata->SetDevSwitchEvt(flashdirect);
        return TRUE;
    }

    /*
     * Get usbworkdata that in free or in switch.
     * fix_map means whether logical port (portstatui) fix map physical port.
     * this feature will be useful when multiport port download, it help operator
     * to make sure whether the device in a specific is flashed.
     */

    for (UINT i=0; i < GetPortNum(); i++) {
        workdata = m_workdata[i];
        DeviceInterfaces* item = workdata->mMapDevIntf;
        if (item == NULL)
            item = workdata->mActiveDevIntf;

        if (workdata->GetStatus() == USB_STAT_WORKING)
            continue;

        while(NULL != (idleDev = mDevCoordinator.GetValidDevice(mAppConf.IsUseAdbShell()))) {
            idleDev->Dump(__FUNCTION__);
            if (workdata->GetStatus() == USB_STAT_SWITCH) {
                ASSERT(item != NULL);
                if (!item->Match(idleDev))
                    continue;

                workdata->SetDevSwitchEvt(flashdirect);
                break;
            }

            if (mAppConf.GetPortDevFixedFlag() == FALSE || item == NULL || item->Match(idleDev)) {
                workdata->SetDevice(idleDev, flashdirect);
                workdata->Start(mThreadProc != NULL ? mThreadProc : RunMiFiPST);
                break;
            }
        }
    }
    LOGD("==========END ScheduleDeviceWork==============");

    return TRUE;
}

BOOL PSTManager::Reset() {
    for (UINT i= 0; i < GetPortNum(); i++) {
        m_workdata[i]->Reset();
    }
    mHttpServer.StopHttpServer();
    return TRUE;
}


BOOL PSTManager::EnumerateAdbDevice(BOOL schedule) {
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
                                            mAppConf.IsUseAdbShell(),
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
                                            mAppConf.IsUseAdbShell(),
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
    if (success && schedule)
        ScheduleDeviceWork();
    return success;
}

BOOL PSTManager::HandleComDevice(BOOL schedule) {
    vector<CDevLabel> devicePath;

    if (!m_bWork) {
        INFO("!!!!do not work now.");
        return FALSE;
    }

    GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_JRDUSBSER, devicePath, false);
    //GetDevLabelByGUID(&GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, SRV_JRDUSBSER, devicePath, false);
    //for  COM1, GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //GetDevLabelByGUID(&GUID_DEVCLASS_PORTS , SRV_SERIAL, devicePath, false);
    vector<CDevLabel>::iterator iter;
    //DeviceInterfaces* devintf;
    BOOL success = FALSE;

    for (iter = devicePath.begin(); iter != devicePath.end();++iter) {
        iter->Dump(__FUNCTION__);
        if(mDevCoordinator.AddDevice(*iter,
                                   DEVTYPE_DIAGPORT,
                                   mAppConf.IsUseAdbShell(),
                                   NULL))
            success = TRUE;
    }
    devicePath.clear();
    if (success && schedule)
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
        if (mAppConf.IsUseAdbShell()) {
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

BOOL PSTManager::HttpServerGetFileCB (PVOID data, string filename, CString& filePath) {
    PSTManager *m_PSTManager =  (PSTManager *)data;
    flash_image* packageImage = m_PSTManager->GetProjectPackage();

    try {
        filePath =packageImage->GetOpenWrtFilePath(filename);
    } catch (const std::out_of_range& oor) {
        std::cerr << "Out of Range error: " << oor.what() << '\n';
        LOGE("can not find file %s", filename.c_str());
        return FALSE;
    }

    return TRUE;
}

VOID PSTManager::HttpServerMessageCB (PVOID data, int uiPort, CString message) {
    PSTManager *m_PSTManager =  (PSTManager *)data;
      UsbWorkData *workData = m_PSTManager->GetWorkData(uiPort);
      if (workData == NULL) {
        return;
      }
      workData->SetInfo(PROMPT_TEXT, message);
}

UINT PSTManager::RunTelnetServer(LPVOID wParam){
    PSTManager *manager = (PSTManager*)wParam;
    string gateway;
    string host;
    string result;
    NicManager * nicManager;
    NetCardStruct nic;
    string command;
    flash_image *packageImage = manager->GetProjectPackage();
    map<string, CString> openwrtFiles = packageImage->GetOpenWrtFiles();
    UsbWorkData *workData = manager->GetWorkData();

    if (openwrtFiles.size() <= 0) {
        LOGE("There are none images to download");
        workData->SetInfo(PROMPT_TEXT, _T("There are none images to download"));
        return -1;
    }

    nicManager = manager->GetNicManager();
    nic = nicManager->GetDefaultNic();
    nicManager->UpdateNic(nic);

//#define DESKTOP_TEST

#ifdef DESKTOP_TEST
    gateway = "10.0.0.2";
    nic.GetHostIp(host);
#else
    if (!nic.GetGatewayIp(gateway) || !nic.GetHostIp(host)) {
        LOGE("no memory");
        return ENOMEM;
    }
#endif

    workData->AddDevInfo(_T("Host Connection"), nic.mConnectionName);
    workData->AddDevInfo(_T("Device IP Address"), nic.mGateway);

    workData->SetInfo(PROMPT_TEXT, _T("telent: connect to device"));
    SOCKET socket = ConnectServer(gateway.c_str(), TELNET_PORT);
    if (socket == INVALID_SOCKET) {
        return -1;
    }
    TelnetClient tn(socket, 2000, TRUE);
    tn.set_nbio(true);

    workData->SetInfo(PROMPT_TEXT, _T("telent: receive welcome message"));
    tn.send_command(NULL, result);
    tn.send_command(NULL, result);

#ifdef DESKTOP_TEST
    LOGE("User Login, enter user ");
    workData->SetInfo(PROMPT_TEXT, _T("telent: auto user login"));
    tn.send_command("zen", result, false);
    LOGE("Send password ");
    tn.send_command("zen", result, false);
#endif

    workData->SetInfo(PROMPT_TEXT, _T("telent: switch usb to PC"));
    //tn.send_command("send_data 254 0 0 8 1 0 0", result);
    tn.send_command("usb_switch_to PC", result);

    //begin USB download
    manager->CPEModemPST(workData);

    workData->SetInfo(PROMPT_TEXT, _T("telent: switch usb to IPQ"));
    //tn.send_command("send_data 254 0 0 8 0 0 0", result);
    tn.send_command("usb_switch_to IPQ", result);

    tn.set_nbio(false);
    workData->SetInfo(PROMPT_TITLE, _T("telent: request download file"));
    //  m_MmiDevInfo = nic.mConnectionName;
    //for (map<string, CString>::iterator it=openwrtFiles.begin(); it != openwrtFiles.end(); it ++) {
        //tn.send_command("cd /tmp/", result);
        command = "wget http://";
        command += host;
        command.append("/");
        std::ostringstream oss;
        oss << workData->GetIndex();
        command.append(oss.str());
        command.append("/");
        command += openwrtFiles.begin()->first;
        command += " -O /tmp/";
        command += openwrtFiles.begin()->first;
        //command += it->first;
        tn.send_command(command.c_str(), result, FALSE);
    //}

    workData->SetInfo(PROMPT_TEXT, _T("telent: execute sysupgrade"));
    command = "sysupgrade /tmp/";
    command.append(openwrtFiles.begin()->first);
    tn.send_command(command.c_str(), result);

    workData->SetInfo(PROMPT_TEXT, _T("command is sent, please wait for device upgrade"));
    return 0;
}

VOID PSTManager::CPEModemPST(UsbWorkData *workData) {
    DeviceInterfaces * dev;
    ASSERT(workData);
     HandleComDevice(FALSE);
     EnumerateAdbDevice(FALSE);

     //workData->WaitForDevSwitchEvt();
     int i = 0;
     while((dev = mDevCoordinator.GetValidDevice(mAppConf.IsUseAdbShell())) == NULL) {
        if (i++ > 15) {
            LOGE("timeout for update CPE modem module");
            return;
        }
        //::WaitForSingleObject(mScheduleEvt, 10 * 1000);
        SLEEP(2000);
     };


     workData->SetDevice(dev, mAppConf.GetFlashDirectFlag());

     RunMiFiPST(workData);
}

/*
* update flow is done here. Do update business logically.
*/
UINT PSTManager::RunMiFiPST(LPVOID wParam) {
    UsbWorkData* data = (UsbWorkData*)wParam;
    usb_handle * handle;
    flash_image  *img;
    DeviceInterfaces *dev;
//    int result;
    usb_dev_t status;
    BOOL useAdb = TRUE;
    BOOL flashdirect = TRUE;
    AppConfig      *config;

    if (data == NULL ||  !data->CheckValid()) {
        data->SetInfo(FLASH_DONE, "Bad parameter");
        return -1;
    }

    dev = data->mActiveDevIntf;
    img = data->mProjectPackage;
    config = data->mPAppConf;
    status = dev->GetDeviceStatus();
    useAdb = config->IsUseAdbShell();
    flashdirect = config->GetFlashDirectFlag();

    data->SetInfo(TITLE, dev->GetDevTag());
    if (status == DEVICE_PLUGIN) {
        DiagPST pst(data, img->GetFileBuffer());
        data->SetInfo(PROMPT_TITLE, "Begin download by Diag");
        bool result = pst.DownloadCheck();
        if(result)
            result = pst.RunTimeDiag();

        if(result || pst.IsEmergencyDownloadMode()) {
            if (!pst.IsEmergencyDownloadMode() && useAdb) {
                int count = 0;
                do {
                    if (count != 0)
                        SLEEP(3000);
                    data->UpdateUsbHandle(FALSE, flashdirect);
                }while(data->usb == NULL && count++ < 5);
            }
            /*
            * If device enter TPST status, we does not find adb device.
            */
            if (!pst.IsEmergencyDownloadMode() && useAdb && data->usb != NULL) {
                    AdbPST adbPST(config->GetForceUpdateFlag(), MODULE_M850);
                    adbPST.Reboot(data, dev);
                //} else {
                //    result = FALSE;
                //    LOGE("There are no adb device when use adb reboot");
                //}
            } else {
                result = pst.DownloadPrg(data->mPAppConf);
                if(result) {
                    result = pst.DownloadImages(img);
                }
            }
        } else {
            LOGE("download check failed");
        }

        if(result) {
            dev->SetDeviceStatus(DEVICE_FLASH);
            data->SetInfo(REBOOT_DEVICE, "Enter fastboot");
            data->WaitForDevSwitchEvt();
        } else {
            data->SetInfo(FLASH_DONE, "Diag PST occur error! Please check log");
            return 0;
        }
    }

    handle = data->usb;
    status = dev->GetDeviceStatus();
    if (handle == NULL) {
        data->SetInfo(FLASH_DONE, "Bad parameter");
        return -1;
    }

    if (status == DEVICE_CHECK) {
 //       AdbPST pst(data->mPAppConf->GetForceUpdateFlag(), m_module_name);
//        pst.DoPST(data, img, dev);

    } else if (status == DEVICE_FLASH) {
        fastboot fb(handle);
        FlashImageInfo const * image;
        data->SetInfo(PROMPT_TITLE, "fastboot download");

        fb.fb_queue_display("product","product");
        fb.fb_queue_display("version","version");
        fb.fb_queue_display("serialno","serialno");
        fb.fb_queue_display("kernel","kernel");
#if 0
        //this is check new image version and firmware version by adb
        for (int index = 0; index < data->partition_nr; index++) {
            image = data->flash_partition[index];
            if (image->need_download) {
                fb.fb_queue_flash(image->partition_str, image->data, image->size);
            }
        }
#endif
        image = img->image_enum_init();
        for(;image != NULL ; ) {
            if (image->need_download) {
                fb.fb_queue_flash(image->partition_str, image->data, image->size);
            }
            image = img->image_enum_next(image);
        }

        //  fb.fb_queue_reboot();
        fb.fb_execute_queue(handle, data, img->GetDiagDlImgSize());
        data->SetInfo(FLASH_DONE, NULL);
        dev->SetDeviceStatus(DEVICE_REMOVED);
    }
    return 0;
}

UsbWorkData::UsbWorkData(int index, CWnd* dlg,
    AppConfig *appConf, flash_image* package) {
    hWnd = dlg;
    pCtl = new CPortStateUI;
    pCtl->Create(IDD_PORT_STATE, dlg);
    mIndex = index;
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
    if (NULL != work) {
        LOGE("destory when there are still have thread run");
        AfxEndThread(-1);
        work = NULL;
    }

    ::CloseHandle(mDevSwitchEvt);
}

BOOL UsbWorkData::ShowSubWindow(BOOL show) {
    pCtl->ShowWindow(show ? SW_SHOW :  SW_HIDE);
    return TRUE;
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

BOOL UsbWorkData::SetDevice(DeviceInterfaces* pDevIntf, BOOL flashdirect) {
    ASSERT(pDevIntf != NULL);
    mActiveDevIntf = pDevIntf;

  UpdateUsbHandle(TRUE, flashdirect);

  usb_set_work(usb, TRUE);
  mActiveDevIntf->SetAttachStatus(true);
  stat = USB_STAT_WORKING;
  start_time_tick = now();
  return TRUE;
}

BOOL UsbWorkData::Start( AFX_THREADPROC pfnThreadProc) {
  LOGD("Start thread to work!");
  pCtl->Reset();
  work = AfxBeginThread(pfnThreadProc, this);

  if (work != NULL) {
    INFO("Schedule work for %s !", mActiveDevIntf->GetDevTag());
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

BOOL UsbWorkData::AddDevInfo(CString name, CString value) {
  UIInfo* info = new UIInfo;
  info->infoType = PORTUI_DEVINFO;
  info->sVal = value;
  info->mInfoName = name;
  hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)this);
  return TRUE;
}

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


