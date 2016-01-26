/*=============================================================================
					   *BASIC TYPE DEFINE*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-08-30  jianwen.he      Init first version

=============================================================================*/

#ifndef DEFINE_H
#define DEFINE_H

#include <iostream>


using namespace std;
typedef signed char int8;         /* 8 bit signed */
typedef unsigned char uint8;      /* 8 bit unsigned */
typedef short int16;              /* 16 bit signed */
typedef unsigned short uint16;    /* 16 bit unsigned */
typedef int int32;                /* 32 bit signed */
typedef unsigned int uint32;      /* 32 bit unsigned */

#define word		uint16

#define OPENLOG     "OPENLOG.txt"

// MDM6246 mobile_model_id=33
#define MDM6246_MOBILE_ID		33
// MDM6290 mobile_model_id=29
#define MDM6290_MOBILE_ID		29
// MDM6200 mobile_model_id=4043
#define MDM6200_MOBILE_ID		4043
#define MDM8200A_MOBILE_ID      8
#define MDM6270_MOBILE_ID		19
/*Begin: add by jie.li 2011-07-04 MDM9200 compatible*/
// MDM9200 mobile_model_id=4051
#define MDM9200_MOBILE_ID		4051
/*End.*/

//add by jie.li for MDM9x15
#define MDM9x15_MOBILE_ID		4070
//end add

//add by minghui.zhang for MDM9x25   2013-11-02
#define MDM9x25_MOBILE_ID		4087
// define 9x30 mobile id
#define MDM9x30_MOBILE_ID		4093


typedef enum
{
        /* General error */
        EOK = 0,               // No Error
        EFAILED,               // General Error
        EINVALIDPARAM,         // Invalid parameters Error
        ENOMEMORY,             // Out of memory
        EBUFFERBROKEN,

        /* com port layer error */
        EPORTOPEN,             // Port Open Error
        EPORTSEND,             // Port Send Error
        EPORTSENDTIMEOUT,      // Port Send Timeout Error
        EPORTRECEIVE,          // Port Receive Error
        EPORTRECEIVETIMEOUT,   // Port Receive Timeout Error
        EPORTCLOSE,            // Port Close Error

        /* packet layer error */
        EPACKETSEND,           // Packet Send Error
        EPACKETRECEIVECRC,     // Packet Receive CRC Error
        EPACKETRECEIVETOOLARGE,// Packet Receive Too Large Error
        EPACKETRECEIVETOOSHORT,// Packet Receive Too Short Error
        EPACKETBROKEN,         // Packet Broken (overflow)
        EPACKETUNKNOWN,        // Packet Unknown (not support)

        /* command layer general error */
        ECMDSEND,
        ERSPRECEIVE,

        /* command layer EFS operation error */
        EEFSOPHELLO,
        EEFSOPSTAT,
        EEFSOPOPEN,
        EEFSOPREAD,
        EEFSOPWRITE,
        EEFSOPCLOSE,

        /* PRG download command error */
        EACKPACKET,            // can not receive a ack packte, (ACK or NAK)
        EACKFAILED,            // receive a NAK cmd

        /* Data download command error */
        EDATARSP,
        EDATACRC,

        /* Image error */
        EIMAGEFAILED,

        /* Get Device Info failed */
        EGETDEVINFO,

        /* Version */
        EVERSION,

        /* DLPrg error */
        //EDLOADNOP,            // send nop cmd error
        //EDLOADWRITE,          // send write cmd error
        //EDLOADGO,             // send go cmd error

        /* Host download error */
        EMODEUNSUPPORT,         // unknown mode
        EHOSTDLDUMMY,
        EHOSTDLHELLO,         // send hello packet error
        EHOSTDLSECMODE,       // send security mode error
        EHOSTDLPRTNTBL,       // send partition table error
        EHOSTDLPRTNTBLDIFF,   // partition table different (unmatch)
        EHOSTDLOPEN,          // send open packet error
        EHOSTDLWRITE,         // send write packet error
        EHOSTDLCLOSE,         // send close packet error

        EBACKUPNV,
        EBACKUPSIMLOCK,
        EBACKUPTRACE,
        ERESTORENV,
        ERESTORESIMLOCK,
        ERESTORETRACE,
        EHOSTDLDASHBOARD,
        EWRITETOPC,
        EATOPERTE,

        /* module layer error */
        ECUSTBACKUP,
        ECUSTBACKUPNV,
        ECUSTBACKUPSIMLOCK,
        ECUSTBACKUPTRACE,

        EDLOADPRG,

        EDLOADBOOT,
        EDLOADAMSS,
        EDLOADEFS,

        ECUSTRESTORE,
        ECUSTRESTORENV,
        ECUSTRESTORESIMLOCK,
        ECUSTRESTORETRACE,

        EDLDASHBOARD,
        EWRITEQCN,
        EWRITEXML,
        EWRITESIMLOCK,
        EERASEEFS,
        EPARTITION,
        EXMLINCORRECT,
        EMAXERROR = 0xFF,
} TResult;

