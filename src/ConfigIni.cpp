#include "StdAfx.h"
#include "utils.h"
#include "device.h"
#include "resource.h"

#include <ConfigIni.h>


ConfigIni::ConfigIni() : log_file(NULL), log_tag(NULL), log_level(NULL),
m_pack_img(FALSE),m_fix_port_map(TRUE), m_flashdirect(TRUE),
m_forceupdate(FALSE), m_bWork(FALSE),
switch_timeout(300), work_timeout(600),
m_nPort(1), m_nPortRow(1)
{

  memset(pkg_conf_file, 0, sizeof pkg_conf_file);
   memset(pkg_qcn_file, 0, sizeof pkg_qcn_file);
   memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
}

BOOL ConfigIni::ReadConfigIni(const wchar_t * ini){
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


  //StartLogging(log_file, log_level, log_tag);
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

    data_len = GetPrivateProfileString(PKG_SECTION,
                                     PKG_PATH,
                                     GetAppPath(appPath).GetString(),
                                     pkg_dir,
                                     MAX_PATH,
                                     lpFileName);

  if (pkg_dir[data_len - 1] != L'\\' ) {
    if ( data_len > MAX_PATH - 2) {
    LOGE("bad package directory in the section path.");
        return FALSE;
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
    return TRUE;
}

ConfigIni::~ConfigIni() {
  if(log_file) free(log_file);
  if(log_tag) delete log_tag;
  if(log_level) delete log_level;
}

void ConfigIni:: ScanDir (const wchar_t *szDirectory)
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
    lstrcat (szFileSpec, _T("\\*config.ini"));
    hFind = FindFirstFile (szFileSpec, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
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

    }
    while (FindNextFile (hFind, & FindData));

    FindClose (hFind);

}

BOOL ConfigIni::GetDiagPSTNandPrg(wchar_t *filename, int size, BOOL emergency) {
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