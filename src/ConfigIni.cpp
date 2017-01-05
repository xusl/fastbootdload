#include "StdAfx.h"
#include "utils.h"
#include "device.h"
#include "resource.h"

#include <ConfigIni.h>


AppConfig::AppConfig() :
        log_file(NULL),
        log_tag(NULL),
        log_level(NULL),
        m_pack_img(FALSE),
        m_fix_port_map(TRUE),
        m_flashdirect(TRUE),
        m_forceupdate(FALSE),
        m_bWork(FALSE),
        switch_timeout(300),
        work_timeout(600),
        m_nPort(1),
        m_nPortRow(1),
        mProjectConfig(_T("\\."))
{
   m_UseAdb = TRUE;
   memset(pkg_conf_file, 0, sizeof pkg_conf_file);
   memset(pkg_qcn_file, 0, sizeof pkg_qcn_file);
   memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
}

BOOL AppConfig::ReadConfigIni(const wchar_t * ini){
  LPCTSTR lpFileName;
  int data_len;
  wchar_t log_conf[MAX_PATH]= {0};
  CString appPath= L".\\";

  GetAppPath(appPath);
  m_ConfigPath = appPath;
  m_ConfigPath += ini;

  lpFileName = m_ConfigPath.GetString();
  //  m_ConfigPath.GetBuffer(int nMinBufLength)

  //read configuration for log system and start log.
  data_len = GetPrivateProfileString(L"log",L"file",NULL,log_conf, MAX_PATH,lpFileName);
  if (data_len) log_file = wcsdup(log_conf);

  memset(log_conf, 0, sizeof log_conf);
  data_len = GetPrivateProfileString(L"log",L"tag",L"all",log_conf, MAX_PATH,lpFileName);
  if (data_len) log_tag = WideStrToMultiStr(log_conf);

  memset(log_conf, 0, sizeof log_conf);
  data_len = GetPrivateProfileString(L"log",L"level",NULL,log_conf,MAX_PATH,lpFileName);
  if (data_len) log_level = WideStrToMultiStr(log_conf);

  StartLogging(log_file, log_level, log_tag);
  //construct update software package. get configuration about partition information.
  //if (NULL!=m_image)
  //    delete m_image;
  //m_image = new flash_image(lpFileName);

  //init app setting.
  m_pack_img = GetPrivateProfileInt(L"app", L"pack_img", 1,lpFileName);;
  m_fix_port_map = GetPrivateProfileInt(L"app", L"fix_port_map",1,lpFileName);
  switch_timeout = GetPrivateProfileInt(L"app", L"switch_timeout", 300,lpFileName);
  work_timeout = GetPrivateProfileInt(L"app", L"work_timeout",600,lpFileName);

  m_flashdirect = GetPrivateProfileInt(L"app", L"flashdirect", 1,lpFileName);
  m_forceupdate = GetPrivateProfileInt(L"app", L"forceupdate", 0,lpFileName);
  m_bWork = GetPrivateProfileInt(L"app",L"autowork", 0, lpFileName);

  //layout setting.
  m_nPort = GetPrivateProfileInt(L"app", L"port_num",1,lpFileName);
  m_nPortRow = GetPrivateProfileInt(L"app", L"port_row",1,lpFileName);
  if ( m_nPort < 1 )
    m_nPort = 1;
  else if (m_nPort > PORT_NUM_MAX)
    m_nPort = PORT_NUM_MAX;

  if (m_nPortRow < 1 )
    m_nPortRow  = 1;
  else if (m_nPortRow > m_nPort)
    m_nPortRow =  m_nPort;

  if (m_pack_img) {
    m_forceupdate = TRUE; /*Now fw build system can not handle config.xml, so set it to true*/
  }

  ScanDir(appPath.GetString());
  SetupPackageInformation();
  return TRUE;
}

BOOL AppConfig::SetPackageDir(const wchar_t * dir) {
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
    WritePrivateProfileString(PKG_SECTION, PKG_PATH, dir, m_ConfigPath.GetString());
    SetupPackageInformation();
    return TRUE;
}

void AppConfig::SetupPackageInformation() {
  size_t data_len;
CString appPath= L".\\";
//TODO?? , how to determined invalid package.
    data_len = GetPrivateProfileString(PKG_SECTION,
                                         PKG_PATH,
                                         GetAppPath(appPath).GetString(),
                                         pkg_dir,
                                         MAX_PATH,
                                         m_ConfigPath.GetString());

      if (pkg_dir[data_len - 1] != L'\\' ) {
        if ( data_len > MAX_PATH - 2) {
        LOGE("bad package directory in the section path.");
            return ;
        }
        pkg_dir[data_len] = L'\\';
        pkg_dir[data_len + 1] = L'\0';
        data_len++;
      }


  memset(pkg_conf_file, 0, sizeof pkg_conf_file);
  wcsncpy(pkg_conf_file, pkg_dir, data_len+1);
  wcsncat(pkg_conf_file, PKG_CONFIG_XML, COUNTOF(pkg_conf_file) - data_len);

  memset(pkg_qcn_file, 0, sizeof pkg_qcn_file);
  wcsncpy(pkg_qcn_file, pkg_dir, data_len+1);
  wcsncat(pkg_qcn_file, PKG_STATIC_QCN, sizeof(pkg_qcn_file) / sizeof(pkg_qcn_file[0]) - data_len);

  wchar_t * candidate[] = {L"Download.img", L"..\\DownloadImage\\Download.img"};

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
}

