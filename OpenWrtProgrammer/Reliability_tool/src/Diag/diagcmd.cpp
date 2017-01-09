#include "../log/log.h"
#include "diagcmd.h"

//-----------------------------------------------------------------------------
#define MAX_RESEND (2)

/*--------------------------------------------------------------------------

  Command Codes between the Diagnostic Monitor and the mobile. Packets
  travelling in each direction are defined here, while the packet templates
  for requests and responses are distinct.	Note that the same packet id
  value can be used for both a request and a response.	These values
  are used to index a dispatch table in diag.c, so

  DON'T CHANGE THE NUMBERS ( REPLACE UNUSED IDS WITH FILLERS ). NEW IDs
  MUST BE ASSIGNED AT THE END.

----------------------------------------------------------------------------*/

/* Version Number Request/Response			  */
#define DIAG_VERNO_F	0

/* Mobile Station ESN Request/Response		  */
#define DIAG_ESN_F		1

/* Peek byte Request/Response				  */
#define DIAG_PEEKB_F	2

/* Peek word Request/Response				  */
#define DIAG_PEEKW_F	3

/* Peek dword Request/Response				  */
#define DIAG_PEEKD_F	4

/* Poke byte Request/Response				  */
#define DIAG_POKEB_F	5

/* Poke word Request/Response				  */
#define DIAG_POKEW_F	6

/* Poke dword Request/Response				  */
#define DIAG_POKED_F	7

/* Byte output Request/Response 			  */
#define DIAG_OUTP_F 	8

/* Word output Request/Response 			  */
#define DIAG_OUTPW_F	9

/* Byte input Request/Response				  */
#define DIAG_INP_F		10

/* Word input Request/Response				  */
#define DIAG_INPW_F 	11

/* DMSS status Request/Response 			  */
#define DIAG_STATUS_F	12

/* 13-14 Reserved */

/* Set logging mask Request/Response		  */
#define DIAG_LOGMASK_F	15

/* Log packet Request/Response				  */
#define DIAG_LOG_F		16

/* Peek at NV memory Request/Response		  */
#define DIAG_NV_PEEK_F	17

/* Poke at NV memory Request/Response		  */
#define DIAG_NV_POKE_F	18

/* Invalid Command Response 				  */
#define DIAG_BAD_CMD_F	19

/* Invalid parmaeter Response				  */
#define DIAG_BAD_PARM_F 20

/* Invalid packet length Response			  */
#define DIAG_BAD_LEN_F	21

/* 22-23 Reserved */

/* Packet not allowed in this mode
   ( online vs offline )					  */
#define DIAG_BAD_MODE_F 	24

/* info for TA power and voice graphs		  */
#define DIAG_TAGRAPH_F		25

/* Markov statistics						  */
#define DIAG_MARKOV_F		26

/* Reset of Markov statistics				  */
#define DIAG_MARKOV_RESET_F 27

/* Return diag version for comparison to
   detect incompatabilities 				  */
#define DIAG_DIAG_VER_F 	28

/* Return a timestamp						  */
#define DIAG_TS_F			29

/* Set TA parameters						  */
#define DIAG_TA_PARM_F		30

/* Request for msg report					  */
#define DIAG_MSG_F			31

/* Handset Emulation -- keypress			  */
#define DIAG_HS_KEY_F		32

/* Handset Emulation -- lock or unlock		  */
#define DIAG_HS_LOCK_F		33

/* Handset Emulation -- display request 	  */
#define DIAG_HS_SCREEN_F	34

/* 35 Reserved */

/* Parameter Download						  */
#define DIAG_PARM_SET_F 	36

/* 37 Reserved */

/* Read NV item 							  */
#define DIAG_NV_READ_F	38
/* Write NV item							  */
#define DIAG_NV_WRITE_F 39
/* 40 Reserved */

/* Mode change request						  */
#define DIAG_CONTROL_F	  41

/* Error record retreival					  */
#define DIAG_ERR_READ_F   42

/* Error record clear						  */
#define DIAG_ERR_CLEAR_F  43

/* Symbol error rate counter reset			  */
#define DIAG_SER_RESET_F  44

/* Symbol error rate counter report 		  */
#define DIAG_SER_REPORT_F 45

/* Run a specified test 					  */
#define DIAG_TEST_F 	  46

/* Retreive the current dip switch setting	  */
#define DIAG_GET_DIPSW_F  47

/* Write new dip switch setting 			  */
#define DIAG_SET_DIPSW_F  48

/* Start/Stop Vocoder PCM loopback			  */
#define DIAG_VOC_PCM_LB_F 49

/* Start/Stop Vocoder PKT loopback			  */
#define DIAG_VOC_PKT_LB_F 50

/* 51-52 Reserved */

/* Originate a call 						  */
#define DIAG_ORIG_F 53
/* End a call								  */
#define DIAG_END_F	54
/* 55-57 Reserved */

/* Switch to downloader 					  */
#define DIAG_DLOAD_F 58
/* Test Mode Commands and FTM commands		  */
#define DIAG_TMOB_F  59
/* Test Mode Commands and FTM commands		  */
#define DIAG_FTM_CMD_F	59
/* 60-62 Reserved */

#ifdef FEATURE_HWTC
#define DIAG_TEST_STATE_F 61
#endif /* FEATURE_HWTC */

/* Return the current state of the phone	  */
#define DIAG_STATE_F		63

/* Return all current sets of pilots		  */
#define DIAG_PILOT_SETS_F	64

/* Send the Service Prog. Code to allow SP	  */
#define DIAG_SPC_F			65

/* Invalid nv_read/write because SP is locked */
#define DIAG_BAD_SPC_MODE_F 66

/* get parms obsoletes PARM_GET 			  */
#define DIAG_PARM_GET2_F	67

/* Serial mode change Request/Response		  */
#define DIAG_SERIAL_CHG_F	68

/* 69 Reserved */

/* Send password to unlock secure operations
   the phone to be in a security state that
   is wasn't - like unlocked.				  */
#define DIAG_PASSWORD_F 	70

/* An operation was attempted which required  */
#define DIAG_BAD_SEC_MODE_F 71

/* Write Preferred Roaming list to the phone. */
#define DIAG_PR_LIST_WR_F	72

/* Read Preferred Roaming list from the phone.*/
#define DIAG_PR_LIST_RD_F	73

/* 74 Reserved */

/* Subssytem dispatcher (extended diag cmd)   */
#define DIAG_SUBSYS_CMD_F	75

/* 76-80 Reserved */

/* Asks the phone what it supports			  */
#define DIAG_FEATURE_QUERY_F   81

/* 82 Reserved */

/* Read SMS message out of NV				  */
#define DIAG_SMS_READ_F 	   83

/* Write SMS message into NV				  */
#define DIAG_SMS_WRITE_F	   84

/* info for Frame Error Rate
   on multiple channels 					  */
#define DIAG_SUP_FER_F		   85

/* Supplemental channel walsh codes 		  */
#define DIAG_SUP_WALSH_CODES_F 86

/* Sets the maximum # supplemental
   channels 								  */
#define DIAG_SET_MAX_SUP_CH_F  87

/* get parms including SUPP and MUX2:
   obsoletes PARM_GET and PARM_GET_2		  */
#define DIAG_PARM_GET_IS95B_F  88

/* Performs an Embedded File System
   (EFS) operation. 						  */
#define DIAG_FS_OP_F		   89

/* AKEY Verification.						  */
#define DIAG_AKEY_VERIFY_F	   90

/* Handset emulation - Bitmap screen		  */
#define DIAG_BMP_HS_SCREEN_F   91

/* Configure communications 				  */
#define DIAG_CONFIG_COMM_F		  92

/* Extended logmask for > 32 bits.			  */
#define DIAG_EXT_LOGMASK_F		  93

/* 94-95 reserved */

/* Static Event reporting.					  */
#define DIAG_EVENT_REPORT_F 	  96

/* Load balancing and more! 				  */
#define DIAG_STREAMING_CONFIG_F   97

/* Parameter retrieval						  */
#define DIAG_PARM_RETRIEVE_F	  98

 /* A state/status snapshot of the DMSS.	  */
#define DIAG_STATUS_SNAPSHOT_F	  99

/* Used for RPC 							  */
#define DIAG_RPC_F				 100

/* Get_property requests					  */
#define DIAG_GET_PROPERTY_F 	 101

/* Put_property requests					  */
#define DIAG_PUT_PROPERTY_F 	 102

/* Get_guid requests						  */
#define DIAG_GET_GUID_F 		 103

/* Invocation of user callbacks 			  */
#define DIAG_USER_CMD_F 		 104

/* Get permanent properties 				  */
#define DIAG_GET_PERM_PROPERTY_F 105

/* Put permanent properties 				  */
#define DIAG_PUT_PERM_PROPERTY_F 106

/* Permanent user callbacks 				  */
#define DIAG_PERM_USER_CMD_F	 107

/* GPS Session Control						  */
#define DIAG_GPS_SESS_CTRL_F	 108

/* GPS search grid							  */
#define DIAG_GPS_GRID_F 		 109

/* GPS Statistics							  */
#define DIAG_GPS_STATISTICS_F	 110

/* Packet routing for multiple instances of diag */
#define DIAG_ROUTE_F			 111

/* IS2000 status							  */
#define DIAG_IS2000_STATUS_F	 112

/* RLP statistics reset 					  */
#define DIAG_RLP_STAT_RESET_F	 113

/* (S)TDSO statistics reset 				  */
#define DIAG_TDSO_STAT_RESET_F	 114

/* Logging configuration packet 			  */
#define DIAG_LOG_CONFIG_F		 115

/* Static Trace Event reporting */
#define DIAG_TRACE_EVENT_REPORT_F 116

/* SBI Read */
#define DIAG_SBI_READ_F 		  117

/* SBI Write */
#define DIAG_SBI_WRITE_F		  118

/* SSD Verify */
#define DIAG_SSD_VERIFY_F		  119

/* Log on Request */
#define DIAG_LOG_ON_DEMAND_F	  120

/* Request for extended msg report */
#define DIAG_EXT_MSG_F			  121

/* ONCRPC diag packet */
#define DIAG_ONCRPC_F			  122

