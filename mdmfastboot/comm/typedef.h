/*=============================================================================
DESC:

CHANGE HISTORY:
when		who 	   what
----------	---------  --------------------------------------------------------
2010-02-25	jianwen.he  Init first version

=============================================================================*/
#ifndef _DLTYPE_H_
#define _DLTYPE_H_

#include "comdef.h"
#include <vector>
#include <afxmt.h>

//-----------------------------------------------------------------------------

enum {
	WM_USER_PROGRESS = WM_USER + 10,
	WM_USER_STATUS,
	WM_USER_SET_VERSION,
	WM_USER_COMSTATE,
	WM_USER_DEVICELIST,
	WM_USER_DEVICE_CHANGE,
	WM_USER_CHANGE_PORT,
	WM_USER_REGTASK,
	WM_USER_UNREGTASK,
	WM_USER_SETTASKSTATE,
	WM_USER_GETTASKSTATE,
	WM_USER_SETDLSTATUS			// 20110524 add by jianwen.he
};
//-----------------------------------------------------------------------------

#ifdef FEATURE_FAST_DLOAD
	#define MAX_CMD_BUFFER_LEN (10*KB) // 10KB
#else
	#define MAX_CMD_BUFFER_LEN (4*KB)  // 4KB
#endif

#define MAX_RSP_BUFFER_LEN (MAX_CMD_BUFFER_LEN)


/* Common image buffer type
*/
typedef struct {
	uint32           len;
	uint8*           data;
} TImgBufType;

/* This enum type is used to indicate in what state
 * is a device com port.
*/

typedef enum _PORT_STATE
{
	PS_READY = 0,
	PS_BUSY,
	PS_WAIT,
	PS_TIMEOUT,
	PS_ARRIVE,	
	PS_FAIL,
	PS_RETRY,
	PS_WARN,
	PS_DONE,
	PS_UNAVAILABLE,
	PS_MAX = 0xFFFF
}PORT_STATE;

/* Callback information
*/
struct tagTCbInfo;

typedef void (TCbProgressProc)(struct tagTCbInfo *pTCbInfo);

typedef struct tagTCbInfo {
	uint32           Size;
	HWND             hWnd;
	TCbProgressProc* pProgressProc;			// Call back function pointer
	//uint32           nTotal;				// Total of data. (byte)
	//uint32           nSent;				// Sent data length (byte)
	//uint32           nPercent;			// Account percent
	uint32           ratio;					// ratio of download images in the whole process
	void*            pUserData;				// carry extra user data
} TDLCbInfoType;

typedef struct tagImgLens{

	uint32 uQcsblHdLen;
	uint32 uQcsblLen;
	uint32 uOemsblLen;	
	uint32 uAmssImgLen;
	uint32 uDashboardImgLen;

}ImagLensStruct;

/* Callback user data type
*/
typedef struct {
	/* HWND to which is the message being sent
	 */
	//HWND			 hWnd;
	/* In MultiPort download, use the port number
	 * to update each download progress
	 */
	uint32			 port;
	/* Used by the download thread to wait for
	 * the device reset
	 */
	CEvent* 		 pEvent;
} TDLUserDataType;


#define IMAGE_HDR_LEN        (512)
#define PROJECT_NAME_LEN     (32)
#define BOOT_VERSION_LEN     (16)
#define FIRMWARE_VERSION_LEN (16)
#define EFS_VERSION_LEN      (16)

/* CAUTION:
 *   Make sure PADDING_LEN > 0
 */
#define PADDING_LEN          (IMAGE_HDR_LEN          \
                              -BOOT_VERSION_LEN      \
                              -FIRMWARE_VERSION_LEN  \
                              -EFS_VERSION_LEN       \
                              -sizeof(uint32)*6)

#define IMG_MAGIC_1          (0x5F7C8E1A)
#define IMG_MAGIC_2          (0x3B2D9A35)

/* The first three elements of the TModemImgHdrType
 * should never be changed to keep all version
 * compatible.
 */
typedef struct {
	/* Magic numbers (8 bytes)
	 */
	uint32     magic1;
	uint32     magic2;

	/* Header format version. Since the header format
	 * might be changed in future, we use this field
	 * to distinguish between different format. (4 bytes)
	 */
	uint32     header_version;
	//--------------------------------------------------
	/* Image mask, i.e. what kinds of images is contained.
	 * Refer to TMiTypeMaskEnumType. (4 bytes)
	 */
	uint32     mask;

	/* Image data total length, only data length is counted.
	 * (4 bytes)
	 */
	uint32     length;

	/* Number of images are contained. (4 bytes)
	 */
	uint32     count;
	
	/* Image project name, such as "Lemans" (32 bytes)
	 */
	char	   project_name[PROJECT_NAME_LEN];
	
	char	   boot_version[BOOT_VERSION_LEN];
	
	/* Image project version, such as "L0147000"
	 */
	char	   firmware_version[FIRMWARE_VERSION_LEN];
	
	char	   efs_version[EFS_VERSION_LEN];

	/* Padding field, packing the header size to be 512 bytes
	 */
	uint8      padding[PADDING_LEN];
} TModemImgHdrType;


/* MAX (HDLC) packet size has 4 bytes more than MAX_CMD_BUFFER_LEN.
* 2 bytes for CRC, 2 bytes for leading and ending 0x7E. 
*/
#define  MAX_PACKET_LEN  (MAX_CMD_BUFFER_LEN + 4)

