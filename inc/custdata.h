#ifndef __CUSTDATA_H__
#define __CUSTDATA_H__

#include "stdafx.h"
#include "log.h"
#include "pkt.h"
#include "utils.h"
//#include "../xml/DomParser.h"

#define NV_DATA_HEADER_LEA 512

#define PRJ_NAME_LEN 64

#define RESERVED_LEN  3

//modify by yanbin.wan 20130708 from 60 to 128
#define DIAG_FS_MAX_FILENAME_LEN       128
#define DIAG_FS_BUFFER_SIZE            1024

#define MAX_PC_PATH_LENGTH             1024

#define EFS_FILE_SIMLOCK        "jrdcfg/perso/perso.txt"
#define EFS_FILE_TRACE          "jrdcfg/traceability.txt"
#define CONFIG_XML_NAME         "jrdcfg/config.xml"
#define EFSCONFIG_INI_NAME      "jrdcfg/EfsConfig.ini"
#define DASHBOARD_NAME          "b.vhd"
#define EFS_FILE_WEBS_CONFIG    "webs_config.xml"		//add by jie.li 20120830 for Y580 to parse the webs_config
#define EFS_FILE_SSID_CONFIG    "jrdcfg/Ssid_config.xml"

enum {
	CUST_ACTION_NONE            = 0,
	CUST_BACKUP_RESTORE_NV      = LEFT_SHIFT_BITS(0),
	CUST_BACKUP_RESTORE_SIMLOCK = LEFT_SHIFT_BITS(1),
	CUST_BACKUP_RESTORE_TRACE   = LEFT_SHIFT_BITS(2),
	CUST_BACKUP_RESTORE_SMS     = LEFT_SHIFT_BITS(3),
	CUST_BACKUP_RESTORE_PB      = LEFT_SHIFT_BITS(4),
	// more ...
	CUST_ACTION_MAX             = LEFT_SHIFT_BITS(31),
};
typedef enum
{
	/* each nv data contains a Tag, a Length and a Value */
	NV_DATA_FORMAT_TLV = 0,
	// ...
	NV_DATA_FORMAT_MAX = 0xFF, // pack item into a byte
}nv_data_format_e_type;

typedef struct
{
	uint16						itemid;
	uint32						length;
	uint8*						data;
} nv_data_item_s_type;

/* NV data header information (totoally 512 bytes)
*/
typedef struct
{
	/* project name */
	char					project_name[PRJ_NAME_LEN];

	/* data format */
	nv_data_format_e_type	format;

	/* total item number */
	uint16					total_number;

	/* total item data length, excluding header size */
	uint32					total_length;

	/* use to pack the header size to 512 bytes */
	uint8					reserved[RESERVED_LEN];
}nv_data_header_s_type;


typedef struct
{
        bool				backup_restore;
	nv_data_header_s_type		header;
	nv_data_item_s_type*		items;
} nv_data_info_s_type;


typedef struct
{
        bool				backup_restore;
        uint8*				data;
} simlock_data_info_s_type;


typedef struct
{
        bool				backup_restore;
        uint8*				data;
} trace_data_info_s_type;


typedef struct
{
        nv_data_info_s_type		nv_info;
	simlock_data_info_s_type	simlock_info;
	trace_data_info_s_type		trace_info;
} cust_data_info_s_type;


/* For customer backup & restore images,
 * such as SimLock/Trace.
*/

typedef struct
{
	//bool			  bBackupRestore;
	char	          path[DIAG_FS_MAX_FILENAME_LEN];
	//TFileBufType       FileBuf;
} TCustImageType;

