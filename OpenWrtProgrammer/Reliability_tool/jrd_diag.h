#ifndef JRD_DIAG_H
#define JRD_DIAG_H
#define JRD_DIAG_CMD_F 254
#define NV_UE_IMEI_SIZE 9
#define  MAX_SSID_LEN      32
#define  MAX_PASSWORD_LEN   48

typedef unsigned short   uint16;
typedef unsigned char    uint8;
typedef unsigned int     uint32;
typedef int              int32;
typedef unsigned char    boolean;

#include <QObject>
#include "src/define/define.h"


/* -------------------------------------------------------------------------
** Shared statement
**
** rw_flag: 0 - write; 1 - read
**
** diag_errno: 0 - success; other - failure
**
** enable/disable: 0 - disable; 1 - enable
**
** lock: 0 - lock; 1 - unlock
** ------------------------------------------------------------------------- */
/* ============= Version session start =============*/


typedef enum{
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
  E_JRD_VERSION_DIAG_MAX,
}e_jrd_version_type;
/* ============= Version session end =============*/

/** Service status; indicates service availability.
*/
typedef enum
{
  E_JRD_SYS_SRV_STATUS_NONE = -1,  /* FOR INTERNAL USE ONLY! */
  E_JRD_SYS_SRV_STATUS_NO_SRV, /**< No service. */
  E_JRD_SYS_SRV_STATUS_LIMITED, /**< Limited service. */
  E_JRD_SYS_SRV_STATUS_SRV, /**< Service available. */
  E_JRD_SYS_SRV_STATUS_LIMITED_REGIONAL, /**< Limited regional service. */
  E_JRD_SYS_SRV_STATUS_PWR_SAVE,  /**< MS is in power save or deep sleep. */
  E_JRD_SYS_SRV_STATUS_MAX   /* FOR INTERNAL USE OF CM ONLY! */
} jrd_sys_srv_status_e_type;

typedef enum{
  E_JRD_SUCCESS = 0,
  E_JRD_FAILURE,
}e_jrd_error_status;

typedef enum{
  E_JRD_READ = 0,
  E_JRD_WRITE = 1,
}e_jrd_rw_flag;

typedef enum{
  E_JRD_DISABLE = 0,
  E_JRD_ENABLE,
}e_jrd_enable_status;

typedef enum{
  E_JRD_LOCK = 0,
  E_JRD_UNLOCK,
}e_jrd_lock_status;

typedef enum{
  E_JRD_BACKUP_CLEAR = 0,
  E_JRD_BACKUP_CREAT,
  E_JRD_BACKUP_UPDATE,
  E_JRD_BACKUP_RECOVERY,
}e_jrd_backup_handle_type;

/* ============= Diagnostic class code  start =============*/
typedef enum{
  E_INVALID_JRD_DIAG_CODE = -1,
  E_JRD_DIAG_SYS=0, /* SW version read, switch, serial port loack, OEM efs lock, back up aread clear/create/update, force recover, EFS erase, and so on */
  E_JRD_DIAG_NETWORK=1,
  E_JRD_DIAG_PERIPHERAL=2, /* Key, LED, Screen, SD card, SIM card, USB and so on */
  E_JRD_DIAG_WIFI=3,
  E_JRD_DIAG_MAX=4,
}e_jrd_diag_main_code_type;
/* ============= Diagnostic class code  end =============*/

/* ============= System diagnostic session start =============*/
typedef enum{
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
    /*BEGIN swith sim slot with diag cmd yuan.ma 2013-12-16 ADDED*/
    E_JRD_DIAG_SYS_CONTROL = 13,
    /*END swith sim slot with diag cmd yuan.ma 2013-12-16 ADDED*/
    /* start yong.xiong 20140123 used for oem partition operation interface */
    E_JRD_DIAG_SYS_OEM_PARTITION_OP=14,
    /* end yong.xiong 20140123 used for oem partition operation interface */

    E_JRD_DIAG_SYS_GET_HW_VARIANT=15,

    E_JRD_DIAG_SYS_MANUDATA_PARTITION_OP=16,

    E_JRD_DIAG_SYS_MEID_WRITE = 20,
    E_JRD_DIAG_SYS_SPC_WRITE = 21,
    E_JRD_DIAG_SYS_OTKSL_WRITE = 22,
    E_JRD_DIAG_SYS_CMD_MAX,
}e_jrd_diag_sys_id_type;

