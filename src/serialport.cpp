/*=============================================================================
                        *Serial Port Communication class*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-09-13  jianwen.he      Init first version

=============================================================================*/
#include "StdAfx.h"
#include "serialport.h"
#include "stdio.h"
#include "log.h"


#define RECEIVE_BUFFER_SIZE  4096

#define SEND_BUFFER_SIZE (1024*1024)

#define PORT_DATA_MAX_SIZE (1024*1024)		// set the max data size for port operation.


CSerialPort::CSerialPort(void)
{
    m_nPort = -1;
    m_bPortOpen = false;
    m_fd = -1;
	memset(m_strDevice, 0, 128);

    m_hComm = NULL;
    memset(&m_WriteOverlapped, 0, sizeof(OVERLAPPED));
    memset(&m_ReadOverlapped, 0, sizeof(OVERLAPPED));
    m_TimeOut = 5000;  // default timeout
    this->m_CommTimeout.ReadIntervalTimeout = 0;
    this->m_CommTimeout.ReadTotalTimeoutMultiplier = 1;
    this->m_CommTimeout.ReadTotalTimeoutConstant = 5000;
    this->m_CommTimeout.WriteTotalTimeoutMultiplier = 1;
    this->m_CommTimeout.WriteTotalTimeoutConstant = 5000;
}

CSerialPort::CSerialPort(CSerialPort &serialPort)
{
    m_bPortOpen = serialPort.m_bPortOpen;
    m_fd = serialPort.m_fd;
}

CSerialPort::CSerialPort(COMMTIMEOUTS timeout)
{
	m_nPort = -1;
	m_hComm = NULL;
	memset(&m_WriteOverlapped, 0, sizeof(OVERLAPPED));
	memset(&m_ReadOverlapped, 0, sizeof(OVERLAPPED));
	memset(&this->m_CommTimeout, 0, sizeof(COMMTIMEOUTS));
	this->m_CommTimeout.ReadIntervalTimeout = timeout.ReadIntervalTimeout;
	this->m_CommTimeout.ReadTotalTimeoutMultiplier = timeout.ReadTotalTimeoutMultiplier;
	this->m_CommTimeout.ReadTotalTimeoutConstant = timeout.ReadTotalTimeoutConstant;
	this->m_CommTimeout.WriteTotalTimeoutMultiplier = timeout.WriteTotalTimeoutMultiplier;
	this->m_CommTimeout.WriteTotalTimeoutConstant = timeout.WriteTotalTimeoutConstant;
}

CSerialPort::~CSerialPort(void)
{
	Close();
}


TResult CSerialPort::Open(unsigned short uPort,
                          int nBaud,
                          int nParity,
                          int nStopbits,
                          int nDatabits)
{
    char  sComPort[20];

	// If current COM PORT has been used, so to close the PORT.
	if (NULL != m_hComm)
		Close();

	m_nPort = uPort;

	// Create events for READ and WRITE
	m_WriteOverlapped.hEvent = CreateEvent(NULL, true, false, NULL);
	m_ReadOverlapped.hEvent  = CreateEvent(NULL, true, false, NULL);
	if (NULL == m_WriteOverlapped.hEvent
		|| NULL == m_ReadOverlapped.hEvent)
	{
		Close();
		return EPORTOPEN;
	}

        sprintf(sComPort, "\\\\.\\COM%d", uPort);
        m_hComm = CreateFileA(sComPort,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);

	if (INVALID_HANDLE_VALUE == m_hComm)
	{
		Close();
		return EPORTOPEN;
	}

	//Set input/output buffer size
	if (false == SetupComm(m_hComm, RECEIVE_BUFFER_SIZE, SEND_BUFFER_SIZE))
	{
		Close();
		return EPORTOPEN;
	}

	DCB 			dcb;				 // define data control block
	memset(&dcb, 0, sizeof(DCB));

	if (false == GetCommState(m_hComm, &dcb))
	{
		Close();
		return EPORTOPEN;
	}

	dcb.DCBlength = sizeof(DCB);
	// Set baudrate
	dcb.BaudRate		= nBaud;
	dcb.Parity			= nParity;
	dcb.ByteSize		= nDatabits;
	dcb.StopBits		= nStopbits;
	dcb.fOutxCtsFlow	= false;
	dcb.fOutxDsrFlow	= false;
	dcb.fDtrControl 	= DTR_CONTROL_DISABLE;
	dcb.fRtsControl 	= RTS_CONTROL_DISABLE;
	dcb.fDsrSensitivity = false;
	dcb.fTXContinueOnXoff = false;
	dcb.fOutX			= false;
	dcb.fInX			= false;

	if (false == SetCommState(m_hComm, &dcb)) { //config com port param
		Close();
		return EPORTOPEN;
	}

	if (EOK != SetTimeOut(this->m_CommTimeout))
	{
		Close();
		return EPORTOPEN;
	}

	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	m_bPortOpen = true;
	return EOK;
}


char*  CSerialPort::GetDevice()
{
	return m_strDevice;
}


