/*
 * From TFTP.H
 * Changes made by Ph. Jounin : Constant have been prefixed by TFTP_
 */

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tftp.h	8.1 (Berkeley) 6/2/93
 */



#ifndef _ARPA_TFTP_H
#define _ARPA_TFTP_H
#include "global.h"
#include "md5.h"
/*
 * Trivial File Transfer Protocol (IEN-133)
 */
#define TFTP_SEGSIZE          512     /* data segment size */
#define TFTP_MAXSEGSIZE     16384       /* data segment size */
#define TFTP_MINSEGSIZE         8       /* data segment size */
/*
 * Packet types.
 */
#define TFTP_RRQ    01          /* read request */
#define TFTP_WRQ    02          /* write request */
#define TFTP_DATA   03          /* data packet */
#define TFTP_ACK    04          /* acknowledgement */
#define TFTP_ERROR  05          /* error code */
#define TFTP_OACK   06          /* option acknowledgement */


struct    tftphdr {
  short   th_opcode;      /* packet type */
  union {
          short   tu_block;   /* block # */
          short   tu_code;    /* error code */
          char    tu_stuff[1];    /* request packet stuff */
  } th_u;
  char    th_data[2];     /* data or error string */
};


#define  th_block    th_u.tu_block
#define  th_code     th_u.tu_code
#define  th_stuff    th_u.tu_stuff
#define  th_msg      th_data

#define TFTP_DATA_HEADERSIZE ( offsetof (struct tftphdr, th_data ) )
#define TFTP_ACK_HEADERSIZE  (  offsetof (struct tftphdr, th_block )  \
                            + sizeof ( ((struct tftphdr *) (0))->th_block) )

/*
 * Error codes.
 */
#define  EUNDEF      0       /* not defined */
#define  ENOTFOUND   1       /* file not found */
#define  EACCESS     2       /* access violation */
#define  ENOSPACE    3       /* disk full or allocation exceeded */
#define  EBADOP      4       /* illegal TFTP operation */
#define  EBADID      5       /* unknown transfer ID */
#define  EEXISTS     6       /* file already exists */
#define  ENOUSER     7       /* no such user */
#define  EBADOPTION  8        /* bad option */
#define  ECANCELLED 99      /* cancelled by administrator */


/*
 * options
 */
#define TFTP_OPT_TSIZE     "tsize"
#define TFTP_OPT_TIMEOUT   "timeout"
#define TFTP_OPT_BLKSIZE   "blksize"
#define TFTP_OPT_MCAST     "multicast"
#define TFTP_OPT_PORT      "udpport"

#define IS_OPT(s,opt)   (lstrcmpi (s, opt)==0)

#define PKTSIZE             TFTP_SEGSIZE+4
#define MAXPKTSIZE          TFTP_MAXSEGSIZE+4

#define  PLURAL(a)  ((a)>1 ? "s" : "")

#define  SizeOfTab(x)   (sizeof (x) / sizeof (x[0]))
#define  MakeMask(x)    ( 1 << (x) )

////////////////////////////////////////////////////////////
// The transfer/thread structure
// This whole structure is allocated for each transfer = for each thread
// (since a new thread is started for each new transfer)
////////////////////////////////////////////////////////////

enum e_TftpRetCode { TFTP_TRF_RUNNING, TFTP_TRF_SUCCESS, TFTP_TRF_STOPPED, TFTP_TRF_ERROR };

// settings for the current transfer
struct S_Trf_Settings
{
    DWORD           dwPacketSize;     // Size of a data packet : Note this is a key data
                                      // since if size of received packet != dwPacketSize
                                      // transfer is terminated
    DWORD           dwTimeout;        // Timeout
    unsigned        TftpMode;         // transfer mode, only binary mode is supported
    unsigned        ExtraWinSize;     // Data to sent without waiting for ACK
    DWORD           dwFileSize;       // -1 if not set
    DWORD           dwMcastAddr;      // Multicast address
};      // struct S_Trf_Settings

// Buffers
struct S_Trf_Buffers
{
    char           buf[MAXPKTSIZE];     // one piece of file
    char           ackbuf[PKTSIZE];
    SOCKADDR_STORAGE       from ;          // stack of address of remote peers
    char           cnx_frame[PKTSIZE];  // The 'connexion' datagram, contains file name
} ;         // struct S_Trf_Buffers