#define XML_KEY_CONTENT				"CONTENT"
#define XML_KEY_HARDWARE            "Hardware"
#define XML_KEY_PACKAGE				"Package"
#define XML_KEY_CUSTOMERID			"CustomerID"
#define XML_KEY_BAND				"Band"
#define XML_KEY_PCB					"PCB"
#define XML_KEY_FLASH				"Flash"
#define XML_KEY_RF_SWITCHER			"RF_switcher"
#define XML_KEY_DIV_CALIBRATION		"Div_calibration"  //add by jie.li 2012-04-05 for Diversity NV

#define XML_KEY_SOFTWARE			"Software"
#define XML_KEY_DASHBOARD			"Dashboard"
#define XML_KEY_FIRMWARE_VER        "Firmware_Internal_Ver"
#define XML_KEY_PTS_VER				"PTS_Number"
#define XML_KEY_QCN_VER             "QCN"
#define XML_KEY_LTEBANDCFG			"LTEBANDCFG"   //add by jie.li 2011-11-17
#define XML_KEY_WINDOWS_VER			"WINDOWS"
#define XML_KEY_MAC_VER				"MAC"
#define XML_KEY_LINUX_VER			"LINUX"
#define XML_KEY_WEBUI				"WEBUI"			//add by jie.li 2011-11-17
//add by jie.li for MDM9x15
#define XML_KEY_Q6_RESOURCE_VER		"Q6_Resource_Ver"
#define XML_KEY_LINUX_KERNEL_VER	"Linux_Kernel_Ver"
#define XML_KEY_LINUX_APP_VER		"Linux_UserData_Ver"
#define XML_KEY_LINUX_SYS_VER		"Linux_SYS_Ver"
//end add

/* Add by jianwen.he 2010-07-15 */
#define XML_KEY_BANDCONF			"Band_conf"
#define XML_KEY_MODE				"Mode"
#define XML_KEY_ORDER				"Order"
#define XML_KEY_DOMAIN				"Domain"
/* End */

/* Add by jianwen.he 2010-07-10 */
#define XML_KEY_SWCONFIG			"SW_config"
#define XML_KEY_VOICE_ENABLE		"Voice_call_enable"
#define XML_KEY_SNIMEI_ENABLE		"SN_IMEI_access"
#define XML_KEY_SPECIAL_AT			"Special_AT_Support"
#define XML_KEY_DISK_ATTR			"Disk_attribute"
#define XML_KEY_SETUP_MOPDE			"Device_setup_mopde"
#define XML_KEY_LED_MODE			"LED_test_mode"
#define XML_KEY_EFS_DIR_ENABLE		"EFS_directory_accessible"
#define XML_KEY_DIAG_ENABLE			"Diag_port_enable"
#define XML_KEY_CONF_BAND			"Band"
/* End */

/* Add by jianwen.he 2010-07-21 */
#define XML_KEY_HSU_COMPID			"HSU_Comp_id"
/* End */

#define XML_ATTRIBUTE_VERSION		"version"
#define XML_ATTRIBUTE_MANDATORY     "Mandatory"
#define XML_ATTRIBUTE_UMTS2100      "UMTS2100"
#define XML_ATTRIBUTE_UMTS1900      "UMTS1900"
#define XML_ATTRIBUTE_UMTS1700      "UMTS1700"
#define XML_ATTRIBUTE_UMTS900       "UMTS900"
#define XML_ATTRIBUTE_UMTS850       "UMTS850"
#define XML_ATTRIBUTE_UMTS800       "UMTS800"	//add by jie.li 2011-11-07 for Y580
#define XML_ATTRIBUTE_UMTS1800      "UMTS1800"
#define XML_ATTRIBUTE_PCS1900       "PCS1900"
#define XML_ATTRIBUTE_DCS1800       "DCS1800"
#define XML_ATTRIBUTE_EGSM900       "EGSM900"
#define XML_ATTRIBUTE_GSM850        "GSM850"

//add by jie.li 011-11-22 for LTE Band
#define XML_ATTRIBUTE_LTE2100		"LTE2100"
#define XML_ATTRIBUTE_LTE1800		"LTE1800"
#define XML_ATTRIBUTE_LTE2600		"LTE2600"
#define XML_ATTRIBUTE_LTE900		"LTE900"
#define XML_ATTRIBUTE_LTE800		"LTE800"
//end add
//add by jie.li 2012-10-16 for LTE band
#define XML_ATTRIBUTE_LTE700		"LTE700"
#define XML_ATTRIBUTE_LTE1700		"LTE1700"
//end add
//add by jie.li 2013-1-21 for LTE band
#define XML_ATTRIBUTE_LTE850		"LTE850"
#define XML_ATTRIBUTE_LTE1900		"LTE1900"
//end add

typedef const char*  CONSTPCHAR;

typedef struct _XmlDataStruct{
	CONSTPCHAR key;
	CONSTPCHAR value;
}XmlDataStruct;

typedef std::vector<XmlDataStruct> CXMLVector;

//add by jie.li for 9200
typedef struct _FileBufStruct{
uint8*	strFileName;
uint32	uFileLens;
uint8*  strFileBuf;
}FileBufStruct;
typedef std::vector<FileBufStruct> CFileBufVector;
//end add

#endif //_DLTYPE_H_

