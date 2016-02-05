#ifndef __DIAGCMD_H__
#define __DIAGCMD_H__

#include "stdafx.h"
#include "pkt.h"

/*
 * Permitted operations.
 */
#define EFS2_DIAG_HELLO     0 /* Parameter negotiation packet               */
#define EFS2_DIAG_QUERY     1 /* Send information about EFS2 params         */
#define EFS2_DIAG_OPEN      2 /* Open a file                                */
#define EFS2_DIAG_CLOSE     3 /* Close a file                               */
#define EFS2_DIAG_READ      4 /* Read a file                                */
#define EFS2_DIAG_WRITE     5 /* Write a file                               */
#define EFS2_DIAG_SYMLINK   6 /* Create a symbolic link                     */
#define EFS2_DIAG_READLINK  7 /* Read a symbolic link                       */
#define EFS2_DIAG_UNLINK    8 /* Remove a symbolic link or file             */
#define EFS2_DIAG_MKDIR     9 /* Create a directory                         */
#define EFS2_DIAG_RMDIR    10 /* Remove a directory                         */
#define EFS2_DIAG_OPENDIR  11 /* Open a directory for reading               */
#define EFS2_DIAG_READDIR  12 /* Read a directory                           */
#define EFS2_DIAG_CLOSEDIR 13 /* Close an open directory                    */
#define EFS2_DIAG_RENAME   14 /* Rename a file or directory                 */
#define EFS2_DIAG_STAT     15 /* Obtain information about a named file      */
#define EFS2_DIAG_LSTAT    16 /* Obtain information about a symbolic link   */
#define EFS2_DIAG_FSTAT    17 /* Obtain information about a file descriptor */
#define EFS2_DIAG_CHMOD    18 /* Change file permissions                    */
#define EFS2_DIAG_STATFS   19 /* Obtain file system information             */
#define EFS2_DIAG_ACCESS   20 /* Check a named file for accessibility       */
#define EFS2_DIAG_NAND_DEV_INFO    21 /* Get NAND device info               */
#define EFS2_DIAG_FACT_IMAGE_START 22 /* Start data output for Factory Image*/
#define EFS2_DIAG_FACT_IMAGE_READ  23 /* Get data for Factory Image         */
#define EFS2_DIAG_FACT_IMAGE_END   24 /* End data output for Factory Image  */
#define EFS2_DIAG_PREP_FACT_IMAGE  25 /* Prepare file system for image dump */
#define EFS2_DIAG_PUT_DEPRECATED   26 /* Write an EFS item file             */
#define EFS2_DIAG_GET_DEPRECATED   27 /* Read an EFS item file              */
#define EFS2_DIAG_ERROR    28 /* Semd an EFS Error Packet back through DIAG */
#define EFS2_DIAG_EXTENDED_INFO 29 /* Get Extra information.                */
#define EFS2_DIAG_CHOWN         30 /* Change ownership                      */
#define EFS2_DIAG_BENCHMARK_START_TEST  31 /* Start Benchmark               */
#define EFS2_DIAG_BENCHMARK_GET_RESULTS 32 /* Get Benchmark Report          */
#define EFS2_DIAG_BENCHMARK_INIT        33 /* Init/Reset Benchmark          */
#define EFS2_DIAG_SET_RESERVATION       34 /* Set group reservation         */
#define EFS2_DIAG_SET_QUOTA             35 /* Set group quota               */
#define EFS2_DIAG_GET_GROUP_INFO        36 /* Retrieve Q&R values           */
#define EFS2_DIAG_DELTREE               37 /* Delete a Directory Tree       */
#define EFS2_DIAG_PUT                   38 /* Write a EFS item file in order*/
#define EFS2_DIAG_GET                   39 /* Read a EFS item file in order */
#define EFS2_DIAG_TRUNCATE              40 /* Truncate a file by the name   */
#define EFS2_DIAG_FTRUNCATE             41 /* Truncate a file by a descriptor */
#define EFS2_DIAG_STATVFS_V2            42 /* Obtains extensive file system info */

