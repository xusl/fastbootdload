// ziUsbDisk.h: interface for the CziUsbDisk class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIUSBDISK_H__01EF7B4E_EAB5_45F1_8805_08830B925E18__INCLUDED_)
#define AFX_ZIUSBDISK_H__01EF7B4E_EAB5_45F1_8805_08830B925E18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <QString>

#include "ziUsbDevice.h"


#ifdef Q_OS_WIN32
//===================================== SCSI ============================================
#include <WINIOCTL.H>
// macro
#define BIT(x, value)					((value) & (1<<(x)))      //ȡλ
#define SCSI_IOCTL_DATA_OUT				0					  
#define	SCSI_IOCTL_DATA_IN				1
#define IOCTL_SCSI_BASE					FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_PASS_THROUGH			CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define TIMEOUT_VALUE					100

//struct
typedef struct _SCSI_PASS_THROUGH {
	USHORT m_usLength;
	UCHAR m_cScsiStatus;
	UCHAR m_cPathId;
	UCHAR m_cTargetId;
	UCHAR m_cLun;
	UCHAR m_cCdbLength;
	UCHAR m_cSenseInfoLength;
	UCHAR m_cDataIn;
	ULONG m_ulDataTransferLength;
	ULONG m_ulTimeOutValue;
	ULONG m_ulDataBufferOffset;
	ULONG m_ulSenseInfoOffset;
	UCHAR m_pbCdb[16];
}SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;

typedef struct _SCSI_PASS_THROUGH_DIRECT {
    USHORT m_usLength;
    UCHAR m_cScsiStatus;
    UCHAR m_cPathId;
    UCHAR m_cTargetId;
    UCHAR m_cLun;
    UCHAR m_cCdbLength;
    UCHAR m_cSenseInfoLength;
    UCHAR m_cDataIn;
    ULONG m_ulDataTransferLength;
    ULONG m_ulTimeOutValue;
    PVOID m_pDataBuffer;
    ULONG m_ulSenseInfoOffset;
    UCHAR m_pbCdb[16];
}SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFER {
	SCSI_PASS_THROUGH m_strSpt;
	ULONG             m_ulFiller;      // realign buffers to double word boundary
	UCHAR             m_pbSenseBuf[32];
	UCHAR             m_pbDataBuf[64 * 1024];	
} SCSI_PASS_THROUGH_WITH_BUFFER, *PSCSI_PASS_THROUGH_WITH_BUFFER;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER{
	SCSI_PASS_THROUGH_DIRECT m_strSptd;
    ULONG             m_ulFiller;      // realign buffer to double word boundary
    UCHAR             m_pbSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct _SCSIINQUIRY
{
    unsigned char  m_cDeviceType    :5;     //��Χ�豸����
    unsigned char  m_cReserved0     :3;
	
    unsigned char  m_cReserved1     :7;
    unsigned char  m_cRmb           :1;     //�̷��ɼ���־
	
    unsigned char  m_cAnsiVersion   :3;
    unsigned char  m_cEcmaVersion   :3;
    unsigned char  m_cIsoVesion     :2;
	
    unsigned char  m_cRepDataFormat :4;
    unsigned char  m_cReserved2     :4;
	
    unsigned char  m_cAdditionalLen;
    unsigned char  m_cReserved3[3];
    unsigned char  m_pbVendorInfo[8];        //V_NAME
    unsigned char  m_pbProductInfo[16];        //P_NAME  
    unsigned char  m_pbProductRevisionlevel[4]; //�̼��汾��
	
    unsigned char  m_pbVendorSpec[20];
    unsigned char  m_cReserved4;
    unsigned char  m_pbVendorSpecPara[64];
}SCSIINQUIRY, *PSCSIINQUIRY;
//===================================== SCSI END ==========================================

class CziUsbDisk : public CziUsbDevice  
{
public:

	void CloseDeviceHandle();
	//���캯��
	CziUsbDisk();
	CziUsbDisk(LPCSTR szDriverName);
	CziUsbDisk(char cUdiskName);
	CziUsbDisk(char cUdiskName, HANDLE hDevice);
	virtual ~CziUsbDisk();
	
	void SetUdiskName(char cUdiskName); //�����̷�
	void SetDriverName(LPCSTR szDriverName);

	//U���������
	virtual BOOL UDiskIO(
		PVOID     pCBD,					//CBDָ����
		DWORD     dwCBDLen,					//CBDָ����ĳ���					  
		DWORD     dwDataTransferLength = 0,     //���ݴ��ͳ���
		PVOID     pBuff = NULL,				//����ָ�룬������ΪNULL	  					  			
		BYTE      cDataIn = SCSI_IOCTL_DATA_IN, //���ݴ��䷽�� SCSI_IOCTL_DATA_OUT OR SCSI_IOCTL_DATA_IN =1
		//�������豸���䵽����ΪSCSI_IOCTL_DATA_IN					  
		PDWORD    pdwRetLength = NULL,               //ʵ�ʷ������ݵĳ��ȣ�
        BYTE      cLun = 0                            //����������豸��LUN��
		);
	
	BOOL GetDeviceHandle();          //����豸���
	
protected:
	SCSIINQUIRY m_strScsiinquiry;
	char m_cUdiskName;               //�̷�
    QString m_szDriverName;
	
};

#endif // !defined(AFX_ZIUSBDISK_H__01EF7B4E_EAB5_45F1_8805_08830B925E18__INCLUDED_)

#endif