/* Diagnostics protocol loopback. */
#define DIAG_PROTOCOL_LOOPBACK_F  123

/* Extended build ID text */
#define DIAG_EXT_BUILD_ID_F 	  124

/* Request for extended msg report */
#define DIAG_EXT_MSG_CONFIG_F	  125

/* Extended messages in terse format */
#define DIAG_EXT_MSG_TERSE_F	  126

/* Translate terse format message identifier */
#define DIAG_EXT_MSG_TERSE_XLATE_F 127

/* Subssytem dispatcher Version 2 (delayed response capable) */
#define DIAG_SUBSYS_CMD_VER_2_F    128

/* Get the event mask */
#define DIAG_EVENT_MASK_GET_F	   129

/* Set the event mask */
#define DIAG_EVENT_MASK_SET_F	   130

/* RESERVED CODES: 131-139 */

/* Command Code for Changing Port Settings. */
#define DIAG_CHANGE_PORT_SETTINGS  140

/* Number of packets defined. */
#define DIAG_MAX_F				   140

/* Subsys code for WCDMA */
#define WCDMA_STATUS_F				   14



//-----------------------------------------------------------------------------

/*typedef struct {
  uint8 cmd_code;
  uint8 subsys_id;                //* File descriptor
  uint16 subsys_cmd_code;  
} diagpkt_subsys_header_type;*/

/*
 * Default window sizes. Set to large numbers because the target can support
 * essentially unlimited windowing without doing any extra work.
 */
#define FS_TARG_PKT_WINDOW_DEFAULT  0x100000
#define FS_TARG_BYTE_WINDOW_DEFAULT 0x100000
#define FS_HOST_PKT_WINDOW_DEFAULT  0x100000
#define FS_HOST_BYTE_WINDOW_DEFAULT 0x100000
#define FS_ITER_PKT_WINDOW_DEFAULT  0x100000
#define FS_ITER_BYTE_WINDOW_DEFAULT 0x100000

/*
 * Protocol version information.
 */
#define FS_DIAG_VERSION     0x0001
#define FS_DIAG_MIN_VERSION 0x0001
#define FS_DIAG_MAX_VERSION 0x0001

/*
 * Feature Bits.
 */
#define FS_FEATURE_BITS 0x00000000

//modify by yanbin.wan 20130708 from 60 to 128
#define DIAG_FS_MAX_FILENAME_LEN       128
#define DIAG_FS_BUFFER_SIZE            1024

typedef  struct {
  diagpkt_subsys_header_type hdr;
  uint32 targ_pkt_window;  /* Window size in packets for sends from phone  */
  uint32 targ_byte_window; /* Window size in bytes for sends from phone    */
  uint32 host_pkt_window;  /* Window size in packets for sends from host   */
  uint32 host_byte_window; /* Window size in bytes for sends from host     */
  uint32 iter_pkt_window;  /* Window size in packets for dir iteration     */
  uint32 iter_byte_window; /* Window size in bytes for dir iteration       */
  uint32 version;          /* Protocol version number                      */
  uint32 min_version;      /* Smallest supported protocol version number   */
  uint32 max_version;      /* Highest supported protocol version number    */
  uint32 feature_bits;     /* Feature bit mask; one bit per feature        */
} efs2_diag_hello_req_type;

typedef struct {
  diagpkt_subsys_header_type hdr;
  uint32 targ_pkt_window;  /* Window size in packets for sends from phone  */
  uint32 targ_byte_window; /* Window size in bytes for sends from phone    */
  uint32 host_pkt_window;  /* Window size in packets for sends from host   */
  uint32 host_byte_window; /* Window size in bytes for sends from host     */
  uint32 iter_pkt_window;  /* Window size in packets for dir iteration     */
  uint32 iter_byte_window; /* Window size in bytes for dir iteration       */
  uint32 version;          /* Protocol version number                      */
  uint32 min_version;      /* Smallest supported protocol version number   */
  uint32 max_version;      /* Highest supported protocol version number    */
  uint32 feature_bits;     /* Feature bit mask; one bit per feature        */
} efs2_diag_hello_rsp_type;

typedef struct {
  uint32  targ_pkt_window;       /* Target window size in packets            */
  uint32  targ_byte_window;      /* Target window size in bytes              */
  uint32  host_pkt_window;       /* Host window size in packets              */
  uint32  host_byte_window;      /* Host window size in bytes                */
  uint32  iter_pkt_window;       /* Dir iteration window size in packets     */
  uint32  iter_byte_window;      /* Dir iteration window size in bytes       */
  uint32  version;               /* Protocol version number                  */
  uint32  min_version;           /* Minimum supported protocol version       */
  uint32  max_version;           /* Maximum supported protocol version       */
  uint32  feature_bits;          /* Bit mask of supported features           */
} efs2_diag_params_type;

const efs2_diag_params_type efs2_diag_default_params = {
  FS_TARG_PKT_WINDOW_DEFAULT,
  FS_TARG_BYTE_WINDOW_DEFAULT,
  FS_HOST_PKT_WINDOW_DEFAULT,
  FS_HOST_BYTE_WINDOW_DEFAULT,
  FS_ITER_PKT_WINDOW_DEFAULT,
  FS_ITER_BYTE_WINDOW_DEFAULT,
  FS_DIAG_VERSION,
  FS_DIAG_MIN_VERSION,
  FS_DIAG_MAX_VERSION,
  FS_FEATURE_BITS
};


typedef  struct {
  diagpkt_subsys_header_type 	hdr;
  /* Pathname (null-terminated string) */
  char							path[DIAG_FS_MAX_FILENAME_LEN]; 
} efs2_diag_stat_req_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 diag_errno;        /* Error code if error, 0 otherwise             */
  int32 mode;              /* File mode                                    */
  int32 size;              /* File size in bytes                           */
  int32 nlink;             /* Number of links                              */
  int32 atime;             /* Time of last access                          */
  int32 mtime;             /* Time of last modification                    */
  int32 ctime;             /* Time of last status change                   */
} efs2_diag_stat_rsp_type;


typedef struct {
	diagpkt_subsys_header_type hdr;
	/* Pathname (null-terminated string)  */
	char  path[DIAG_FS_MAX_FILENAME_LEN];           
} efs2_diag_unlink_req_type;

typedef struct {
	diagpkt_subsys_header_type hdr;
	int32 diag_errno;        /* Error code if error, 0 otherwise  */
} efs2_diag_unlink_rsp_type;

typedef struct {
	uint8 cmd_code;
} efs2_diag_get_version_req_type;

typedef struct
{
	byte cmd_code;
	byte msm_hw_version_format;
	byte reserved[2]; /* for alignment / future use */
	uint32 msm_hw_version;
	uint32 mobile_model_id;
	/* The following character array contains 2 NULL terminated strings:
	'build_id' string, followed by 'model_string' */
	char ver_strings[32];
} efs2_diag_get_version_rsp_type;


typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 oflag;             /* Open flags                                   */
  int32 mode;              /* Mode                                         */
  char  path[DIAG_FS_MAX_FILENAME_LEN];           /* Pathname (null-terminated string)            */
} efs2_diag_open_req_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 fd;                /* File descriptor if successful, -1 otherwise  */
  int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_open_rsp_type;


typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32  fd;               /* File descriptor                              */
  uint32 nbyte;            /* Number of bytes to read                      */
  uint32 offset;           /* Offset in bytes from the origin              */
} efs2_diag_read_req_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32  fd;               /* File descriptor                              */
  uint32 offset;           /* Requested offset in bytes from the origin    */
  int32  bytes_read;       /* bytes read if successful, -1 otherwise       */
  int32  diag_errno;       /* Error code if error, 0 otherwise             */
  char   data[DIAG_FS_BUFFER_SIZE+4];          /* The data read out                            */
} efs2_diag_read_rsp_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 fd;                /* File descriptor                              */
  uint32 offset;           /* Offset in bytes from the origin              */
  char  data[DIAG_FS_BUFFER_SIZE + 4];           /* The data to be written                       */
} efs2_diag_write_req_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 fd;                /* File descriptor                              */
  uint32 offset;           /* Requested offset in bytes from the origin    */
  int32 bytes_written;     /* The number of bytes written                  */
  int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_write_rsp_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 fd;                /* File descriptor                              */
} efs2_diag_close_req_type;

typedef  struct {
  diagpkt_subsys_header_type hdr;
  int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_close_rsp_type;

//add by jie.li 2011-09-28
typedef struct{
    diagpkt_subsys_header_type hdr;
    char  path[DIAG_FS_MAX_FILENAME_LEN];           /* Pathname (null-terminated string)            */
} efs2_diag_opendir_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    uint32 dirp;             /* Directory pointer. NULL if error             */
    int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_opendir_rsp_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int16 mode;              /* The creation mode                            */
    char  path[DIAG_FS_MAX_FILENAME_LEN];           /* Pathname (null-terminated string)            */
} efs2_diag_mkdir_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_mkdir_rsp_type;
//end add

//add by jie.li 2012-06-27
typedef struct{
    diagpkt_subsys_header_type hdr;
    uint32 dirp;             /* Directory pointer.                           */
    int32 seqno;             /* Sequence number of directory entry to read   */
} efs2_diag_readdir_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    uint32 dirp;             /* Directory pointer.                           */
    int32  seqno;            /* Sequence number of directory entry           */
    int32  diag_errno;       /* Error code if error, 0 otherwise             */
    int32  entry_type;       /* 0 if file, 1 if directory, 2 if symlink      */
    int32  mode;             /* File mode                                    */
    int32  size;             /* File size in bytes                           */
    int32  atime;            /* Time of last access                          */
    int32  mtime;            /* Time of last modification                    */
    int32  ctime;            /* Time of last status change                   */
    char   entry_name[DIAG_FS_MAX_FILENAME_LEN];    /* Name of directory entry (not full pathname)  */
} efs2_diag_readdir_rsp_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    char  path[DIAG_FS_MAX_FILENAME_LEN];           /* Pathname (null-terminated string)            */
} efs2_diag_rmdir_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_rmdir_rsp_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 dirp;              /* Directory pointer.                           */
} efs2_diag_closedir_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 diag_errno;        /* Error code if error, 0 otherwise             */
} efs2_diag_closedir_rsp_type;
//end add