/*Add by jianwen.he 2010-03-10*/
#define SEND_RESTORE_CMD	 1  /* Send restore command to firmware*/
#define REQ_FIRMWARE_VER_N	 2  /* Request firmware version*/
#define SEND_SIM_LOCK		 3  /* Send sim lock command*/
#define REQ_FLASH_TYPE_N	 4  /* Request flash type command*/
/*end*/

/*Add by jianwen.he 2010-05-14*/
// The first parameter
// 0: Disable
// 1: Enable
// The second parameter
// 0: Temporary
// 1: Permanent
#define SEND_ENABLE_DIAG	 5  /* enable DIAG server */

/*end*/

/* Add by jianwen.he 2010-06-27*/
#define REQ_ROOT_VER		 6
/* end */

/*Add by jie.li 2011-07-04*/
#define SEND_OPER_DASHBOARD_FLAG	200
#define GET_FLAG_OPER				0
#define SET_FLAG_OPER				1
#define DASHBOARD_FLAG_OK			0
#define DASHBOARD_FLAG_FAIL			1
/*End.*/

#define READ_MAC_ADDRESS			11
#define READ_CUST_MODE_NAME			13
#define WRITE_SSID					12
#define READ_WIFI_PWD_FLAG			15
#define GENERATE_FT_FILES			14
#define EXTEND_DIAG_IMEI_READ       18

//add by jie.li for MDM9x15
#define GET_QCNVERSION				16		/* get the get QCNVERSION.*/
#define GET_DASHBOARD_VERSION		17		/* get the get DASHBOARD_VERSION.*/
#define GET_Q6RESOURCEVER			18		/* get the get Q6RESOURCEVER.*/
#define GET_LINUX_VERSION			19		/* get the linux version*/
#define STORE_ANIMAL_PIC			20		/* store the pic of starting device*/
#define RESTORE_FUNC_FIVE			23		/* set at+func=5,0*/
//end add


/*
 * Any error codes in fs_errno.h can be returned by the EFS2 Diag interface.
 * In addition, these additional diag-specific values are possible:
 *
 * Error codes generated by the EFS2 Diag interface. Note that we use values
 * about 0x40000000 to avoid clashing with the errno values used by EFS2.
 */
#define FS_DIAG_INCONSISTENT_STATE  0x40000001
#define FS_DIAG_INVALID_SEQ_NO      0x40000002
#define FS_DIAG_DIR_NOT_OPEN        0x40000003
#define FS_DIAG_DIRENT_NOT_FOUND    0x40000004
#define FS_DIAG_INVALID_PATH        0x40000005
#define FS_DIAG_PATH_TOO_LONG       0x40000006
#define FS_DIAG_TOO_MANY_OPEN_DIRS  0x40000007
#define FS_DIAG_INVALID_DIR_ENTRY   0x40000008
#define FS_DIAG_TOO_MANY_OPEN_FILES 0x40000009
#define FS_DIAG_UNKNOWN_FILETYPE    0x4000000a
#define FS_DIAG_NOT_NAND_FLASH      0x4000000b
#define FS_DIAG_UNAVAILABLE_INFO    0x4000000c


/* File Open Flag. */
#define O_ACCMODE          0003
#define O_NOCTTY           0400 /* not fcntl */
#define O_NONBLOCK        04000
#define O_NDELAY        O_NONBLOCK
#define O_SYNC           010000
#define FASYNC           020000 /* fcntl, for BSD compatibility */
#define O_DIRECT         040000 /* direct disk access hint */
#define O_LARGEFILE     0100000
#define O_DIRECTORY     0200000 /* must be a directory */
#define O_NOFOLLOW      0400000 /* don't follow links */
#define O_ITEMFILE     01000000 /* Create special ITEM file. */

#define EFS_IOFBF       0       /* Full buffering.    */
#define EFS_IOLBF       1       /* Line buffering.    */
#define EFS_IONBF       2       /* No buffering.      */