/* ============= System diagnostic session end =============*/

/* ============= Network diagnostic session start =============*/
typedef enum{
  E_INVALID_JRD_DIAG_NETWORK = -1,
  E_JRD_DIAG_NW_SET_MODE_BAND = 0,
  E_JRD_DIAG_NW_READ_MODE = 1,
  E_JRD_DIAG_NW_READ_BAND = 2,
  E_JRD_DIAG_NW_CONECTION_MODE = 3,
  E_JRD_DIAG_NW_SEL_MODE = 4,
  E_JRD_DIAG_NW_REG_STATUS = 5,
  E_JRD_DIAG_NW_RMNET_AUTOCONNECT = 6,
  E_JRD_DIAG_NW_CMD_MAX,
}e_jrd_diag_network_id_type;

/* ============= Network diagnostic session end =============*/

/* ============= Peripheral diagnostic session start =============*/
typedef enum{
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

/* ============= Peripheral diagnostic session end =============*/

/* ============= LCD Screen session start =============*/
typedef enum{
   EXIT_LCD_TEST = 0,
   ENTRY_LCD_TEST = 1,
   LCD_TURN_ON = 2,
   LCD_TURN_OFF= 3,
   }e_jrd_diag_LCD_id_type;
/* ============= LCD Screen session end =============*/

/* ============= LED session start =============*/
typedef enum{
   EXIT_LED_TEST = 0,
   ENTRY_LED_TEST = 1,
   RED_LED_TEST = 2,
   GREEN_LED_TEST = 3,
   BLUE_LED_TEST = 4,
 }e_jrd_diag_LED_id_type;
/* ============= LED session end =============*/

typedef enum{
   LED_LIGHT_off = 0,
   LED_LIGHT = 1,
 }e_jrd_diag_LED_light_type;

 /* ============= KEY session start =============*/
typedef enum{
   EXIT_KEY_TEST = 0,
   ENTRY_KEY_TEST = 1,
   QUERY_KEY_TEST = 2,
   }e_jrd_diag_KEY_id_type;
/* ============= KEY session end =============*/

typedef enum{
   LED1 = 1,
   LED2 = 2,
   LED3 = 3,
   LED4 = 4,
 }e_jrd_diag_LED_Number_type;

/* ============= WIFI diagnostic session start =============*/
typedef enum{
  E_INVALID_JRD_DIAG_WIFI = -1,
  E_JRD_DIAG_WIFI_SSID_RW = 0, /*ssid read and write*/
  E_JRD_DIAG_WIFI_MAC_RW = 1, /*mac address read and write*/
  E_JRD_DIAG_WIFI_SECURITY_RW = 2, /* Include security mode & password | password generate rule */
  E_JRD_DIAG_WIFI_ONOFF = 3, /* Include disable & enable & reset*/
  E_JRD_DIAG_WIFI_CONNECT_STATUS = 4,/* return connecting STA number*/
  E_JRD_DIAG_WIFI_SET_TESTMODE = 5,  /* set the wifi to test mode*/
  E_JRD_DIAG_WIFI_FTM_TEST_CMD = 6,  /* send the wmiconfig to ftm test*/
  E_JRD_DIAG_WIFI_FTM_UPDATA_FILES = 7,   /*updata the athtestcmd file to /usr/bin*/
  E_JRD_DIAG_WIFI_MODE_RW = 8,   /*wlan mode read and write*/
  E_JRD_DIAG_WIFI_GAIN_FILE_W = 9, /*write bdata.bin*/
  E_JRD_DIAG_WIFI_CMD_MAX,


}e_jrd_diag_wifi_id_type;

typedef enum{
    AP1_SSID_READ = 0,
    AP1_SSID_WRITE = 1,
    AP2_SSID_READ = 2,
    AP2_SSID_WRITE = 3,
}ssid_rw_enum;

typedef enum{
    AP1_SEC_INFO_READ = 0,
    AP1_SEC_INFO_WRITE = 1,
    AP2_SEC_INFO_READ = 2,
    AP2_SEC_INFO_WRITE = 3,
}sec_info_rw_enum;

typedef enum{
        MODE_READ = 0,
        MODE_WRITE = 1,
}WIFI_mode_rw_enum;

typedef enum{
    AP_TURN_ON = 1,
    AP_TURN_OFF = 2,
    AP_RESTART = 3,
}switch_ap_enum;

typedef enum
{
   SEC_DISABLED        = 0, // No security, open authentication
   SEC_STATIC_WEP      = 1, // Static WEP, WEP 40 bit or WEP 104 bit
   SEC_WPA_PSK         = 2, // WPA Pre-shared Key
   SEC_WPA2_PSK        = 3, // WPA 2 Pre-shared Key
   SEC_MIXED_WPA_PSK   = 4, // WPA/WPA2 mixed mode

}wlan_security_mode_enum_type;

typedef enum
{
   WPA_TKIP = 0,
   WPA_CCMP = 1,
   WPA_AUTO = 2, //TKIP and CCMP
}wlan_wpa_encryption_enum_type;

typedef enum
{
   OPEN_SYSTEM = 0, // Open system authentication
   SHARED_KEY = 1,      // Shared Key Authentication
}wlan_wep_enum_type;
/* ============= WIFI diagnostic session end =============*/

/* ============= Trace diagnostic session end =============*/
typedef enum{
  TRACE_INFO_READ = 0,
  TRACE_INFO_WRITE = 1,
}trace_info_rw_flag_enum;
/* ============= Trace diagnostic session end =============*/


/* ============= Trace Wifi Mode start =============*/
typedef enum{
  WIFI_2G = 0,
  WIFI_5G = 1,
  WIFI_2G_and_5G = 2
}Wifi_mode_flag_enum;
/* ============= Trace Wifi Mode end =============*/




#pragma pack(1);
/* -------------------------------------------------------------------------
** Command Packet Definitions
** ------------------------------------------------------------------------- */
typedef struct{
  uint16 cmd_entry; /* command entry */
  uint8 class_code; /* which class - use e_jrd_diag_main_code_type */
  uint8 cmd_code; /* which module process - use e_jrd_diag_xxx_id_type */
} jrd_diag_hdr_type;


typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  uint8 mode;
  int32 diag_errno;
} jrd_diag_wifi_mode_wr_req_rsp_type;