//add by jie.li for MDM9x15
/*typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 version_len;
} extend_diag_get_version_req_type;

typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 version_len;
    char version[32];        ///*version info
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
}extend_diag_store_pic_req_type;*/
//end add

//#ifdef FEATHER_CUSTOM_IDEA
typedef struct{
    diagpkt_subsys_header_type hdr;
    int32 diag_error;/* Error code if error, 0 otherwise */
    uint8 imei[8]; /*Upgrade tools shouldn?¡¥t convert this field to any other formats*/
}extend_diag_read_imei_req_type, extend_diag_read_imei_rsp_type;

struct ImeiNum{
    QString strNum;
};

typedef struct{
      diagpkt_subsys_header_type hdr;
      char action;             /* Open flags */
      char mode;              /* Mode       */
      uint16 reserved;
      int32 diag_error;
      char active_code[12]; /*Only need 10 Bytes actually, 12 just for aligning*/
}extend_diag_open_rsp_type;

typedef struct{
      diagpkt_subsys_header_type hdr;
      char action;             /* Open flags */
      char mode;              /* Mode       */
      uint16 reserved;
      int32 diag_error;
      char active_code[12];
}extend_diag_open_req_type;
//#endif //FEATHER_CUSTOM_IDEA

//-----------------------------------------------------------------------------


/* Though the cmd_buffer_s_type has a MAX_CMD_BUFFER_LEN size of pool,
 * as to diagnostic command, its maximum length is ?
 * Refer to Qualcomm spec 80-V1294-1_YG_DMSS_Serial_Data_ICD.pdf
*/
//#define MAX_DIAG_CMD_LEN 128

#define NV_ITEM_DATA_LEN 128
#define NV_ITEM_NON_DATA_LEN 5 //cmd_code_len (1) + nv_item_len (2) + status_len (2)

#define DECLARE_CMD_PTR(cmd_ptr) \
		cmd_buffer_s_type* cmd_ptr = &this->cmd

#define DECLARE_RSP_PTR(rsp_ptr) \
		rsp_buffer_s_type* rsp_ptr = &this->rsp

#define INITIATING_CMD(cmd_ptr) \
		initiating_diag_cmd(cmd_ptr)

#define START_BUILDING_CMD(cmd_code) \
		start_building_diag_cmd(cmd_ptr, cmd_code)

#define START_BUILDING_SUBSYS_CMD(cmd_code, subsys_id, subsys_cmd_code) \
		start_building_extend_diag_cmd(cmd_ptr, cmd_code, subsys_id, subsys_cmd_code)

#define START_BUILDING_RSP() \
		start_building_diag_rsp(rsp_ptr)

static void initiating_diag_cmd
(
	  cmd_buffer_s_type* cmd_ptr
)
{
	//ASSERT(cmd_ptr != NULL);
	memset(cmd_ptr->buf, 0, sizeof(cmd_ptr->buf));
	cmd_ptr->type   = CMD_TYPE_DIAG;
	cmd_ptr->broken = false;
	cmd_ptr->length = 0;
}

static void start_building_diag_cmd
(
	  cmd_buffer_s_type* cmd_ptr,
	 uint8 cmd_code
)
{
	//ASSERT(cmd_ptr != NULL);
	memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
	cmd_ptr->type   = CMD_TYPE_DIAG;
	cmd_ptr->broken = false;
	cmd_ptr->buf[0] = cmd_code;
	cmd_ptr->length = 1;
}

static void start_building_extend_diag_cmd
(
	cmd_buffer_s_type* cmd_ptr,
	uint8 cmd_code,
	uint8 subsys_id,
	uint16 subsys_cmd_code
)
{
	//ASSERT(cmd_ptr != NULL);
	memset(cmd_ptr->buf, 0x00, sizeof(cmd_ptr->buf));
	cmd_ptr->type = CMD_TYPE_DIAG;
	cmd_ptr->broken = false;
	cmd_ptr->length = 0;
	cmd_ptr->buf[cmd_ptr->length++] = cmd_code;
	cmd_ptr->buf[cmd_ptr->length++] = subsys_id;

	if (subsys_id == DIAG_SUBSYS_FS || 
		subsys_id == DIAG_SUBSYS_WCDMA) 
	{
		uint8* buf = (uint8*)&subsys_cmd_code;
		cmd_ptr->buf[cmd_ptr->length++] = buf[0];
		cmd_ptr->buf[cmd_ptr->length++] = buf[1];
	}
}

static void start_building_diag_rsp
(
	  rsp_buffer_s_type* rsp_ptr
)
{
	//ASSERT(rsp_ptr != NULL);
	memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
	rsp_ptr->type   = CMD_TYPE_DIAG;
	rsp_ptr->broken = false;
	/* length might need to be reset according to specific cmd */
	rsp_ptr->length = MAX_RSP_BUFFER_LEN;
}

static bool check_diag_cmd_ok
(
	cmd_buffer_s_type* cmd_ptr
)
{
	if (cmd_ptr == NULL) 
	{
		return false;
	}

	if (cmd_ptr->broken == true) 
	{
		ERROR(FILE_LINE, "check_diag_cmd_ok, cmd_ptr is broken!");
		return false;
	}

	if (cmd_ptr->length >= MAX_CMD_BUFFER_LEN) 
	{
		ERROR(FILE_LINE, "check_diag_cmd_ok, cmd_ptr is broken!");
		cmd_ptr->broken = true;
		return false;
	}

	return true;
}

static bool check_diag_rsp_ok
(
	 cmd_buffer_s_type* cmd_ptr,
	 rsp_buffer_s_type* rsp_ptr
)
{
	if ((cmd_ptr->buf[0] != rsp_ptr->buf[0])) 
	{
//		ERROR(FILE_LINE, tr("check_diag_rsp_ok failure, cmd_code = %d, rsp_code = %d!")\
//			  .arg(cmd_ptr->buf[0]).arg(rsp_ptr->buf[0]))
			
		return false;
	}

	//if ((rsp_ptr->length <= NV_ITEM_NON_DATA_LEN)
	//	|| rsp_ptr->length > NV_ITEM_NON_DATA_LEN + NV_ITEM_DATA_LEN)
	//{
	//	return false;
	//}

	return true;
}


static bool check_nv_status_ok(uint16 status)
{
	if (status > NV_NOTALLOC_S) 
	{
//		ERROR(FILE_LINE, "check_nv_status_ok failure, status = %d", status);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------

CDIAGCmd::CDIAGCmd(CPacket& packetDll)
{
	m_packetDll = &packetDll;
}

bool CDIAGCmd::NV_Read_Item_Array
(
	  uint16 	item,
	  uint8   index,
	 uint8* 	data,
	  uint32 	len,
	 uint16* status
)
{
	TResult result = EOK;
	uint32  rlen;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	if ((data == NULL) || (status == NULL)) 
	{
		ERROR(FILE_LINE, "NV_Read_Item, Invalid params!!!");
		return false;
	}

	*status = 0;

	/* build a diag cmd filled in with basic info */
	START_BUILDING_CMD(DIAG_NV_READ_F);

	/* fill in item id field */
	uint8*  buf = (uint8*)&item;
	memcpy(&cmd_ptr->buf[cmd_ptr->length], buf, sizeof(item));
	cmd_ptr->length += sizeof(item);

	cmd_ptr->buf[cmd_ptr->length++] = index;

	// make room for data field
	cmd_ptr->length += len;

	/* fill in status field */
	memcpy(&cmd_ptr->buf[cmd_ptr->length], status, sizeof(*status));
	cmd_ptr->length += sizeof(*status);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "NV_Read_Item, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item SendCmd failure, result = %d", result);
		return false;
	}

	START_BUILDING_RSP();
	/* NV read rsp length is the same with read cmd, refer to spec */
	rsp_ptr->length = cmd_ptr->length;	
	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item Receive failure, result = %d", result);
		return false;
	}

	if (rsp_ptr->length != rlen) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item failure, rsp_ptr->length = %d, rlen = %d!",
//			rsp_ptr->length, rlen);
		return false;
	}

	/* check if the rsp data is ok, such as rsp code matches
	 * with cmd code.
	 */
	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	/* copy out nv item data */
	memcpy(data, rsp_ptr->buf + 3, rsp_ptr->length - NV_ITEM_NON_DATA_LEN);

	/* get nv read status */
	*status = (uint16)*(rsp_ptr->buf + 3 + NV_ITEM_DATA_LEN);

	if (!check_nv_status_ok(*status)) 
	{
		return false;
	}

	return true;
}

bool CDIAGCmd::NV_Read_Item
(
	  uint16 	item,
	 uint8* 	data,
	  uint32 	len,
	 uint16* status
)
{
	TResult result = EOK;
	uint32  rlen;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	if ((data == NULL) || (status == NULL)) 
	{
		ERROR(FILE_LINE, "NV_Read_Item, Invalid params!!!");
		return false;
	}

	*status = 0;

	/* build a diag cmd filled in with basic info */
	START_BUILDING_CMD(DIAG_NV_READ_F);

	/* fill in item id field */
	uint8*  buf = (uint8*)&item;
	memcpy(&cmd_ptr->buf[cmd_ptr->length], buf, sizeof(item));
	cmd_ptr->length += sizeof(item);

	// make room for data field
	cmd_ptr->length += len;

	/* fill in status field */
	memcpy(&cmd_ptr->buf[cmd_ptr->length], status, sizeof(*status));
	cmd_ptr->length += sizeof(*status);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "NV_Read_Item, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item SendCmd failure, result = %d", result);
		return false;
	}

	START_BUILDING_RSP();
	/* NV read rsp length is the same with read cmd, refer to spec */
	rsp_ptr->length = cmd_ptr->length;	
	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item Receive failure, result = %d", result);
		return false;
	}

	if (rsp_ptr->length != rlen) 
	{
//		ERROR(FILE_LINE, "NV_Read_Item failure, rsp_ptr->length = %d, rlen = %d!",
//			rsp_ptr->length, rlen);
		return false;
	}

	/* check if the rsp data is ok, such as rsp code matches
	 * with cmd code.
	 */
	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	/* copy out nv item data */
	memcpy(data, rsp_ptr->buf + 3, rsp_ptr->length - NV_ITEM_NON_DATA_LEN);

	/* get nv read status */
	*status = (uint16)*(rsp_ptr->buf + 3 + NV_ITEM_DATA_LEN);

	if (!check_nv_status_ok(*status)) 
	{
		return false;
	}

	return true;
}

