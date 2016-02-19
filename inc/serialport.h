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

#include "StdAfx.h"
#include "define.h"
/***************************************************************************/
/***************************************************************************/
#define PORT_TIMEOUT  5000			// timeout each operation default

class CSerialPort
{
public:
    CSerialPort(void);
    CSerialPort(CSerialPort &serialPort);
    ~CSerialPort(void);

    CSerialPort(COMMTIMEOUTS timeout);
    TResult Open(unsigned short uPort = 4, int nBaud = 115200,
                 int nParity = 1, int nStopbits = 1,
                 int nDatabits = 8);
   TResult SetTimeOut(COMMTIMEOUTS timeout);

    TResult Close(void);
    TResult Send(uint8* pBuff, uint32 lSize, uint32 *pCount);
    TResult Receive(uint8* pBuff, uint32 lSize, uint32 *pCount);
    TResult ClearBuffer(void);
    TResult SetTimeOut(uint32 Millisecond = PORT_TIMEOUT);

    unsigned int	GetTimeOut(void);
    unsigned short  GetComPort(void);
	char*           GetDevice();

    bool IsOpen();                                                              //device is open?

private:
    bool     m_bPortOpen;           //device is open?
    int      m_fd;
    int      m_nPort;
    int      m_TimeOut;
	  char     m_strDevice[128];

    HANDLE	 m_hComm;
    COMMTIMEOUTS m_CommTimeout;
    OVERLAPPED m_WriteOverlapped;
    OVERLAPPED m_ReadOverlapped;
};

#endif //__SERIALPORT_H__


