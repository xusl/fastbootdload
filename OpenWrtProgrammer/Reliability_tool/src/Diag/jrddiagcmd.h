#ifndef JRDDIAGCMD_H
#define JRDDIAGCMD_H

#include "src/define/define.h"
#include "src/packet/pkt.h"
#include "src/log/log.h"

#define JRD_DIAG_CMD_F      254
#define VERSION_LEN   /*32*/100

typedef enum
{
  E_INVALID_JRD_VERSION_CODE = -1,
  E_JRD_Q6_FIRMWARE_VERSION  = 0,
  E_JRD_QCN_VERSION          = 1,
  E_JRD_DASHBOARD_VERSION    = 2,
  E_JRD_Q6_RESOURCE_VERSION  = 3,
  E_JRD_LINUXKER_VERSION     = 4,
  E_JRD_LINUXSYS_VERSION     = 5,
  E_JRD_LINUXUSER_VERSION    = 6,
  E_JRD_JRD_RESOURCE_VERSION = 7,
  E_JRD_SBL_VERSION = 8,
  E_JRD_SDI_VERSION = 9,
  E_JRD_TZ_VERSION = 10,
  E_JRD_MBA_VERSION = 11,
  E_JRD_RPM_VERSION = 12,
  E_JRD_RECOVERY_VERSION = 13,
  E_JRD_RECOVERYFS_VERSION = 14,
  E_JRD_WEBUI_VERSION = 15,
  E_JRD_PARTITION_VERSION = 16,
  E_JRD_EXTERNAL_VERSION = 17,
  E_JRD_APPSBL_VERSION = 18,
  E_JRD_ADSP_VERSION = 19,

  E_JRD_VERSION_DIAG_MAX,
}e_jrd_version_type;

typedef enum
{
  E_JRD_SUCCESS = 0,
  E_JRD_FAILURE,
}e_jrd_error_status;

typedef enum
{
  E_JRD_READ = 0,
  E_JRD_WRITE,
}e_jrd_rw_flag;

typedef enum
{
  E_JRD_DISABLE = 0,
  E_JRD_ENABLE,
}e_jrd_enable_status;

typedef enum
{
  E_JRD_DEACTIVE = 0,
  E_JRD_ACTIVE,
  E_JRD_ERASE,
}e_jrd_simlock_status;


typedef enum
{
  E_JRD_CONFIG_XML = 0,
  E_JRD_CUSTOM_INFO_XML,
}e_jrd_XML_version;

typedef enum
{
  E_JRD_LOCK = 0,
  E_JRD_UNLOCK,
}e_jrd_lock_status;

typedef enum
{
  E_JRD_BACKUP_CLEAR = 0,
  E_JRD_BACKUP_CREAT,
  E_JRD_BACKUP_UPDATE,
  E_JRD_BACKUP_RECOVERY,
}e_jrd_backup_handle_type;

/* ============= Diagnostic class code  start =============*/
typedef enum
{
  E_INVALID_JRD_DIAG_CODE = -1,
  E_JRD_DIAG_SYS, /* SW version read, switch, serial port loack, OEM efs lock, back up aread clear/create/update, force recover, EFS erase, and so on */
  E_JRD_DIAG_NETWORK,
  E_JRD_DIAG_PERIPHERAL, /* Key, LED, Screen, SD card, SIM card, USB and so on */
  E_JRD_DIAG_WIFI,
  E_JRD_DIAG_MAX,
}e_jrd_diag_main_code_type;
/* ============= Diagnostic class code  end =============*/

/* ============= System diagnostic session start =============*/
typedef enum
{
  E_INVALID_JRD_DIAG_SYS = -1,
  E_JRD_DIAG_SYS_TRACE_INFO = 0,
  E_JRD_DIAG_SYS_CUREF_INFO = 1,
  E_JRD_DIAG_SYS_GET_VER = 2,
  E_JRD_DIAG_SYS_USB_SWITCH = 3,
  E_JRD_DIAG_SYS_PORT_LOCK = 4,
  E_JRD_DIAG_SYS_EFS_LOCK = 5,
  E_JRD_DIAG_SYS_BACKUP_HANDLE = 6,
  E_JRD_DIAG_SYS_EFS_DUMP_ENABLE = 7,
  E_JRD_DIAG_SYS_IMEI_WRITE = 8,
  E_JRD_DIAG_SYS_TRACE2_INFO = 9,
  E_JRD_DIAG_SYS_GET_FLASH_TYPE_INFO = 10,
  E_JRD_DIAG_SYS_STORE_BOOT_IMAGE = 11,
  E_JRD_DIAG_READ_WRITE_CONFIG_XML = 12,
  E_JRD_DIAG_SYS_CMD_MAX,
}e_jrd_diag_sys_id_type;