typedef enum {
  DIAG_SUBSYS_OEM				 = 0,		/* Reserved for OEM use */
  DIAG_SUBSYS_ZREX				 = 1,		/* ZREX */
  DIAG_SUBSYS_SD				 = 2,		/* System Determination */
  DIAG_SUBSYS_BT				 = 3,		/* Bluetooth */
  DIAG_SUBSYS_WCDMA 			 = 4,		/* WCDMA */
  DIAG_SUBSYS_HDR				 = 5,		/* 1xEvDO */
  DIAG_SUBSYS_DIABLO			 = 6,		/* DIABLO */
  DIAG_SUBSYS_TREX				 = 7,		/* TREX - Off-target testing environments */
  DIAG_SUBSYS_GSM				 = 8,		/* GSM */
  DIAG_SUBSYS_UMTS				 = 9,		/* UMTS */
  DIAG_SUBSYS_HWTC				 = 10,		/* HWTC */
  DIAG_SUBSYS_FTM				 = 11,		/* Factory Test Mode */
  DIAG_SUBSYS_REX				 = 12,		/* Rex */
  DIAG_SUBSYS_OS				 = DIAG_SUBSYS_REX,
  DIAG_SUBSYS_GPS				 = 13,		/* Global Positioning System */
  DIAG_SUBSYS_WMS				 = 14,		/* Wireless Messaging Service (WMS, SMS) */
  DIAG_SUBSYS_CM				 = 15,		/* Call Manager */
  DIAG_SUBSYS_HS				 = 16,		/* Handset */
  DIAG_SUBSYS_AUDIO_SETTINGS	 = 17,		/* Audio Settings */
  DIAG_SUBSYS_DIAG_SERV 		 = 18,		/* DIAG Services */
  DIAG_SUBSYS_FS				 = 19,		/* File System - EFS2 */
  DIAG_SUBSYS_PORT_MAP_SETTINGS  = 20,		/* Port Map Settings */
  DIAG_SUBSYS_MEDIAPLAYER		 = 21,		/* QCT Mediaplayer */
  DIAG_SUBSYS_QCAMERA			 = 22,		/* QCT QCamera */
  DIAG_SUBSYS_MOBIMON			 = 23,		/* QCT MobiMon */
  DIAG_SUBSYS_GUNIMON			 = 24,		/* QCT GuniMon */
  DIAG_SUBSYS_LSM				 = 25,		/* Location Services Manager */
  DIAG_SUBSYS_QCAMCORDER		 = 26,		/* QCT QCamcorder */
  DIAG_SUBSYS_MUX1X 			 = 27,		/* Multiplexer */
  DIAG_SUBSYS_DATA1X			 = 28,		/* Data */
  DIAG_SUBSYS_SRCH1X			 = 29,		/* Searcher */
  DIAG_SUBSYS_CALLP1X			 = 30,		/* Call Processor */
  DIAG_SUBSYS_APPS				 = 31,		/* Applications */
  DIAG_SUBSYS_SETTINGS			 = 32,		/* Settings */
  DIAG_SUBSYS_GSDI				 = 33,		/* Generic SIM Driver Interface */
  DIAG_SUBSYS_TMC				 = 34,		/* Task Main Controller */
  DIAG_SUBSYS_USB				 = 35,		/* Universal Serial Bus */
  DIAG_SUBSYS_PM				 = 36,		/* Power Management */
  DIAG_SUBSYS_DEBUG 			 = 37,
  DIAG_SUBSYS_QTV				 = 38,
  DIAG_SUBSYS_CLKRGM			 = 39,		/* Clock Regime */
  DIAG_SUBSYS_DEVICES			 = 40,
  DIAG_SUBSYS_WLAN				 = 41,		/* 802.11 Technology */
  DIAG_SUBSYS_PS_DATA_LOGGING	 = 42,		/* Data Path Logging */
  DIAG_SUBSYS_PS				 = DIAG_SUBSYS_PS_DATA_LOGGING,
  DIAG_SUBSYS_MFLO				 = 43,		/* MediaFLO */
  DIAG_SUBSYS_DTV				 = 44,		/* Digital TV */
  DIAG_SUBSYS_RRC				 = 45,		/* WCDMA Radio Resource Control state */
  DIAG_SUBSYS_PROF				 = 46,		/* Miscellaneous Profiling Related */
  DIAG_SUBSYS_TCXOMGR			 = 47,
  DIAG_SUBSYS_NV				 = 48,		/* Non Volatile Memory */
  DIAG_SUBSYS_AUTOCONFIG		 = 49,
  DIAG_SUBSYS_PARAMS			 = 50,		/* Parameters required for debugging subsystems */
  DIAG_SUBSYS_MDDI				 = 51,		/* Mobile Display Digital Interface */
  DIAG_SUBSYS_DS_ATCOP			 = 52,
  DIAG_SUBSYS_L4LINUX			 = 53,		/* L4/Linux */
  DIAG_SUBSYS_MVS				 = 54,		/* Multimode Voice Services */
  DIAG_SUBSYS_CNV				 = 55,		/* Compact NV */
  DIAG_SUBSYS_APIONE_PROGRAM	 = 56,		/* apiOne */
  DIAG_SUBSYS_HIT				 = 57,		/* Hardware Integration Test */
  DIAG_SUBSYS_DRM				 = 58,		/* Digital Rights Management */
  DIAG_SUBSYS_DM				 = 59,		/* Device Management */
  DIAG_SUBSYS_FC				 = 60,		/* Flow Controller */
  DIAG_SUBSYS_MEMORY			 = 61,		/* Malloc Manager */
  DIAG_SUBSYS_FS_ALTERNATE		 = 62,		/* Alternate File System */
  DIAG_SUBSYS_REGRESSION		 = 63,		/* Regression Test Commands */
  DIAG_SUBSYS_SENSORS			 = 64,		/* The sensors subsystem */
  DIAG_SUBSYS_FLUTE 			 = 65,		/* FLUTE */

  DIAG_SUBSYS_LAST,

  /* Subsystem IDs reserved for OEM use */
  DIAG_SUBSYS_RESERVED_OEM_0	 = 250,
  DIAG_SUBSYS_RESERVED_OEM_1	 = 251,
  DIAG_SUBSYS_RESERVED_OEM_2	 = 252,
  DIAG_SUBSYS_RESERVED_OEM_3	 = 253,
  DIAG_SUBSYS_RESERVED_OEM_4	 = 254,
  DIAG_SUBSYS_LEGACY			 = 255,
} diagpkt_subsys_cmd_enum_type;

