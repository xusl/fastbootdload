//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File threading.h:    Manage threads
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////

#pragma once
//#include "async_log.h"
#include "log.h"

// Events created for main threads
struct S_ThreadMonitoring
{
    int     gRunning;    // thread status
    HANDLE  tTh;         // thread handle
    HANDLE  hEv;         // wake up event
    SOCKET  skt;         // Listening SOCKET
	int     bSoftReset;  // Thread will be reset
	BOOL    bInit;		 // inits are terminated
}  ;


struct S_RestartTable
{
	int oldservices;
	int newservices;
	int flapservices;
};


void StartTftpd32Services (void *);
void StopTftpd32Services (void);

// Access to console
int SendMsgRequest (int type,				// msg type
					const void *msg_stuff,	// data
					int size,				// size of data
					BOOL bBlocking,			// block thread until msg sent
					BOOL bRetain );			// retain msg if GUI not connected

BOOL Tftpd32ReadSettings (void);
BOOL Tftpd32SaveSettings (void);
BOOL Tftpd32DestroySettings (void);
void Tftpd32UpdateServices (void *lparam);

// Send the IP interfaces
int	AnswerIPList (void);

// Complex actions handled by console thread
void SendDirectoryContent (void);

// interaction between tftp and console (statistics)
DWORD WINAPI StartTftpTransfer (LPVOID pThreadArgs);
int ReportNewTrf (const struct LL_TftpInfo *pTftp);
void ConsoleTftpGetStatistics (void);

// Actions called by Scheduler
int PoolNetworkInterfaces (void);
int GetIPv4Address (const char *szIf, char *szIP);
