#include "StdAfx.h"
#include "utils.h"
#include "device.h"
#include "resource.h"
#include <ConfigIni.h>
#include "QcnParser.h"

#define PACKAGE_HISTORY_MAX 32
#define PACKAGE_BUFFER_LEN ((PACKAGE_HISTORY_MAX) * (MAX_PATH))

AppConfig::AppConfig() :
        log_file(NULL),
        log_tag(NULL),
        log_level(NULL),
        m_pack_img(FALSE),
        m_fix_port_map(TRUE),
        m_flashdirect(TRUE),
        m_forceupdate(FALSE),
        m_bWork(FALSE),
        switch_timeout(3000),
        work_timeout(6000),
        m_nPort(1),
        mProjectConfig(_T("\\."))
{
   m_UseAdb = TRUE;
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
  if (data_len) log_file = _wcsdup(log_conf);

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

  if (m_pack_img) {
    m_forceupdate = TRUE; /*Now fw build system can not handle config.xml, so set it to true*/
  }

  ScanDir(appPath.GetString());
  ReadPackageHistory();
  //if some directory delete after latest run, we should write first.
  WritePackageHistory();
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

    wcscpy_s(pkg_dir, dir);

#if 0
    list<CString>::iterator it;
    for (it = m_PackageDirs.begin(); it != m_PackageDirs.end(); ++it) {
        if (*it == pkg_dir) {
            m_PackageDirs.erase(it);
            return TRUE;
        }
    }
#endif
    m_PackageDirs.remove(pkg_dir);
    m_PackageDirs.push_front(pkg_dir);

    //WritePrivateProfileString(PKG_SECTION, PKG_PATH, dir, m_ConfigPath.GetString());
    WritePackageHistory();
    SetupPackageInformation();

    return TRUE;
}

void AppConfig::WritePackageHistory() {
    list<CString>::iterator it = m_PackageDirs.begin();

    for (; it != m_PackageDirs.end(); ++it) {
        CString key = PKG_PATH;
        if (it != m_PackageDirs.begin())
            key.Format(PKG_PATH _T("%d"), std::distance(m_PackageDirs.begin(), it));
        WritePrivateProfileString(PKG_SECTION, key.GetString(), it->GetString(), m_ConfigPath.GetString());
    }
}

void AppConfig::ReadPackageHistory() {
  wchar_t history[PACKAGE_BUFFER_LEN] = {0};
  wchar_t filename[MAX_PATH];
  wchar_t *cursor;
  size_t length;
  int data_len;

  data_len = GetPrivateProfileString(PKG_SECTION,
                                     NULL,
                                     NULL,
                                     history,
                                     PACKAGE_BUFFER_LEN,
                                     m_ConfigPath.GetString());

  if (data_len == 0) {
    WARN("no %S exist, load default partition table.");
    return ;
  }

  cursor = history;
  length = wcslen(cursor);

  while (length > 0) {
    data_len = GetPrivateProfileString(PKG_SECTION,
                                       cursor,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       m_ConfigPath.GetString());
    if (data_len > 0 && PathFileExists(filename)) {
        m_PackageDirs.push_back(filename);
    }

    cursor = cursor + length + 1;
    length = wcslen(cursor);
  }
}

void AppConfig::SetupPackageInformation() {
  size_t data_len;
  CString appPath= L".\\"; //GetCurrentDirectory();
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

    mPackageConfig.Set(pkg_dir);
    SetProjectCode(mPackageConfig.GetProjectCode());
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
#if 0
    FILETIME    FtLocal;
    SYSTEMTIME  SysTime;
    wchar_t        szLine [256];
    wchar_t        szDate [sizeof "jj/mm/aaaa"];
#endif
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
        ParseProjectConfig(configFile);
    }while (FindNextFile (hFind, & FindData));
    FindClose (hFind);

#if 1
    map<CString, ProjectConfig>::iterator it;
    for (it = m_SupportProject.begin(); it != m_SupportProject.end(); it++) {
        LOGE("key %S, project code %S", it->first.GetString(), it->second.GetProjectCode().GetString());
    }
#endif
}


void AppConfig::ParseProjectConfig(CString &configFile) {
    ProjectConfig projectConfig(configFile);
    list<CString> codes;
    list<CString>::iterator it;

    if (!projectConfig.ReadConfig(codes))
        return;

    for (it = codes.begin(); it != codes.end(); ++it) {
        projectConfig.SetProjectCode(*it);
        m_SupportProject.insert(std::pair<CString, ProjectConfig>(projectConfig.GetProjectCode(), projectConfig));
    }
}

BOOL AppConfig::SetProjectCode(CString &projectCode) {
    //CString code( projectCode.c_str() );
    map<CString,ProjectConfig>::iterator iter=m_SupportProject.find(projectCode);
    if(iter!=m_SupportProject.end()) {
        mProjectConfig = m_SupportProject.at(projectCode);
        return TRUE;
    }
    return FALSE;
}

