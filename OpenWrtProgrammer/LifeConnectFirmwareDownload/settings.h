
//////////////////////////////////////////////////////
// Projet TFTPD32.  Mars 2000 Ph.jounin
//
// File settings.h
// settings structure declaration
//
// released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////
#pragma once

#include <vector>
#include <list>
#include <algorithm>
#include <string>

using namespace std;

enum e_SecurityLevels { SECURITY_NONE, SECURITY_STD, SECURITY_HIGH, SECURITY_READONLY };

#define MAXLEN_IPv6             40
#define IPADDR_BUFFER_LEN       16

struct S_Tftpd32Settings
{
    unsigned              Timeout;
    unsigned              Retransmit;
    unsigned              WinSize;
    enum e_SecurityLevels SecurityLvl;
    unsigned              Port;
    BOOL                  bNegociate;
    BOOL                  bPXECompatibility;
    BOOL                  bDirText;
    BOOL                  bMD5;
    BOOL                  bUnixStrings;
    BOOL                  bBeep;
    BOOL                  bVirtualRoot;
	// changed in release 4 : szTftpLocalIP is either an IP address or an interface descriptor
    char                  szTftpLocalIP [max (MAXLEN_IPv6, MAX_ADAPTER_DESCRIPTION_LENGTH+4)];

    unsigned              nTftpLowPort;
    unsigned              nTftpHighPort;
	BOOL                  bPortOption;			// experimental port option
	BOOL                  bIgnoreLastBlockAck;
	BOOL                  bIPv4;
	BOOL                  bIPv6;

	// unsaved settings
	DWORD				  dwMaxTftpTransfers;
    char                  szWorkingDirectory [_MAX_PATH];
    DWORD                 dwRefreshInterval;
	BOOL                  bTftpOnPhysicalIf;
};

extern struct S_Tftpd32Settings sSettings;          // The settings,used anywhere in the code


#define  LOOPBACK_STRING        "127.0.0.1"
#define  DIR_TEXT_FILE          "dir.txt"

#  define TFTP_TIMEOUT            3
#  define TFTP_RETRANSMIT         6
#  define TFTPD32_DEF_LOG_LEVEL   8
#  define TFTP_DEFPORT           69

#define INVALID_TRANSFERID       0XFFFFFFFF
#define TFTP_MAXRETRIES          50 // do not resent same block more than # times
#define TIME_FOR_LONG_TRANSFER   10 // more than 10 seconds -> beep

#define PKGFILES_SECTION        _T("PackageFiles")
#define PKGVERSION_SECTION      _T("PackageVersion")
#define APP_SECTION             _T("App")
#define TELNET_SECTION          _T("Telnet")
#define NETWORK_SECTION         _T("Network")
#define PKG_PATH                _T("PackagePath")
#define PKG_CONFIG              _T("PackageConfig")

static const int FIRMWARE_NUM_MAX = 32;
static const int FIRMWARE_NAME_LEN = MAX_PATH;
static const int FIRMWARE_TBL_LEN = FIRMWARE_NUM_MAX * FIRMWARE_NAME_LEN;

#define USER_LEN_MAX        32
#define PASSWD_LEN_MAX      32
#define FW_CUSTOMID_LEN     32
#define FW_BUILDID_LEN     32

class ConfigIni{
public:
    ConfigIni();
    ~ConfigIni();
    BOOL           ReadConfigIni(const char * ini = _T("Config.ini"));
    const char *   GetPackageDir(void) { return pkg_dir;};
    const char *   GetAppConfIniPath(void) { return m_ConfigPath.GetString();};
    // const char *GetPkgConfXmlPath(void) {return pkg_conf_file;};
    BOOL           GetForceUpdateFlag(void) { return m_forceupdate;};
    BOOL           GetAutoWorkFlag(void) { return m_bWork;};
    list<char *>   GetFirmwareFiles(void) { return m_FirmwareFiles;};
    BOOL           ReadFirmwareFiles(const char* packageFolder, BOOL dummy = FALSE);
    int            SetPackageDir(const char* config, BOOL updateConfig);
    const char * const GetNetworkSegment() {  return m_NetworkSegment;};
    const char * const GetLoginUser() { return  m_User;};
    const char * const GetLoginPassword() { return m_Passwd;};
    const char * const GetFirmwareCustomId() { return m_FirmwareCustomId;};
    const char * const GetFirmwareBuildId() { return m_FirmwareBuildId;};
    VOID           GetFirmwareVersion(CString &version) {
      version = m_FirmwareCustomId;
      version += "_";
      version += m_FirmwareBuildId;
    };
    BOOL            IsLoginTelnet() { return m_Login;};
    BOOL           IsPackageChecked() { return ReadFirmwareFiles(pkg_dir, TRUE);}
    int            GetHostIPStart() { return m_HostIPStart;};
    int            GetHostIPEnd() { return m_HostIPEnd;};
    int            GetTelnetTimeoutMs() { return m_TelnetTimeoutMs;};
private:
    BOOL           DestroyFirmwareFiles();
    BOOL           AddFirmwareFiles(const char* const file, BOOL dummy);
    VOID           AssignPackageDir(const char *dir);
private:
    CString                 mModulePath;
    CString                 m_ConfigPath;
    CString                 m_FirmwareConfig;
    volatile BOOL           m_bWork;
    BOOL                    m_forceupdate; // do not check version, if not exist config.xml or version rule is not match
    char                    pkg_dir[MAX_PATH];
    //   char                 pkg_conf_file[MAX_PATH];
    char                    m_NetworkSegment[IPADDR_BUFFER_LEN];
    char                    m_User[USER_LEN_MAX];
    char                    m_Passwd[PASSWD_LEN_MAX];
    int                     m_HostIPStart;
    int                     m_HostIPEnd;
    BOOL                    m_Login;
    int                     m_TelnetTimeoutMs;
    char                    m_FirmwareCustomId[FW_CUSTOMID_LEN];
    char                    m_FirmwareBuildId[FW_BUILDID_LEN];
    list<char *>            m_FirmwareFiles;
};


