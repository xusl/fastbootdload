//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

// some shortcurts
#include "StdAfx.h"
#include <stdio.h>
#include "Tftp.h"
#include "settings.h"
#include "log.h"

using namespace std;

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
    m_FirmwareFiles(),
    m_forceupdate(FALSE),
    m_bWork(FALSE),
    m_PackageChecked(FALSE){
    memset(pkg_dir, 0, sizeof pkg_dir);
}

#define CONFIG_BUFFER_LEN 32
BOOL ConfigIni::ReadConfigIni(const char * ini){
    LPCTSTR lpFileName;
    int data_len;
    char log_conf[MAX_PATH]= {0};
    CString appPath= _T(".\\");

    char path_buffer[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char filename[MAX_PATH] = {0};
    char buffer[CONFIG_BUFFER_LEN] = {0};
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
    GetPrivateProfileString(NETWORK_SECTION, _T("NetworkSegment"), "192.168.1",
                        m_NetworkSegment, IPADDR_BUFFER_LEN, lpFileName);

    LOGD("network segment : %s", m_NetworkSegment);
    m_HostIPStart = GetPrivateProfileInt(NETWORK_SECTION, _T("HostIPStart"), 2, lpFileName);
    m_HostIPEnd = GetPrivateProfileInt(NETWORK_SECTION, _T("HostIPEnd"), 10, lpFileName);

    m_TelnetTimeoutMs = GetPrivateProfileInt(TELNET_SECTION, _T("TimeoutMs"), 6000, lpFileName);
    GetPrivateProfileString(TELNET_SECTION, _T("User"), "root",
                        m_User, USER_LEN_MAX, lpFileName);
    GetPrivateProfileString(TELNET_SECTION, _T("Password"), "root",
                        m_Passwd, PASSWD_LEN_MAX, lpFileName);

    memset(buffer, 0, sizeof buffer);
    GetPrivateProfileString(TELNET_SECTION, _T("Login"), "false",
                        buffer, CONFIG_BUFFER_LEN, lpFileName);
    if (stricmp(buffer, "false") == 0)
        m_Login = FALSE;
    if (stricmp(buffer, "true") == 0)
        m_Login = TRUE;

    data_len = GetPrivateProfileString(APP_SECTION,
                                       PKG_PATH,
                                       mModulePath.GetString(),
                                       path_buffer,
                                       MAX_PATH,
                                       lpFileName);
    if (data_len > 0)
        AssignPackageDir(path_buffer);

    strncpy(sSettings.szWorkingDirectory, pkg_dir, sizeof sSettings.szWorkingDirectory);

    m_PackageChecked = ReadFirmwareFiles(pkg_dir);
    return TRUE;
}

int ConfigIni::SetPackageDir(const char* config) {
    if (config == NULL) {
        return -1;
    }

    AssignPackageDir(config);
    WritePrivateProfileString(APP_SECTION, PKG_PATH, pkg_dir, m_ConfigPath.GetString());
    strncpy(sSettings.szWorkingDirectory, pkg_dir, sizeof sSettings.szWorkingDirectory);
    DestroyFirmwareFiles();
    ReadFirmwareFiles(pkg_dir);

    return 0;
}

VOID ConfigIni::AssignPackageDir(const char *dir) {
    ASSERT(dir != NULL );
    int data_len = strlen(dir);
    if (data_len == 0 || data_len >= sizeof pkg_dir) {
        LOGE("DIR is null or it is too long");
        return;
    }
    strncpy(pkg_dir, dir, data_len);
        if (pkg_dir[data_len - 1] != _T('\\') ) {
        if ( data_len > MAX_PATH - 2) {
            LOGE("bad package directory in the section path.");
            return;
        }
        pkg_dir[data_len] = _T('\\');
        pkg_dir[data_len + 1] = _T('\0');
    }
}
int ConfigIni::ReadFirmwareFiles(const char* packageFolder, BOOL dummy) {
  char firmware_tbl[FIRMWARE_TBL_LEN] = {0};
  char filename[MAX_PATH];
  char *firmware;
  size_t firmware_len;
  int data_len;
  const char* config = m_ConfigPath.GetString();
  BOOL result = TRUE;

  if (packageFolder == NULL || strlen(packageFolder) == 0) {
    LOGE("not specified PACKAGE folder or it is invalid");
    return FALSE;
  }


  data_len = GetPrivateProfileString(PKG_SECTION,
                                     NULL,
                                     NULL,
                                     firmware_tbl,
                                     FIRMWARE_TBL_LEN,
                                     config);

  if (data_len == 0) {
    return FALSE;
  }

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
        string path = packageFolder;
        path += "\\";
        path += filename;
        result = result && AddFirmwareFiles(path.c_str(), dummy);
//      AddFirmwareFiles(path.GetString());
//      path.GetBuffer()
//      path.ReleaseBuffer();
    }

    firmware = firmware + firmware_len + 1;
    firmware_len = strlen(firmware);
  }

  return result;
}

BOOL ConfigIni::AddFirmwareFiles(const char* const file, BOOL dummy){
    if (file == NULL) {
        LOGE("Bad parameter");
        return FALSE;
    }

    //if(GetFileAttributes(file) != INVALID_FILE_ATTRIBUTES)
     if(!PathFileExists(file)) {
        return FALSE;
     }

    if (dummy == FALSE) {
        LOGI("add file %s", file);
        m_FirmwareFiles.push_back(strdup(file));
    }
    return TRUE;
}

BOOL ConfigIni::DestroyFirmwareFiles() {
    list<char*>::iterator it;
    for (it = m_FirmwareFiles.begin(); it != m_FirmwareFiles.end(); ++it) {
        char* item = *it;
        //item->DeleteMemory();
        memset(item, 0, strlen(item));
//        delete item;
        *it = NULL;
    }
    m_FirmwareFiles.clear();
    return TRUE;
}

ConfigIni::~ConfigIni() {
    DestroyFirmwareFiles();
}