//Common image buffer type
typedef struct
{
        uint32          len;
        uint8*          data;
} TImgBufType;

/* For firmware download*/
typedef struct {
        TImgBufType             prg;
        TImgBufType             prtn;

        TImgBufType             oemsblhd;

        TImgBufType             amsshd;
        /* add by jianwen.he 2010-05-17*/
    //add by huangzhongping 2011-03-24 for 8200A
    /* add by jie.li 2011-07-04   MDM9200 compatible*/
    /* End.*/
        //add by jie.li for MDM9x15
        uint8                   dashboardVer[128];   //the version of Win+Mac+Linux
        uint32                  lenDashboardVer;
        //end add

        //add by minghui.zhang for MDM9x25


        //end add


        //add by zhanghao for MDM9x30 2014-09-05

} TDLImgInfoType;

//port work state define
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

#if 0
//version type define
typedef enum _VERSION_TYPE
{
    FIRWARE_VER = 0,
    WIN_VER,
    MAC_VER,
    LINUX_VER,
    QCN_VER,
    PTS_VER
} VERSION_TYPE;
#endif

typedef enum
{
    E_PRG_REGIS_TASK_OK = 0,
    E_PRG_PREPARE_DL_DATA,
    E_PRG_DL_IMAGES,
    E_PRG_CLOSE_PORT,
    E_PRG_WRITE_EFS_CONFIG,
    E_PRG_DL_PRG,
    E_PRG_DL_DASHBOARD,
    E_PRG_CLEAR_EFS_CONFIG,
    E_PRG_DEVICE_IN_DL_MODE,
    E_PRG_STORE_PIC,
    E_PRG_ERASE_SIMLOCK,
    E_PRG_REQUEST_FW_VERSION,
    E_PRG_REQUEST_MOBILE_ID,
    E_PRG_CLEAR_PBM,
    E_PRG_WRITE_STATIC_NV,
    E_PRG_DL_CONFIGXML,
    E_PRG_GETTING_DEVICE_STATUS,
    E_PRG_COMPARE_CONFIGXML,
    E_PRG_WRITE_NV_MORE_THAN20000,
    E_PRG_READ_CONFIGXML,
    E_PRG_REMOVE_REDL,
    E_PRG_REMOVE_RETRY,
    E_PRG_RESET_DEVICE,
    E_PRG_DL_FINISH,

    E_PRG_MAX = 0XFFF

}DL_PROGRESS_STATE_EVENT;

