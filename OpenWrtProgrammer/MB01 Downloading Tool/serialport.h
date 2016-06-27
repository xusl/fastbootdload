/*=============================================================================
                        *Serial port communication*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-09-13  jianwen.he      Init first version

=============================================================================*/

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
#include <Windows.h>
#include "stdafx.h"


//#include "../define/stdafx.h"

/***************************************************************************/
#ifndef Q_OS_WIN32

#include <stdio.h>
//#include <termios.h>
#include <errno.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>
//#include <sys/select.h>
#include <string.h>
#include <fcntl.h>

#endif //Q_OS_WIN32
/***************************************************************************/
typedef enum {
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



#define uint8		unsigned char
#define uint32		unsigned int
#define uint16 unsigned short
#define OK 0
#define SUCCESS(result) ((result == OK) ? TRUE : FALSE)
#define FAILURE(result) ((result != OK) ? TRUE : FALSE)

#ifndef byte
typedef unsigned char byte;
#endif


#define PORT_TIMEOUT  100			// timeout each operation default


class CSerialPort
{
public:
	CSerialPort(void);
	CSerialPort(COMMTIMEOUTS timeout);
	~CSerialPort(void);

	TResult Open(unsigned short uPort = 4, int nBaud = 115200,
		int nParity = 1, int nStopbits = 1,
		int nDatabits = 8);

	TResult Close(void);
	TResult Send(byte* pBuff, size_t lSize, size_t *pCount);
	TResult Receive(byte* pBuff, size_t lSize, size_t *pCount);
	TResult ClearBuffer(void);
	TResult SetTimeOut(int Millisecond = PORT_TIMEOUT);
	TResult SetTimeOut(COMMTIMEOUTS timeout);
	unsigned int	GetTimeOut(void);
	unsigned short  GetComPort(void);

private:
	int    m_nPort;
	HANDLE m_hComm;
	int    m_TimeOut;
	COMMTIMEOUTS m_CommTimeout;
	OVERLAPPED m_WriteOverlapped;
	OVERLAPPED m_ReadOverlapped;
};
/*
class CSerialPort
{
public:
    CSerialPort(void);
    CSerialPort(CSerialPort &serialPort);
    ~CSerialPort(void);

#ifdef Q_OS_WIN32
    CSerialPort(COMMTIMEOUTS timeout);
    TResult Open(unsigned short uPort = 4, int nBaud = 115200,
                 int nParity = 1, int nStopbits = 1,
                 int nDatabits = 8);
   TResult SetTimeOut(COMMTIMEOUTS timeout);
#else
   TResult Open(const char *Dev);                                           //open the device   
#endif

    TResult Close(void);
    TResult Send(uint8* pBuff, uint32 lSize, uint32 *pCount);
    TResult Receive(uint8* pBuff, uint32 lSize, uint32 *pCount);
    TResult ClearBuffer(void);
    TResult SetTimeOut(uint32 Millisecond = 5000);

    unsigned int	GetTimeOut(void);
    unsigned short      GetComPort(void);
	char*           GetDevice();

    bool IsOpen();                                                              //device is open?
#ifndef Q_OS_WIN32
    int  SetDevice(int baud, int databits, int stopbits, int parity);           //set device
#endif

private:
    bool     m_bPortOpen;           //device is open?
    int      m_fd;
    int      m_nPort;
    int      m_TimeOut;
    char     m_strDevice[128];

#ifdef Q_OS_WIN32
    HANDLE	 m_hComm;
    COMMTIMEOUTS m_CommTimeout;
    OVERLAPPED m_WriteOverlapped;
    OVERLAPPED m_ReadOverlapped;
#else
    //struct termios Posix_CommConfig;
    //struct timeval Posix_Timeout;
   // struct timeval Posix_Copy_Timeout;
#endif //Q_OS_WIN32
};*/

#endif //__SERIALPORT_H__