/*typedef struct{
  uint8 cmd_code;
  PACK(void * PACKED_POST) (*func_ptr) (PACK(void *PACKED_POST) req_pkt_ptr, uint16 pkt_len);
} jrd_diag_user_table_entry_type; */

/*typedef struct{
  uint8 class_code; // Module code defined in e_jrd_diag_main_code_type //
  jrd_diag_user_table_entry_type *user_table; // Entry to Client's Packet Dispatch Table //
} jrd_diag_master_table_type;*/

/* -------------------------------------------------------------------------
** System Packet Definitions
** ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  int32 trace_info_len;
  char trace_info[512]; /* tarce version infomation */
} jrd_diag_trace_info_req_type, jrd_diag_curef_info_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  int32 diag_errno;
  int32 trace_info_len;
  char trace_info[512]; /* tarce version infomation */
} jrd_diag_trace_info_rsp_type, jrd_diag_curef_info_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  uint8 index;
  int32 trace_info_len;
  char trace_info[512]; /* tarce version infomation */
} jrd_diag_trace2_info_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  uint8 index;
  int32 diag_errno;
  int32 trace_info_len;
  char trace_info[512]; /* tarce version infomation */
} jrd_diag_trace2_info_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  /* 0:E_JRD_Q6_FIRMWARE_VERSION
     1:E_JRD_QCN_VERSION
     2:E_JRD_DASHBOARD_VERSION
     3:E_JRD_Q6_RESOURCE_VERSION
     4:E_JRD_LINUXKER_VERSION
     5:E_JRD_LINUXSYS_VERSION
     6:E_JRD_LINUXUSER_VERSION
     7:E_JRD_JRD_RESOURCE_VERSION */
  uint8 which_version; /* e_jrd_version_type */
} jrd_diag_get_version_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 which_version; /* e_jrd_version_type */
  int32 version_len;/* 0 - version not exist */
  char version[32];
} jrd_diag_get_version_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 value;/* 0 - usb func5 0; 1 - usb func5 1 */
} jrd_diag_sys_usb_switch_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 value;/* 0 - usb func5 0; 1 - usb func5 1 */
  int32 diag_errno;
} jrd_diag_sys_usb_switch_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 lock;
} jrd_diag_sys_port_lock_req_type,jrd_diag_sys_efs_lock_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 lock;
  int32 diag_errno;
} jrd_diag_sys_port_lock_rsp_type,jrd_diag_sys_efs_lock_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 type;/* 0 - clear; 1 - create; 2 - update; 3 - recovery */
} jrd_diag_sys_backup_handle_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_sys_backup_handle_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 enable;
} jrd_diag_sys_efs_dump_enable_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 enable;
  int32 diag_errno;
} jrd_diag_sys_efs_dump_enable_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  uint8 imei[NV_UE_IMEI_SIZE];
  int32 diag_errno;
} jrd_diag_sys_imei_write_req_rsp_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
  //char status_data[PSENSOR_STATUS_LEN];
}jrd_diag_get_psensor_status_rsp_type;