BOOL AppConfig::GetDiagPSTNandPrg(string &prgName, BOOL emergency) {
    wchar_t     filename[MAX_PATH] = {0};
    mProjectConfig.GetDiagPSTNandPrg(filename, MAX_PATH, emergency);
    char * fn = WideStrToMultiStr(filename);
    if (fn == NULL)
        return FALSE;
    prgName = fn;
    DELETE_IF(fn);
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

BOOL ProjectConfig::ReadConfig(list<CString> &codes) {
     WCHAR     buffer[MAX_PATH] = {0};
     int data_len;
     PCTSTR configFile = mProjectConfigPath.GetString();
    if (configFile == NULL)
        return FALSE;

    if (!PathFileExists(configFile))
        return FALSE;

     data_len = GetPrivateProfileString(PROJECT_SECTION,
                           _T("code"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
   if (data_len == 0) {
        return FALSE;
    }

  #define CODE_DELIMINATE L" ,.-"
  //wchar_t wcs[] = L"- This, a sample string.";
  wchar_t * token;
  wchar_t * state;
  //wprintf (L"Splitting wide string \"%ls\" into tokens:\n",wcs);
  token = wcstok_s (buffer, CODE_DELIMINATE, &state);
  while (token != NULL)
  {
    //wprintf (L"%ls\n",token);
    codes.push_back(CString(token));
    token = wcstok_s (NULL, CODE_DELIMINATE, &state);
  }

     mCode = codes.front();//buffer;

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

     memset(buffer, 0, sizeof buffer);
     data_len = GetPrivateProfileString(PROJECT_SECTION,
                           _T("USBVid"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
    if (data_len > 0) {
       mVid = wcstol(buffer, NULL, 16);
    }

     memset(buffer, 0, sizeof buffer);
     data_len = GetPrivateProfileString(PROJECT_SECTION,
                           _T("USBPid"),
                           NULL,
                           buffer,
                           MAX_PATH,
                           configFile);
    if (data_len > 0) {
     //mPid = atoi(buffer);//strtol buffer;
     mPid = wcstol(buffer, NULL, 16);
    }

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


PackageConfig::PackageConfig()
{
   memset(pkg_conf_file, 0, sizeof pkg_conf_file);
   memset(pkg_qcn_file, 0, sizeof pkg_qcn_file);
   memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
}
PackageConfig::~PackageConfig()
{

}

BOOL  PackageConfig::Set(CString pkg_dir) {
  size_t data_len = pkg_dir.GetLength();
  memset(pkg_conf_file, 0, sizeof pkg_conf_file);
  wcsncpy_s(pkg_conf_file, pkg_dir.GetString(), data_len+1);
  wcsncat_s(pkg_conf_file, PKG_CONFIG_XML, COUNTOF(pkg_conf_file) - data_len);

  memset(pkg_qcn_file, 0, sizeof pkg_qcn_file);
  wcsncpy_s(pkg_qcn_file, pkg_dir.GetString(), data_len+1);
  wcsncat_s(pkg_qcn_file, PKG_STATIC_QCN, sizeof(pkg_qcn_file) / sizeof(pkg_qcn_file[0]) - data_len);

  wchar_t * candidate[] = {L"Download.img", L"..\\DownloadImage\\Download.img"};

  memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
  for (int i = 0; i < COUNTOF(candidate); i++) {
      wcsncpy_s(pkg_dlimg_file, pkg_dir.GetString(), data_len+1);
      wcsncat_s(pkg_dlimg_file, candidate[i], COUNTOF(pkg_dlimg_file) - data_len);
      if(GetFileAttributes(pkg_dlimg_file) != INVALID_FILE_ATTRIBUTES) {
          break;
      } else {
          memset(pkg_dlimg_file, 0, sizeof pkg_dlimg_file);
      }
  };
    m_ConfigXmlParser.Parse(GetPkgConfXmlPath());
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
    const wchar_t* pkg_dir = mAppConfig->GetPkgDir();
    read_fastboot_config(projectConfigFile, pkg_dir);
    read_diagpst_config(projectConfigFile, pkg_dir);
    read_openwrt_config(projectConfigFile, pkg_dir);
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


int flash_image::read_openwrt_config(const wchar_t* config, const wchar_t* pkg_dir) {
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

  data_len = GetPrivateProfileString(OPENWRT_SECTION,
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
    data_len = GetPrivateProfileString(OPENWRT_SECTION,
                                       partition,
                                       NULL,
                                       filename,
                                       MAX_PATH,
                                       config);

    partition = partition + partition_len + 1;
    partition_len = wcslen(partition);

    if (data_len <= 0) {
        continue;
    }
    path = pkg_dir;
    path += filename;
    if (PathFileExists(path.GetString())) {
        string basename;
        CStringToString(GetBaseName(path), basename);
        m_OpenWrtFiles.insert(std::pair<string,CString>(basename, path));
    } else {
        LOGE("%S is not exist", path.GetString());
        continue;
    }
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

        if (chQcn.OpenDocument(mAppConfig->GetPackageConfig()->GetPkgQcnPath()) == FALSE)
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