bool CDIAGCmd::NV_Write_Item_Array
(
	uint16  item,
	uint8   index,
	uint8*  data,
	uint32  len,
	uint16* status
)
{
	TResult result = EOK;
	uint32  rlen;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	if ((data == NULL) || (status == NULL)) 
	{
		ERROR(FILE_LINE, "NV_Write_Item, Invalid params!!!");
		return false;
	}

	/* build a diag cmd filled in with basic info */
	START_BUILDING_CMD(DIAG_NV_WRITE_F);

	/* fill in item id info */
	uint8*  buf = (uint8*)&item;
	memcpy(&cmd_ptr->buf[cmd_ptr->length], buf, sizeof(item));
	cmd_ptr->length += sizeof(item);

	/* pack index field */
	cmd_ptr->buf[cmd_ptr->length++] = index;

	/* fill in nv item data info */
	memcpy(&cmd_ptr->buf[cmd_ptr->length], data, len);
	cmd_ptr->length += len;

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "NV_Write_Item, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();
	
	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	/* fill in rsp data length, which includes cmd_code, item id,
	 * item data and status.
	 */
	//rsp_ptr->length = rlen;

	/* check if the rsp data is ok, such as rsp code matches
	 * with cmd code, valid data length and such on.
	 */
	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	/* set status */
	*status = (uint16)*(rsp_ptr->buf + 3 + NV_ITEM_DATA_LEN);
	if (!check_nv_status_ok(*status)) 
	{
		return false;
	}

	return true;
}

bool CDIAGCmd::NV_Write_Item
(
	  uint16 	item,    // item id
	  uint8* 	data,    // item data
	  uint32 	len,     // data length
	 uint16* status   // write status
)
{
	TResult result = EOK;
	uint32  rlen;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	if ((data == NULL) || (status == NULL)) 
	{
		ERROR(FILE_LINE, "NV_Write_Item, Invalid params!!!");
		return false;
	}

	/* build a diag cmd filled in with basic info */
	START_BUILDING_CMD(DIAG_NV_WRITE_F);

	/* fill in item id info */
	uint8*  buf = (uint8*)&item;
	memcpy(&cmd_ptr->buf[cmd_ptr->length], buf, sizeof(item));
	cmd_ptr->length += sizeof(item);

	/* fill in nv item data info */
	memcpy(&cmd_ptr->buf[cmd_ptr->length], data, len);
	cmd_ptr->length += len;

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "NV_Write_Item, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	/* fill in rsp data length, which includes cmd_code, item id,
	 * item data and status.
	 */
	//rsp_ptr->length = rlen;

	/* check if the rsp data is ok, such as rsp code matches
	 * with cmd code, valid data length and such on.
	 */
	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	/* set status */
	*status = (uint16)*(rsp_ptr->buf + 3 + NV_ITEM_DATA_LEN);
	if (!check_nv_status_ok(*status)) 
	{
		return false;
	}

	return true;
}

TResult CDIAGCmd::VerifySPC(const char* strSPC)
{
	TResult result = EOK;
	uint32  rlen = 0;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);
	
	START_BUILDING_CMD(DIAG_SPC_F);

	memcpy(cmd_ptr->buf+1,strSPC,strlen(strSPC));
	cmd_ptr->length = cmd_ptr->length + strlen(strSPC);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Verify SPC , cmd is NOT ok!!!");
		return EFAILED;
	}

	result = this->SendCmd(cmd_ptr, &rlen);

	if (FAILURE(result)) 
	{
		return EFAILED;
	}

	START_BUILDING_RSP();

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return EFAILED;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return EFAILED;
	}
	if (rsp_ptr->buf[1] != 1)
		return EFAILED;

	return result;
}

bool CDIAGCmd::RestartDevice()
{
	TResult result = EOK;
	uint32  rlen = 0;
    //changed by jie.li 2011-12-21
    //uint16 wMode = 6; //MODE_CHANGE_POWER_OFF;
    uint16 wMode = 2;  //MODE_CHANGE_RESET
    //end changed

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(DIAG_CONTROL_F);
	uint16 myMode = wMode;//MODE_CHANGE_OFFLINE_DIGITAL_MODE;
	memcpy(cmd_ptr->buf+1,&myMode,sizeof(uint16));
	cmd_ptr->length = cmd_ptr->length + sizeof(uint16);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Offline mode, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);

	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	return true;	
}

bool CDIAGCmd::SwitchToOfflineMode(uint16 wMode)
{
	TResult result = EOK;
	uint32  rlen = 0;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(DIAG_CONTROL_F);
	uint16 myMode = wMode;//MODE_CHANGE_OFFLINE_DIGITAL_MODE;
	memcpy(cmd_ptr->buf+1,&myMode,sizeof(uint16));
	cmd_ptr->length = cmd_ptr->length + sizeof(uint16);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Offline mode, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);

	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	return true;	
}

bool CDIAGCmd::DLoadMode(void)
{
	TResult result = EOK;
	uint32  rlen = 0;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_CMD(DIAG_DLOAD_F);

	/* check if cmd buffer is ok (overflow and others?) */
	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "DLoadMode, cmd is NOT ok!!!");
		return false;
	}

	result = this->SendCmd(cmd_ptr, &rlen);

	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();

	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		return false;
	}

	return true;
}

bool CDIAGCmd::WcdmaStatus(void)
{
	TResult result = EOK;
	uint32  rlen = 0;
	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	START_BUILDING_SUBSYS_CMD(DIAG_SUBSYS_CMD_F, DIAG_SUBSYS_WCDMA, WCDMA_STATUS_F);

	result = this->SendCmd(cmd_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	START_BUILDING_RSP();
	result = m_packetDll->Receive(rsp_ptr, &rlen);
	if (FAILURE(result)) 
	{
		return false;
	}

	return true;
}

TResult CDIAGCmd::EfsOpStat
(
 bool   bArmEfs,
	  const char* filename, 
	 int32& mode, 
	 int32& size
)
{
	TResult result = EOK;
	efs2_diag_stat_req_type 	req = {0};
	efs2_diag_stat_rsp_type 	rsp = {0};
	uint32 len  = sizeof(diagpkt_subsys_header_type) + strlen(filename) + 1;;
	uint32 rlen = sizeof(rsp);
	
	req.hdr.cmd_code  = DIAG_SUBSYS_CMD_F;
	if (!bArmEfs)  //·Ç8200A
	req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_STAT;
	strcpy(req.path, filename);

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
		return EFAILED;
	}

	mode = rsp.mode;
	size = rsp.size;

	return EOK;
}

TResult CDIAGCmd::EfsDelFile(bool bArmEfs, const char* filename)
{
	TResult result = EOK;
	efs2_diag_unlink_req_type 	req = {0};
	efs2_diag_unlink_rsp_type 	rsp = {0};
	uint32 len = sizeof(req) - DIAG_FS_MAX_FILENAME_LEN 
                 + strlen(filename) + 1;
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	if (!bArmEfs)
	req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_UNLINK;
	strcpy(req.path, filename);

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
		return EFAILED;
	}
	return EOK;
}

TResult CDIAGCmd::EfsOpOpen
(
	bool  bArmEfs,
	  const char* filename, 
	  int32 oflag, 
	  int32 mode, 
	 int32& fd
)
{
	TResult result = EOK;
	efs2_diag_open_req_type 	req = {0};
	efs2_diag_open_rsp_type 	rsp = {0};
	uint32 len = sizeof(req) - DIAG_FS_MAX_FILENAME_LEN 
				+ strlen(filename) + 1;
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	if (!bArmEfs)
	req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_OPEN;
	req.oflag = oflag;
	req.mode = mode;
	strcpy(req.path, filename);

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
		return EFAILED;
	}

	if (rsp.fd == -1) 
	{
		return EFAILED;
	}

	fd = rsp.fd;

	return EOK;
}

TResult CDIAGCmd::EfsOpRead
(
    bool   bArmEfs,
    int32  fd,
    uint32 flen,
    uint8* fdata
)
{
	TResult result = EOK;
	efs2_diag_read_req_type    req = {0};
	efs2_diag_read_rsp_type    rsp = {0};
	uint32 len  = sizeof(req);
	uint32 rlen = sizeof(rsp);
	
	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

	if (!bArmEfs)
		req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_READ;
	req.fd = fd;

	uint32 nread = 0;
	uint32 offset = 0;
	for (;;) 
	{
		nread = (flen > DIAG_FS_BUFFER_SIZE) ? DIAG_FS_BUFFER_SIZE : flen;
		req.nbyte = nread;
		req.offset = offset;
		result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
		if (FAILURE(result) || rsp.diag_errno != 0) 
		{
//			ERROR(FILE_LINE, "COM%d: EfsOpRead failure, result=%d, errno=%d!", 
//				m_packetDll->GetComPort(), result, rsp.diag_errno);
			return EFAILED;
		}
		memcpy(fdata+offset, rsp.data, rsp.bytes_read);
		offset += rsp.bytes_read;
		flen -= rsp.bytes_read;

		if (flen == 0) 
		{
			break;
		}
	}

	return EOK;
}