typedef enum
{
	CUST_MASK_NONE               = 0,
	CUST_MASK_BACKUP_NV          = LEFT_SHIFT_BITS(0),
	CUST_MASK_BACKUP_SIMLOCK     = LEFT_SHIFT_BITS(1),
	CUST_MASK_BACKUP_TRACE       = LEFT_SHIFT_BITS(2),
	CUST_MASK_RESTORE_NV         = LEFT_SHIFT_BITS(3),
	CUST_MASK_RESTORE_SIMLOCK    = LEFT_SHIFT_BITS(4),
	CUST_MASK_RESTORE_TRACE      = LEFT_SHIFT_BITS(5),
	CUST_MASK_MAX                = LEFT_SHIFT_BITS(31),
} TCustMaskEnumType;

typedef enum
{
	MODE_CHANGE_OFFLINE_ANALOG_MODE,		/* Go to offline analog */
	MODE_CHANGE_OFFLINE_DIGITAL_MODE,		/* Go to offline digital */
	MODE_CHANGE_RESET,						/* Reset. Only exit from offline */
	MODE_CHANGE_OFFLINE_FACTORY_TEST_MODE,	/* FTM mode - if supported */
	MODE_CHANGE_ONLINE,						/* Online mode - if supported */
	MODE_CHANGE_LPM,						/* LPM mode - if supported */
	MODE_CHANGE_POWER_OFF,					/* Power off mode */
	MODE_CHANGE_MAX
}MODE_CHANGE_E_TYPE;

#define CUST_BACKUP_RESTORE_NV      (CUST_MASK_BACKUP_NV | CUST_MASK_RESTORE_NV)
#define CUST_BACKUP_RESTORE_SIMLOCK (CUST_MASK_BACKUP_SIMLOCK | CUST_MASK_RESTORE_SIMLOCK)
#define CUST_BACKUP_RESTORE_TRACE   (CUST_MASK_BACKUP_TRACE | CUST_MASK_RESTORE_TRACE)