typedef enum
{
    E_RES_OK_SWITCH_OFFLINE = 0,
    E_RES_OK_DL,
    E_RES_ERR_DL,
    E_RES_ERR_OPEN_PORT,
    E_RES_ERR_OPEN_DEVICE,
    E_RES_ERR_DL_IMAGES,
    E_RES_ERR_IMG_PACKAGE_MATCH,
    E_RES_ERR_DL_DASHBOARD,
    E_RES_ERR_WRITE_HSU_COMP_ID,
    E_RES_ERR_WRITE_SOFTWARE_CONFIG,
    E_RES_ERR_FW_VERSION_SUPPORT,
    E_RES_ERR_WRITE_CONFIG_FILE,
    E_RES_ERR_WRITE_LTE_EFS_CONFIG,
    E_RES_ERR_WRITE_EFS_CONFIG,
    E_RES_ERR_STORE_PIC,
    E_RES_ERR_READ_WEBS_CONFIG,
    E_RES_ERR_WRITE_SSID,
    E_RES_ERR_READ_CUST_MODE_NAME,
    E_RES_ERR_READ_MAC_ADDR,
    E_RES_ERR_READ_IMEI,
    E_RES_ERR_READ_AUTO_GEN_WIFI_PWDFLAG,
    E_RES_ERR_WRITE_NV,
    E_RES_ERR_GENERATE_FT_FILES,
    E_RES_ERR_READ_SSID_CONFIG,
    E_RES_ERR_WRITE_NV453,
    E_RES_ERR_SET_FUNC_FIVE,
    E_RES_ERR_ERASE_SIMLOCK,
    E_RES_ERR_REQUEST_PARTITION,
    E_RES_ERR_PARTITION_TYPE_MATCH,
    E_RES_ERR_REQUEST_MOBILE_ID,
    E_RES_ERR_DL_PRG,
    E_RES_ERR_READ_FW_VERSION,
    E_RES_ERR_SWITCH_OFFLINE_MODE,
    E_RES_ERR_READ_INI_FILES,
    E_RES_ERR_READ_SPC,
    E_RES_ERR_VERIFY_SPC,
    E_RES_ERR_FLASH_TYPE_MATCH,
    E_RES_ERR_WRITE_STATIC_NV,
    E_RES_ERR_DL_CONFIGXML,
    E_RES_ERR_READ_FLASH_TYPE,
    E_RES_ERR_WRITE_BAND_CONFIG,
    E_RES_ERR_WRITE_NV_MORE_THAN20000,
    E_RES_ERR_PARSE_CONFIGXML,
    E_RES_ERR_CUSTOMER_ID_MATCH,
    E_RES_ERR_BAND_COMPARE,
    E_RES_ERR_READ_CONFIGXML,

    E_RES_MAX = 0XFFF

}DL_RESULT_STATE_EVENT;

/*-------------------------------------------------------------------------*/

/* SB Architecture 1.0 */
#define PRGCOMBINE_NAME			"nandprgcombined.mbn"
#define PARTITION_NAME			"partition.mbn"
#define QCSBLHDCFG_NAME			"qcsblhd_cfgdata.mbn"
//#define QCSBL_NAME                      "qcsbl.mbn"
#define OEMSBLHD_NAME			"oemsblhd.mbn"
//#define OEMSBL_NAME                     "oemsbl.mbn"
#define AMSSHD_NAME                     "amsshd.mbn"

/* SB Architecture 2.0 */

#define DSP1_NAME                       "dsp1.mbn"
#define DSP2_NAME                       "dsp2.mbn"

/* efs file */
#define DASHBOARD_NAME			"b.vhd"
#define QCN_NAME                        "static.qcn"
#define SIMLOCK_NAME			"perso.txt"
#define XML_NAME                        "config.xml"
#define DYNAMIC_NAME			"dynamic_nv.xml"

//add by jie.li for MDM9x15
#define SBL1_NAME                       "sbl1.mbn"
#define SBL2_NAME                       "sbl2.mbn"
#define APPSBOOT_NAME			"appsboot.mbn"
#define RPM_NAME                        "rpm.mbn"
#define DSP3_NAME                       "dsp3.mbn"
#define BOOTOE_NAME                     "boot-oe-msm9615.img"
#define BOOTIMG_NAME			"9615-cdp-image-9615-cdp.yaffs2"
#define USERIMG_NAME			"9615-cdp-usr-image.usrfs.yaffs2"
#define EFS_NAME                        "efs.mbn"
//end add

//add by minghui.zhang for MDM9x25
#define MBA_NAME                        "mba.mbn"
#define QDSP6SW_NAME 			"qdsp6sw.mbn"
#define SDI_NAME                        "sdi.mbn"
#define TZ_NAME                         "tz.mbn"
#define USRESOURCE_NAME			"jrd-resource.usrfs.yaffs2"
#define USRIMAGE_NAME			"mdm9625-usr-image.usrfs.yaffs2"
#define BOOTIMAGE_NAME                  "mdm-image-boot-mdm9625.img"
#define MDMIMG_NAME                     "mdm-image-mdm9625.yaffs2"

#define MDM_RECV_BOOT_IMG               "mdm-recovery-image-boot-mdm9625.img"
#define MDM_RECV_YAFFS                  "mdm-recovery-image-mdm9625.yaffs2"
//end add

