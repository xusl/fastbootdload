// ziUsbDisk.cpp: implementation of the CziUsbDisk class.
//
//////////////////////////////////////////////////////////////////////

#include "ziUsbDisk.h"

#ifdef Q_OS_WIN32

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CziUsbDisk::CziUsbDisk()
{
	m_hDevice=INVALID_HANDLE_VALUE;
}

CziUsbDisk::CziUsbDisk(LPCSTR szDriverName)
{
	SetDriverName(szDriverName);
	GetDeviceHandle();
}

CziUsbDisk::CziUsbDisk(char cUdiskName)
{
	SetUdiskName(cUdiskName);
	GetDeviceHandle();
}

CziUsbDisk::CziUsbDisk(char cUdiskName, HANDLE hDevice)
{
	SetUdiskName(cUdiskName);
	m_hDevice = hDevice;
}

CziUsbDisk::~CziUsbDisk()
{
	CloseDeviceHandle();	
}

void CziUsbDisk::SetDriverName(LPCSTR szDriverName)
{
	m_szDriverName = szDriverName;
}

BOOL CziUsbDisk::GetDeviceHandle()
{
    m_hDevice = CreateFileA(m_szDriverName.toLatin1().data(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if(INVALID_HANDLE_VALUE==m_hDevice)
	{
		return FALSE;
	}
	return TRUE;
}

void CziUsbDisk::SetUdiskName(char cUdiskName)
{
	m_cUdiskName = cUdiskName;
	
	char szDriverName[100] = "\\\\.\\C:\0";		
	szDriverName[4] = cUdiskName;
	
	if (cUdiskName < 'A')        //取物理驱动器
	{
		sprintf(szDriverName, "\\\\.\\PHYSICALDRIVE%c", cUdiskName);		
	}
	m_szDriverName = szDriverName;
}

BOOL CziUsbDisk::UDiskIO(PVOID pCBD,  DWORD dwCBDLen,
						 DWORD dwDataTransferLength /* = 0 */,  
						 PVOID pBuff /* = NULL */,  
						 BYTE cDataIn /* = SCSI_IOCTL_DATA_IN */,   
						 PDWORD pdwRetLength /* = NULL   */,
						 BYTE cLun /* =0 */
						 )
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER    szSptdwb;//bulk-only协议包
	ZeroMemory(&szSptdwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
	szSptdwb.m_strSptd.m_usLength	= sizeof(SCSI_PASS_THROUGH_DIRECT);			
	szSptdwb.m_strSptd.m_cPathId = 0; 
	szSptdwb.m_strSptd.m_cTargetId = 0; 
	szSptdwb.m_strSptd.m_cLun = cLun; 
	szSptdwb.m_strSptd.m_cSenseInfoLength = 31; 
	szSptdwb.m_strSptd.m_ulTimeOutValue = TIMEOUT_VALUE; 
	szSptdwb.m_strSptd.m_ulSenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, 
		m_pbSenseBuf); 
	

	PDWORD pdwRetLen = NULL;
	DWORD dwRetLen;
	if(pdwRetLength != NULL)
	{
		pdwRetLen = pdwRetLength;
	}
	else
	{
		pdwRetLen = &dwRetLen;
	}

	if(dwDataTransferLength != 0 && pBuff == NULL)
	{
		return FALSE;
	}
	if(dwCBDLen == 0)
	{
		return FALSE;
	}

	szSptdwb.m_strSptd.m_cDataIn= cDataIn; 
	szSptdwb.m_strSptd.m_pDataBuffer = pBuff; 
	szSptdwb.m_strSptd.m_cCdbLength = (UCHAR)dwCBDLen;
	memcpy(szSptdwb.m_strSptd.m_pbCdb, pCBD, dwCBDLen);
	szSptdwb.m_strSptd.m_ulDataTransferLength = dwDataTransferLength;
	BOOL bRet = DeviceIoControl(m_hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&szSptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
		&szSptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER), pdwRetLen, NULL);
	ZeroMemory(&szSptdwb,sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
	if(!bRet)
	{
/*		CString strRet;
		strRet.Format("%d",GetLastError());
        AfxMessageBox(strRet);*/
	}
	return bRet;
}

void CziUsbDisk::CloseDeviceHandle()
{
	if(m_hDevice != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hDevice);	
		m_hDevice = INVALID_HANDLE_VALUE;
	}
}

#endif
