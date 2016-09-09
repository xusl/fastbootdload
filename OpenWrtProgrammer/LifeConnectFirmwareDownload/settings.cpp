//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

// registry key :
//       HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32

// some shortcurts
#include "StdAfx.h"
#include <stdio.h>
#include "Tftp.h"
#include "settings.h"

struct S_Tftpd32Settings sSettings =
{
	  ".",                   // Base directory
	  TFTP_TIMEOUT,          // default timeout
	  TFTP_RETRANSMIT,       // def retransmission7
	  0,                     // WinSize
	  SECURITY_STD,          // Security
	  TFTP_DEFPORT,          // Tftp Port
	  TRUE,                  // RFC 1782-1785 Negociation
	  FALSE,                 // PXE Compatibility
	  FALSE,                 // do not create dir.txt file
	  FALSE,                 // do not create MD5 file
	  TRUE,                  // Unix like files "/tftpboot/.."
	  FALSE,                 // Do not beep for long transfert
	  FALSE,                 // Virtual Root is not enabled
	  "",                    // do not filter TFTP'slistening interface
//	  TFTPD32_ALL_SERVICES,  // all services are enabled
	  0,  0,                 // use ports assigned by Windows
	  FALSE,                 // do not support port option
	  5,					 // after 5 seconds delete Tftp record
	  FALSE,				 // wait for ack of last TFTP packet
	  TRUE,					 // IPv4
	  TRUE,					 // IPv6

	  // unsaved
	  100,                   // Max Simultaneous Transfers
	  ".",                   // Working Directory
	  2000,                  // refresh Interval
};