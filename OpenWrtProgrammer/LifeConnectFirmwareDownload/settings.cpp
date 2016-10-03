//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

// registry key :
//       HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32

// some shortcurts
#include "StdAfx.h"
#include <stdio.h>
#include "Tftp.h"
#include "settings.h"
#include "log.h"
struct S_Tftpd32Settings sSettings =
{
	  ".",                   // Base directory
	  TFTP_TIMEOUT,          // default timeout
	  TFTP_RETRANSMIT,       // def retransmission7
	  0,                     // WinSize
	  SECURITY_STD,          // Security
	  TFTP_DEFPORT,          // Tftp Port
	  TRUE,                  // RFC 1782-1785 Negociation
	  FALSE,                 // PXE Compatibility
	  FALSE,                 // do not create dir.txt file
	  FALSE,                 // do not create MD5 file
	  TRUE,                  // Unix like files "/tftpboot/.."
	  FALSE,                 // Do not beep for long transfert
	  FALSE,                 // Virtual Root is not enabled
	  "",                    // do not filter TFTP'slistening interface
//	  TFTPD32_ALL_SERVICES,  // all services are enabled
	  0,  0,                 // use ports assigned by Windows
	  FALSE,                 // do not support port option
	  5,					 // after 5 seconds delete Tftp record
	  FALSE,				 // wait for ack of last TFTP packet
	  TRUE,					 // IPv4
	  TRUE,					 // IPv6

	  // unsaved
	  100,                   // Max Simultaneous Transfers
	  ".",                   // Working Directory
	  2000,                  // refresh Interval
};

ConfigIni::ConfigIni() :
    m_forceupdate(FALSE),
    m_bWork(FALSE){
//    memset(pkg_conf_file, 0, sizeof pkg_conf_file);
    memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
}

BOOL ConfigIni::ReadConfigIni(const char * ini){
    LPCTSTR lpFileName;
    int data_len;
    char log_conf[MAX_PATH]= {0};
    CString appPath= _T(".\\");

    char path_buffer[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char filename[MAX_PATH] = {0};
	SYSTEMTIME time;

//	GetCurrentDirectory(MAX_PATH, currdir);
	GetModuleFileName(NULL, path_buffer, MAX_PATH);
	_splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0);
	mModulePath.Format("%s%s", drive, dir);

    m_ConfigPath = mModulePath;
    m_ConfigPath += ini;

    lpFileName = m_ConfigPath.GetString();

    //init app setting.
    m_bWork = GetPrivateProfileInt(APP_SECTION,_T("autowork"), 0, lpFileName);


    GetPrivateProfileString(_T("MISC"), _T("NetworkSegment"), _T("192.168.1"),
                        m_NetworkSegment, IPADDR_BUFFER_LEN, lpFileName);

    LOGD("network segment : %s", m_NetworkSegment);
    data_len = GetPrivateProfileString(APP_SECTION,
                                       PKG_PATH,
                                       mModulePath.GetString(),
                                       pkg_dir,
                                       MAX_PATH,
                                       lpFileName);

    if (pkg_dir[data_len - 1] != _T('\\') ) {
        if ( data_len > MAX_PATH - 2) {
            LOGE("bad package directory in the section path.");
            return FALSE;
        }
        pkg_dir[data_len] = _T('\\');
        pkg_dir[data_len + 1] = _T('\0');
        data_len++;
    }

#if 0
    memset(pkg_conf_file, 0, sizeof pkg_conf_file);
    wcsncpy(pkg_conf_file, pkg_dir, data_len+1);

    char * candidate[] = {_T("Download.img"), _T("..\\DownloadImage\\Download.img")};

    memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
    for (int i = 0; i < COUNTOF(candidate); i++) {
        wcsncpy(pkg_dlimg_file, pkg_dir, data_len+1);
        wcsncat(pkg_dlimg_file, candidate[i], COUNTOF(pkg_dlimg_file) - data_len);
        if(GetFileAttributes(pkg_dlimg_file) != INVALID_FILE_ATTRIBUTES) {
            break;
        } else {
            memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
        }
    }
#endif
    return TRUE;
}

int ConfigIni::SetPackageDir(const char* config) {
    if (config == NULL) {
        return -1;
    }

    WritePrivateProfileString(APP_SECTION, PKG_PATH, config,m_ConfigPath.GetString());

    return 0;
}

int ConfigIni::ReadFirmwareFiles(const char* config) {
  char firmware_tbl[FIRMWARE_TBL_LEN] = {0};
  char filename[MAX_PATH];
  char *firmware;
  size_t firmware_len;
  CString path;
  int data_len;

  if (config == NULL) {
    LOGE("not specified config file name");
    return -1;
  }

  data_len = GetPrivateProfileString(PKG_SECTION,
                                     NULL,
                                     NULL,
                                     firmware_tbl,
                                     FIRMWARE_TBL_LEN,
                                     config);
#if 0
  if (data_len == 0) {
    LOGW("no %S exist, load default firmware table.", config);
    char *imgs[] = {
      _T("mibib"), _T("sbl1.mbn"),
      _T("sbl2"), _T("sbl2.mbn"),
      _T("rpm"), _T("rpm.mbn"),
      _T("dsp1"), _T("dsp1.mbn"),
      _T("dsp3"), _T("dsp3.mbn"),
      _T("dsp2"), _T("dsp2.mbn"),
      _T("aboot"), _T("appsboot.mbn"),
      _T("boot"), _T("boot-oe-msm9615.img"),
      _T("system"), _T("9615-cdp-image-9615-cdp.yaffs2"),
      _T("userdata"), _T("9615-cdp-usr-image.usrfs.yaffs2"),
    };

    for (int i = 0; i < sizeof(imgs)/ sizeof(imgs[0]); i += 2) {
        //parameter push stack from right to left
        add_image(imgs[i], imgs[i+1], TRUE, config);
    }

    set_package_dir(GetAppPath(path).GetString(), config);
    return 0;
  }
#endif
  firmware = firmware_tbl;
  firmware_len = strlen(firmware);

  while (firmware_len > 0) {
    data_len = GetPrivateProfileString(PKG_SECTION,
                                       firmware,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);
    if (data_len > 0) {
        path = pkg_dir;
        path += filename;
      //add_image(firmware, path.GetBuffer(), 0, config);
      path.ReleaseBuffer();
    }

    firmware = firmware + firmware_len + 1;
    firmware_len = strlen(firmware);
  }

  return 0;
}

const char * const ConfigIni::getNetworkSegment() {
    return m_NetworkSegment;
}

ConfigIni::~ConfigIni() {
}
