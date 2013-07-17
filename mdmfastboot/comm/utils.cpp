/*=============================================================================
DESCRIPTION
	This module provide some utility functions for external call.

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2010-03-02  jianwen.he Add GetAppPath
2009-07-28  dawang.xu  Add PrintBufferString, modify FormatBufferString definition.
2009-04-10  dawang.xu  Add memory dump for memory allocate/release call.
2009-03-26  dawang.xu  Add FormatBufferString for printing buffer stream
2009-03-17	dawang.xu  Include <stdio.h> for multi-projects building.
2009-02-07  dawang.xu  Init first version

=============================================================================*/
#include "stdafx.h"
#include <stdio.h>
#include "utils.h"
#include "memmgr.h"


/*===========================================================================

DESCRIPTION
Get the path of the application

DEPENDENCIES
None

RETURN VALUE
Return to the path of the application

===========================================================================*/
CString GetAppPath()
{
	char path_buffer[MAX_PATH];	
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	GetModuleFileName(NULL, path_buffer, MAX_PATH);
	_splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, 0, 0, 0, 0);

	CString sPath = drive;
	sPath += dir;

	return sPath;
}

int select_any(char path[MAX_PATH], char *title) 
{
	BROWSEINFO bi;
	ITEMIDLIST *pidl;

	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = path;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_EDITBOX;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;

	pidl = SHBrowseForFolder(&bi);
	return (pidl && SHGetPathFromIDList(pidl, path));

}

BOOL IsFileExist(CString strFile)
{
	WIN32_FIND_DATA fData;

	HANDLE hFind = ::FindFirstFile(strFile, &fData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		return TRUE;
	}
	return FALSE;
}

bool SetHideAttribute( const TCHAR * lpName ) 
{ 
	DWORD dwResult = ::GetFileAttributes( lpName ); 
	if( INVALID_FILE_ATTRIBUTES == dwResult ) 
	{ 
		return false; 
	} 
	if( !(FILE_ATTRIBUTE_HIDDEN & dwResult) ) // 如果它不是隐藏的 
	{ 
		if( INVALID_FILE_ATTRIBUTES == ::SetFileAttributes( lpName, dwResult | FILE_ATTRIBUTE_HIDDEN ) ) 
		{ 
			return false; 
		} 
		return true; 
	} 
	else// 如果它已经是隐藏的，就当设置成功了 
	{ 
		return true; 
	} 
}

