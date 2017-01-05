#pragma once
#include "stdafx.h"
#include "define.h"
#include <map>
#include <string>


using namespace std;

static const int PORT_NUM_MAX = 9;


class ConfigIni{
public:
    ConfigIni();
    ~ConfigIni();
    BOOL         ReadConfigIni(const wchar_t * ini = L"mdmconfig.ini");
    const wchar_t *GetUpdateImgPkgDir(void) { return pkg_dir;};
    const wchar_t *GetAppConfIniPath(void) { return m_ConfigPath.GetString();};
    const wchar_t *GetPkgDlImgPath(void) {return pkg_dlimg_file;};
    const wchar_t *GetPkgConfXmlPath(void) {return pkg_conf_file;};
    const wchar_t *GetLogFilePath(void) {return log_file;};
    const char  *GetLogTag(void) { return log_tag;};
    const char  *GetLogLevel(void) { return log_level;};
    BOOL         GetFlashDirectFlag(void) { return m_flashdirect;};
    BOOL         GetPortDevFixedFlag(void) { return m_fix_port_map;};
    BOOL         GetForceUpdateFlag(void) { return m_forceupdate;};
    BOOL         GetAutoWorkFlag(void) { return m_bWork; };
    BOOL         GetAfterSaleMode(void) { return m_pack_img;};
    int          GetUiPortTotalCount(void) { return m_nPort;};
    int          GetUiPortRowCount(void) { return m_nPortRow;};
    int          GetPSTWorkTimeout(void) { return work_timeout;};
    BOOL         GetDiagPSTNandPrg(wchar_t *filename, int size, BOOL emergency);
    /*
    this is indicate that use debug mode and use adb command reboot-bootloader to enter fastboot.
    */
    BOOL         IsUseAdb() { return m_UseAdb; }
private:
    void         ScanDir (const wchar_t *szDirectory);

private:
    CString                 m_ConfigPath;
    BOOL                    m_pack_img;
    BOOL                    m_fix_port_map;
    BOOL                    m_flashdirect;  //if device is in fastboot when attach, whether can flash image
    BOOL                    m_forceupdate; // do not check version, if not exist config.xml or version rule is not match
    volatile BOOL           m_bWork;
    int                     m_nPort;
    int                     m_nPortRow;
    int                     switch_timeout;
    int                     work_timeout;
    BOOL                    m_UseAdb;
    wchar_t                 *log_file;
    char                    *log_tag;
    char                    *log_level;
    wchar_t                 pkg_dir[MAX_PATH];
    wchar_t                 pkg_conf_file[MAX_PATH];
    wchar_t                 pkg_dlimg_file[MAX_PATH];
    wchar_t                 pkg_qcn_file[MAX_PATH];
    CString m_strModuleName;
};