/*=============================================================================

=============================================================================*/
TResult CDIAGCmd::EfsOpWrite
(
	bool    bArmEfs,
    int32   fd,      // file descriptor
    uint8*  pdata,   // origin address of data
    uint32& offset,  // start()/end() address of write ptr
    uint32  len      // length of data to be written
)
{
    TResult result = EOK;
    efs2_diag_write_req_type    req = {0};
    efs2_diag_write_rsp_type    rsp = {0};
    uint32 ilen = 0;
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_WRITE;
    req.fd = fd;

    // bytes to write
    uint32 nwrite = 0;
    uint32 pos = offset;

    for (;;)
    {
        memset(req.data, 0, sizeof(req.data));
        nwrite = (len > DIAG_FS_BUFFER_SIZE) ? DIAG_FS_BUFFER_SIZE : len;
        ilen   = nwrite + sizeof(req) - sizeof(req.data);

        req.offset = pos;
        memcpy(req.data, pdata+pos, nwrite);

        result = this->EfsOperate(ilen, (uint8*)&req, &rlen, (uint8*)&rsp);
        if (FAILURE(result) || rsp.diag_errno != 0)
        {
            //			ERROR(FILE_LINE, "COM%d: EfsOpWrite failure, result=%d, errno=%d!",
            //				m_packetDll->GetComPort(), result, rsp.diag_errno);
            return EFAILED;
        }
        if ((rsp.hdr.cmd_code != req.hdr.cmd_code)
            || (rsp.hdr.subsys_cmd_code != req.hdr.subsys_cmd_code))
            {
            //			ERROR(FILE_LINE, "COM%d: EfsOpWrite failure, cmd_code=%d, subsys_cmd_code=%d!",
            //				m_packetDll->GetComPort(), rsp.hdr.cmd_code, rsp.hdr.subsys_cmd_code);
            return EFAILED;
        }
        if ((int32)len < rsp.bytes_written)
        {
            //			ERROR(FILE_LINE, "COM%d: EfsOpWrite failure, len (%d) < rsp.bytes_written (%d)!",
            //				m_packetDll->GetComPort(), len, rsp.bytes_written);
            return EFAILED;
        }

        pos += rsp.bytes_written;
        len -= rsp.bytes_written;

        if (len == 0)
        {
            break;
        }
    }
    offset = pos;

    return EOK;
}

TResult CDIAGCmd::EfsOpClose(bool bArmEfs, int32 fd)
{
	TResult result = EOK;
	efs2_diag_close_req_type req = {0};
	efs2_diag_close_rsp_type rsp = {0};
	uint32 len  = sizeof(req);
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

	if (!bArmEfs)
		req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_CLOSE;
	req.fd = fd;

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
//		ERROR(FILE_LINE, "COM%d: EfsOpClose failure, result=%d, errno=%d!", 
//				m_packetDll->GetComPort(), result, rsp.diag_errno);
		return EFAILED;
	}

	return EOK;
}

TResult CDIAGCmd::EfsOpHello(bool bArmEfs)
{
	TResult result = EOK;
	efs2_diag_hello_req_type	req;
	efs2_diag_hello_rsp_type	rsp;
	uint32 len = sizeof(req);
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

	if (!bArmEfs)
		req.hdr.subsys_id = DIAG_SUBSYS_FS;
	else
		req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

	req.hdr.subsys_cmd_code = EFS2_DIAG_HELLO;
	memcpy(&req.targ_pkt_window, &efs2_diag_default_params, sizeof(efs2_diag_params_type));

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result)) 
	{
//		WARN(FILE_LINE, "COM%d: EfsOpHello failure!", m_packetDll->GetComPort());
		return result;
	}

	if ((req.hdr.subsys_cmd_code != rsp.hdr.subsys_cmd_code)
		||(req.hdr.subsys_id!= rsp.hdr.subsys_id))
	{
//		ERROR(FILE_LINE, "COM%d: EfsOpHello failure, cmd_code=%d, subsys_cmd_code=%d!", 
//				m_packetDll->GetComPort(), rsp.hdr.cmd_code, rsp.hdr.subsys_cmd_code);
		return EFAILED;
	}
	
	return EOK;
}

TResult CDIAGCmd::SendRestoreCmd(void)
{
	TResult result = EOK;
	diagpkt_subsys_header_type	req;	
	uint32 len = sizeof(req);
	uint32  rlen = 0;

	req.cmd_code = DIAG_SUBSYS_CMD_F;
	req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.subsys_cmd_code = SEND_RESTORE_CMD;	

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
//		ERROR(FILE_LINE, "Send Restore command, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Send Restore command failure, result: %d!", result);
			continue;
		}

		START_BUILDING_RSP();	

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Send Restore command failure, result: %d!", result);
//			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "Send Restore command failure, result: %d!", result);
		return result;
	}	

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
//		ERROR(FILE_LINE, "Send Restore command check_diag_rsp_ok failure!");
		return EFAILED;
	}

	return EOK;
}

TResult CDIAGCmd::RequestFirmwareVer_N(char* fdata)
{
	TResult result = EOK;
	efs2_diag_get_version_req_type	req;
	efs2_diag_get_version_rsp_type  rsp;
	
	memset(&rsp, 0, sizeof(efs2_diag_get_version_rsp_type));
	uint32 len = sizeof(req);
	uint32  rlen = 0;

	if (fdata == NULL)
	{
//		ERROR(FILE_LINE, "COM%d: Request firmware version failure, EINVALIDPARAM!");
		return EINVALIDPARAM;		
	}

	req.cmd_code = DIAG_EXT_BUILD_ID_F;

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
//		ERROR(FILE_LINE, "Request firmware version, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Request firmware version failure, result: %d!", result);
			continue;
		} 

        rsp_ptr->length = sizeof(efs2_diag_get_version_rsp_type);
		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Receive firmware version failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "RequestFirmwareVer_N failure, result: %d!", result);
		return result;
	}	

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
//		ERROR(FILE_LINE, "RequestFirmwareVer_N check_diag_rsp_ok failure!");
		return EFAILED;
	}	
	rsp = *(efs2_diag_get_version_rsp_type *)(rsp_ptr->buf);
	if (DIAG_EXT_BUILD_ID_F != rsp.cmd_code)
		return EFAILED;

	memcpy(fdata,&rsp,sizeof(efs2_diag_get_version_rsp_type));
	return EOK;
}

TResult CDIAGCmd::EnableDiagServer()
{
    TResult result = EOK;
    diagpkt_subsys_header_type	req;
    uint32 len = sizeof(req);
    uint32  rlen = 0;

	req.cmd_code = DIAG_SUBSYS_CMD_F;
	req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.subsys_cmd_code = SEND_ENABLE_DIAG;	

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->buf[len] = 1;
	cmd_ptr->buf[len + 1] = 0;
	cmd_ptr->length += len + 2;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
//		ERROR(FILE_LINE, "Enable diag server, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
			continue;
		}

		START_BUILDING_RSP();	

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "Enable diag server failure, result: %d!", result);
		return result;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
//		ERROR(FILE_LINE, "Enable diag server check_diag_rsp_ok failure!");
		return EFAILED;
	}	
	
	return EOK;
}

TResult CDIAGCmd::DisableDiagServer()
{
	TResult result = EOK;
	diagpkt_subsys_header_type	req;	
	uint32 len = sizeof(req);
	uint32  rlen = 0;

	req.cmd_code = DIAG_SUBSYS_CMD_F;
	req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.subsys_cmd_code = SEND_ENABLE_DIAG;	

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->buf[len] = 0;
	cmd_ptr->buf[len + 1] = 1;
	cmd_ptr->length += len + 2;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
//		ERROR(FILE_LINE, "Enable diag server, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
			continue;
		}

		START_BUILDING_RSP();	

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Enable diag server failure, result: %d!", result);
			continue;
		}
		break;
	}

	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "Enable diag server failure, result: %d!", result);
		return result;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
//		ERROR(FILE_LINE, "Enable diag server check_diag_rsp_ok failure!");
		return EFAILED;
	}	

	return EOK;
}

TResult CDIAGCmd::GetRootVer_N(uint8& ver)
{
	TResult result = EOK;
	diagpkt_subsys_header_type	req;	
	uint32 len = sizeof(req);
	uint32  rlen = 0;

	req.cmd_code = DIAG_SUBSYS_CMD_F;
	req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.subsys_cmd_code = REQ_ROOT_VER;	

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Request root version, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Request root version failure, result: %d!", result);
			continue;
		}

		START_BUILDING_RSP();	

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Receive root version failure, result: %d!", result);
			continue;
		}
		break;
	}

	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "GetRootVer_N failure, result: %d!", result);
		return result;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		ERROR(FILE_LINE, "GetRootVer_N check_diag_rsp_ok failure!");
		return EFAILED;
	}

	ver = rsp_ptr->buf[4];
	return EOK;
}

TResult CDIAGCmd::RequestFlashType_N(uint8& type)
{
	TResult result = EOK;
	diagpkt_subsys_header_type	req;	
	uint32 len = sizeof(req);
	uint32  rlen = 0;

	req.cmd_code = DIAG_SUBSYS_CMD_F;
	req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.subsys_cmd_code = REQ_FLASH_TYPE_N;	

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
//		ERROR(FILE_LINE, "Request flash type, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Request flash type failure, result: %d!", result);
			continue;
		}

		START_BUILDING_RSP();	

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
//			LOG(FILE_LINE, "Receive flash type failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
//		ERROR(FILE_LINE, "RequestFlashType_N failure, result: %d!", result);
		return result;
	}

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		ERROR(FILE_LINE, "RequestFlashType_N check_diag_rsp_ok failure!");
		return EFAILED;
	}	
	type = rsp_ptr->buf[4];
	return EOK;
}

TResult CDIAGCmd::SendCmd(cmd_buffer_s_type* cmd_ptr, uint32* rlen)
{
	if (cmd_ptr == NULL || rlen == NULL) 
	{
		return EINVALIDPARAM;
	}

	return m_packetDll->Send(cmd_ptr, rlen);
}


//-----------------------------------------------------------------------------