/*===========================================================================

DESCRIPTION
Read download file to buffer

DEPENDENCIES
None

RETURN VALUE
Return to the document buffer

===========================================================================*/
uint8* ReadDLFile
(
	const char* pname,   // File name to be read
	uint32& rlen         // Read byte length 
)
{
	if (pname == NULL) {
		return NULL;
	}

	FILE* fp = NULL;
	uint32 len = 0;
	uint8* pdata = NULL;

	fp = fopen(pname, "rb");
	if (fp == NULL) {
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	NEW_ARRAY(pdata, uint8, len+1);
	if (pdata == NULL) {
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);

	int count = fread(pdata, len, 1, fp);
	pdata[len] = 0;
	rlen = len;

	return pdata;
}

#define FORMAT_FILE_HEADER_LENGTH (512)

/*===========================================================================

DESCRIPTION
Read format file

DEPENDENCIES
None

RETURN VALUE
If the function succeeds, the return value is nonzero.
If the function fails, the return value is zero (0). 

===========================================================================*/
BOOL ReadFormatFile
(
	const char*    pname,     // File name to be read
	uint8**        pphdr,     // File header
	uint32         hdr_len,   // File header length
	uint8**        ppdata,    // File data
	uint32&        data_len   // File data length
)
{
	if (pname == NULL || pphdr == NULL 
		|| ppdata == NULL || hdr_len == 0) 
	{
		return FALSE;
	}

	*pphdr = NULL;
	*ppdata = NULL;
	data_len = 0;
	FILE* fp = NULL;
	uint32 total = 0;
	uint8* pdata = NULL;

	fp = fopen(pname, "rb");
	if (fp == NULL) {
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);
	total = ftell(fp);
	if (total <= hdr_len) {
		return FALSE;
	}

	// allocate buffer for the whole data
	NEW_ARRAY(pdata, uint8, total);
	if (pdata == NULL) {
		return FALSE;
	}
	// read total data 
	fseek(fp, 0, SEEK_SET);
	if (fread(pdata, total, 1, fp) != 1) {
		RELEASE_ARRAY(&pdata);
		return FALSE;
	}

	// read header
	NEW_ARRAY(*pphdr, uint8, hdr_len+1);
	if (*pphdr == NULL) {
		RELEASE_ARRAY(&pdata);
		return FALSE;
	}
	memset(*pphdr, 0, hdr_len+1);
	(*pphdr)[hdr_len] = 0;
	memcpy((*pphdr), pdata, hdr_len);

	// read data body 
	data_len = total - hdr_len;
	NEW_ARRAY(*ppdata, uint8, data_len+1);
	if (*ppdata == NULL) {
		RELEASE_ARRAY(&pdata);
		RELEASE_ARRAY(pphdr);
		return FALSE;
	}
	memset(*ppdata, 0, data_len+1);
	(*ppdata)[data_len] = 0;
	memcpy((*ppdata), pdata+hdr_len, data_len);
	RELEASE_ARRAY(&pdata);

	return TRUE;
}

BOOL ReadNVDataFile
(
	const char* pname, 
	uint8** pphdr, 
	uint8** ppdata, 
	uint32& data_len
)
{
	return ReadFormatFile(
				pname, 
				pphdr, 
				FORMAT_FILE_HEADER_LENGTH, 
				ppdata, 
				data_len
				);
}

BOOL ReadImgFiles
(
	const char* pname, 
	uint8** pphdr, 
	uint8** ppdata, 
	uint32& data_len
)
{
	return ReadFormatFile(
				pname, 
				pphdr, 
				FORMAT_FILE_HEADER_LENGTH, 
				ppdata, 
				data_len
				);
}

BOOL ReadImgFiles( uint8* pbuf, 
				   uint32 buf_len,  
				   uint8** pphdr, 
				   uint8** ppdata, 
				   uint32& data_len)
{
	if (pbuf == NULL || pphdr == NULL || ppdata == NULL) {
		return FALSE;
	}

	// to be continue ...

	return FALSE;
}

BOOL WriteImgFiles
(
	const char* pname, 
	uint8* phdr, 
	uint32 hdr_len, 
	uint8* pdata, 
	uint32 data_len
)
{
	if (pname == NULL || phdr == NULL || pdata == NULL) {
		return FALSE;
	}

	FILE* fp = fopen(pname, "wb");
	if (fp == NULL) {
		return FALSE;
	}

	fwrite(phdr, hdr_len, 1, fp);
	fwrite(pdata, data_len, 1, fp);
	fclose(fp);

	return TRUE;
}

/*=============================================================================

DESCRIPTION
	Format a byte stream to string stream for printing.

DEPENDENCY
	None

RETURN
	NULL or formated string

SIDE EFFECT
	A chunk of memory will be allocated internally, and this chunk of memory
	should be released after the caller done with the return string.
	
=============================================================================*/
char* FormatBufferString
(
	const uint8* buffer,  // Raw data buffer to be formatted
	const uint32 len      // Length of raw data buffer
)
{
	// 00 01 02 03
	uint32 str_len = 3*len; // 3 chars for each byte, e.g. 1 --> "01 "
	char* pstream = NULL;
	NEW_ARRAY(pstream, char, str_len+1);
	char temp[4] = {0};
	if (pstream == NULL) {
		return NULL;
	}
	memset(pstream, 0, str_len);

	for (uint32 i=0; i<len; ++i) {
		memset(temp, 0, COUNTOF(temp));
		sprintf(temp, "%02x ", buffer[i]);
		strcat(pstream, temp);
	}

	return pstream;
}

/*=============================================================================

DESCRIPTION
	Print raw data stream and extra message to string stream.

DEPENDENCY
	None

RETURN
	None

SIDE EFFECT
	None
		
=============================================================================*/
void PrintBufferString
(
	const uint8* buffer,  // Raw data buffer    
	const uint32 len,     // Length of raw buffer
	const char* fmtstr,   // Extra information  
	...
)
{
#define MAX_BUF_LEN     (512)
	int   nBuf;
	char  szBuffer[MAX_BUF_LEN] = {0};

	// Handle format string
	va_list args;
	va_start(args, fmtstr);
	nBuf = _vsntprintf(szBuffer, COUNTOF(szBuffer), fmtstr, args);
	ASSERT(nBuf >= 0);
	va_end(args);

	// Handle buffer string
	char* pstream = NULL;
	pstream = FormatBufferString(buffer, len);
	uint32 slen = strlen(pstream);

	// Catenate the two parts
	uint32 total = nBuf + slen + 1;
	char* pbuf = NULL;
	NEW_ARRAY(pbuf, char, total);
	strcpy(pbuf, szBuffer);
	strcat(pbuf, pstream);
	RELEASE_ARRAY(&pstream);

	// Print out formatted buffer
	DEBUG(FILE_LINE, pbuf);

	RELEASE_ARRAY(&pbuf);
}


CString LoadStringEx(UINT id)
{
	CString str;
	if(!CStringTable::Instance().GetString(id, str))
	{
		str.Format("%d: Message not defined", id);
	}
	return str;
}

const CString&GetCurrentPath()
{
	static  CString strCurPath ;
	if(strCurPath.IsEmpty())
	{
		strCurPath = __argv[0] ;
		int nIndex = strCurPath.ReverseFind('\\') ;
		strCurPath = strCurPath.Left(nIndex+1);
	}
	return strCurPath;
}

HBITMAP  LoadBmpFormFile(LPCSTR lpFileName) 
{
	if(lpFileName == NULL)
		return NULL;

	CString strFullPath(lpFileName) ;
	if(lpFileName[1] != ':')
	{
		strFullPath = GetCurrentPath()+lpFileName;
	}
	int nCount = 0 ;
	HBITMAP hBitmap = NULL ;
	while(hBitmap == NULL)
	{
		nCount++;
		hBitmap = (HBITMAP)LoadImage(AfxGetInstanceHandle(),strFullPath,IMAGE_BITMAP,0,0,LR_LOADFROMFILE) ;
		if(nCount>4)
			break ;
		if(hBitmap == NULL)
			Sleep(1) ;
	}
	return hBitmap ;
}

HICON  LoadIconFormFile(LPCSTR lpFileName) 
{
	if(lpFileName == NULL)
		return NULL;

	CString strFullPath(lpFileName) ;
	if(lpFileName[1] != ':')
	{
		strFullPath = GetCurrentPath()+lpFileName;
	}
	int nCount = 0 ;
	HICON hIcon = NULL ;
	while(hIcon == NULL)
	{
		nCount++;
		hIcon = (HICON)LoadImage(AfxGetInstanceHandle(),strFullPath,IMAGE_ICON,0,0,LR_LOADFROMFILE) ;
		if(nCount>4)
			break ;
		if(hIcon == NULL)
			Sleep(1) ;
	}
	return hIcon ;
}

#ifdef FEATURE_TPST_SSID
/*=============================================================================

DESCRIPTION
Parse the webs_config for Y580.add by jie.li 20120830

DEPENDENCY
None

RETURN
None

SIDE EFFECT
None

=============================================================================*/
BOOL websconfigParse(uint8* pXmlBuf, WebsXMLInfo* pxmlinfo,int iFlag)
{
	TiXmlDocument* m_pXmlDoc = NULL;
	TiXmlNode* node = 0;
	TiXmlNode* parentnode = 0;
	TiXmlElement* todoElement = 0;
	TiXmlElement* itemElement = 0;
	const char* tText = NULL;
	int i_temp=0;

	m_pXmlDoc = new TiXmlDocument();
	m_pXmlDoc->Parse((const char*)pXmlBuf);

	node = m_pXmlDoc->FirstChild( "webs_config" );
	if (node == NULL)
	{
		return FALSE;
	}
	todoElement = node->ToElement();
	if (todoElement == NULL)
	{
		return FALSE;
	}

	node = todoElement->FirstChildElement();
	if (node == NULL)
		return FALSE;
	tText = node->Value();
	while(node)
	{
		todoElement = node->ToElement();
		node = todoElement->FirstChildElement();
		if (node)
			itemElement = node->ToElement();
		else 
			break;

		if(strcmp(tText, "device")==0 ) 
		{
			break;
		}
		else
		{
			node = todoElement->NextSibling();
			if(node)
				tText = node->Value();
		}
	}

	node = todoElement->FirstChild( "dev_name" );
	todoElement = node->ToElement();
	if (todoElement == NULL)
	{
		return FALSE;
	}
	tText = todoElement->GetText();
	strcpy(pxmlinfo->webs_dev_name,tText);

	node = todoElement->Parent();
	todoElement = node->ToElement();

	node = todoElement->FirstChild( "cust_model_name" );
	todoElement = node->ToElement();
	if (todoElement == NULL)
	{
		return FALSE;
	}
	tText = todoElement->GetText();
	strcpy(pxmlinfo->webs_cust_model_name,tText);
	tText= todoElement->Attribute( "Ext_SSID_Rule" );
	if(tText == NULL)
	{
		strcpy(pxmlinfo->webs_Ext_SSID,"MAC_L4");
	}
	else
	{
		strcpy(pxmlinfo->webs_Ext_SSID,tText);
	}

	if (m_pXmlDoc)
	{
		delete m_pXmlDoc;
		m_pXmlDoc = NULL;
	}

	return TRUE;
}
#endif