TResult CSerialPort::Close(void)
{
    if (m_hComm != NULL)
    {
        PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
        SetCommMask(m_hComm, 0 );  // Forces the WaitCommEvent to return
    }

    if (NULL != m_WriteOverlapped.hEvent)
    {
        CloseHandle(m_WriteOverlapped.hEvent);
    }

    if (NULL != m_ReadOverlapped.hEvent)
    {
        CloseHandle(m_ReadOverlapped.hEvent);
    }

    if (NULL != m_hComm)
    {
        CloseHandle(m_hComm);
    }

    // reset all member variable to initialized
    m_nPort = -1;
    m_hComm = NULL;
    m_TimeOut = 5000;  // default timeout
    memset(&m_WriteOverlapped, 0, sizeof(OVERLAPPED));
    memset(&m_ReadOverlapped, 0, sizeof(OVERLAPPED));
    m_bPortOpen = false;
    return EOK;
}


TResult CSerialPort::Send(uint8* pBuff, uint32 lSize, uint32 *pCount)
{
    ClearBuffer();

    DWORD BytesSent = 0, LastError;

    if (lSize > PORT_DATA_MAX_SIZE)
    {
        return EPORTSEND;
    }

    if (NULL != pCount)
    {
        *pCount = 0;
    }

    if (0 == lSize)
    {
        return EOK;
    }

    if (NULL == m_hComm)
    {
        return EPORTSEND;
    }

    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    if (!WriteFile(m_hComm, pBuff, lSize, &BytesSent, &m_WriteOverlapped))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            return EPORTSEND;
        }

        // wait for operation complete
        if (false == GetOverlappedResult(m_hComm, &m_WriteOverlapped, &BytesSent, true))
        {
            LastError = GetLastError();

            return EPORTSEND;
        }
    }

    if (pCount != NULL)
    {
         *pCount = BytesSent;
    }
    return EOK;
}


TResult CSerialPort::Receive(uint8* pBuff, uint32 lSize, uint32 *pCount)
{
    DWORD lHasRead = 0, LastError;

    if (lSize > PORT_DATA_MAX_SIZE)
    {
        return EINVALIDPARAM;
    }

    if (NULL != pCount)
    {
        *pCount = 0;
    }

    if (0 == lSize)
    {
        return EOK;
    }

    if (NULL == m_hComm)
    {
        return EPORTRECEIVE;
    }

    if (false == ReadFile(m_hComm, pBuff, lSize, &lHasRead, &m_ReadOverlapped))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            return EPORTRECEIVE;
        }

        // wait for operation complete
        if (false == GetOverlappedResult(m_hComm, &m_ReadOverlapped, &lHasRead, true))
        {
            LastError = GetLastError();
            switch (LastError)
            {
            case ERROR_GEN_FAILURE: //
                break;
            default:
                break;
            }

            return EPORTRECEIVE;
        }
    }

    if (pCount != NULL)
    {
            *pCount = lHasRead;
    }
    return EOK;
}


TResult CSerialPort::ClearBuffer(void)
{
    if (m_hComm == NULL)
    {
        return EFAILED;
    }

    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);

    if (FlushFileBuffers(m_hComm) == 0)
    {
        return EFAILED;
    }
    return EOK;
}

bool CSerialPort::IsOpen()
{
    return m_bPortOpen;
}

TResult CSerialPort::SetTimeOut(uint32 Millisecond)
{
    TResult result = EOK;

    COMMTIMEOUTS	to;

    if (m_hComm == NULL)
    {
        return EFAILED;
    }

    m_TimeOut = Millisecond;

    if (SUCCESS(result) && GetCommTimeouts(m_hComm, &to) == false)
    {
        result = EFAILED;
    }
    else
    {
        this->m_CommTimeout.ReadTotalTimeoutConstant = m_TimeOut;
        this->m_CommTimeout.WriteTotalTimeoutConstant = m_TimeOut;
    }

    if (SUCCESS(result) && SetCommTimeouts(m_hComm, &this->m_CommTimeout) == false)
    {	//set RW timeout
        result = EFAILED;
    }

    return result;
}


TResult CSerialPort::SetTimeOut(COMMTIMEOUTS timeout)
{
    if (m_hComm == NULL)
    {
        return EFAILED;
    }

    this->m_CommTimeout.ReadIntervalTimeout = timeout.ReadIntervalTimeout;
    this->m_CommTimeout.ReadTotalTimeoutMultiplier = timeout.ReadTotalTimeoutMultiplier;
    this->m_CommTimeout.ReadTotalTimeoutConstant = timeout.ReadTotalTimeoutConstant;
    this->m_CommTimeout.WriteTotalTimeoutMultiplier = timeout.WriteTotalTimeoutMultiplier;
    this->m_CommTimeout.WriteTotalTimeoutConstant = timeout.WriteTotalTimeoutConstant;

    if (!SetCommTimeouts(m_hComm, &this->m_CommTimeout))
    {
        return EFAILED;
    }
    return EOK;
}

uint32 CSerialPort::GetTimeOut(void)
{
	return this->m_TimeOut;
}

uint16 CSerialPort::GetComPort(void)
{
	return this->m_nPort;
}


