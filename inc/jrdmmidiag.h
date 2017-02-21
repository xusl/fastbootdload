#ifndef JRD_DIAG_H
#define JRD_DIAG_H
#include "jrddiagcmd.h"
#define NV_UE_IMEI_SIZE 9
#define  MAX_SSID_LEN      32
#define  MAX_PASSWORD_LEN   48

typedef unsigned short   uint16;
typedef unsigned char    uint8;
typedef unsigned int     uint32;
typedef int              int32;
typedef unsigned char    boolean;



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
#if 0
typedef enum{
  E_INVALID_JRD_VERSION_CODE = -1,
  E_JRD_Q6_FIRMWARE_VERSION,
  E_JRD_QCN_VERSION,
  E_JRD_DASHBOARD_VERSION,
  E_JRD_Q6_RESOURCE_VERSION,
  E_JRD_LINUXKER_VERSION,
  E_JRD_LINUXSYS_VERSION,
  E_JRD_LINUXUSER_VERSION,
  E_JRD_JRD_RESOURCE_VERSION,
  E_JRD_VERSION_DIAG_MAX,
}e_jrd_version_type;
#endif
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

 /* ============= KEY session start =============*/
typedef enum{
   EXIT_KEY_TEST = 0,
   ENTRY_KEY_TEST = 1,
   QUERY_KEY_TEST = 2,
   }e_jrd_diag_KEY_id_type;
/* ============= KEY session end =============*/


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

#pragma pack(1)

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
typedef struct {
    jrd_diag_hdr_type hdr;
    uint8           ring_toggle;
} jrd_diag_rj11_req_type;

typedef struct {
    jrd_diag_hdr_type hdr;
    int32 diag_errno;
    char status[36];
} jrd_diag_rj11_rsp_type;

/* ------------------------------------------------------------------------- */
typedef struct{
  jrd_diag_hdr_type hdr;
  int32 event_type; /* 0:exit LED test mode;
                       1:entry LED test mode;
                       2:test red LED state
                       3:test green LED state
                       4:test blue LED state
                       .......*/
} jrd_diag_led_req_type;

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

typedef struct{
  jrd_diag_hdr_type hdr;
  int32 diag_errno;
} jrd_diag_key_rsp_type;
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


#pragma pack()

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
    SSID_Read_2G,
    SSID_Read_5G,
    Led_test,
    Lan_check,
    USB_check,
    Software_Version_check,
    //Trace_Information_check,
    MAC_Add_check,
    Side_Key_check,
    Call_Out_Check,
    Call_In_Check,

};
int ConnectMS(unsigned short comport);
int DisconnectMs();
VOID DIAG_ReadSSID(char *ssid);
VOID DIAG_WriteSSID(char *ssid);
VOID DIAG_RestartWiFi(void);
VOID DIAG_DisableSecurity(void);
VOID DIAG_CheckStaConnect(void);
VOID DIAG_Charger_TEST(void);
VOID DIAG_EnterLCDScreenTEST(void);
VOID DIAG_LCDScreen_Turn_On(void);
VOID DIAG_LCDScreen_Turn_Off(void);
VOID DIAG_EXITLCDScreenTEST(void);
VOID  DIAG_EnterLEDTEST(void);
VOID  DIAG_EXITLEDTEST(void);
VOID  DIAG_REDLEDTEST(void);    //LED
VOID  DIAG_GREENLEDTEST(void);  //OLED
VOID  DIAG_BLUELEDTEST(void);
VOID  DIAG_ENTRYKEYTEST(void);
VOID  DIAG_KEYTEST(void);
VOID  DIAG_EXITKEYTEST(void);
VOID  DIAG_ReadFIRMWAREVersion(char* FirmVer);
VOID  DIAG_CheckSD_Card(void);
BOOL  DIAG_CheckSIM_Card(string& msg);
BOOL  DIAG_TurnTelRing(BOOL on, string& msg);
BOOL  DIAG_IMEIWrite(string& imei, string& msg);
VOID  DIAG_CheckNetWork(void);
VOID  DIAG_TraceAbilityRead(TraceInfo *pTrace);
VOID  DIAG_TraceAbilityWrite(char *traceInfo);
VOID  DIAG_TraceAbilityLocal(TraceInfo *pTrace);
#endif // JRD_DIAG_H