//add for mdm9x30
#define NON_HLOS_9X30_NAME              "NON-HLOS.yaffs2"
#define BOOT_9X30_NAME                  "mdm9635-boot.img"
#define SYSFS_YAFFS_9X30_NAME           "mdm9635-sysfs.yaffs2"
#define USRFS_YAFFS_9X30_NAME           "mdm9635-usrfs.yaffs2"
#define RECV_YAFFS_9X30_NAME            "mdm-recovery-image-mdm9635.yaffs2"
#define NAND_PRG_9X30_NAME              "NPRG9x35.mbn"

/*******************config.xml define Begin*************************/

#define XML_KEY_CUSTOMERID		"CustomerID"
#define XML_KEY_FLASH			"Flash"

#define XML_KEY_FIRMWARE_VER            "Firmware_Internal_Ver"
#define XML_KEY_FIRMWARE_EX_VER         "Firmware_External_Ver"
#define XML_KEY_PTS_VER			"PTS_Number"
#define XML_KEY_QCN_VER                 "QCN"
#define XML_KEY_WINDOWS_VER		"WINDOWS"
#define XML_KEY_MAC_VER			"MAC"
#define XML_KEY_LINUX_VER		"LINUX"
#define XML_KEY_WEBUI_VER               "WEBUI"         //add by jie.li 2011-11-29 for Y580



//add by jie.li for MDM9x15
#define XML_KEY_Q6_RESOURCE_VER		"Q6_Resource_Ver"
#define XML_KEY_LINUX_KERNEL_VER	"Linux_Kernel_Ver"
#define XML_KEY_LINUX_APP_VER		"Linux_UserData_Ver"
#define XML_KEY_LINUX_SYS_VER		"Linux_SYS_Ver"
//end add

//add by minghui.zhang for MDM9X25
//add end


#define XML_KEY_SETUP_MOPDE		"Device_setup_mopde"
#define XML_KEY_HSU_COMPID		"HSU_Comp_id"


//add by jie.li 011-11-29 for LTE Band
//end add

//add by jie.li 2012-10-16
//end add

//add by yanbin.wan 2013-1-25 for LTE band
//end add

//add by jie.li 2012-04-06 for Diversity NV

//end add

//add by minghui.zhang 2013-11-14
#define DASHBOARD_VER_SEG               "######"

//add by yanbin.wan 20130826
//example
/*<BAND>
<FDDLTE B1-2100="no" B2-1900="no" B3-1800="no" B4-AWS="no" B5-850="no" B7-2600="no" B8-900="no" B17-700="no" B20-800="no" ></FDDLTE>
<TDDLTE B35-1900="no" B36-1900="no" B37-1900="no" B38-2600="no" B39-1900="no" B40-2300="no" B41-2600="no" B42-3500="no" B43-3700="no" ></TDDLTE>
<UMTS B1-2100="yes" B2-1900="no" B3-1800="no" B4-AWS="no" B5-850="no" B6-800="no" B8-900="yes" B9-1700="no" >B1-2100,B8-900</UMTS>
<CDMA></CDMA>
<TDSCDMA></TDSCDMA>
<GSM EGSM900="yes" DCS1800="yes" GSM850="yes" PCS1900="yes">EGSM900,DCS1800,GSM850,PCS1900</GSM>
</BAND>*/

//band type



//FDDLTE

//TDDLTE

//end add

//add by minghui.zhang  2014-03-14  for xml version 5.0


#define XML_KEY_FLASH_CODE              "Flash_Code"




/*******************config.xml define End*************************/




/* Data size unit */
#define KB (1024)	   // kilobyte
//#define MB (1024*1024)    // megabyte

#ifdef FEATURE_FAST_DLOAD
#define MAX_CMD_BUFFER_LEN (10*KB) // 10KB
#else
#define MAX_CMD_BUFFER_LEN (4*KB)  // 4KB
#endif

#define MAX_RSP_BUFFER_LEN (MAX_CMD_BUFFER_LEN)

//-----------------------------------------------------------------------------
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define  B_PTR(var)  ((byte *) (void *) &(var))
#define  W_PTR(var)  ((word *) (void *) &(var))
#define  D_PTR(var)  ((dword *) (void *) &(var))

#define OK 0
#define SUCCESS(result) ((result == OK) ? true : false)
#define FAILURE(result) ((result != OK) ? true : false)


/* metric */
// memery operation define

/****************color Define*********************/
#define PROGRESS_ERR_COLOR      QColor("red")
#define PROGRESS_WARN_COLOR     QColor(220,220,100)
#define PROGRESS_OK_COLOR       QColor("green")
#define PROGRESS_REDOWN_COLOR   QColor(100,100,100)

