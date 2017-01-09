/*=============================================================================
                        *Serial Port Communication class*
DESC:

CHANGE HISTORY:
when        who             what
----------  ---------       --------------------------------------------------
2010-09-13  jianwen.he      Init first version

=============================================================================*/

#include "serialport.h"
#include "stdio.h"


#define RECEIVE_BUFFER_SIZE  4096

#define SEND_BUFFER_SIZE (1024*1024)

#define PORT_DATA_MAX_SIZE (1024*1024)		// set the max data size for port operation.


CSerialPort::CSerialPort(void)
{
    m_nPort = -1;
    m_bPortOpen = false;
    m_fd = -1;
	memset(m_strDevice, 0, 128);

#ifdef Q_OS_WIN32
    m_hComm = NULL;
    memset(&m_WriteOverlapped, 0, sizeof(OVERLAPPED));
    memset(&m_ReadOverlapped, 0, sizeof(OVERLAPPED));
    m_TimeOut = 5000;  // default timeout
    this->m_CommTimeout.ReadIntervalTimeout = 0;
    this->m_CommTimeout.ReadTotalTimeoutMultiplier = 1;
    this->m_CommTimeout.ReadTotalTimeoutConstant = 5000;
    this->m_CommTimeout.WriteTotalTimeoutMultiplier = 1;
    this->m_CommTimeout.WriteTotalTimeoutConstant = 5000;
#else

#endif

}

CSerialPort::CSerialPort(CSerialPort &serialPort)
{
    m_bPortOpen = serialPort.m_bPortOpen;
    m_fd = serialPort.m_fd;
#ifdef Q_OS_WIN32
#else
    memcpy(&Posix_Timeout, &serialPort.Posix_Timeout, sizeof(struct timeval));
    memcpy(&Posix_Copy_Timeout, &serialPort.Posix_Copy_Timeout, sizeof(struct timeval));
    memcpy(&Posix_CommConfig, &serialPort.Posix_CommConfig, sizeof(struct termios));
#endif
}

#ifdef Q_OS_WIN32
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
#endif

CSerialPort::~CSerialPort(void)
{
	Close();	
}

#ifdef Q_OS_WIN32
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

#else

TResult CSerialPort::Open(const char *Dev)
{
    typeqDebug("\n###openport = %s####\n", Dev);
	if (m_fd >= 0)
	{
		Close();
	}
	
    int fd = open(Dev, O_RDWR|O_NOCTTY);//|O_NDELAY );
    if (-1 == fd)
    {
		memset(m_strDevice, 0, 128);
        perror("Can''t Open Serial Port");
        return EFAILED;
    }
    else
    {
		m_fd = fd;
		SetTimeOut(5000);
        m_bPortOpen = true;
		memcpy(m_strDevice, Dev, strlen(Dev));		
        typeqDebug("COMID:%d\n",m_fd);
        SetDevice(115200, 8, 1, 1);
        return EOK;
    }

    return EFAILED;
}

#endif //Q_OS_WIN32

char*  CSerialPort::GetDevice()
{
	return m_strDevice;
}

#ifndef Q_OS_WIN32
//设置串口，波特率，数据位，停止位，校验
int CSerialPort::SetDevice(int baud, int databits, int stopbits, int parity)
{
        typeqDebug("setport:comid:%d\n", m_fd);
        int baudrate;
        struct   termios   newtio;

        switch(baud)
        {
        case 300:
                baudrate = B300;
                break;
        case 600:
                baudrate = B600;
                break;
        case 1200:
                baudrate = B1200;
                break;
        case 2400:
                baudrate = B2400;
                break;
        case 4800:
                baudrate = B4800;
                break;
        case 9600:
                baudrate = B9600;
                break;
        case 19200:
                baudrate = B19200;
                break;
        case 38400:
                baudrate = B38400;
                break;
        case 115200:
                baudrate = B115200;
                break;
        default :
                baudrate = B9600;
                break;
        }
        tcgetattr(m_fd, &newtio);
        bzero(&newtio, sizeof(newtio));
           //setting   c_cflag
        newtio.c_cflag   &=~CSIZE;

        switch (databits) /*设置数据位数*/
        {
        case 7:
                newtio.c_cflag |= CS7; //7位数据位
                break;
        case 8:
                newtio.c_cflag |= CS8; //8位数据位
                break;
        default:
                newtio.c_cflag |= CS8;
                break;
        }

        switch (parity) //设置校验
        {
        case 'n':
        case 'N':
                newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
                newtio.c_iflag &= ~INPCK;     /* Enable parity checking */
                break;
        case 'o':
        case 'O':
                newtio.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
                newtio.c_iflag |= INPCK;             /* Disnable parity checking */
                break;
        case 'e':
        case 'E':
                newtio.c_cflag |= PARENB;     /* Enable parity */
                newtio.c_cflag &= ~PARODD;   /* 转换为偶效验*/
                newtio.c_iflag |= INPCK;       /* Disnable parity checking */
                break;
        case 'S':
        case 's':  /*as no parity*/
                newtio.c_cflag &= ~PARENB;
                newtio.c_cflag &= ~CSTOPB;break;
        default:
                newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
                newtio.c_iflag &= ~INPCK;     /* Enable parity checking */
                break;
        }

        switch (stopbits)//设置停止位
        {
        case 1:
                newtio.c_cflag &= ~CSTOPB;  //1
                break;
        case 2:
                newtio.c_cflag |= CSTOPB;  //2
                break;
        default:
                newtio.c_cflag &= ~CSTOPB;
                break;
        }

        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 0;
        newtio.c_cflag |= (CLOCAL|CREAD);
        newtio.c_oflag|=OPOST;
        newtio.c_iflag &=~(IXON|IXOFF|IXANY);
        cfsetispeed(&newtio,baudrate);
        cfsetospeed(&newtio,baudrate);
        tcflush(m_fd, TCIFLUSH);
        if (tcsetattr(m_fd,TCSANOW,&newtio) != 0)
        {
                perror("SetupSerial 3");
                return -1;
        }
        return 0;
}
#endif


