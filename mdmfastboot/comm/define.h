#ifndef DEFINE_H
#define DEFINE_H

#include "stdafx.h"

//type define
enum APP_TYPE{
	APP_TYPE_FIRMWARE = 0x01,        //firmware
	APP_TYPE_DASHBOARD,				 //dashboard
	APP_TYPE_NV,					 //nv
	APP_TYPE_SIMLOCK,				 //simlock	
	APP_TYPE_MAX
};

/******************************************************************
	Note: The agreed package of documents before the 768 bytes 
		  of the file header information.
	Include: the first 200 bytes of storage applications, the 
	      type and version number, and 568 each store to be 
		  packaged into the file name, location and length of
		  information.
*******************************************************************/

//Define file head structure
#define IMG_VERSION			1           // 6246 6290
#define IMG_VERSION_2		2           // 6200 6270
#define IMG_VERSION_3       3           // 8200A
#define IMG_VERSION_4		4			// 9200  add by jie.li 2011-08-25
#define IMG_VERSION_5		5			//add by jie.li for MDM9x15

typedef struct _IMGVerS{	
	WORD	  imgVer;				//img version	
}IMGVerS;

#define VERSION_HEAD_LEN	200
typedef struct _PacketHeadInfoS{
	APP_TYPE  appType;				//app type£ºFirmware¡¢dashboard¡¢nv¡¢simlock..
	char	  appVer[32];			//app version	
}PacketHeadInfoS;

//Define file store structure
#define FILEINFO_HEAD_LEN		568
//#define FILEINFO_HEAD_LEN_NEW   20000	//changed by jie.li 2011-09-01 for 9200
#define FILEINFO_HEAD_LEN_NEW   150000  //changed by jie.li 2011-11-26 for Y580 so many files
typedef struct _FilePosInfoS{
	char      fileName[20]; 		//file name
	DWORD	  beginPos;				//begin position
	DWORD	  fileLen;				//file length
}FilePosInfoS;

//add by jie.li 2011-09-01 for 9200
typedef struct _FilePosInfoNewS{
	char      fileName[70];			//file name
	DWORD	  beginPos;				//begin position
	DWORD	  fileLen;				//file length
}FilePosInfoNewS;
//end add
/******************************************************************
	Note£ºThe last two bytes of the packet CRC code
*******************************************************************/

//Define file head structure
#define CRC_CODE_LEN		2
typedef struct _PacketTailInfoS{	 
	WORD	  dwCRCCode;			 //CRC	
}PacketTailInfoS;

//Define packet structure
typedef struct _WriteFileInfoS{
	UINT uNum;						//file counter
	CStringArray arrFileName;		//store file name
	CString strPacketName;			//packet name

}WriteFileInfoS;

typedef struct {
	DWORD exeOffset;
	DWORD exeLength;
	DWORD imgOffset;
	DWORD imgLength;
	DWORD picOffset;
	DWORD picLength;
	DWORD txtOffset;
	DWORD txtLength;
	DWORD iconOffset;
	DWORD iconLength;
	DWORD nvFileOffset;
	DWORD nvFileLength;
	DWORD chmFileOffset;
	DWORD chmFileLength;
#ifdef PACKET_FILE
	DWORD patchFileOffset;
	DWORD patchFileLength;
#endif	
} TDataPosType;

#endif