/*******************************************************/
// memory operation define
#define NEW(ptr, type)							\
do {											\
		ptr = new type;							\
	} while (0)

/* Allocate count*sizeof(type) size of memory for "ptr".
 * Use "RELEASE_ARRAY" to release the memory.
 * Make sure "ptr == NULL" before allocating memory.
 */

#define NEW_ARRAY(ptr, type, count)				\
do {											\
		ptr = new type[count];					\
		if ((ptr) != NULL)						\
		{										\
			memset(ptr, 0, count*sizeof(type));	\
		}										\
	} while(0)


#define RELEASE(pptr)							\
do {											\
	if ((pptr) && *(pptr)) {					\
		delete (*(pptr));						\
		*(pptr) = NULL;							\
	}											\
} while (0)

/* Release (array) memory previously allocated to "*pptr".
 */
#define RELEASE_ARRAY(pptr)						\
do {											\
	if ((pptr) && *(pptr)) {					\
		delete [] *(pptr);						\
		*(pptr) = NULL;							\
	}											\
} while (0)

/*********************************************************************/


/*******************************************************************/

/**************************************************************************/
/*========================== Parse IMG type define======================= */

/******************************************************************
	Note: The agreed package of documents before the 768 bytes
		  of the file header information.
	Include: the first 200 bytes of storage applications, the
	      type and version number, and 568 each store to be
		  packaged into the file name, location and length of
		  information.
*******************************************************************/

//Define file head structure
#define IMG_VERSION_1		1
#define IMG_VERSION_2		2
#define CRC_CODE_LEN		2
#define VERSION_HEAD_LEN	200
#define FILEINFO_HEAD_LEN       568
//#define FILEINFO_HEAD_LEN_NEW   20000  //add by jie.li 2011-09-28 for efs config files
#define FILEINFO_HEAD_LEN_NEW   150000   //changed by jie.li 2011-11-29

#define IMG_VERSION_3           3           // 8200A
#define IMG_VERSION_4           4       //add by jie.li 2011-09-28
#define IMG_VERSION_5		5       //add by jie.li for MDM9x15

#define IMG_VERSION_6           6       //add by minghui.zhang for MDM9x25
#define IMG_VERSION_7           7       //add by minghui.zhang for M850
#define IMG_VERSION_8           8       //add by zhanghao for 9X30 2014-09-04
#define IMG_VERSION_9           9       //add by xiaohua.lan 2016.1.12
    #define OPEN_MULTI_CMD                                   0x1B

    /* List to use for which image to open for */
    typedef enum {

        /*	   Config Data					*/
        OPEN_MULTI_MODE_OEMSBL                  = 0x04,	 /* OEM 2ndary Boot Loader			  */
        OPEN_MULTI_MODE_AMSS                    = 0x05,	 /* AMSS modem executable			  */

        /* add by jianwen.he 2010.05.17 */
        OPEN_MULTI_MODE_CUSTOM                  = 0x0E,    /* Image for user defined partition  */

                                      } TDLMode;

//type define
enum APP_TYPE{
	APP_TYPE_FIRMWARE = 0x01,        //firmware
        APP_TYPE_DASHBOARD,             //dashboard
        APP_TYPE_NV,                    //nv
        APP_TYPE_SIMLOCK,               //simlock
	APP_TYPE_MAX
};

typedef struct _IMGVerS{
        uint16	  imgVer;                   //img version
}IMGVerS;

typedef struct _PacketHeadInfoS{
        APP_TYPE  appType;                  //app type£ºFirmware¡¢dashboard¡¢nv¡¢simlock..
        char	  appVer[32];               //app version
}PacketHeadInfoS;

//Define file store structure
typedef struct _FilePosInfoS{
	char      fileName[20];			//file name
        uint32	  beginPos;                     //begin position
        uint32	  fileLen;                      //file length
}FilePosInfoS;

//add by jie.li 2011-09-28 for efs config files
typedef struct _FilePosInfoNewS{
    char          fileName[70];			//file name
    uint32	  beginPos;                     //begin position
    uint32	  fileLen;                      //file length
}FilePosInfoNewS;
//end add


/******************************************************************
	Note£ºThe last two bytes of the packet CRC code
*******************************************************************/