TResult CDIAGCmd::EfsOperate
(
    uint32 	len,
    uint8* 	INdata,
    uint32* outLen,
    uint8* 	OUTdata
)
{
    TResult result = EOK;
    uint32  rlen;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    if (INdata == NULL)
    {
        ERROR(FILE_LINE, "EfsOperate failure, Invalid params!!!");
        return EINVALIDPARAM;
    }

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, INdata, len);
    cmd_ptr->length += len;

    /* check if cmd buffer is ok (overflow and others?) */
    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "EfsOperate, cmd is NOT ok!");
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            //	LOG(FILE_LINE, "EfsOperate SendCmd failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();
        rsp_ptr->length = *outLen;
        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            //	LOG(FILE_LINE, "EfsOperate Receive failure, result: %d!", result);
            continue;
        }
        break;
    }

    if (FAILURE(result))
    {
        //	ERROR(FILE_LINE, "EfsOperate failure, result: %d!", result);
        return result;
    }

    /*if (rsp_ptr->length != rlen)
{
return false;
}*/
/* check if the rsp data is ok, such as rsp code matches
	 * with cmd code.
         */
    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "EfsOperate check_diag_rsp_ok failure!");
        return EFAILED;
    }

    /* copy out nv item data */
    memcpy(OUTdata, rsp_ptr->buf, rlen);
    *outLen = rlen;
    return EOK;
}

//add by jie.li 2011-07-04
TResult CDIAGCmd::OperDashboardFlag(int operType, int& flag, uint8& retCode)
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 4;

    cmd_ptr->buf[0] = SEND_OPER_DASHBOARD_FLAG; // CMD_CODE.  1 Byte.
    cmd_ptr->buf[1] = operType;				    // Operation. 1 Byte. 0: read dashboard status from flash
                                                //                  1: write dashboard status into flash


    cmd_ptr->buf[2] = flag;                     // Flag value. 0: OK; !0:Fail
    cmd_ptr->buf[3] = 0;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "OperDashboardFlag, cmd is not ok!");
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "operation dashboard flag failure, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();
        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Receive OperDashboardFlag failure, result: %d!", result);
            continue;
        }

        break;
    }

    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "OperDashboardFlag failure, result: %d!", result);
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        flag = rsp_ptr->buf[2];
        return EOK;
    }

    memcpy(&retCode, rsp_ptr->buf+3, 1);
    return EFAILED;
}
//end add

int32 CDIAGCmd::EfsOpOpenDir(bool bArmEfs, const char* filename)
{
    TResult result = EOK;
    efs2_diag_opendir_req_type 	req = {0};
    efs2_diag_opendir_rsp_type 	rsp = {0};
    uint32 len = sizeof(req) - DIAG_FS_MAX_FILENAME_LEN + strlen(filename) + 1;
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_OPENDIR;
    strcpy(req.path, filename);

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_errno != 0)
    {
        return -1;
    }
    return rsp.dirp;
}

TResult CDIAGCmd::EfsOpMKDir(bool bArmEfs, const char* filename)
{
    TResult result = EOK;
    efs2_diag_mkdir_req_type 	req = {0};
    efs2_diag_mkdir_rsp_type 	rsp = {0};
    uint32 len = sizeof(req) - DIAG_FS_MAX_FILENAME_LEN + strlen(filename) + 1;
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_MKDIR;
    strcpy(req.path, filename);

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || (rsp.diag_errno != 0))
    {
        return EFAILED;
    }
    return EOK;
}

//add by jie.li 2011-09-28 for y580
TResult CDIAGCmd::SendRawRequest(int& retType)
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 1;

    cmd_ptr->buf[0] = 0;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "SendRawRequest, cmd is not ok!");
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "SendRawRequest, result: %d!", result);
            continue;
        }

        START_BUILDING_RSP();
        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Receive SendRawRequest failure, result: %d!", result);
            continue;
        }

        break;
    }

    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "SendRawRequest failure, result: %d!", result);
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        unsigned char LBuf[] = "FACPACZM";   //is Lighter
        unsigned char YBuf[] = "FACPASZM";	 //is Y580
        bool bFindL = false;
        bool bFindY	= false;
        int iCountL = 0;
        int iCountY = 0;
        for (int i=0; i<sizeof(rsp_ptr->buf)/sizeof(rsp_ptr->buf[0]); i++)
        {
            //Find Lighter
            if (rsp_ptr->buf[i] == LBuf[0])
            {
                int k = i;
                for (int j=0; j<sizeof(LBuf)/sizeof(LBuf[0]); j++)
                {
                    if (rsp_ptr->buf[k] == LBuf[j])
                    {
                        k++;
                        iCountL++;
                    }
                    else
                    {
                        bFindL = false;
                        break;
                    }
                    bFindL = true;
                }
            }

            if (iCountL >= 8)
            {
                break;
            }

            //Find Y580
            if (rsp_ptr->buf[i] == YBuf[0])
            {
                int k = i;
                for (int j=0; j<sizeof(YBuf)/sizeof(YBuf[0]); j++)
                {
                    if (rsp_ptr->buf[k] == YBuf[j])
                    {
                        k++;
                        iCountY++;
                    }
                    else
                    {
                        bFindY = false;
                        break;
                    }
                    bFindY = true;
                }
            }

            if (iCountY >= 8)
            {
                break;
            }
        }

        if (bFindL && !bFindY)
        {
            retType = 1;  //1 is Lighter
        }
        else if (!bFindL && bFindY)
        {
            retType = 2;  //2 is Y580
        }
        else
        {
            return EFAILED;
        }

        return EOK;
    }

    return EOK;
}
//end add

TResult CDIAGCmd::ReadMacAddress(QString & strMacAddress, int iType)
{
    TResult result = EOK;
    diagpkt_subsys_header_type	req;
    uint32 len = sizeof(req);
    uint32  rlen = 0;

    req.cmd_code = DIAG_SUBSYS_CMD_F;
    req.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.subsys_cmd_code = READ_MAC_ADDRESS;

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
       
       if (iType == 1)
       { 
       		QString strNum = "";
        	char name[2];
        	int byte1 = rsp_ptr->buf[8];
        	sprintf(name, "%.02X", byte1);
        	strMacAddress = QString(name);
        	int byte2 = rsp_ptr->buf[9];
        	sprintf(name, "%.02X", byte2);
        	strNum = QString(name);
        	strMacAddress.append(strNum);

        	if (strMacAddress.length() != 4)
        	{
            	return EFAILED;
        	}
       }
       else if(iType ==0)
       	{
       		QString strNum = "";
        	char name[2];
        	int byte1 = rsp_ptr->buf[4];
        	sprintf(name, "%.02X", byte1);
        	strMacAddress = QString(name);
        	int byte2 = rsp_ptr->buf[5];
        	sprintf(name, "%.02X", byte2);
        	strNum = QString(name);
        	strMacAddress.append(strNum);

        	if (strMacAddress.length() != 4)
        	{
            	return EFAILED;
        	}
       }
       else if(iType == 2)
       {
           QString strNum = "";
           QString strNum1= "";
           char name[2];
           int byte1 = rsp_ptr->buf[7];
           sprintf(name,"%.02X", byte1);
           strMacAddress = QString(name);

           int byte2 = rsp_ptr->buf[8];
           sprintf(name, "%.02X", byte2);
           strNum = QString(name);
           strMacAddress.append(strNum);

           int byte3 = rsp_ptr->buf[9];
           sprintf(name, "%.02X", byte3);
           strNum1 = QString(name);
           strMacAddress.append(strNum1);

           if (strMacAddress.length() != 6)
           {
               return EFAILED;
           }
       }
        return EOK;
    }

    return EFAILED;
}

TResult CDIAGCmd::ReadCustModeName(QString & strModeName)
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 29;

    cmd_ptr->buf[0] = DIAG_SUBSYS_CMD_F;
    cmd_ptr->buf[1] = DIAG_SUBSYS_RESERVED_OEM_0;
    cmd_ptr->buf[2] = READ_CUST_MODE_NAME;
    cmd_ptr->buf[3] = 0;

    for (int i=1; i<=25; i++)
    {
        cmd_ptr->buf[i+3] = 0;
    }

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        uint8 Len = rsp_ptr->buf[1];
        uint8* pName = new uint8[Len + 1];
        memcpy(pName, &rsp_ptr->buf[5], Len);
        QByteArray byte;
        byte = QByteArray((char*)pName);
        strModeName = QString(byte);
        return EOK;
    }

    return EFAILED;
}

TResult CDIAGCmd::WriteSSID(QString strSSID)
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 41;

    cmd_ptr->buf[0] = DIAG_SUBSYS_CMD_F;
    cmd_ptr->buf[1] = DIAG_SUBSYS_RESERVED_OEM_0;
    cmd_ptr->buf[2] = WRITE_SSID;
    cmd_ptr->buf[3] = 0;
    cmd_ptr->buf[4] = 1;

    //int iStrSSID = strSSID.GetLength();
    int iStrSSID = strSSID.size();
    uint8* buf;
    buf = (uint8*)qstrdup(strSSID.toAscii().constData());
    memcpy(&cmd_ptr->buf[5], buf, iStrSSID);

    for (int i=1; i<=36-iStrSSID; i++)
    {
        cmd_ptr->buf[i+4+iStrSSID] = 0;
    }

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        int test = sizeof(rsp_ptr);
        return EOK;
    }

    return EFAILED;
}

TResult CDIAGCmd::ReadWifiPwdFlag(int &retFlagVal)
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 10;

    cmd_ptr->buf[0] = DIAG_SUBSYS_CMD_F;
    cmd_ptr->buf[1] = DIAG_SUBSYS_RESERVED_OEM_0;
    cmd_ptr->buf[2] = READ_WIFI_PWD_FLAG;
    cmd_ptr->buf[3] = 0;
    cmd_ptr->buf[4] = 1;   //<operation>£ºlen:  one byte. 1 read, 0 write


    for (int i=1; i<=5; i++)
    {
        cmd_ptr->buf[i+4] = 0;
    }

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        retFlagVal = rsp_ptr->buf[5];
        return EOK;
    }

    return EFAILED;
}

TResult CDIAGCmd::GenerateFTFiles()
{
    TResult result = EOK;
    uint32 rlen = 0;
    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);

    cmd_ptr->length = 8;

    cmd_ptr->buf[0] = DIAG_SUBSYS_CMD_F;
    cmd_ptr->buf[1] = DIAG_SUBSYS_RESERVED_OEM_0;
    cmd_ptr->buf[2] = GENERATE_FT_FILES;
    cmd_ptr->buf[3] = 0;
    cmd_ptr->buf[4] = 0;
    cmd_ptr->buf[5] = 0;
    cmd_ptr->buf[6] = 0;
    cmd_ptr->buf[7] = 0;


    if (!check_diag_cmd_ok(cmd_ptr))
    {
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }

        START_BUILDING_RSP();

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        return result;
    }

    if (check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        if ((rsp_ptr->buf[4] != 0) || (rsp_ptr->buf[5] != 0)
            || (rsp_ptr->buf[6] != 0) || (rsp_ptr->buf[7] != 0))
        {
            return EFAILED;
        }

        return EOK;
    }

    return EFAILED;
}