typedef enum {
	NV_DONE_S,			/* Request completed okay */
	NV_BUSY_S,			/* Request is queued */
	NV_BADCMD_S,		/* Unrecognizable command field */
	NV_FULL_S,			/* The NVM is full */
	NV_FAIL_S,			/* Command failed, reason other than NVM was full */
	NV_NOTACTIVE_S, 	/* Variable was not active */
	NV_BADPARM_S,		/* Bad parameter in command block */
	NV_READONLY_S,		/* Parameter is write-protected and thus read only */
	NV_BADTG_S, 		/* Item not valid for Target */
	NV_NOMEM_S, 		/* free memory exhausted */
	NV_NOTALLOC_S,		/* address is not a valid allocation */
	NV_STAT_ENUM_PAD = 0x7FFF	  /* Pad to 16 bits on ARM */
} nv_stat_enum_type;
//add by jie.li for MDM9x15

typedef struct {
        uint8 cmd_code;
        uint8 subsys_id;                //File descriptor
        uint16 subsys_cmd_code;
} diagpkt_subsys_header_type;


typedef struct{
	diagpkt_subsys_header_type hdr;
	int32 version_len;
} extend_diag_get_version_req_type;

typedef struct{
	diagpkt_subsys_header_type hdr;
	int32 version_len;
        char version[32];        //version info
}extend_diag_get_version_rsp_type;

#define BUFF_LENGTH             32
typedef struct{
	//char requst[4];
	diagpkt_subsys_header_type hdr;
	int kernel_ver_num;
	char kernel_ver[BUFF_LENGTH];
	int system_ver_num;
	char system_ver[BUFF_LENGTH];
	int userdata_ver_num;
	char userdata_ver[BUFF_LENGTH];
}extend_get_ker_sys_ver_req_type, extend_get_ker_sys_ver_rsp_type;

typedef struct{
	diagpkt_subsys_header_type hdr;
	byte pic[2048];
}extend_diag_store_pic_req_type;

typedef struct{
	diagpkt_subsys_header_type hdr;
	byte value;
} extend_diag_factory_file_gen_req_type, extend_diag_set_func_five_req_type;

typedef struct{
	diagpkt_subsys_header_type hdr;
        int32 diag_errno;          // Error code if error, 0 otherwise
} extend_diag_factory_file_gen_rsp_type, extend_diag_set_func_five_rsp_type;