//Define file head structure
typedef struct _PacketTailInfoS{
	uint16	  dwCRCCode;			 //CRC
}PacketTailInfoS;

typedef struct {
        uint32 exeOffset;
        uint32 exeLength;
        uint32 imgOffset;
        uint32 imgLength;
        uint32 picOffset;
        uint32 picLength;
        uint32 txtOffset;
        uint32 txtLength;
        uint32 iconOffset;
        uint32 iconLength;
        uint32 nvFileOffset;
        uint32 nvFileLength;
        uint32 chmFileOffset;
        uint32 chmFileLength;
#ifdef FEATHER_DASHBOARD_ONLY
        uint32 prgFileOffset;
        uint32 prgFileLength;
        uint32 partFileOffset;
        uint32 partFileLength;
#endif
#ifdef FEATHER_PATCH
        uint32 patchFileOffset;
        uint32 patchFileLength;
#endif
} TDataPosType;

/*====================================================================== */
/**************************************************************************/




// define download thread state
typedef struct _DLThreadState{
        uint32          uPort;
        PORT_STATE      pState;
}DLThreadState;

// For Rf Nv backup & restore
typedef struct
{
	uint16		id;
	uint16		len;
	uint8		data[128];
} TNviItemType;

typedef struct
{
	uint16		itemid;
	uint16		itemLen;
}TNvItemType;

//NV item packet struct
typedef struct _NV_ITEM_PACKET_INFO
{
        uint16		packetLen;              //Packet length
	uint16		packetReserve;		//reserve
        uint16		nvItemID;               //NV ID
        uint16		nvItemIndex;            //NV Index
	uint8		itemData[128];		//NV data
}NV_ITEM_PACKET_INFO;

#if 0
//NV item packet struct
typedef struct _NV_ITEM_PACKET_INFO{
	USHORT packetLen;			//Packet length, size of NV_ITEM_PACKET_INFO structure
	USHORT packetReserve;		//reserve
	USHORT nvItemID;			//NV ID
	USHORT nvItemIndex;			//NV Index
	BYTE   itemData[128];		//NV data
}NV_ITEM_PACKET_INFO;
#endif

typedef struct
{
        uint32          count;
        TNvItemType*    NvItem;
} TRfNvInfoType;

typedef struct
{
        uint32          count;
        uint8*          data;
}DASHBOARD_DATA,NV_PACKET_DATA,PERSO_TXT_DATA,CONFIG_XML_DATA;

// For customer backup & restore operation
typedef struct TCustDataInfoType_
{

TCustDataInfoType_ (void) {
  memset(&dashboardData, 0, sizeof dashboardData);
  memset(&nvPacketData, 0, sizeof nvPacketData);
  memset(&persoTxtData, 0, sizeof persoTxtData);
  memset(&configxmlData, 0, sizeof configxmlData);
}

	DASHBOARD_DATA    dashboardData;
	NV_PACKET_DATA	  nvPacketData;
	PERSO_TXT_DATA    persoTxtData;
	CONFIG_XML_DATA	  configxmlData;
} TCustDataInfoType;

typedef struct _FileBufStruct
{
        uint8*          strFileName;
	uint32		uFileLens;
	uint8*		strFileBuf;
        uint8           Area[20];
        bool            isDownload;
        uint8             comand;
       _FileBufStruct()
        {
            isDownload=true;
            memset(Area, 0x00, sizeof(Area));
            Area[0]  = OPEN_MULTI_CMD;
            Area[1]=OPEN_MULTI_MODE_CUSTOM;

        }
}FileBufStruct;

typedef struct tagImgLens
{
	uint32		uQcsblHdLen;
	uint32		uQcsblLen;
	uint32		uOemsblLen;
	uint32		uAmssImgLen;
	uint32		uDashboardImgLen;

}ImagLensStruct;

//add by jie.li 20120830 for Y580 to parse the webs_config
typedef struct
{
    char             webs_dev_name[20];
    char             webs_cust_model_name[20];
    char             webs_Ext_SSID[20];
} WebsXMLInfo;
//end add


typedef struct _XmlStruct
{
        string       key;
        string       value;

}XmlStruct;

typedef vector<XmlStruct> XMLInfoVector;

typedef vector<string> nvItemList;

#define XML_BUF_LEN		   5096

#endif          //DEFINE_H
