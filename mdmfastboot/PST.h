#pragma once

#include "define.h"
#include <string>
#include <map>
#include "device.h"

using namespace std;

static const int PARTITION_NUM_MAX = 32;
static const int PARTITION_NAME_LEN = 32;
static const int PARTITION_TBL_LEN = PARTITION_NUM_MAX * PARTITION_NAME_LEN;

#define PKG_CONFIG_XML          L"config.xml"
#define PKG_STATIC_QCN          L"static.qcn"

#define PARTITIONTBL_SECTION    L"partition_table"
#define PARTITIONTBL_DL		      L"partition_dl"
#define DIAGPST_SECTION         L"pst_diag"
#define PKG_SECTION             L"package"
#define PKG_PATH                L"path"
#define PST_NPRG                 L"pst_nprg"
#define PST_ENPRG               L"pst_enprg"


typedef struct FlashImageInfo {
    struct FlashImageInfo *next;
    wchar_t *partition;
    char *partition_str;
    wchar_t *lpath;
	void *data;
	unsigned size;
	bool need_download;
}FlashImageInfo;

class flash_image{
  public:
    flash_image(const wchar_t* config_file);
    ~flash_image();
    const FlashImageInfo* get_partition_info(wchar_t *partition, void **ppdata, unsigned *psize);
    const FlashImageInfo* image_enum_init (void) ;
    const FlashImageInfo* image_enum_next (const FlashImageInfo* img);
    const wchar_t * get_package_dir(void);
    //config.xml file path
    const wchar_t * get_package_config(void);
    //static qcn file path
    const wchar_t * get_package_qcn_path(void);

    BOOL qcn_cmds_enum_init (char *cmd);
    const char* qcn_cmds_enum_next (unsigned int index);

    BOOL set_package_dir(const wchar_t * dir, const wchar_t* config, BOOL reset=FALSE);
    BOOL get_pkg_a5sw_sys_ver(CString &version);
    BOOL get_pkg_a5sw_usr_ver(CString &version);
    BOOL get_pkg_a5sw_kern_ver(CString &version);
    BOOL get_pkg_fw_ver(CString &version);
    BOOL get_pkg_qcn_ver(CString &version);
	  BOOL set_download_flag(CString strPartitionName, bool bDownload);
    int read_fastboot_config(const wchar_t* config);
    int read_diagpst_config(const wchar_t* config);

     bool AddFileBuffer(const wchar_t *partition, const wchar_t *pkgPath, const wchar_t *filName);
     map<string,FileBufStruct> GetFileBuffer() { return m_dlFileBuffer;};

  protected:
    virtual int parse_pkg_sw(CString & node, CString & text);
    virtual int parse_pkg_hw(CString & node, CString & text);

  private:
    int add_image(wchar_t *partition, const wchar_t *lpath, BOOL write =FALSE, const wchar_t* config = NULL);
    void read_package_version(const wchar_t * package_conf);
    BOOL reset(BOOL free_only);

  private:
    FlashImageInfo *image_list;
    FlashImageInfo *image_last;
    wchar_t pkg_dir[MAX_PATH];
    wchar_t pkg_conf_file[MAX_PATH];
    wchar_t pkg_qcn_file[MAX_PATH];
    unsigned int nv_num;
    char ** nv_buffer;
    char * nv_cmd;
    map<string,FileBufStruct>  m_dlFileBuffer;
    CString a5sw_kern_ver;
    CString a5sw_usr_ver;
    CString a5sw_sys_ver;
    CString qcn_ver;
    CString fw_ver;
};

typedef enum
{
	FIRMWARE_VER,
	QCN_VER,
	LINUX_VER,
	SYSTEM_VER,
	USERDATA_VER,
	PTS_VER,
	TITLE,
	PROGRESS_VAL,
	PROMPT_TITLE,
	PROMPT_TEXT,
	ADB_CHK_ABORT,
	REBOOT_DEVICE,
	FLASH_DONE,
	UI_DEFAULT,
}UI_INFO_TYPE;

typedef struct _UIInfo_
{
	UI_INFO_TYPE	infoType;
	int				    iVal;
	CString			  sVal;

  _UIInfo_() {
    infoType = UI_DEFAULT;
    sVal = "";
    iVal = -1;
  }
}UIInfo;


class CmdmfastbootDlg;
class CPortStateUI;

class UsbWorkData{
  public:
    UsbWorkData(int index, CmdmfastbootDlg *dlg, DeviceCoordinator *coordinator);
    ~UsbWorkData();
    BOOL Clean(BOOL noCleanUI=TRUE);
    BOOL IsIdle();
    BOOL Reset(VOID);
    BOOL Abort(VOID);
    BOOL Start(DeviceInterfaces* devIntf, UINT nElapse, BOOL flashdirect);
    BOOL Finish(VOID);
    BOOL SwitchDev(UINT nElapse);
    BOOL SetSwitchedStatus();
    UINT ui_text_msg(UI_INFO_TYPE info_type, PCCH msg);
    UINT SetProgress(int progress);
    UINT SetPromptMsg(PCCH msg) { return ui_text_msg(PROMPT_TEXT, msg);};
    const char *GetDevTag() { return devIntf->GetDevTag();};
    BOOL Log(const char * msg);

  private:
    DeviceCoordinator *pCoordinator;

  public:
    CmdmfastbootDlg  *hWnd;
    CPortStateUI      *pCtl;
    CWinThread       *work;
    usb_handle       *usb;
    //this is the serial number for logical ui.
    DeviceInterfaces*  devIntf;
    int              stat;
    FlashImageInfo const * flash_partition[PARTITION_NUM_MAX];
    short           partition_nr;
    BOOL            update_qcn;
} ;

