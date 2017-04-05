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


#define CPE_SKIP_DIAG_PORT


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
  CString errMsg;
  if (mAppConf.SetPackageDir(dir, errMsg))//m_PackagePath.GetBuffer(/*MAX_PATH*/)))
    		/*m_ConfigPath.GetBuffer(MAX_PATH))*/ {
        m_image->reset(FALSE);
    	m_image->ReadPackage();
        return TRUE;
}
  if (errMsg.GetLength() > 0)

  AfxMessageBox(errMsg, MB_OK);
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
      mTelnetClientThread = AfxBeginThread(RunTelnetServer, this);
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

BOOL PSTManager::FlashDeviceDone() {
	if (mAppConf.GetPlatformType() == PLATFORM_CPE)  {
		LOGD("CPE platform now donot support automatically schedule mechaism.");
		return TRUE;
	}

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
	    LOGD("==========END ScheduleDeviceWork (CPE)==============");
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

        while(NULL != (idleDev = mDevCoordinator.GetValidDevice(DEVTYPE_ANY))) {
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
	mDevCoordinator.Reset();
	//TODO:: Stop telnet client.
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

	GetDevLabelByGUID(&GUID_DEVINTERFACE_COMPORT, SRV_ALCATELUSBSER, devicePath, false);
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

        //data->hWnd->KillTimer( (UINT_PTR)data);
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

    LOGE("get path %S for file %s", filePath, filename.c_str());
    return TRUE;
}

VOID PSTManager::HttpServerMessageCB (PVOID data, int uiPort, string message) {
    PSTManager *m_PSTManager =  (PSTManager *)data;
      UsbWorkData *workData = m_PSTManager->GetWorkData(uiPort);
      if (workData == NULL) {
        return;
      }
      workData->SetPromptMsg(message.c_str());
}

UINT PSTManager::RunTelnetServer(LPVOID wParam){
    PSTManager *manager = (PSTManager*)wParam;
    string gateway;
    string host;
    string result;
    NicManager * nicManager;
    NetCardStruct nic;
    string command;
    int error;
    flash_image *packageImage = manager->GetProjectPackage();
    map<string, CString> openwrtFiles = packageImage->GetOpenWrtFiles();
    UsbWorkData *workData = manager->GetWorkData();

    std::ostringstream ossPort;
    //use system locale, but numpunct use default , so input number do not add a thounsands separator,
    //for example, 8081 => 8,081
    ossPort.imbue(locale::classic()/*std::locale()*/);
	
	if(workData->GetStatus() == USB_STAT_WAITING_CLOSE) {
		LOGE("workdata status is USB_STAT_WAITING_CLOSE, return");
		return 0;
	}

    if (openwrtFiles.size() <= 0) {
        workData->SetInfo(OPENWRT_UPDATED,  "None images to download");
        return -1;
    }

    nicManager = manager->GetNicManager();
    nic = nicManager->GetDefaultNic();
    nicManager->UpdateNic(nic);

    workData->Clean(FALSE);
    workData->SetParallelMode(FALSE);
    workData->SetStatus(USB_STAT_WORKING);

    //#define DESKTOP_TEST

#ifdef DESKTOP_TEST
    gateway = "10.0.0.2";
    nic.GetHostIp(host);
#else
    if (!nic.GetGatewayIp(gateway) || !nic.GetHostIp(host)) {
        workData->SetInfo(OPENWRT_UPDATED, "No device found");
        return ENOMEM;
    }
#endif

    //for thread sync problem, when we invoke mHttpServer.IsServerWorks,
    //http server may not ready, and it will return false;
    if (!manager->mHttpServer.IsServerWorks()) {
        workData->SetInfo(OPENWRT_UPDATED, "File server is not ready.");
        return ENOENT;
    }

    workData->AddDevInfo(_T("Host Connection"), nic.mConnectionName);
    workData->AddDevInfo(_T("Device IP Address"), nic.mGateway);

    workData->SetPromptMsg("Connect to device", PROMPT_TITLE);
    SOCKET socket = ConnectServer(gateway.c_str(), TELNET_PORT);
    if (socket == INVALID_SOCKET) {
        workData->SetInfo(OPENWRT_UPDATED, "Can not connect to device");
        return -1;
    }
    TelnetClient tn(socket, 2000, TRUE);

    tn.set_keepalive(true);
    tn.set_nbio(true);
    tn.register_callback(wParam, HttpServerMessageCB, workData->GetIndex());

    workData->SetPromptMsg("receive welcome message");
	
	Sleep(2000);
	error = tn.send_command(NULL, result);
    error += tn.send_command(NULL, result);
	
#ifdef DESKTOP_TEST
    LOGE("User Login, enter user ");
    workData->SetPromptMsg("telent: auto user login");
    tn.send_command("zen", result, false);
    LOGE("Send password ");
    tn.send_command("zen", result, false);
#else
	//do {
    //	error = tn.send_command(NULL, result);
	//} while (result.find_first_of("@:#$") == string::npos);
#endif

    tn.send_command("echo $PATH", result);
    //workData->SetDevicePortText(PROMPT_TEXT, CString(result.c_str()));

    workData->SetPromptMsg("Update Modem Image", PROMPT_TITLE);
    tn.send_command("/bin/usb_switch_to IPQ", result);
    workData->SetPromptMsg("switch usb to PC");
    //tn.send_command("send_data 254 0 0 8 1 0 0", result);
    tn.send_command("/bin/usb_switch_to PC", result);

    //begin USB download
    error = manager->CPEModemPST(workData);

    //workData->SetDevicePortText(PROMPT_TEXT, _T("switch usb to IPQ"));
    //tn.send_command("send_data 254 0 0 8 0 0 0", result);
    //tn.send_command("/bin/usb_switch_to IPQ", result);

    if (error != 0) {
        workData->SetInfo(OPENWRT_UPDATED, "Update modem failed");
        return 2;
    }

    //tn.set_nbio(false);
    workData->SetPromptMsg("Update Router Image", PROMPT_TITLE);
    //  m_MmiDevInfo = nic.mConnectionName;
    //for (map<string, CString>::iterator it=openwrtFiles.begin(); it != openwrtFiles.end(); it ++) {
    //tn.send_command("cd /tmp/", result);
#if 0
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
#endif
    ossPort << "wget http://" << host << ":" << manager->mHttpServer.GetPort();
    ossPort << "/" << workData->GetIndex() << "/" << openwrtFiles.begin()->first;
    ossPort << " -O /tmp/" << openwrtFiles.begin()->first;

    tn.send_command(ossPort.str().c_str(), result, FALSE);
    //}

    //tn.set_receive_timeout(1000);
    tn.set_verbose(false);
    command = "sysupgrade -n -v /tmp/";
    command.append(openwrtFiles.begin()->first);
    tn.send_command(command.c_str(), result, FALSE);

	size_t found;
    int i = 0;
    BOOL done = FALSE;
	BOOL receivedData = result.size() > 0;
	/*for nbio, timeout is 0.2, and erase a partition may cosume 20+s.*/
    while (i < 500) {
        if (!receivedData) {
            LOGD("(%d) empty message", i);
			if (i++ > 0)
	            goto RCV_NEXT_DATA;
        } else {
	        i = 0;
    	}

        if(result.find("Upgrade completed") != string::npos ||
           result.find("Rebooting system") != string::npos) {
            done = TRUE;
            LOGE("receive message, %s", result.c_str());
            break;
        }
        if (result.find("not a valid FIT image") != string::npos) {
            LOGE("receive error message, %s", result.c_str());
            break;
        }
		
		if (result.back() == '\n') {//result.at(result.size() - 1) == '\n'                 
		    found = result.rfind("\n", result.size() - 2);
        } else {
            found = result.rfind("\n");
        }
		if (found != string::npos) {
            result.erase(0, found + 1);
        }				
        workData->SetPromptMsg(result.c_str());

RCV_NEXT_DATA:
        receivedData = tn.get_command_output(result);	
    };

    if (!done) {
        LOGE("Timeout exceed limit");
	    workData->SetInfo(OPENWRT_UPDATED,  "Update main board failed");
    } else {
		//tn.send_command("halt"/* "poweroff", not use "reboot"*/, result, FALSE);
		workData->AddDevInfo(_T("Router updated"), _T(""));
	
		if (workData->RebootFastboot())
	        workData->SetInfo(OPENWRT_UPDATED,  "Please wait for device reboot.");		
		else
	    	workData->SetInfo(OPENWRT_UPDATED,  "Remove both USB and power cable to reboot.");		

	    nicManager->WaitAddrChanged();
		workData->SetPromptMsg("Device updated", PROMPT_TITLE);
	}
    
    return 0;
}

UINT PSTManager::CPEModemPST(UsbWorkData *workData) {
    DeviceInterfaces * dev;
    //BOOL testFastboot = FALSE;
    ASSERT(workData);

	if (kill_adb_server(DEFAULT_ADB_PORT) == 0 || StopAdbServer()) {
		Sleep(2000);
	}
	
    HandleComDevice(FALSE);
    EnumerateAdbDevice(FALSE);

    int i = 0;
#ifdef CPE_SKIP_DIAG_PORT
    while((dev = mDevCoordinator.GetValidDevice(DEVTYPE_ADB )) == NULL)
#else
    while((dev = mDevCoordinator.GetValidDevice(DEVTYPE_ADB | DEVTYPE_DIAGPORT )) == NULL)
#endif
    {
    	if (workData->IsIdle()) {
			LOGE("Work force stop by user.");
			return 2;
		}
		
        if (i++ > 30) {
	        workData->AddDevInfo(_T("ERROR"), _T("No device found. Please check USB cable."));
            LOGE("timeout for update CPE modem module");
            return 1;
        }

        if ((dev = mDevCoordinator.GetValidDevice(DEVTYPE_FASTBOOT) ) != NULL) {
			LOGE("Found fastboot first! No adb or diag device.");
            if (mAppConf.GetFlashDirectFlag())
                break;
            else {
                workData->AddDevInfo(_T("ERROR"), _T("Modem already in download state."));
                return -1;
        	}
            /*
               if (testFastboot) {
               workData->SetDevicePortText(PROMPT_TEXT, _T("Modem not flash whole image"));
               LOGE("modem still in fastboot state, although tool reboot it.");
               break;
               } else if (dev->GetFastbootHandle()) {
               fastboot fb(NULL);
               fb.fb_queue_reboot();
               workData->SetDevice(dev, mAppConf.GetFlashDirectFlag());
               fb.fb_execute_queue(dev->GetFastbootHandle(), workData, 0);
               LOGE("modem in fastboot state, reboot it.");
               workData->SetDevicePortText(PROMPT_TEXT, _T("Modem already enter download state, reset device"));
               //mDevCoordinator.RemoveDevice(dev);
               mDevCoordinator.RemoveDevice(data->mActiveDevIntf)
               }
               testFastboot = TRUE;
               continue;
               */
        }

        //::WaitForSingleObject(mScheduleEvt, 10 * 1000);
        string text = "Waiting for usb devices";
        for(int k = 0; k <= i % 3; k++)
            text += " ...";
        workData->SetPromptMsg(text.c_str());
        workData->WaitForDevSwitchEvt(FALSE, 2 * 1000);
    };
    //     SLEEP(15000);
    
#ifdef CPE_SKIP_DIAG_PORT
	 if (dev->GetDeviceStatus() == DEVICE_PLUGIN) {
	 	dev->SetDeviceStatus(DEVICE_CHECK);
 	}
#endif

    workData->SetDevice(dev, mAppConf.GetFlashDirectFlag());

    return RunMiFiPST(workData);
}

/*
* update flow is done here. Do update business logically.
*/
UINT PSTManager::RunMiFiPST(LPVOID wParam) {
    UsbWorkData* data = (UsbWorkData*)wParam;
    flash_image  *img;
    DeviceInterfaces *dev;
    BOOL useAdb = TRUE;
    BOOL flashdirect = TRUE;
    AppConfig      *config;

	if (data == NULL ) {
		LOGE("Null parameter, no work data");
		return -2;
	}

    if (!data->CheckValid()) {
        data->SetInfo(FLASH_DONE, "Bad parameter");
        return -1;
    }

    dev = data->mActiveDevIntf;
    img = data->mProjectPackage;
    config = data->mPAppConf;
    useAdb = config->IsUseAdbShell();
    flashdirect = config->GetFlashDirectFlag();

    data->SetPromptMsg(dev->GetDevTag(), TITLE);

    if (dev->GetDeviceStatus() == DEVICE_PLUGIN) {
        DiagPST pst(data, img->GetFileBuffer());
        data->SetPromptMsg("Begin download by Diag", PROMPT_TITLE);
        bool result = pst.DownloadCheck();
        if(result)
            result = pst.RunTimeDiag(data->mPAppConf);

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
            data->SetPromptMsg("Modem enter download state");
            data->WaitForDevSwitchEvt(TRUE, 100 * 1000);
        } else {
            data->SetInfo(FLASH_DONE, "Diag PST occur error! Please check log");
            return 1;
        }
    }
    
    if (( data->usb) == NULL) {
		LOGE("No USB handle for adb device");
        data->SetInfo(FLASH_DONE, "No adb device");
        return -1;
    }

    if (dev->GetDeviceStatus() == DEVICE_CHECK) {
         AdbPST adbPST(config->GetForceUpdateFlag(), MODULE_M850);
		 adbPST.Reboot(data, dev);
//        adbPST.DoPST(data, img, dev);

        dev->SetDeviceStatus(DEVICE_FLASH);
        data->SetPromptMsg("Modem enter download state");
        data->WaitForDevSwitchEvt(TRUE, 60 * 1000);
    }

	/*for HH70, sometimes after send reboot-bootloader command, it will enumerate adb device again.*/
    while((data->usb) == NULL) {		
        data->SetPromptMsg("wait for device enter download state", PROMPT_TITLE);		
	    if (data->WaitForDevSwitchEvt(TRUE, 60 * 1000) != WAIT_OBJECT_0) {
			LOGE("No USB handle for fastboot");
	        data->SetInfo(FLASH_DONE, "No fastboot device");
	        return -1;
	    }
	}

	if (dev->GetDeviceStatus() == DEVICE_FLASH) {
        fastboot fb(data->usb);
        FlashImageInfo const * image;
        data->SetPromptMsg("fastboot download", PROMPT_TITLE);

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
        fb.fb_execute_queue(data->usb, data, img->GetDiagDlImgSize());
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
	m_ParallelMode = TRUE;
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

/*
* the begin of device switch. 
*/
DWORD  UsbWorkData::WaitForDevSwitchEvt(BOOL changeStatus, DWORD dwMilliseconds) {
	DWORD waited;
	if (changeStatus) {
	  usb_close(usb);
	  usb = NULL;
	  stat = USB_STAT_SWITCH;
	  mActiveDevIntf->SetAttachStatus(false);

	  /*Set switch timeout*/
	  //hWnd->SetTimer((UINT_PTR)this, nElapse * 1000, NULL);    
	}
	waited = ::WaitForSingleObject(mDevSwitchEvt,dwMilliseconds);
	if (waited == WAIT_OBJECT_0)
		LOGD("Get signaled, device arrived.");
	else if (waited == WAIT_TIMEOUT)
		LOGI("WaitForSingleObject return WAIT_TIMEOUT");
	else if (waited == WAIT_ABANDONED)
		LOGI("WaitForSingleObject return WAIT_ABANDONED ");
	else 
		LOGE("WaitForSingleObject return %d, error is %d", waited, GetLastError());
    return waited;
}

/*
* the end of device switch. 
*/
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
	if (!m_ParallelMode && stat == USB_STAT_SWITCH)
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
  
  SetStatus(USB_STAT_IDLE);
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
    LOGD("UsbWorkData Do reset");
    if (stat == USB_STAT_WORKING) {
	    if (work != NULL) { //Delete, or ExitInstance
	      work->PostThreadMessage( WM_QUIT, NULL, NULL );
	      //usb_close(usb);
	      //hWnd->KillTimer((UINT_PTR)this);
	  	} else {
	      	SetStatus(USB_STAT_WAITING_CLOSE);
			return TRUE;
	  	}
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

    Clean(FALSE);
    return TRUE;
}

BOOL UsbWorkData::Abort(VOID) {
	SetStatus(USB_STAT_ERROR);
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
  SetStatus(USB_STAT_WORKING);
  start_time_tick = now();
  return TRUE;
}

BOOL UsbWorkData::Start( AFX_THREADPROC pfnThreadProc) {
  LOGD("Start thread to work!");
  pCtl->Reset();
  work = AfxBeginThread(pfnThreadProc, this);
  //GetCurrentThread

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
  //set mMapDevIntf after the first device is updated.
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

/*invoke in work thread*/
BOOL UsbWorkData::SetInfo(UI_INFO_TYPE info_type, PCCH msg) {
    CString prompt;
	
    switch(info_type) {
    case ADB_CHK_ABORT:
        // WHEN ABORT, the device need remove manually, do not schedule next device into this UI port.
        Abort();
        return SetPromptMsg(msg);
        break;

    case FLASH_DONE:
        if (mPAppConf->GetPlatformType() == PLATFORM_CPE) {
            prompt.Format(_T("elapse %.3f Seconds"), GetElapseSeconds());
	        AddDevInfo(_T("Modem updated"), prompt);
			pCtl->SetProgressVisible(FALSE);
			return TRUE;
    	} else {        	
	        if (msg != NULL) {
	            SetPromptMsg(msg);
	            SetPromptMsg("", PROMPT_TITLE);
	        } else {
	            //prompt.Format(_T("Update device sucessfully, elapse %.3f Seconds"), GetElapseSeconds());
	            SetPromptMsg("Device updated", PROMPT_TITLE);
    		}
			Finish();				
        }        
		break;

	case OPENWRT_UPDATED:
		//add for HH70 one thread only		
		Finish();
		SetPromptMsg(msg);
        break;
    }

 // UIInfo* info = new UIInfo(info_type, CString((msg != NULL) ? msg : ""));

  //if (FLASH_DONE && data->hWnd->m_fix_port_map)
  //  sleep(1);
  return hWnd->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info_type,
                          (LPARAM)NULL);   
}


BOOL UsbWorkData::AddDevInfo(CString name, CString value) {
  UIInfo* info = new UIInfo(PORTUI_DEVINFO, value);
  info->mInfoName = name;
  return pCtl->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)this);  
}