typedef enum{
  P_Sonser_Close = 3,
  P_Sonser_Open = 2,
  P_Sonser_Querry = 7,
  Disable_power_saving   =10
}P_Sonser;

/* -------------------------------------------------------------------------
** Network Packet Definitions
** ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint32 band_conf; /* e_lte_band */
  uint32 mode_conf; /* e_gwl_mode */
  uint32 acq_order_conf;
  uint32 srv_domain_conf;
} jrd_diag_nw_set_mode_band_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_nw_set_mode_band_rsp_type;

typedef struct{
  jrd_diag_hdr_type hdr;
} jrd_diag_nw_read_mode_req_type, jrd_diag_nw_read_band_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 value;
  int32 diag_errno;
} jrd_diag_nw_read_mode_rsp_type, jrd_diag_nw_read_band_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  int32 connection_mode; /* 0 - Manual
                            1 - Auto , roaming not allowed
                            2 - auto+roaming+national+international
                            3 - auto+roaming+national
                            4 - auto+roaming+international */
} jrd_diag_nw_connection_mode_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 connection_mode;
  int32 diag_errno;
} jrd_diag_nw_connection_mode_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  uint8 sel_mode; /* 0:auto register; 1:mannual register; */
} jrd_diag_nw_sel_mode_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 sel_mode; /* 0:auto register; 1:mannual register; */
  int32 diag_errno;
} jrd_diag_nw_sel_mode_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
} jrd_diag_nw_register_status_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 register_status; /* 0:E_JRD_SYS_SRV_STATUS_NO_SRV // No service
                            1:E_JRD_SYS_SRV_STATUS_LIMITED // Limited service
                            2:E_JRD_SYS_SRV_STATUS_SRV // Service available
                            3:E_JRD_SYS_SRV_STATUS_LIMITED_REGIONAL // Limited regional service
                            4:E_JRD_SYS_SRV_STATUS_PWR_SAVE // MS is in power save or deep sleep */
  int32 diag_errno;
} jrd_diag_nw_register_status_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag; /* read or write flag */
  uint8 index;
  uint8 enable;
  int32 diag_errno;
} jrd_diag_nw_rmnet_autoconnect_req_type, jrd_diag_nw_rmnet_autoconnect_rsp_type;
/* -------------------------------------------------------------------------
** Peripheral Packet Definitions
** ------------------------------------------------------------------------- */
typedef struct{
    jrd_diag_hdr_type hdr;
} jrd_diag_sim_status_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
  int32 status_len;
  char sim_status[32]; //
} jrd_diag_sim_status_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 lock;
  uint8 key_len;
  char key_content[16];
} jrd_diag_sim_lock_req_type;


typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 event_type; /* 0:exit LED test mode;
                       1:entry LED test mode;
                       2:test red LED state
                       3:test green LED state
                       4:test blue LED state
                       .......*/
  uint8 LedLight_Order;
  //uint16 LedLight2;
  uint8 Light_Switch;
} jrd_diag_led_req_Y858_type;


typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_sim_lock_rsp_type;
/* ------------------------------------------------------------------------- */

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 event_type; /* 0:exit LED test mode;
                       1:entry LED test mode;
                       2:test red LED state
                       3:test green LED state
                       4:test blue LED state
                       .......*/
  uint16 LedLight0;
  uint32 LedLight1;
  //uint8 LedLight2;
} jrd_diag_led_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
 #ifdef Y901
   int32 event_type;
#else

  uint8 event_type; /* 0:exit LED test mode;
                       1:entry LED test mode;
                       2:test red LED state
                       3:test green LED state
                       4:test blue LED state
                       .......*/

#endif
} jrd_diag_Screen_req_type;


typedef struct
{
  jrd_diag_hdr_type hdr;
  uint8 LedLight_Order;
  uint32 leds_state;
}jrd_leds_msg_general_req_type;

typedef struct
{
  jrd_diag_hdr_type hdr;
  int diag_errno;//diag_cmd_ret
}jrd_leds_msg_general_rsp_type;


typedef struct{
        jrd_diag_hdr_type hdr;
        uint8 AT_flag;   //0-AT0  1-AT1
        uint8 wifi_channel;
        int gain_value;
        int32 diag_errno;
}jrd_diag_wifi_dB_gain_adjust;


typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_led_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  char pid_vid[32];
} jrd_diag_pid_vid_req_type;
typedef struct{
  jrd_diag_hdr_type hdr;
  char pid_vid[32];
  int32 diag_errno;
} jrd_diag_pid_vid_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 sdcard_status;  // 0 - ÓÐSD¿¨; 1 - Ã»ÓÐSD¿¨
  int32 diag_errno;
} jrd_diag_sdcard_status_req_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;


  int32 event_type; /* 0:exit key test mode;
                       1:entry key test mode;
                       2:query test result */
} jrd_diag_key_req_type;

#ifndef Y860

/*typedef struct{

  jrd_diag_hdr_type hdr;
#ifndef Y854
  uint8 key_index;
#endif
  int32 diag_errno;
} jrd_diag_key_rsp_type;*/

#if defined Y853||defined Y854||defined Y856
typedef struct{

  jrd_diag_hdr_type hdr;

  int32 diag_errno;
} jrd_diag_key_rsp_type;
#else
typedef struct{

  jrd_diag_hdr_type hdr;
  uint8 key_index;
  int32 diag_errno;
} jrd_diag_key_rsp_type;
#endif


#else

typedef struct{
  jrd_diag_hdr_type hdr;

  //uint8 key_index;

  int32 diag_errno;
} jrd_diag_key_rsp_type;
#endif

#if  defined Y853|| defined Y854
typedef enum{
  No_key     = 3,
  Power_key  = 1,
  Reset_key  = 2,

  All_key    = 0
}Key_Test_flag_enum;
#else

typedef enum{
  No_key = 0,
  Power_key = 1,
  Reset_key = 2,
  All_key = 3
}Key_Test_flag_enum;
#endif

/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 charger_status;
  int32 diag_errno;
} jrd_diag_charger_status_req_rsp_type;
/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 event_type;/* 0:exit screen test mode;
                      1:entry screen test mode;
                      2:turn on screen
                      3:turn off screen */
} jrd_diag_screen_req_type;


typedef enum
{
         JRD_SCREEN_INVALID_MSG_ID    = -1,
         JRD_SCREEN_EXIT_MSG_ID       = 0,        // exit testing
         JRD_SCREEN_ENTRY_MSG_ID      = 1,        //enter testing
         JRD_SCREEN_TURN_ON_MSG_ID    = 2,       //light
         JRD_SCREEN_TURN_OFF_MSG_ID   = 3,        //turn off
         /*BEGIN JRD_DIAG_SCREEN_LCD_TEST wei.huang 2014-8-22 ADDED*/
         JRD_SCREEN_TURN_RED_MSG_ID   = 4,        //red
         JRD_SCREEN_TURN_GREEN_MSG_ID = 5,         //green
         JRD_SCREEN_TURN_BLUE_MSG_ID  = 6,         //blue
         /*END JRD_DIAG_SCREEN_LCD_TEST wei.huang 2014-8-22 ADDED*/
         JRD_SCREEN_MAX_MSG_ID,
}screen_msg_id;


typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_screen_rsp_type;

/* -------------------------------------------------------------------------
** WIFI Packet Definitions
** ------------------------------------------------------------------------- */
//ssid write and read
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  char ssid[MAX_SSID_LEN+1];
  int32 diag_errno;
} jrd_diag_wifi_ssid_wr_req_rsp_type;

//mac address write and read
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;

  unsigned char mac_addr[6];
  int32 diag_errno;
} jrd_diag_wifi_mac_addr_wr_req_rsp_type;

//security read and write
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 rw_flag;
  uint8 security_mode;  //no, wep, wpa...
  uint8 sub_mode;       //tkip,aes...
  char password[MAX_PASSWORD_LEN+1];
  int32 diag_errno;
} jrd_diag_wifi_sec_wr_req_rsp_type;

//turn on,off,reset
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 op; //turn on, off and reset wifi
  int32 diag_errno;
} jrd_diag_wifi_switch_req_rsp_type;

//get the connecting sta number
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_wifi_connect_sta_req_type;

typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 con_sta_num;   //connecting sta number
  int32 diag_errno;
} jrd_diag_wifi_connect_sta_rsp_type;

//set wifi to test mode
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_wifi_set_testmode_req_rsp_type;
typedef struct{
        jrd_diag_hdr_type hdr;
        uint8 W_file_flag;   //write - 1
        int32 diag_errno;
}jrd_diag_wifi_gain_file_w_type;


//for wifi ftm test
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 test_step;  //the step correspond the athtestcmd command
  int32 diag_errno;
} jrd_diag_wifi_ftm_test_req_rsp_type;

//rx report the number of packets
typedef struct{
  jrd_diag_hdr_type hdr;
  uint8 test_step;  //the step correspond the athtestcmd command
  uint32 report_11M;
  uint32 report_54M;
  uint32 report_65M;
  int32 diag_errno;
} jrd_diag_wifi_ftm_test_report_rsp_type;


#pragma pack();


#define TRACE_INFO_LEN 512

typedef struct
{
    char All[513] ;
    char PCBAInfo[27+1] ;
    char HandsetInfo[23+1] ;
    char MiniInfo[43+1];
    char GoldenSample[4+1] ;
    char HDTB[6+1] ;
    char PT1Test[8+1] ;
    char PT2Test[8+1] ;
    char PT3Test[8+1] ;
    char PT4Test[132+1] ;
    char BluetoothTest[8+1] ;
    char WiFiTest[8+1] ;
    char GPSTest[9+1] ; //updata to simlock 4(write simlock ok1+ymd)+4(Check simlock ok1+ymd)+check imei flag
    char FinalTest[8+1];
    char HDT[6+1] ;
    char CUSWinfo[123+1];
    char SSID[47+1];
    char PasswordAT[15+1];
    char SpaceRegion[45+1];
}TraceInfo;

enum
{
    Wifi_Connecting_2G,
    Wifi_Connecting,
    Wifi_Connecting_5G,
    Wifi_Connecting_5G_and_2G,
    Side_Key_Checking,
    Sd_Card,
    SIM_Card,
    Network_Register_Checking,
    OLED_Screen,
    Single_LED_Test,
    Charger_Test,
    Three_LED_Test,        //for L850 red,green,blue LED test
    LED_Test,             //for Y860
    Screen_Test,          //for 901 screen
    Firmware_Version,
    SSID_Read,
    SSID_Write,
    MAC_Read,
    MAC_Write,
    IMEI_Write,
    IMEI_Read,
    Activate_Sim_Lock,
    Erase_Sim_Lock,
    WIFI_PassWord_Write,
    WIFI_PassWord_Read,
    Wifi_Calibration

};

#endif // JRD_DIAG_H