AppConfig::~AppConfig() {
  if(log_file) free(log_file);
  if(log_tag) delete log_tag;
  if(log_level) delete log_level;
}

void AppConfig:: ScanDir (const wchar_t *szDirectory)
{
    HANDLE      hFind;
    WIN32_FIND_DATA  FindData;
    wchar_t     szFileSpec [_MAX_PATH + 5];
    FILETIME    FtLocal;
    SYSTEMTIME  SysTime;
    wchar_t        szLine [256];
    wchar_t        szDate [sizeof "jj/mm/aaaa"];

    szFileSpec [_MAX_PATH - 1] = 0;
    lstrcpyn (szFileSpec, szDirectory, _MAX_PATH);
//    lstrcat (szFileSpec, "\\*.*");
    lstrcat (szFileSpec, _T("\\*ProjectConfig.ini"));
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        LOGE("scan dir initialize failed");
        return;
    }

    do {
        // display only files, skip directories
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
#if 0
        FileTimeToLocalFileTime (& FindData.ftCreationTime, & FtLocal);
        FileTimeToSystemTime (& FtLocal, & SysTime);
        GetDateFormat (LOCALE_SYSTEM_DEFAULT,
                       DATE_SHORTDATE,
                       & SysTime,
                       NULL,
                       szDate, sizeof szDate);
        szDate [sizeof "jj/mm/aaaa" - 1]=0;    // truncate date
        FindData.cFileName[62] = 0;      // truncate file name if needed
        // dialog structure allow up to 64 char
        wsprintf (szLine, "%s\t%s\t%d",
                  FindData.cFileName, szDate, FindData.nFileSizeLow);
#endif
        LOGD("Find file %S", FindData.cFileName);
        CString configFile = szDirectory;
        configFile += _T("\\");
        configFile += FindData.cFileName;
        ProjectConfig projectConfig(configFile);
        if (projectConfig.ReadConfig())
            m_SupportProject.insert(std::pair<CString, ProjectConfig>(projectConfig.GetProjectCode(), projectConfig));
    }
    while (FindNextFile (hFind, & FindData));
    FindClose (hFind);
}


BOOL AppConfig::SetProjectCode(string &projectCode) {
    CString code( projectCode.c_str() );
    map<CString,ProjectConfig>::iterator iter=m_SupportProject.find(code);
    if(iter!=m_SupportProject.end()) {
        mProjectConfig = m_SupportProject.at(code);
        return TRUE;
    }
    return FALSE;
}

BOOL AppConfig::GetDiagPSTNandPrg(wchar_t *filename, int size, BOOL emergency) {
    if (filename == NULL || size == 0) {
        LOGE("Bad parameter");
        return FALSE;
    }

   const wchar_t  *prg = PST_NPRG;
   // wchar_t     filename[MAX_PATH];
    TResult     result = EOK;

    if (emergency)
        prg = PST_ENPRG;

    int data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                           prg,
                                           NULL,
                                           filename,
                                           size,//MAX_PATH,
                                           m_ConfigPath.GetString());

    if (data_len == 0) {
        LOGE("Can not found prg file %S in configuration file %S.", prg, m_ConfigPath.GetString());
        return FALSE;
    }

    return TRUE;
}


#define PROJECT_SECTION         _T("project") //L"pst_diag"
ProjectConfig::ProjectConfig(CString configFile):
    mProjectConfigPath(configFile),
    mCode(_T("NullProjectCode")),
    mPlatform(_T("NullProjectPlatform")),
    mVersion(_T("NullProjectVersion")),
    mIsValidConfig(FALSE)
{
}

BOOL ProjectConfig::ReadConfig() {
     WCHAR     buffer[MAX_PATH] = {0};
     int data_len;
     PCTSTR configFile = mProjectConfigPath.GetString();
    if (configFile == NULL)
        return FALSE;

    if (!PathFileExists(configFile))
        return FALSE;

     data_len =  GetPrivateProfileString(PROJECT_SECTION,
                           _T("code"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
   if (data_len == 0) {
        return FALSE;
    }
     mCode = buffer;

     memset(buffer, 0, sizeof buffer);
     data_len = GetPrivateProfileString(PROJECT_SECTION,
                           _T("platform"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
        if (data_len == 0) {
        return FALSE;
    }
     mPlatform = buffer;

     memset(buffer, 0, sizeof buffer);
     data_len = GetPrivateProfileString(PROJECT_SECTION,
                           _T("version"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
        if (data_len == 0) {
        return FALSE;
    }
     mVersion = buffer;
     mIsValidConfig = TRUE;
     return TRUE;
}

ProjectConfig::~ProjectConfig() {
    ;
}
BOOL ProjectConfig::GetDiagPSTNandPrg(wchar_t *filename, int size, BOOL emergency) {
    if (filename == NULL || size == 0) {
        LOGE("Bad parameter");
        return FALSE;
    }

   const wchar_t  *prg = PST_NPRG;
   // wchar_t     filename[MAX_PATH];
    TResult     result = EOK;

    if (emergency)
        prg = PST_ENPRG;

    int data_len = GetPrivateProfileString(DIAGPST_SECTION,
                                           prg,
                                           NULL,
                                           filename,
                                           size,//MAX_PATH,
                                           mProjectConfigPath.GetString());

    if (data_len == 0) {
        LOGE("Can not found prg file %S in configuration file %S.", prg, mProjectConfigPath.GetString());
        return FALSE;
    }

    return TRUE;

}