typedef enum
{
        E_JRD_LEDS_INVALID_MSG_ID    = -1,
        E_JRD_LEDS_EXIT_MSG_ID       = 0,
        E_JRD_LEDS_ENTRY_MSG_ID      = 1,
        E_JRD_LEDS_RED_OP_MSG_ID    = 2,
        E_JRD_LEDS_BLUE_OP_MSG_ID   = 3,
        E_JRD_LEDS_GREEN_OP_MSG_ID   = 4,
        E_JRD_LEDS_WHITE_OP_MSG_ID    = 5,
        E_JRD_P_SENSOR_STATUS_OP_MSG_ID = 6,
        E_JRD_P_SENSOR_MSG_UP_STATUS_OP_MSG_ID = 7,
        E_JRD_HALL_SENSOR_GET_STATUS_OP_MSG_ID = 8,
        E_JRD_HALL_SENSOR_FUNTCTION_ENABEL_OP_MSG_ID =9,
        E_JRD_HALL_SENSOR_FUNTCTION_DISNABEL_OP_MSG_ID=10,
        E_JRD_HALL_SENSOR_FUNTCTION_STATUS_OP_MSG_ID =11,
        E_JRD_POWER_SAVE_MSG_ID =12,
        E_JRD_POWER_SAVE_GET_STATUS_MSG_ID =13,
        E_JRD_GET_HW_PCB_VERSION_MSG_ID =14,
        E_JRD_LEDS_MAX_MSG_ID,
}leds_msg_id;


/* ============= Peripheral diagnostic session start =============*/
typedef enum
{
  E_INVALID_JRD_DIAG_PERIPHERAL = -1,
  E_JRD_DIAG_PERI_SIM_STATUS = 0,
  E_JRD_DIAG_PERI_SIM_LOCK = 1,
  E_JRD_DIAG_PERI_SIM_SWITCH = 2,
  E_JRD_DIAG_PERI_LED = 3,
  E_JRD_DIAG_PERI_USB_PID_VID = 4,
  E_JRD_DIAG_PERI_SD_CARD_STATUS = 5,
  E_JRD_DIAG_PERI_KEY = 6,
  E_JRD_DIAG_PERI_CHARGER_STATUS = 7,
  E_JRD_DIAG_PERI_SCREEN = 8,
  E_JRD_DIAG_PERI_CMD_MAX,
}e_jrd_diag_peripheral_id_type;

/* ============= WIFI diagnostic session start =============*/
typedef enum
{
  E_INVALID_JRD_DIAG_WIFI = -1,
  E_JRD_DIAG_WIFI_SSID_RW = 0,          /*ssid read and write*/
  E_JRD_DIAG_WIFI_MAC_RW = 1,           /*mac address read and write*/
  E_JRD_DIAG_WIFI_SECURITY_RW = 2,      /* Include security mode & password | password generate rule */
  E_JRD_DIAG_WIFI_ONOFF = 3,            /* Include disable & enable & reset*/
  E_JRD_DIAG_WIFI_CONNECT_STATUS = 4,   /* return connecting STA number*/
  E_JRD_DIAG_WIFI_SET_TESTMODE = 5,     /* set the wifi to test mode*/
  E_JRD_DIAG_WIFI_FTM_TEST_CMD = 6,     /* send the wmiconfig to ftm test*/
  E_JRD_DIAG_WIFI_CMD_MAX,
}e_jrd_diag_wifi_id_type;


