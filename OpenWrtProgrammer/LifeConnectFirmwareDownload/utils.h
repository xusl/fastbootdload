/*=============================================================================
DESC:

CHANGE HISTORY:
when        who        what
----------  ---------  --------------------------------------------------------
2009-02-07  dawang.xu  init first version

=============================================================================*/
#ifndef __UTILS_H__
#define __UTILS_H__

#include "comdef.h"
//#include "typedef.h"
//#include "Error.h"
#include <vector>
using std::vector;
//#pragma   comment(lib, "TCL_MESDB_VC.lib")

inline void int2bytes
(
	int value,
	byte buf[4]
)
{
	buf[0] = value & 0xFF;
	buf[1] = (value >> 8) & 0xFF;
	buf[2] = (value >> 16) & 0xFF;
	buf[3] = (value >> 24) & 0xFF;
}

inline void bytes2int
(
	int& value,
	byte buf[4]
)
{
	value = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#define  B_PTR(var)  ((byte *) (void *) &(var))
#define  W_PTR(var)  ((word *) (void *) &(var))
#define  D_PTR(var)  ((dword *) (void *) &(var))


#define ISNULLPTR(ptr) ((ptr == NULL) ? TRUE : FALSE)

#define SETPTRNULL(pptr) {    \
	*pptr = NULL;		      \
	}

/* Release buffer memory and set ptr to NULL */
#define DELETE_IF(pptr)       \
do {                          \
	if (pptr && *pptr) {      \
		delete *pptr;         \
		*pptr = NULL;         \
	}                         \
} while (0)

/* Release array buffer memory and set ptr to NULL */
#define DELETE_ARRAY_IF(pptr) \
do {                          \
	if (pptr && *pptr) {      \
		delete [] *pptr;      \
		*pptr = NULL;         \
	}                         \
} while (0)

/* Get array element count */
#define COUNTOF(array) (sizeof(array)/sizeof(array[0]))

/* Get a mask with nth bits to be 1 */
#define LEFT_SHIFT_BITS(n) (1 << (n))

/* New a buffer and memset it with 0 if new ok */
#define NEW_ZERO_BUFFER(ptr, length)  \
do {                                  \
	ptr = (uint8*)new uint8[length];  \
	if (ptr != NULL) {                \
		memset(ptr, 0, length);       \
	}                                 \
} while (0)

/* Format byte stream to string stream for printing */

char * trim(char *str);
void AddSpaceToString(char *s_String, int i_Len);
void AddStarToString(char *s_String, int i_Len);

// Function LastErrorText
// A wrapper for FormatMessage : retrieve the message text for a system-defined error 
char *LastErrorText (void);


//v3.9.0
int InitReportFile(char s_CommRefStr[], char HDTPATH[]);
int ValidFolderExists(char *p_FileName);
int GenReport(int i_Slot, char *p_ErrorCode, int i_second);
int PortToPortNum(int Port, int &PortNum);
int GaliSNfromWIFI(char *TargetCam, char *SourceCam);
int GetCaliBit(char *InData, int InDataLen);
int GenSAV_NEW(char *s_IMEI, char *s_PCBASerial, char *s_PTS, char *s_PTM, char *s_HandsetIndRef,
				char *s_BenchName, DWORD dwTimeDiff, int i_Result, char *s_MAC, 
				char *s_SSID, char *s_WIFIpassword, char *s_MAC2, char *s_SSID2,char *s_memo);
int PTSTxtFileFind(char *lpPath, char *FileName);
int GetBDAddr(char *p_BDAddr);
int ValidCompanyID(char *p_CompanyID);
int ValidBDAddr(char *p_BDAddr);
int RefreshBDAddr(void);
int LoadFactoryBDRange(void);
int GetWIFIfromDatabase(int i_Slot, char *s_PCBNO, char s_WIFI[]);
int GetBDAddrFromFile(char *p_BDAddr);
#endif //__UTILS_H__