typedef enum cm_mode_pref_e
{

  CM_MODE_PREF_NONE=-1,
    /**< @internal */
  CM_MODE_PREF_AMPS_ONLY=0,
    /**< = NV_MODE_ANALOG_ONLY, Service is limited to analog only */
  CM_MODE_PREF_DIGITAL_ONLY=1,
    /**< = NV_MODE_DIGITAL_ONLY, Service is limited to digital
    ** (CDMA, HDR, GW) only */
  CM_MODE_PREF_AUTOMATIC=2,
    /**< = NV_MODE_AUTOMATIC, Determine the mode automatically */
  CM_MODE_PREF_EMERGENCY=3,
    /**< = NV_MODE_EMERGENCY, Emergency mode
    **
    **  It's intended to be used internally by CM.
    **  Client is not allowed to use it as parameter to change system
    **  selection preference command.
    **
     */
  /* For compatibility with QPST, do not change values or
  ** order. We start with NV_MODE_CELL_CDMA_ONLY+1 (i.e. 9).
  */
  CM_MODE_PREF_CDMA_ONLY = 9,
  /**<
    ** Service is limited to CDMA only
    */
  CM_MODE_PREF_HDR_ONLY=10,
    /**<
    ** Service is limited to HDR only
    */
  CM_MODE_PREF_CDMA_AMPS_ONLY=11,
    /**<
    ** Service is limited to CDMA and AMPS only
    */
  CM_MODE_PREF_GPS_ONLY=12,
    /**<
    ** Service is limited to GPS only
    */
  /*
  ** The following enums are the radio access technologies for
  ** WCDMA and GSM.
  */
  CM_MODE_PREF_GSM_ONLY=13,
    /**<
    ** Service is limited to GSM only
    */
  CM_MODE_PREF_WCDMA_ONLY=14,
    /**<
    ** Service is limited to WCMDA only
    */
  /*
  ** Others
  */
  CM_MODE_PREF_PERSISTENT=15,
    /**<
    ** Return to persistent mode preference.
    ** Will be mapped to sth else by CM,
    ** ph_ptr->mode_pref is never set to this value
    */
  CM_MODE_PREF_NO_CHANGE=16,
    /**<
    ** Do not change the mode preference.
    ** Will be mapped to sth else by CM,
    ** ph_ptr->mode_pref is never set to this value
    */
  /*
  ** Even though logically the below belog with
  ** enums listed above, they were added at the end
  ** to not break compatibility with test tools
  */
  CM_MODE_PREF_ANY_BUT_HDR=17,
    /**<
    ** Use any service but HDR
    */
  CM_MODE_PREF_CURRENT_LESS_HDR=18,
    /**<
    ** Remove HDR component from current mode preference.
    ** Will be mapped to sth else by CM,
    ** ph_ptr->mode_pref is never set to this value
    */
  CM_MODE_PREF_GSM_WCDMA_ONLY=19,
    /**<
    ** Service is limited to GSM and WCDMA only
    */
  CM_MODE_PREF_DIGITAL_LESS_HDR_ONLY=20,
    /**< Acquire digital, non-HDR mode systems only
    ** (CDMA or GSM or WCDMA )
    */
  CM_MODE_PREF_CURRENT_LESS_HDR_AND_AMPS=21,
    /**<
    ** Remove HDR and AMPS component from current mode preference.
    ** Will be mapped to sth else by CM,
    ** ph_ptr->mode_pref is never set to this value
    */
  CM_MODE_PREF_CDMA_HDR_ONLY=22,
    /**<
    ** Acquire CDMA or HDR systems only
    */
  CM_MODE_PREF_CDMA_AMPS_HDR_ONLY=23,
    /**<
    ** Acquire CDMA or AMPS or HDR systems only
    */
  CM_MODE_PREF_CURRENT_LESS_AMPS=24,
    /**<
    ** Remove AMPS component from current mode preference.
    ** Will be mapped to sth else by CM,
    ** ph_ptr->mode_pref is never set to this value
    */
  /* Clients check CM_API_WLAN for WLAN support for following
  ** preferences.
  */
  CM_MODE_PREF_WLAN_ONLY=25,
    /**<
    ** Acquire WLAN systems only
    */
  CM_MODE_PREF_CDMA_WLAN=26,
    /**<
    ** Acquire CDMA and WLAN systems only
    */
  CM_MODE_PREF_HDR_WLAN=27,
    /**<
    ** Acquire HDR and WLAN systems only
    */
  CM_MODE_PREF_CDMA_HDR_WLAN=28,
    /**<
    ** Acquire CDMA, HDR and WLAN systems only
    */
  CM_MODE_PREF_GSM_WLAN=29,
    /**<
    ** Acquire GSM and WLAN systems only
    */
  CM_MODE_PREF_WCDMA_WLAN=30,
    /**<
    ** Acquire WCDMA and WLAN systems only
    */
  CM_MODE_PREF_GW_WLAN=31,
    /**<
    ** Acquire GSM/WCDMA and WLAN systems only
    */
  CM_MODE_PREF_CURRENT_PLUS_WLAN=32,
    /**<
    ** Acquire WLAN systems in addition to current system
    */
  CM_MODE_PREF_CURRENT_LESS_WLAN=33,
    /**<
    ** Remove WLAN systems from the current system
    */
  CM_MODE_PREF_CDMA_AMPS_HDR_WLAN_ONLY = 34,
    /**<
    ** Acquire CDMA, AMPS, HDR and WLAN systems.
    */
  CM_MODE_PREF_CDMA_AMPS_WLAN_ONLY = 35,
    /**<
    ** Acquire CDMA, AMPS and WLAN systems.
    */
  CM_MODE_PREF_INTERSECT_OR_FORCE = 36,
    /**< Acquire the mode that is common (intersection of) the current mode and
    ** the requested mode, if the intersection is NULL, then Force the
    ** requested preferences
    */
  CM_MODE_PREF_ANY_BUT_HDR_WLAN = 37,
    /**<
    ** Use any service except HDR and WLAN
    */
  CM_MODE_PREF_LTE_ONLY = 38,
    /**<
    ** Service is limited to LTE only
    */
  CM_MODE_PREF_GWL = 39,
	/**<
    ** Service is limited to GSM, WCDMA or LTE
    */
    /**< @internal */
  CM_MODE_PREF_WCDMA_LTE_ONLY3G = 40,
    /**<
    ** Service is limited to WCDMA or LTE,no 2G ,only 3G
    */
  CM_MODE_PREF_MAX
    /* FOR INTERNAL USE OF CM ONLY!
    */

} cm_mode_pref_e_type;
  typedef enum cm_gw_acq_order_pref_e {

  CM_GW_ACQ_ORDER_PREF_NONE = -1,
    /**< @internal */
  CM_GW_ACQ_ORDER_PREF_AUTOMATIC=0,
    /**< Determine mode automatically from the PRL order */
  CM_GW_ACQ_ORDER_PREF_GSM_WCDMA=1,
    /**< Acqisition order is first GSM followed by WCDMA */
  CM_GW_ACQ_ORDER_PREF_WCDMA_GSM=2,
    /**< Acqisition order is first WCDMA followed by GSM */
  CM_GW_ACQ_ORDER_PREF_NO_CHANGE=3,
    /**< Do not change GSM/WCDMA acquisition order */
  CM_GW_ACQ_ORDER_PREF_PERSISTENT=4,
    /**< Return to acq order preference given by NV.
    **
    ** Clients need to check for CM_API_PERSISTENT_PREF
    ** before using this
    */
  CM_GW_ACQ_ORDER_PREF_MAX
    /**< @internal */

} cm_gw_acq_order_pref_e_type;
  typedef enum cm_srv_domain_pref_e
  {

  CM_SRV_DOMAIN_PREF_NONE=-1,
    /**< No service domain is requested. */

  CM_SRV_DOMAIN_PREF_CS_ONLY=0,
    /**< Prefer Circuit Switched Only */

  CM_SRV_DOMAIN_PREF_PS_ONLY=1,
    /**< Prefer Packet Switched Only */

  CM_SRV_DOMAIN_PREF_CS_PS=2,
    /**< Prefer Circuit and Packet Switched */

  CM_SRV_DOMAIN_PREF_ANY=3,
    /**< Any domain will do. No preference */

  CM_SRV_DOMAIN_PREF_NO_CHANGE=4,
    /**< To be used by clients who do not want to change the service domain */

  CM_SRV_DOMAIN_PREF_PS_ATTACH=5,
    /**<  PS attach on demand.
        Note: This value of service domain preference is not saved in NV  */

  CM_SRV_DOMAIN_PREF_PS_DETACH=6,
    /**< PS detach on demand
       Note: This value of service domain preference is not saved in NV   */


  CM_SRV_DOMAIN_PREF_PERSISTENT=7,
    /**< Return to value stored in NV
    **
    ** Clients need to check for CM_API_PERSISTENT_PREF
    ** before using this
    */

  CM_SRV_DOMAIN_PREF_MAX
    /**< @internal */


} cm_srv_domain_pref_e_type;
//typedef  unsigned short word;        /* Unsigned 16 bit value type. */


	typedef  uint16  nv_mode_enum_type;
  /* CDMA then Analog */
  #define  NV_MODE_DIGITAL_PREF                          ((nv_mode_enum_type)0)
  /* CDMA only */
  #define  NV_MODE_DIGITAL_ONLY                          ((nv_mode_enum_type)1)
  /* Analog then CDMA */
  #define  NV_MODE_ANALOG_PREF                           ((nv_mode_enum_type)2)
  /* Analog only */
  #define  NV_MODE_ANALOG_ONLY                           ((nv_mode_enum_type)3)
  /* Determine mode automatically */
  #define  NV_MODE_AUTOMATIC                             ((nv_mode_enum_type)4)
  /* Emergency mode */
  #define  NV_MODE_E911                                  ((nv_mode_enum_type)5)
  /* Restrict to home only */
  #define  NV_MODE_HOME_ONLY                             ((nv_mode_enum_type)6)
  /* Restrict to PCS home only */
  #define  NV_MODE_PCS_CDMA_ONLY                         ((nv_mode_enum_type)7)
  /* Restrict to cellular home only */
  #define  NV_MODE_CELL_CDMA_ONLY                        ((nv_mode_enum_type)8)

  #define  NV_MODE_CDMA_ONLY                             ((nv_mode_enum_type)9)

  #define  NV_MODE_HDR_ONLY                              ((nv_mode_enum_type)10)

  #define  NV_MODE_CDMA_AMPS_ONLY                        ((nv_mode_enum_type)11)

  #define  NV_MODE_GPS_ONLY                              ((nv_mode_enum_type)12)
  /* Service is limited to GSM only */
  #define  NV_MODE_GSM_ONLY                              ((nv_mode_enum_type)13)
  /* Service is limited to WCDMA only */
  #define  NV_MODE_WCDMA_ONLY                            ((nv_mode_enum_type)14)

  #define  NV_MODE_WLAN_ONLY                             ((nv_mode_enum_type)15)

  #define  NV_MODE_ANY_BUT_HDR                           ((nv_mode_enum_type)16)

  #define  NV_MODE_GSM_WCDMA_ONLY                        ((nv_mode_enum_type)17)

  #define  NV_MODE_DIGITAL_LESS_HDR_ONLY                 ((nv_mode_enum_type)18)

  #define  NV_MODE_CDMA_HDR_ONLY                         ((nv_mode_enum_type)19)

  #define  NV_MODE_CDMA_AMPS_HDR_ONLY                    ((nv_mode_enum_type)20)

  #define  NV_MODE_CDMA_WLAN_ONLY                        ((nv_mode_enum_type)21)

  #define  NV_MODE_HDR_WLAN_ONLY                         ((nv_mode_enum_type)22)

  #define  NV_MODE_CDMA_HDR_WLAN_ONLY                    ((nv_mode_enum_type)23)

  #define  NV_MODE_GSM_WLAN_ONLY                         ((nv_mode_enum_type)24)

  #define  NV_MODE_WCDMA_WLAN_ONLY                       ((nv_mode_enum_type)25)

  #define  NV_MODE_GW_WLAN_ONLY                          ((nv_mode_enum_type)26)

  #define  NV_MODE_CDMA_AMPS_HDR_WLAN_ONLY               ((nv_mode_enum_type)27)

  #define  NV_MODE_CDMA_AMPS_WLAN_ONLY                   ((nv_mode_enum_type)28)

  #define  NV_MODE_ANY_BUT_HDR_WLAN                      ((nv_mode_enum_type)29)

  #define  NV_MODE_LTE_ONLY                              ((nv_mode_enum_type)30)

  #define  NV_MODE_GWL                                   ((nv_mode_enum_type)31)

  #define  NV_MODE_CDMA_LTE_ONLY                         ((nv_mode_enum_type)32)

  #define  NV_MODE_HDR_LTE_ONLY                          ((nv_mode_enum_type)33)

  #define  NV_MODE_GSM_LTE_ONLY                          ((nv_mode_enum_type)34)

  #define  NV_MODE_WCDMA_LTE_ONLY                        ((nv_mode_enum_type)35)

  #define  NV_MODE_CDMA_HDR_LTE_ONLY                     ((nv_mode_enum_type)36)

  #define  NV_MODE_CDMA_GSM_LTE_ONLY                     ((nv_mode_enum_type)37)

  #define  NV_MODE_CDMA_WCDMA_LTE_ONLY                   ((nv_mode_enum_type)38)

  #define  NV_MODE_HDR_GSM_LTE_ONLY                      ((nv_mode_enum_type)39)

  #define  NV_MODE_HDR_WCDMA_LTE_ONLY                    ((nv_mode_enum_type)40)

  #define  NV_MODE_CDMA_LTE_WLAN_ONLY                    ((nv_mode_enum_type)41)

  #define  NV_MODE_HDR_LTE_WLAN_ONLY                     ((nv_mode_enum_type)42)

  #define  NV_MODE_GSM_LTE_WLAN_ONLY                     ((nv_mode_enum_type)43)

  #define  NV_MODE_WCDMA_LTE_WLAN_ONLY                   ((nv_mode_enum_type)44)

  #define  NV_MODE_CDMA_HDR_LTE_WLAN_ONLY                ((nv_mode_enum_type)45)

  #define  NV_MODE_CDMA_GSM_LTE_WLAN_ONLY                ((nv_mode_enum_type)46)

  #define  NV_MODE_CDMA_WCDMA_LTE_WLAN_ONLY              ((nv_mode_enum_type)47)

  #define  NV_MODE_HDR_GSM_LTE_WLAN_ONLY                 ((nv_mode_enum_type)48)

  #define  NV_MODE_HDR_WCDMA_LTE_WLAN_ONLY               ((nv_mode_enum_type)49)