// transfer stats and progress bar : not mandatory
struct S_Trf_Statistics
{
    DWORD          dwTransfert;     // number of simultaned trf
    DWORD          dwTotalBytes;    // number of transferred bytes
    DWORD          dwTotalTimeOut;  // for stat
    DWORD          dwTransferSize;  // transfer size (read from SIZE option)
    time_t         StartTime;
    time_t         dLastUpdate;     // Last gauge update (seconds)
	DWORD          ret_code;
} ;         // struct S_Trf_Statistics
// control data
struct S_Trf_Control
{
    BOOL            bMCast;            // current transfer is multicast
    DWORD           nCount;            // current packet #
    DWORD           nLastBlockOfFile;  // Last block of file
    // DWORD          dwLastAckPacket; // Sorcerer's Apprentice Syndrome
    DWORD           nLastToSend;       // last sent packet
    DWORD           dwBytes;           // taille de la zone Data (th_stuff)
    unsigned        nTimeOut;          // # of consecutive timeouts
    unsigned        nRetries;          // same datagram resent # times
	int             nOAckPort;         // OAck should be sent on this port
};      // struct S_Trf_Control
// resource allocated for the transfer
struct S_Trf_Resource
{
    SOCKET          skt;                // socket de la connexion
    HANDLE          hFile;              // file handler
    // HWND            hGaugeWnd;          // Handler of Gauge Window
} ;         // struct S_Trf_Resource
// Thread management
struct S_Thread_Management
{
   // The thread which manages the transfer
   BOOL            bActive ;           // act as a semaphore
                                       // TRUE if thread is busy
   BOOL            bPermanentThread ;  // is thread permanent
   HANDLE          dwThreadHandle;
   DWORD           dwThreadHandleId;
   HANDLE          hEvent;            // Event used to activate permanent threads
   HWND            hWnd;              // identifiant of main window
   DWORD           dwTransferId;       // transfer id
   int             N;
} ;         // struct  S_Thread_Management
// MD5 computation data
struct S_Trf_MD5
{
   BOOL           bInit; // contains a copy of bMD5 settings (which may be changed)
   MD5_CTX        ctx;
   unsigned char ident [16];
} ;  // struct S_Trf_MD5


// The super structure and pointer to next block
struct LL_TftpInfo
{
    struct S_Thread_Management  tm;
    struct S_Trf_Resource       r;
    struct S_Trf_Control        c;
    struct S_Trf_Settings       s;
    struct S_Trf_Buffers        b;
    struct S_Trf_Statistics     st;
    struct S_Trf_MD5            m;
    HWND                        dlgHwnd;

    struct LL_TftpInfo *next;
} ;


// A new transfer has begun
struct S_TftpTrfNew
{
   DWORD dwTransferId;
   struct S_Trf_Statistics stat;
   int   opcode;
   SOCKADDR_STORAGE from_addr;
   char  szFile [_MAX_PATH];
};
// A transfer has ended
struct S_TftpTrfEnd
{
   DWORD dwTransferId;
   struct S_Trf_Statistics stat;
};

// Stat of current trf
struct subStats
{
   DWORD   dwTransferId;
   /* struct S_Trf_Statistics read from tfpt_struct */
   struct S_Trf_Statistics stat;
};
// transfer statistics
struct S_TftpTrfStat
{
    int nbTrf;      // nb de tranferts
    time_t  dNow;   // current time
    struct subStats t[101];
};

// abort a TFTP transfer
struct S_TftpKill
{
   DWORD dwTransferId;
} ;

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

enum
{
	// UI Messages
	UI_MESSAGE_BASE = (WM_USER + 1000),
	UI_MESSAGE_TFTPINFO,
};

enum e_Types
{
    // msg sent from the service to the GUI
    C_TFTP_TRF_NEW      =  100,
    C_TFTP_TRF_END,
    C_TFTP_TRF_STAT,
    C_DHCP_LEASE,
    C_TFTP_RPLY_SETTINGS,
    C_DHCP_RPLY_SETTINGS,
    C_REPLY_WORKING_DIR,    // working_dir
    C_SYSLOG,               // syslog message available
	C_REPLY_GET_SERVICES,   // get the running services
	C_REPLY_GET_INTERFACES,      // server ip addresses
	C_SERVICES_STARTED,          // init done
    C_REPLY_DIRECTORY_CONTENT,   // list directory
	C_DNS_NEW_ENTRY,              // DNS request
	C_CHG_SERVICE,				  // Service has been started/stopped

    // msg sent from the GUI to the service
    C_CONS_KILL_TRF     = 200,
    C_TFTP_TERMINATE,
    C_DHCP_TERMINATE,
	C_SYSLOG_TERMINATE,
	C_SNTP_TERMINATE,
	C_DNS_TERMINATE,
    C_TERMINATE,			// kill threads (terminating)
	C_SUSPEND,              // kill worker services
    C_START,				// start services
    C_DHCP_RRQ_SETTINGS,
    C_TFTP_RRQ_SETTINGS,
    C_DHCP_WRQ_SETTINGS,
    C_TFTP_WRQ_SETTINGS,
    C_TFTP_RESTORE_DEFAULT_SETTINGS,  // remove all settings
    C_TFTP_CHG_WORKING_DIR,			  // working_dir
    C_RRQ_WORKING_DIR,          // empty
    C_DELETE_ASSIGNATION,
	C_RRQ_GET_SERVICES,		// Request the running services
	C_RRQ_GET_DHCP_ALLOCATION,  // number of allocation
	C_RRQ_GET_INTERFACES,       // IP interfaces
    C_RRQ_DIRECTORY_CONTENT,
	C_TFTP_GET_FULL_STAT,		// request statistics

} ;

////////////////////////////////////////////////////////////
// End of transfer/thread structure
////////////////////////////////////////////////////////////
int nak(struct LL_TftpInfo *pTftp, int error);
int CreateIndexFile (void);
void StartTftpd32Services (void *);
void StopTftpd32Services (void);

// Access to console
int SendMsgRequest (int type,				// msg type
					const void *msg_stuff);	// data


void Tftpd32UpdateServices (void *lparam);

// Complex actions handled by console thread
void SendDirectoryContent (void);

// interaction between tftp and console (statistics)
DWORD WINAPI StartTftpTransfer (LPVOID pThreadArgs);
int ReportNewTrf (const struct LL_TftpInfo *pTftp);


#endif /* _ARPA_TFTP_H */
