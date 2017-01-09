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

#include "../define/stdafx.h"

/***************************************************************************/
#ifndef Q_OS_WIN32

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>

#endif //Q_OS_WIN32
/***************************************************************************/
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
    unsigned short  GetComPort(void);
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
    struct termios Posix_CommConfig;
    struct timeval Posix_Timeout;
    struct timeval Posix_Copy_Timeout;
#endif //Q_OS_WIN32
};

#endif //__SERIALPORT_H__