//add by jie.li 2012-06-27
int32 CDIAGCmd::EfsOpReadDelDir(bool bArmEfs, QString filename)
{
    TResult result = EOK;
    efs2_diag_readdir_req_type 	req = {0};
    efs2_diag_readdir_rsp_type 	rsp = {0};

    req.dirp = EfsOpOpenDir(bArmEfs, filename.toAscii().data());
    if (req.dirp < 0)
    {
        return -1;
    }

    uint32 len  = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_READDIR;

    for (int i=1; ; i++)
    {
        req.seqno = i;
        result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
        if (FAILURE(result) || (rsp.diag_errno != 0))
        {
            return rsp.dirp;
        }

        if (rsp.entry_type == 0)
        {
            if (rsp.entry_name[0] == '\0')
            {
                result = this->EfsOpCloseDir(bArmEfs, filename.toAscii().data(), rsp.dirp);
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
                result = this->EfsOpRMDir(bArmEfs, filename.toAscii().data());
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
            }
            else
            {
                char preName[128];
                memset(preName, 0, 128);
                strcpy(preName, filename.toAscii().data());
                strcat(preName, "/");
                strcat(preName, rsp.entry_name);
                QString	strName(preName);
                result = this->EfsDelFile(bArmEfs, strName.toAscii().data());
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
            }
        }
        else if (rsp.entry_type == 1)
        {
            if (rsp.entry_name[0] == '\0')
            {
                result = this->EfsOpRMDir(bArmEfs, filename.toAscii().data());
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
            }
            else
            {
                char preName[128];
                memset(preName, 0, 128);
                strcpy(preName, filename.toAscii().data());
                strcat(preName, "/");
                strcat(preName, rsp.entry_name);
                QString	strName(preName);
                this->EfsOpReadDelDir(bArmEfs, strName);
            }
        }
        else if (rsp.entry_type == 2)
        {
            if (rsp.entry_name[0] == '\0')
            {
                result = this->EfsOpCloseDir(bArmEfs, filename.toAscii().data(), rsp.dirp);
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
                result = this->EfsDelFile(bArmEfs, filename.toAscii().data());
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
            }
            else
            {
                char preName[128];
                memset(preName, 0, 128);
                strcpy(preName, filename.toAscii().data());
                strcat(preName, "/");
                strcat(preName, rsp.entry_name);
                QString	strName(preName);
                result = this->EfsDelFile(bArmEfs, strName.toAscii().data());
                if (FAILURE(result) || (rsp.diag_errno != 0))
                {
                    return rsp.dirp;
                }
            }
        }
    }

    return rsp.dirp;
}

TResult CDIAGCmd::EfsOpCloseDir(bool bArmEfs, const char* filename, int32 dirp)
{
    TResult result = EOK;
    efs2_diag_closedir_req_type 	req = {0};
    efs2_diag_closedir_rsp_type 	rsp = {0};

    uint32 len  = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_CLOSEDIR;

    req.dirp = dirp;

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_errno != 0)
    {
        return EFAILED;
    }
    return EOK;
}

TResult CDIAGCmd::EfsOpRMDir(bool bArmEfs, const char* filename)
{
    TResult result = EOK;
    efs2_diag_rmdir_req_type 	req = {0};
    efs2_diag_rmdir_rsp_type 	rsp = {0};
    uint32 len = sizeof(req) - DIAG_FS_MAX_FILENAME_LEN + strlen(filename) + 1;
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;

    if (!bArmEfs)
        req.hdr.subsys_id = DIAG_SUBSYS_FS;
    else
        req.hdr.subsys_id = DIAG_SUBSYS_FS_ALTERNATE;

    req.hdr.subsys_cmd_code = EFS2_DIAG_RMDIR;
    strcpy(req.path, filename);

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_errno != 0)
    {
        return EFAILED;
    }
    return EOK;
}
//end add

//#ifdef FEATHER_CUSTOM_IDEA
//#ifdef ATA_TPST
TResult CDIAGCmd::ReadIMEI(QString & strImei)
{
    TResult result = EOK;
    extend_diag_read_imei_req_type req = {0};
    extend_diag_read_imei_rsp_type rsp = {0};
    uint32 len = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.hdr.subsys_cmd_code = EXTEND_DIAG_IMEI_READ;

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_error != 0)
    {
        return EFAILED;
    }

    char name[1];
    int byte = 0;
    byte = rsp.imei[0];
    sprintf(name, "%.02X", byte);
    QString strNum1 = QString(name);
    strImei.append(strNum1);
    byte = rsp.imei[1];
    sprintf(name, "%.02X", byte);
    QString strNum2 = QString(name);
    strImei.append(strNum2);
    byte = rsp.imei[2];
    sprintf(name, "%.02X", byte);
    QString strNum3 = QString(name);
    strImei.append(strNum3);
    byte = rsp.imei[3];
    sprintf(name, "%.02X", byte);
    QString strNum4 = QString(name);
    strImei.append(strNum4);
    byte = rsp.imei[4];
    sprintf(name, "%.02X", byte);
    QString strNum5 = QString(name);
    strImei.append(strNum5);
    byte = rsp.imei[5];
    sprintf(name, "%.02X", byte);
    QString strNum6 = QString(name);
    strImei.append(strNum6);
    byte = rsp.imei[6];
    sprintf(name, "%.02X", byte);
    QString strNum7 = QString(name);
    strImei.append(strNum7);
    byte = rsp.imei[7];
    sprintf(name, "%.02X", byte);
    QString strNum8 = QString(name);
    strImei.append(strNum8);

    return EOK;
}
//#endif
TResult CDIAGCmd::NewEnableDiagServer(QString strCode)
{
    TResult result = EOK;
    extend_diag_open_req_type req = {0};
    extend_diag_open_rsp_type rsp = {0};
    uint32 len = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.hdr.subsys_cmd_code = SEND_ENABLE_DIAG;
    req.action = 1;
    req.mode = 0;
    req.reserved = 0;
    req.diag_error = 0;

    //req.active_code
    for (int i=0; i<10; ++i)
    {
        req.active_code[i] = strCode.at(i).toAscii();
    }

    req.active_code[10] = 0;
    req.active_code[11] = 0;

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_error != 0)
    {
        return EFAILED;
    }

    return EOK;
}

TResult CDIAGCmd::NewDisableDiagServer()
{
    TResult result = EOK;
    extend_diag_open_req_type req = {0};
    extend_diag_open_rsp_type rsp = {0};
    uint32 len = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.hdr.subsys_cmd_code = SEND_ENABLE_DIAG;
    req.action = 0;
    req.mode = 1;
    req.reserved = 0;
    req.diag_error = 0;

    //req.active_code
    for (int i=0; i<12; ++i)
    {
        req.active_code[i] = 0;
    }

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
    if (FAILURE(result) || rsp.diag_error != 0)
    {
        return EFAILED;
    }

    return EOK;
}
//#endif //FEATHER_CUSTOM_IDEA

//add by jie.li for MDM9x15
TResult CDIAGCmd::RequestDashboardVer(char* fdata)
{
    TResult result = EOK;
    extend_diag_get_version_req_type  req = {0};
    extend_diag_get_version_rsp_type  rsp = {0};

    memset(&rsp,0,sizeof(extend_diag_get_version_rsp_type));

    uint32 len = sizeof(req);
    uint32  rlen = 0;

    if (fdata == NULL)
    {
        ERROR(FILE_LINE, "COM%d: Request Dashboard version failure, EINVALIDPARAM!");
        return EINVALIDPARAM;
    }

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.hdr.subsys_cmd_code = GET_DASHBOARD_VERSION;
    req.version_len = 0;
    // 	for (int i=0; i<32; i++)
    // 	{
    // 		req.version[i] = NULL;
    // 	}

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);

    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;

    rsp_ptr->length = sizeof(rsp);

    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Request Dashboard version, cmd is not ok!");
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request Dashboard version failure, result: %d!", result);
            continue;
        }

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Receive Dashboard version failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "RequestDashboardVer failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "RequestDashboardVer check_diag_rsp_ok failure!");
        return EFAILED;
    }
    rsp = *(extend_diag_get_version_rsp_type *)(rsp_ptr->buf);
    if (GET_DASHBOARD_VERSION != rsp.hdr.subsys_cmd_code)
        return EFAILED;

    memcpy(fdata,&rsp,sizeof(extend_diag_get_version_rsp_type));
    return EOK;
}

TResult CDIAGCmd::RequestQCNVer(char* fdata)
{
	TResult result = EOK;
	extend_diag_get_version_req_type  req = {0};
	extend_diag_get_version_rsp_type  rsp = {0};

    memset(&rsp,0,sizeof(extend_diag_get_version_rsp_type));

	uint32 len = sizeof(req);
	uint32  rlen = 0;

	if (fdata == NULL)
	{
		ERROR(FILE_LINE, "COM%d: Request QCN version failure, EINVALIDPARAM!");
		return EINVALIDPARAM;		
	}

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.hdr.subsys_cmd_code = GET_QCNVERSION;
	req.version_len = 0;
// 	for (int i=0; i<32; i++)
// 	{
// 		req.version[i] = NULL;
// 	}

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	rsp_ptr->length = sizeof(rsp);

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Request QCN version, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Request QCN version failure, result: %d!", result);
			continue;
		} 

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Receive QCN version failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
		ERROR(FILE_LINE, "RequestQCNVer failure, result: %d!", result);
		return result;
	}	

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		ERROR(FILE_LINE, "RequestQCNVer check_diag_rsp_ok failure!");
		return EFAILED;
	}	
	rsp = *(extend_diag_get_version_rsp_type *)(rsp_ptr->buf);
	if (GET_QCNVERSION != rsp.hdr.subsys_cmd_code)
		return EFAILED;

	memcpy(fdata,&rsp,sizeof(extend_diag_get_version_rsp_type));
	return EOK;
}