BOOL  UsbWorkData::SetPromptMsg(PCCH msg, UI_INFO_TYPE info_type) { 
  UIInfo* info = new UIInfo(info_type, CString((msg != NULL) ? msg : ""));
  return pCtl->PostMessage(UI_MESSAGE_DEVICE_INFO,
                          (WPARAM)info,
                          (LPARAM)this);	
}

BOOL UsbWorkData::SetProgress(int progress) {
    UIInfo* info = new UIInfo(PROGRESS_VAL);
    info->iVal = progress;
    return pCtl->PostMessage(UI_MESSAGE_DEVICE_INFO,
                  (WPARAM)info,
                  (LPARAM)this);
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

BOOL UsbWorkData::RebootFastboot() {
   fastboot fb(NULL);
   if (mActiveDevIntf == NULL) {
   	LOGE("None active usb device");
	return FALSE;
   	}

   if (mActiveDevIntf->GetFastbootHandle() == NULL) {
   	LOGE("None fastboot device");
	return FALSE;
   }
   
   if (usb == NULL) {
   	LOGE("USB handle is already close");
   	return FALSE;
   }
   
   fb.fb_queue_reboot();
   fb.fb_execute_queue(usb, this, 0);
   
   SetPromptMsg("reboot modem.");
	
   return TRUE;
}
