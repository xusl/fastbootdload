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
#include "XmlParser.h"
#include "Tftp.h"
#include "settings.h"
#include "log.h"

using namespace std;

struct S_Tftpd32Settings sSettings =
{
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
    m_NicToggle(NIC_SETIF_TOGGLE),
    m_FirmwareConfig("")
{
    memset(pkg_dir, 0, sizeof pkg_dir);
#if USE_SIMPLE_CONFIG
    memset(m_FirmwareCustomId, 0, sizeof m_FirmwareCustomId);
    memset(m_FirmwareBuildId, 0, sizeof m_FirmwareBuildId);
#else
    m_FirmwareBuildId = "";
    m_FirmwareCustomId = "";
    m_FirmwareVersion = "";
#endif
    memset(m_NetworkSegment, 0, sizeof m_NetworkSegment);
    memset(m_User, 0, sizeof m_User);
    memset(m_Passwd, 0, sizeof m_Passwd);
    m_NicToggle = NIC_SETUPDI_TOGGLE;
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

    //network section
    m_NicToggle = GetPrivateProfileInt(NETWORK_SECTION, _T("NICToggle"),
                                  NIC_SETUPDI_TOGGLE, lpFileName);
    m_ToggleTimeoutMs = GetPrivateProfileInt(NETWORK_SECTION, _T("ToggleTimeoutMs"),
                                  10000, lpFileName);
    GetPrivateProfileString(NETWORK_SECTION, _T("NetworkSegment"),
                        "192.168.1", m_NetworkSegment, IPADDR_BUFFER_LEN, lpFileName);

    LOGD("network segment : %s", m_NetworkSegment);
    m_HostIPStart = GetPrivateProfileInt(NETWORK_SECTION, _T("DeviceIPStart"), 2, lpFileName);
    m_HostIPEnd = GetPrivateProfileInt(NETWORK_SECTION, _T("DeviceIPEnd"), 10, lpFileName);

    if (m_HostIPStart > 245)
        m_HostIPStart = 244;
    if (m_HostIPEnd > 255)
        m_HostIPEnd = 254;
    if (m_HostIPEnd < m_HostIPStart)
        m_HostIPEnd = m_HostIPStart;

    //telnet section
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

    //app section
    m_bWork = GetPrivateProfileInt(APP_SECTION,_T("autowork"), 0, lpFileName);
    data_len = GetPrivateProfileString(APP_SECTION,
                                       PKG_CONFIG,
                                       _T("config.xml"),
                                       filename,
                                       MAX_PATH,
                                       lpFileName);
    m_FirmwareConfig = filename;

    data_len = GetPrivateProfileString(APP_SECTION,
                                       PKG_PATH,
                                       mModulePath.GetString(),
                                       path_buffer,
                                       MAX_PATH,
                                       lpFileName);
    SetPackageDir(path_buffer, FALSE);

    return TRUE;
}

int ConfigIni::SetPackageDir(const char* config, BOOL updateConfig) {
    if (config == NULL) {
        return -1;
    }

    AssignPackageDir(config);
    if (updateConfig)
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

BOOL ConfigIni::ParseExternalVersion(string &extVersion, string& customId,
    string& versionCode, string& buildId) {
   char *resp = _strdup( extVersion.c_str());
    char *version, *context, *token;
    int i = 0;
    for (version = resp; ; i++, version = NULL) {
        token = strtok_s(version, "_", &context);
        if (token == NULL) {
            break;
        }
        if (i == 1)
            customId = token;
        else if (i == 2)
            versionCode = token;
        else if (i == 3)
            buildId = token;
    }
    if (resp != NULL)
        free(resp);
    return TRUE;
}

int ConfigIni::ReadFirmwareFiles(const char* packageFolder, BOOL dummy) {
    char firmware_tbl[FIRMWARE_TBL_LEN] = {0};
    char filename[MAX_PATH];
    char *firmware;
    size_t firmware_len;
    int data_len;
    CString config;
    BOOL result = TRUE;
    XmlParser confParser;

    if (packageFolder == NULL || strlen(packageFolder) == 0) {
        LOGE("not specified PACKAGE folder or it is invalid");
        return FALSE;
    }

    int Rc ;
    Rc = GetFileAttributes (packageFolder) ;
    if (Rc == INVALID_FILE_ATTRIBUTES || 0 == (Rc & FILE_ATTRIBUTE_DIRECTORY )) {
        LOGE("%s is an invalid folder", packageFolder);
        return FALSE;
    }

    config = packageFolder;
    if (packageFolder[strlen(packageFolder) -1] != '\\')
        config += '\\';

    config += m_FirmwareConfig;

     if(!PathFileExists(config.GetString())) {
        LOGE("%s is not exist", config.GetString());
        return FALSE;
     }

    confParser.Parse(config);

    if (dummy == FALSE) {
#if USE_SIMPLE_CONFIG
        GetPrivateProfileString(PKGVERSION_SECTION, _T("CustomId"), "",
                                m_FirmwareCustomId, FW_CUSTOMID_LEN, config);
        GetPrivateProfileString(PKGVERSION_SECTION, _T("Version"), "",
                                m_FirmwareVersion, FW_VERSION_LEN, config);
        GetPrivateProfileString(PKGVERSION_SECTION, _T("BuildId"), "",
                                m_FirmwareBuildId, FW_BUILDID_LEN, config);
#else
    string extVersion=confParser.get_XML_Value("External_Ver");
    ParseExternalVersion(extVersion, m_FirmwareCustomId, m_FirmwareVersion, m_FirmwareBuildId);
#endif
    }

    string projectPkg=confParser.get_XML_Value("Project_Code");
    projectPkg.append("_").append(PKGFILES_SECTION);
    data_len = GetPrivateProfileString(projectPkg.c_str(), NULL, NULL, firmware_tbl,
                                       FIRMWARE_TBL_LEN, m_ConfigPath.GetString());
    if (data_len == 0) {
        LOGE("None data in section %s, query default section %s",
            projectPkg.c_str(), PKGFILES_SECTION);
        data_len = GetPrivateProfileString(PKGFILES_SECTION, NULL, NULL,
                                           firmware_tbl, FIRMWARE_TBL_LEN,
                                           m_ConfigPath.GetString());
    }

    if (data_len == 0) {
        LOGE("There are none '%s' in .ini", PKGFILES_SECTION);
        return FALSE;
    }

    firmware = firmware_tbl;
    firmware_len = strlen(firmware);

    while (firmware_len > 0) {
        data_len = GetPrivateProfileString(PKGFILES_SECTION, firmware,
                                       NULL, filename, MAX_PATH,
                                       m_ConfigPath.GetString());

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
        memset(item, 0, strlen(item));
        free(item);
        *it = NULL;
    }
    m_FirmwareFiles.clear();
    return TRUE;
}

ConfigIni::~ConfigIni() {
    DestroyFirmwareFiles();
}