TResult CSerialPort::Close(void)
{
#ifdef Q_OS_WIN32
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

#else

    close(m_fd);
    m_bPortOpen = false;
    m_fd = -1;

#endif  //Q_OS_WIN32
    return EOK;
}


TResult CSerialPort::Send(uint8* pBuff, uint32 lSize, uint32 *pCount)
{
    ClearBuffer();
#ifdef Q_OS_WIN32
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

#else

    typeqDebug("write:%s\n", pBuff);
    uint32 n =  write(m_fd, pBuff, lSize);
    typeqDebug("Write Ret:%d\n", n);

    *pCount = n;

    if (n != lSize)
        return EPORTSEND;

#endif //Q_OS_WIN32

    return EOK;
}


TResult CSerialPort::Receive(uint8* pBuff, uint32 lSize, uint32 *pCount)
{
#ifdef Q_OS_WIN32
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
#else

// typeqDebug("read\n");
    int no = 0;
    int rc;
    int rcnum = lSize;
    struct timeval tv;
    fd_set readfd;

    FD_ZERO(&readfd);
    FD_SET(m_fd,&readfd);

    Posix_Timeout = Posix_Copy_Timeout;
    rc = select(m_fd+1, &readfd, NULL, NULL, &Posix_Timeout);
//  typeqDebug("read:m_fd=%d,rc=%d\n",m_fd,rc);

    if(rc>0)
    {
        while(lSize)
        {
            rc = read(m_fd, pBuff, 1);
//			typeqDebug("read:%c\n",buf[no]);
            if(rc>0)
                no = no+1;
            else
            {
                typeqDebug("read:buf=%s\n",pBuff);
                return (TResult)(no+1);
            }
            lSize = lSize-1;
        }

        if(no != rcnum)
            return EPORTRECEIVE;            //如果收到的长度与期望长度不一样，返回-1
        typeqDebug("read:buf=%s\n", pBuff);
        *pCount = rcnum;
        return EOK;                         //收到长度与期望长度一样，返回长度
    }
    else
    {
        return EPORTRECEIVE;
    }
    return EPORTRECEIVE;

#endif //Q_OS_WIN32
    return EOK;
}


TResult CSerialPort::ClearBuffer(void)
{
#ifdef Q_OS_WIN32
    if (m_hComm == NULL)
    {
        return EFAILED;
    }

    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);

    if (FlushFileBuffers(m_hComm) == 0)
    {
        return EFAILED;
    }

#else

    typeqDebug("Flush\n");
    tcflush(m_fd, TCIOFLUSH);

#endif    //Q_OS_WIN32
    return EOK;
}

bool CSerialPort::IsOpen()
{
    return m_bPortOpen;
}

TResult CSerialPort::SetTimeOut(uint32 Millisecond)
{
    TResult result = EOK;

#ifdef Q_OS_WIN32
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

#else   

    m_TimeOut = Millisecond;
    Posix_Copy_Timeout.tv_sec = Millisecond / 1000;
    Posix_Copy_Timeout.tv_usec = Millisecond % 1000;
    if (IsOpen())
    {
        tcgetattr(m_fd, &Posix_CommConfig);
        Posix_CommConfig.c_cc[VTIME] = Millisecond/100;
        tcsetattr(m_fd, TCSAFLUSH, &Posix_CommConfig);
    }

#endif

    return result;
}

#ifdef Q_OS_WIN32
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
#endif //Q_OS_WIN32

uint32 CSerialPort::GetTimeOut(void)
{
	return this->m_TimeOut;
}

uint16 CSerialPort::GetComPort(void)
{
	return this->m_nPort;
}