class CDIAGCmd;

typedef void (*ProgressCallback)(int port,uint16 percent);
class CCustData
{
public:
        CCustData(CPacket& packetDll, TCustDataInfoType* pCustDataInfo);
	~CCustData();

public:
    TResult DLoadDashboard(bool bWriteArm, const char* fileName, uint8* data, long len);
    void    SetRatioParams(uint8 ratio, uint8 base);
    TResult WriteStaticQCN(uint8* data,long len);
    TResult WritePersoTxt(const char* fileName, uint8* data, long len);
    TResult WriteConfigXml(const char* fileName, uint8* data, long len);
    TResult ReadConfigXml(const char* filename,uint8* pdata, long& rlen);
    bool    ChangeOfflineMode(uint16 wMode=MODE_CHANGE_OFFLINE_DIGITAL_MODE);
    TResult WriteFile(bool bWriteArmEfs, const char* fileName,uint8* data, long len, bool bDelExistFile = true);
    bool    WriteBandConfigItems(unsigned int dsat_syssel_val[4]);
    static  ProgressCallback func;
    void    RegisterCallback(ProgressCallback callback);
    TResult DeleteDir(bool bWriteArm, const char* fileName);

    //add by jie.li 20120830 for Y580 to parse the webs_config
    TResult ReadWebsConfigFile(WebsXMLInfo* pwebsxmlinfo);
    bool    websconfigParse(uint8* pXmlBuf, WebsXMLInfo* pxmlinfo, int len);
    //end add

    TResult ReadSSIDConfigFile(WebsXMLInfo* pwebsxmlinfo);
    TResult ReadFile(bool bReadArmEfs, const char* filename, uint8**ppdata, long& rlen);
    TResult DeleteEFSFiles(bool bWriteArm, const char* fileName);
    TResult DeleteFile(bool bWriteArmEfs, const char* fileName);

private:
	TNviItemType* 	   	m_pItems;
	uint32             	m_uCount;
	TCustDataInfoType* 	m_pCustDataInfo;

	//char			   	m_sProjectDir[MAX_PC_PATH_LENGTH];
	//char			   	m_sProjectDir[MAX_PATH];
	bool              	m_isMoreOp;
        TImgBufType             m_simlockData;
        TImgBufType             m_traceData;
        char                    m_PCBNo[15];


	//TResult WriteFile(char* fileName,uint8* data, uint32 len);
	bool RemoveDir(char* sourceDirName);
	bool SendHelloPacket(void);

private:
	CDIAGCmd*            m_pDIAGCmd;
	/* use for progress display control */
	uint8                m_uTotalRatio;
	uint8                m_uRatio;
	uint8                m_uBaseRatio;
        int                  dlPort;
        uint8 lastdone;
};

#endif //__CUSTDATA_H__