//end add
//-----------------------------------------------------------------------------
class CDIAGCmd
{
public:
	CDIAGCmd(CPacket* packetDll);
	~CDIAGCmd(){}

public:
	bool NV_Read_Item(uint16 item, uint8* data, uint32 len, uint16* status);
	bool NV_Read_Item_Array(uint16 item, uint8 index, uint8* data, uint32 len, uint16* status);
	bool NV_Write_Item(uint16 item, uint8* data, uint32 len, uint16* status);
	bool NV_Write_Item_Array(uint16 item, uint8 index, uint8* data, uint32 len, uint16* status);
	bool DLoadMode(void);
	bool WcdmaStatus(void);

	/* Efs operation*/
	TResult EfsOpHello(bool bArmEfs=false);
	TResult EfsOpStat(bool bArmEfs, const char* filename, int32& mode, int32& size);
	TResult EfsOpOpen(bool bArmEfs, const char* filename, int32 oflag, int32 mode, int32& fd);
	TResult EfsOpRead(bool bArmEfs, int32  fd, uint32 flen, uint8* fdata);
	TResult EfsOpWrite(bool bArmEfs, int32  fd, uint8* pdata, uint32& offset, uint32 len);
	TResult EfsOpClose(bool bArmEfs, int32 fd);

	/*Add by jianwen.he 2010-02-25*/
	TResult EfsDelFile(bool bArmEfs, const char* filename);
	TResult SendRestoreCmd(void);
	TResult RequestFirmwareVer_N(char* fdata);
	TResult RequestFlashType_N(uint8& type);
	TResult VerifySPC(const char* strSPC);
	bool SwitchToOfflineMode(uint16 wMode);
	/*end*/

	//add by jie.li for MDM9x15
	TResult RequestDashboardVer(char* fdata);
	TResult RequestQCNVer(char* fdata);
	TResult RequestQ6resVer(char* fdata);
	TResult RequestLinuxVer(char* fdata);
	TResult StorePIC(uint8* data, uint32 len);
	//end add
	/* Add by jianwen.he 2010-05-14*/
	TResult EnableDiagServer();
	TResult DisableDiagServer();
	/*end*/

	/* Add by jianwen.he 2010-06-27*/
	TResult GetRootVer_N(uint8& ver);
	/**/

	/* Add by jianwen.he 2010-07-06*/
	/* Restart device in normal mode*/
	bool RestartDevice();
	/* end */

    /* Add by jie.li 2011-07-04*/
    TResult OperDashboardFlag(int operType, int& flag, uint8& retCode);
    /* end.*/

    /* Add by jie.li 2011-09-28*/
    TResult EfsOpMKDir(bool bArmEfs, const char* filename);     //create the path
    int32   EfsOpOpenDir(bool bArmEfs, const char* filename);	//open the path
    TResult SendRawRequest(int& retType);
    /* end.*/

    //modify by yanbin.wan for redMacAddress 0:MAC_F4 1:MAC_L4 2:MAC_L6
    TResult ReadMacAddress(string & strMacAddress, int iType);
    //TResult ReadMacAddress(QString & strMacAddress,bool bLast);
    TResult ReadCustModeName(string & strModeName);
    TResult WriteSSID(string strSSID);
    TResult ReadWifiPwdFlag(int &retFlagVal);
    TResult GenerateFTFiles();

    //add by jie.li 2012-06-27
    int32   EfsOpReadDelDir(bool bArmEfs, const char* filename);
    TResult EfsOpCloseDir(bool bArmEfs, const char* filename, int32 dirp);		//close the path
    TResult EfsOpRMDir(bool bArmEfs, const char* filename);		//remove the path
    //end add

    //add for MDM9x15
    TResult SetFuncFive(byte valueFuncFive);
    TResult GenerateFTFilesNew(byte valueForce);
    //end add


private:
	TResult EfsOperate(uint32 len, uint8* INdata, uint32* outLen, uint8* OUTdata);

	TResult SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen);

private:
	CPacket* m_packetDll;
	cmd_buffer_s_type cmd;
	rsp_buffer_s_type rsp;

};

#endif //__DIAGCMD_H__