TResult CDIAGCmd::RequestQ6resVer(char* fdata)
{
	TResult result = EOK;
	extend_diag_get_version_req_type  req = {0};
	extend_diag_get_version_rsp_type  rsp = {0};

    memset(&rsp,0,sizeof(extend_diag_get_version_rsp_type));

	uint32 len = sizeof(req);
	uint32  rlen = sizeof(rsp);

	if (fdata == NULL)
	{
		ERROR(FILE_LINE, "COM%d: Request Q6Res version failure, EINVALIDPARAM!");
		return EINVALIDPARAM;		
	}

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.hdr.subsys_cmd_code = GET_Q6RESOURCEVER;
	req.version_len = 0;
// 	for (int i=0; i<32; i++)
// 	{
// 		req.version[i] = NULL;
// 	}

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, len);
	cmd_ptr->length += len;

	rsp_ptr->length = sizeof(rsp);

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Request Q6Res version, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Request Q6Res version failure, result: %d!", result);
			continue;
		} 

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Receive Q6Res version failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
		ERROR(FILE_LINE, "RequestQ6ResVer failure, result: %d!", result);
		return result;
	}	

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		ERROR(FILE_LINE, "RequestQ6ResVer check_diag_rsp_ok failure!");
		return EFAILED;
	}	
	rsp = *(extend_diag_get_version_rsp_type *)(rsp_ptr->buf);
	if (GET_Q6RESOURCEVER != rsp.hdr.subsys_cmd_code)
		return EFAILED;

	memcpy(fdata,&rsp,sizeof(extend_diag_get_version_rsp_type));
	return EOK;
}

TResult CDIAGCmd::RequestLinuxVer(char* fdata)
{
    TResult result = EOK;
    extend_get_ker_sys_ver_req_type  req;
    extend_get_ker_sys_ver_rsp_type  rsp;

    memset(&rsp,0,sizeof(extend_diag_get_version_rsp_type));
    uint32 len = sizeof(req);
    uint32  rlen = sizeof(rsp);
    if (fdata == NULL)
    {
        ERROR(FILE_LINE, "COM%d: Request Dashboard version failure, EINVALIDPARAM!");
        return EINVALIDPARAM;
    }

    req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
    req.hdr.subsys_cmd_code = GET_LINUX_VERSION;
    req.kernel_ver_num = 0;
    req.system_ver_num = 0;
    req.userdata_ver_num = 0;
    for (int i=0; i<32; i++)
    {
        req.kernel_ver[i] = NULL;
        req.system_ver[i] = NULL;
        req.userdata_ver[i] = NULL;
    }

    DECLARE_CMD_PTR(cmd_ptr);
    DECLARE_RSP_PTR(rsp_ptr);
    INITIATING_CMD(cmd_ptr);
    memcpy(cmd_ptr->buf, &req, len);
    cmd_ptr->length += len;
    rsp_ptr->length = len;
    if (!check_diag_cmd_ok(cmd_ptr))
    {
        ERROR(FILE_LINE, "Request Dashboard version, cmd is not ok!");
        return EFAILED;
    }

    for (int i=0; i<MAX_RESEND; ++i)
    {
        result = this->SendCmd(cmd_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Request Dashboard version failure, result: %d!", result);
            continue;
        }

        result = m_packetDll->Receive(rsp_ptr, &rlen);
        if (FAILURE(result))
        {
            LOG(FILE_LINE, "Receive Dashboard version failure, result: %d!", result);
            continue;
        }
        break;
    }
    if (FAILURE(result))
    {
        ERROR(FILE_LINE, "RequestDashboardVer failure, result: %d!", result);
        return result;
    }

    if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr))
    {
        memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
        rsp_ptr->length = 0;
        ERROR(FILE_LINE, "RequestDashboardVer check_diag_rsp_ok failure!");
        return EFAILED;
    }
    rsp = *(extend_get_ker_sys_ver_rsp_type *)(rsp_ptr->buf);

    if (GET_LINUX_VERSION != rsp.hdr.subsys_cmd_code)
        return EFAILED;
    memcpy(fdata,&rsp,sizeof(extend_get_ker_sys_ver_rsp_type));
    return EOK;
}
TResult CDIAGCmd::DIAG_ReadIMEI_9X25(QString & strImei)
{

    TResult result = EOK;
    jrd_diag_sys_imei_write_req_rsp_type req = {0};
    jrd_diag_sys_imei_write_req_rsp_type rsp = {0};
    uint32 len = sizeof(req);
    uint32 rlen = sizeof(rsp);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_IMEI_WRITE;

    req.rw_flag = E_JRD_READ;

    result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);

    if (FAILURE(result))// || rsp.diag_error != 0)
    {
        return EFAILED;
    }

    char name[1];
    int byte = 0;
    byte = rsp.imei[0];
    sprintf(name, "%.02X", byte);
    QString strNum1 = QString(name);
    strImei.append(strNum1);
    byte = rsp.imei[1];
    sprintf(name, "%.02X", byte);
    QString strNum2 = QString(name);
    strImei.append(strNum2);
    byte = rsp.imei[2];
    sprintf(name, "%.02X", byte);
    QString strNum3 = QString(name);
    strImei.append(strNum3);
    byte = rsp.imei[3];
    sprintf(name, "%.02X", byte);
    QString strNum4 = QString(name);
    strImei.append(strNum4);
    byte = rsp.imei[4];
    sprintf(name, "%.02X", byte);
    QString strNum5 = QString(name);
    strImei.append(strNum5);
    byte = rsp.imei[5];
    sprintf(name, "%.02X", byte);
    QString strNum6 = QString(name);
    strImei.append(strNum6);
    byte = rsp.imei[6];
    sprintf(name, "%.02X", byte);
    QString strNum7 = QString(name);
    strImei.append(strNum7);
    byte = rsp.imei[7];
    sprintf(name, "%.02X", byte);
    QString strNum8 = QString(name);
    strImei.append(strNum8);
    byte = rsp.imei[8];
    sprintf(name, "%.02X", byte);
    QString strNum9 = QString(name);
    strImei.append(strNum9);

    strImei=strImei.mid(2,16);
    QString temp2;
   temp2.append(strImei.mid(1,1)).append(strImei.mid(0,1)).append(strImei.mid(3,1)).append(strImei.mid(2,1)).append(strImei.mid(5,1))
         .append(strImei.mid(4,1)).append(strImei.mid(7,1)).append(strImei.mid(6,1)).append(strImei.mid(9,1)).append(strImei.mid(8,1)).append(strImei.mid(11,1))
         .append(strImei.mid(10,1)).append(strImei.mid(13,1)).append(strImei.mid(12,1)).append(strImei.mid(15,1)).append(strImei.mid(14,1))
         ;
    temp2=temp2.mid(1,15);
    strImei=temp2;
    return EOK;

}

TResult CDIAGCmd::StorePIC(uint8* data, uint32 len)
{
	TResult result = EOK;
	extend_diag_store_pic_req_type req = {0};
	
	uint32 rlen = 0;

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.hdr.subsys_cmd_code = STORE_ANIMAL_PIC;

	//uint8*  buf = (uint8*)&data;
	//memcpy(&req.pic, buf, sizeof(data));
	memcpy(&req.pic, data, len);

	DECLARE_CMD_PTR(cmd_ptr);
	DECLARE_RSP_PTR(rsp_ptr);

	INITIATING_CMD(cmd_ptr);
	memcpy(cmd_ptr->buf, &req, (len+4));
	cmd_ptr->length += (len+4);

	if (!check_diag_cmd_ok(cmd_ptr)) 
	{
		ERROR(FILE_LINE, "Store pic, cmd is not ok!");
		return EFAILED;
	}

	for (int i=0; i<MAX_RESEND; ++i) 
	{
		result = this->SendCmd(cmd_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Store pic failure, result: %d!", result);
			continue;
		} 

		result = m_packetDll->Receive(rsp_ptr, &rlen);
		if (FAILURE(result)) 
		{
			LOG(FILE_LINE, "Store pic failure, result: %d!", result);
			continue;
		}
		break;
	}
	if (FAILURE(result)) 
	{
		ERROR(FILE_LINE, "Store pic failure, result: %d!", result);
		return result;
	}	

	if (!check_diag_rsp_ok(cmd_ptr, rsp_ptr)) 
	{
		memset(rsp_ptr->buf, 0x00, sizeof(rsp_ptr->buf));
		rsp_ptr->length = 0;
		ERROR(FILE_LINE, "Store pic check_diag_rsp_ok failure!");
		return EFAILED;
	}	

	return EOK;
}
//end add for MDM9x15

//add by jie.li 2012-11-08 for MDM9x15 to generate the FT files
TResult CDIAGCmd::GenerateFTFilesNew(byte valueForce)
{
	TResult result = EOK;
	extend_diag_factory_file_gen_req_type 	req = {0};
	extend_diag_factory_file_gen_rsp_type 	rsp = {0};

	uint32 len  = sizeof(req);
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.hdr.subsys_cmd_code = GENERATE_FT_FILES;
	req.value = valueForce;

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
		return EFAILED;
	}
	return EOK;
}
//end add

//add by jie.li 2012-11-08 for MDM9x15 to set at+func=5,0
TResult CDIAGCmd::SetFuncFive(byte valueFuncFive)
{
	TResult result = EOK;
	extend_diag_set_func_five_req_type 	req = {0};
	extend_diag_set_func_five_rsp_type 	rsp = {0};

	uint32 len  = sizeof(req);
	uint32 rlen = sizeof(rsp);

	req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
	req.hdr.subsys_id = DIAG_SUBSYS_RESERVED_OEM_0;
	req.hdr.subsys_cmd_code = RESTORE_FUNC_FIVE;
	req.value = valueFuncFive;

	result = this->EfsOperate(len, (uint8*)&req, &rlen, (uint8*)&rsp);
	if (FAILURE(result) || rsp.diag_errno != 0) 
	{
		return EFAILED;
	}
	return EOK;
}
//end add
