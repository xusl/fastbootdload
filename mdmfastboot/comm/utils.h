/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2010-02-22  jianwen.he Add new function
2009-07-28  dawang.xu  Add PrintBufferString, remove FormatBufferString declaration
2009-04-10  dawang.xu  Remove NEW_ZERO_MEMORY/DELETE_IF/DELETE_ARRAY_IF and some
                       unuseful macros.
2009-02-07  dawang.xu  Init first version

=============================================================================*/
#ifndef __UTILS_H__
#define __UTILS_H__

//-----------------------------------------------------------------------------
#include "comdef.h"
#include "StringTable.h"

#ifdef FEATURE_TPST_SSID
#include "../../src/xmllib/tinyxml.h"   //add by jie.li 20120830 for Y580 to parse the webs_config
#endif


//-----------------------------------------------------------------------------
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define  B_PTR(var)  ((byte *) (void *) &(var))
#define  W_PTR(var)  ((word *) (void *) &(var))
#define  D_PTR(var)  ((dword *) (void *) &(var))

// Get array element count
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))

// Get a mask with nth bits to be 1 
#define LEFT_SHIFT_BITS(n) (1 << (n))

//add by jie.li 20120830 for Y580 to parse the webs_config
typedef struct
{
	char	webs_dev_name[20];
	char	webs_cust_model_name[20];
	char	webs_Ext_SSID[20];
} WebsXMLInfo;
//end add

// Print raw data stream and extra message to string stream 
void PrintBufferString(const uint8* buffer, const uint32 len, const char* fmtstr, ...);
uint8* ReadDLFile(const char* pname, uint32& rlen);
BOOL ReadNVDataFile(const char* pname, uint8** pphdr, uint8** ppdata, uint32& data_len);
BOOL ReadImgFiles(const char* pname, uint8** pphdr, uint8** ppdata, uint32& data_len);
BOOL ReadImgFiles(uint8* pbuf, uint32 buf_len, uint8** pphdr, uint8** ppdata, uint32& data_len);
BOOL WriteImgFiles(const char* pname, uint8* phdr, uint32 hdr_len, uint8* pdata, uint32 data_len);
CString GetAppPath();
int select_any(char path[MAX_PATH], char *title);
BOOL IsFileExist(CString strFile);
bool SetHideAttribute( const TCHAR * lpName );
CString LoadStringEx(UINT id);
const CString&GetCurrentPath() ;
HBITMAP  LoadBmpFormFile(LPCSTR lpFileName) ;
HICON  LoadIconFormFile(LPCSTR lpFileName);

#ifdef FEATURE_TPST_SSID
BOOL websconfigParse(uint8* pXmlBuf, WebsXMLInfo* pxmlinfo,int iFlag);  //add by jie.li 20120830 for Y580 to parse the webs_config
#endif

#endif //__UTILS_H__