/* -------------------------------------------------------------------------
** Command Packet Definitions
** ------------------------------------------------------------------------- */
#pragma pack(1)
typedef struct
{
  uint16 cmd_entry;         /* command entry */
  uint8 class_code;         /* which class - use e_jrd_diag_main_code_type */
  uint8 cmd_code;           /* which module process - use e_jrd_diag_xxx_id_type */
} jrd_diag_hdr_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  /* 0:E_JRD_Q6_FIRMWARE_VERSION
     1:E_JRD_QCN_VERSION
     2:E_JRD_DASHBOARD_VERSION
     3:E_JRD_Q6_RESOURCE_VERSION
     4:E_JRD_LINUXKER_VERSION
     5:E_JRD_LINUXSYS_VERSION
     6:E_JRD_LINUXUSER_VERSION
     7:E_JRD_JRD_RESOURCE_VERSION */
  uint8 which_version;          /* e_jrd_version_type */
} jrd_diag_get_version_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 which_version;          /* e_jrd_version_type */
  int32 version_len;            /* 0 - version not exist */
  char version[VERSION_LEN];
} jrd_diag_get_version_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 value;                  /* 0 - usb func5 0; 1 - usb func5 1 */
} jrd_diag_sys_usb_switch_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 value;                  /* 0 - usb func5 0; 1 - usb func5 1 */
  int32 diag_errno;
} jrd_diag_sys_usb_switch_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct
{
  jrd_diag_hdr_type hdr;
  //char Flash_Type;
} jrd_diag_sys_get_flash_type_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  int32 event_type;
  char traceinfo[512];
} jrd_diag_sys_get_hardware_ver_req_type;



typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8  flash_type;
  int32 diag_errno;
}  jrd_diag_sys_get_flash_type_rsp_type;
typedef struct
{
  jrd_diag_hdr_type hdr;
  char flash_type[20];
  int32 diag_errno;
}  jrd_diag_sys_get_flash_type_9X25_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct
{
  jrd_diag_hdr_type hdr;
  int32 pic_len;
  byte pic[2048];
} jrd_diag_sys_store_boot_image_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
}jrd_diag_sys_store_boot_image_rsp_type;
/*--------------------------------------------------------------------------*/


typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 lock;
} jrd_diag_sys_port_lock_req_type,jrd_diag_sys_efs_lock_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 lock;
  //int32 lock;
  int32 diag_errno;
} jrd_diag_sys_port_lock_rsp_type,jrd_diag_sys_efs_lock_rsp_type;
/* ------------------------------------------------------------------------- */

typedef struct
{
  jrd_diag_hdr_type hdr;
  int32 type;/* 0 - clear; 1 - creat; 2 - update; 3 - recovery */
} jrd_diag_sys_backup_handle_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_sys_backup_handle_rsp_type;
/* ------------------------------------------------------------------------- */

#define XML_FILE_LEN 1024

typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  uint8 which_xml;//0-config.xml , 1-custom_info.xml
  int32 offset;
  int32 len;
  char content[XML_FILE_LEN];
} jrd_diag_sys_read_write_config_xml_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  uint8 which_xml;
  int32 cur_len;
  int32 remain_len;
  char content[XML_FILE_LEN];
  int32 diag_errno;
}jrd_diag_sys_read_write_config_xml_rsp_type;


#define SIM_LOCK_CONTENT_LEN 32
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 op_type;                //0-deactive 1-active 2-erase
  uint8 cont_len;
  uint8 content[SIM_LOCK_CONTENT_LEN];
} jrd_diag_sim_lock_req_type;


typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_sim_lock_rsp_type;


/* -------------------------------------------------------------------------*/

#pragma pack()

class JRDdiagCmd
{
public:
    JRDdiagCmd(CPacket& packetDll);
    ~JRDdiagCmd();

    TResult EnableDiagServer();
    TResult DisableDiagServer();

#ifdef FEATHER_CUSTOM_IDEA
    TResult ReadIMEI(QString & strImei);
    TResult NewEnableDiagServer(QString strCode);
    TResult NewDisableDiagServer();
#endif

    TResult RequestVersion(uint8 whichVer,char* fdata);
    TResult RequestFlashType_N(uint8& type);
    TResult RequestHardwareVersion(char * type);
    TResult RequestFlashType_9X25(char *Flash_Type);

    QString Get_Hardversion(char *HardWare);
    TResult StorePIC(uint8* data, uint32 len);

    TResult ReadWifiPwdFlag(int &retFlagVal);
    TResult SetFuncFive(byte valueFuncFive);
    TResult GenerateFTFilesNew(int32 value);
    TResult GetConfigXml(int32 offset,uint8 whichXml,char* content,int32* cur_len, int32* remain_len);
    TResult WriteConfigXml(int32 offset,uint8 whichXml,char* content,int32 len);
    TResult EraseSimLock();

    TResult ReadMacAddress(QString & strMacAddress, int iType);


    TResult SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen);

private:
    CPacket* m_packetDll;
    cmd_buffer_s_type cmd;
    rsp_buffer_s_type rsp;
};

#endif // JRDDIAGCMD_H
