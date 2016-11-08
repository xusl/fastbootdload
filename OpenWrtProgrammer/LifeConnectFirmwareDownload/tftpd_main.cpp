//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - June 2006
// File tftp.c:   worker threads management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include <StdAfx.h>
#include "utils.h"
#include "tftp.h"

struct S_ThreadMonitoring tftpThreadMonitor;// [TH_NUMBER];

// number of permanent worker threads
#define TFTP_PERMANENTTHREADS 2

#define TFTP_MAXTHREADS     100

#undef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))


// #define DEB_TEST

#include <process.h>
#include <stdio.h>
#include "Tftp.h"
#include "settings.h"
#include "log.h"

// First item -> structure belongs to the module and is shared by all threads
struct LL_TftpInfo *pTftpFirst;
static int gSendFullStat=FALSE;		// full report should be sent
HWND    gWndHandle = NULL;
DeviceCoordinator *gDevCoordinator =NULL;
// statistics requested by console
// do not answer immediately since we are in console thread
// and pTftp data may change
void SetTftpGetStatistics (void)
{
	gSendFullStat = TRUE;
} //

int GetIPv4Address (const char *szIf, char *szIP);


// -----------------------
// Retrieve IPv4 assigned to an interface
// -----------------------
int GetIPv4Address (const char *szIf, char *szIP)
{
ULONG outBufLen;
IP_ADAPTER_ADDRESSES       *pAddresses=NULL, *pCurrAddresses;
//IP_ADAPTER_UNICAST_ADDRESS *pUnicast;
int Rc;
char szBuf [MAX_ADAPTER_DESCRIPTION_LENGTH+4];

	szIP[0]=0;
    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses (AF_INET,
							  GAA_FLAG_INCLUDE_PREFIX,
							  NULL,
							  pAddresses,
							 & outBufLen) == ERROR_BUFFER_OVERFLOW)
	 {
           free(pAddresses);
           pAddresses = (IP_ADAPTER_ADDRESSES *) malloc (outBufLen);
     }

    if (pAddresses == NULL) {
        return -1;
    }
    // Make a second call to GetAdapters Addresses to get the
    // actual data we want
	Rc = GetAdaptersAddresses (sSettings.bIPv6 ? AF_UNSPEC : AF_INET,
							   GAA_FLAG_INCLUDE_PREFIX,
							   NULL,
							   pAddresses,
							 & outBufLen);
    if (Rc == NO_ERROR)
	{
		for ( pCurrAddresses = pAddresses ; pCurrAddresses ; pCurrAddresses=pCurrAddresses->Next )
		{
			wsprintf (szBuf, "%ls", pCurrAddresses->Description);
			if (
				   lstrcmp (szBuf, sSettings.szTftpLocalIP) == 0
				&& pCurrAddresses->FirstUnicastAddress !=0
				)
			{SOCKADDR *sAddr = pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;
				lstrcpy ( szIP,
						  inet_ntoa (  ((struct sockaddr_in *) sAddr)->sin_addr ) );
				free (pAddresses);
				return 0;
			}
		}
	}
	free (pAddresses);
return -1;
} // GetIPv4Address

////////////////////////////////////////////////////////////
// TFTP daemon --> Runs at main level
////////////////////////////////////////////////////////////

static SOCKET TftpBindLocalInterface (void)
{
    SOCKET             sListenSocket = INVALID_SOCKET;
    int                Rc;
    ADDRINFO           Hints, *res;
    char               szServ[NI_MAXSERV];
    char               szIPv4 [MAXLEN_IPv6];

    memset(&Hints, 0, sizeof Hints);
    if ( sSettings.bIPv4 && ! sSettings.bIPv6 )
        Hints.ai_family = AF_INET;    // force IPv4
    else if (sSettings.bIPv6  &&  ! sSettings.bIPv4 )
        Hints.ai_family = AF_INET6;  // force IPv6
    else     Hints.ai_family = AF_UNSPEC;    // use IPv4 or IPv6, whichever

    Hints.ai_socktype = SOCK_DGRAM;
    Hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    wsprintf (szServ, "%d", sSettings.Port==0 ? TFTP_DEFPORT : sSettings.Port );
    // retrieve IPv4 address assigned to a physical Interface
    if (sSettings.bTftpOnPhysicalIf)
    {
        Rc = GetIPv4Address (sSettings.szTftpLocalIP, szIPv4);
        if (Rc!=0)  return sListenSocket;
        Rc = getaddrinfo ( szIPv4,
                          sSettings.Port == 69 ? "tftp" : szServ,
                          &Hints, &res);
        if (Rc!=0)  return sListenSocket;
    }
    else
    {
        Rc = getaddrinfo ( isdigit (sSettings.szTftpLocalIP[0]) ? sSettings.szTftpLocalIP : NULL,
                          sSettings.Port == 69 ? "tftp" : szServ,
                          &Hints, &res);
        if (Rc!=0)
        {
            if (GetLastError() == WSAHOST_NOT_FOUND)
            {
                LOGE ("Error %d\n%s\n\n"
                      "Tftpd32 tried to bind the %s port\n"
                      "to the interface %s\nwhich is not available for this host\n"
                      "Either remove the %s service or suppress %s interface assignation",
                      GetLastError (), LastErrorText (),
                      "tftp", sSettings.szTftpLocalIP, "tftp", sSettings.szTftpLocalIP);
            }
            else
            {
                LOGE ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
            }
            return sListenSocket;
        }
    }
    // bind it to the port we passed in to getaddrinfo():

    sListenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sListenSocket == INVALID_SOCKET)
    {
        LOGE ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
        freeaddrinfo (res);
        return sListenSocket;
    }

    // REUSEADDR option in order to allow thread to open 69 port
    if (sSettings.bPortOption==0)
    {
        int True=1;
        Rc = setsockopt (sListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
        if (Rc==0 )
            LOGW ("Port %d may be reused" , sSettings.Port);
        else
            LOGW("setsockopt error");
    }

    // if family is AF_UNSPEC, allow both IPv6 and IPv4 by disabling IPV6_ONLY (necessary since Vista)
    // http://msdn.microsoft.com/en-us/library/windows/desktop/bb513665(v=vs.85).aspx
    if (sSettings.bIPv4 && sSettings.bIPv6)
    {
        int False=0;
        Rc = setsockopt(sListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)& False, sizeof False );
        // do not check Rc, since XP does not support this settings -> error
    }

    // bind the socket to the active interface
    Rc = bind(sListenSocket, res->ai_addr, res->ai_addrlen);

    if (Rc == INVALID_SOCKET)
    {
        char szAddr[MAXLEN_IPv6]="unknown", szServ[NI_MAXSERV]="unknown";
        int KeepLastError = GetLastError();
        // retrieve localhost and port
        getnameinfo(res->ai_addr, res->ai_addrlen,
                    szAddr, sizeof szAddr,
                    szServ, sizeof szServ,
                    NI_NUMERICHOST | AI_NUMERICSERV );
        SetLastError(KeepLastError); // getnameinfo has reset LastError !
        // 3 causes : access violation, socket already bound, bind on an adress
        switch (GetLastError ())
        {
        case WSAEADDRNOTAVAIL :   // 10049
            LOGE ("Error %d\n%s\n\n"
                  "Tftpd32 tried to bind the %s port\n"
                  "to the interface %s\nwhich is not available for this host\n"
                  "Either remove the %s service or suppress %s interface assignation",
                  GetLastError (), LastErrorText (),
                  "tftp", sSettings.szTftpLocalIP, "tftp", sSettings.szTftpLocalIP);
            break;
        case WSAEINVAL :
        case WSAEADDRINUSE :
            LOGE ("Error %d\n%s\n\n"
                  "Tftpd32 can not bind the %s port\n"
                  "an application is already listening on this port",
                  GetLastError (), LastErrorText (),
                  "tftp" );
            break;
        default :
            LOGE ("Bind error %d\n%s", GetLastError (), LastErrorText () );
            break;
        } // switch error type
        closesocket (sListenSocket);
        LOGE ("bind port to %s port %s failed\n", szAddr, szServ);
    }
    freeaddrinfo (res);
    return Rc == INVALID_SOCKET ? Rc : sListenSocket;
} // TftpBindLocalInterface


static void PopulateTftpdStruct (struct LL_TftpInfo *pTftp)
{
    struct LL_TftpInfo *pTmp;
    static DWORD TransferId=467;    // unique identifiant

    if (TransferId == INVALID_TRANSFERID)
        TransferId++;

    // init or reinit struct
    pTftp->s.dwTimeout = sSettings.Timeout;
    pTftp->s.dwPacketSize = TFTP_SEGSIZE;  // default
    pTftp->r.skt = INVALID_SOCKET;
    pTftp->r.hFile = INVALID_HANDLE_VALUE;
    pTftp->c.bMCast = FALSE;    // may be updated later
    pTftp->c.nOAckPort = 0;		// use classical port for OAck
    pTftp->tm.dwTransferId = TransferId++;

    // init statistics
    memset (& pTftp->st, 0, sizeof pTftp->st);
    time (& pTftp->st.StartTime);
    pTftp->st.dLastUpdate = pTftp->st.StartTime;
    pTftp->st.ret_code = TFTP_TRF_RUNNING;
    // count the transfers (base 0)
    for ( pTftp->st.dwTransfert=0, pTmp = pTftpFirst->next ;
         pTmp!=NULL ;
         pTmp = pTmp->next, pTftp->st.dwTransfert++ ) ;
    LOGW ("Transfert #%d", pTftp->st.dwTransfert);

    // init MD5 structure
    pTftp->m.bInit = sSettings.bMD5;

    // clear buffers
    memset (& pTftp->b, 0, sizeof pTftp->b);
    pTftp->dlgHwnd = gWndHandle;
    pTftp->coordinator = gDevCoordinator;
} // PopulateTftpdStruct

// Suppress structure item
static struct LL_TftpInfo *TftpdDestroyThreadItem (struct LL_TftpInfo *pTftp)
{
    struct LL_TftpInfo *pTmp=pTftp;

    LOGD ("thread %d has exited", pTftp->tm.dwThreadHandle);

    LOGD ("removing thread %d (%p/%p/%p)", pTftp->tm.dwThreadHandleId, pTftp, pTftpFirst, pTftpFirst->next);
    // do not cancel permanent Thread
    if (! pTftp->tm.bPermanentThread )
    {
        if (pTftp!=pTftpFirst)
        {
            // search for the previous struct
            for (pTmp=pTftpFirst ; pTmp->next!=NULL && pTmp->next!=pTftp ; pTmp=pTmp->next);
            pTmp->next = pTftp->next;   // detach the struct from list
        }
        else pTftpFirst = pTmp = pTftpFirst->next;

        memset (pTftp, 0xAA, sizeof *pTftp); // fill with something is a good debugging tip
        free (pTftp);
    }

    return pTmp;	// pointer on previous item
} // TftpdDestroyThreadItem


// --------------------------------------------------------
// Filter incoming request
// add-on created on 24 April 2008
// return TRUE if message should be filtered
// --------------------------------------------------------
static int TftpMainFilter (SOCKADDR_STORAGE *from, int from_len, char *data, int len)
{
    static char LastMsg[PKTSIZE];
    static int  LastMsgSize;
    static time_t LastDate;
    static SOCKADDR_STORAGE LastFrom;

    if (len > PKTSIZE) return TRUE;	// packet should really be dropped
    // test only duplicated packets
    if (    len==LastMsgSize
        &&  memcmp (data, LastMsg, len)==0
        &&  memcmp (from, & LastFrom, from_len) == 0
        &&  time (NULL) == LastDate )
    {char szAddr[MAXLEN_IPv6]="", szServ[NI_MAXSERV]="";
        int Rc;
        Rc = getnameinfo ( (LPSOCKADDR) from, sizeof from,
                          szAddr, sizeof szAddr,
                          szServ, sizeof szServ,
                          NI_NUMERICHOST | NI_NUMERICSERV );

        LOGD ("Warning : received duplicated request from %s:%s", szAddr, szServ);
        Sleep (250);	// time for the first TFTP thread to start
        return FALSE;	// accept message nevertheless
    }
    // save last frame

    LastMsgSize = len;
    memcpy (LastMsg, data, len);
    LastFrom = *from;
    time (&LastDate);
    return FALSE; // packet is OK
} // TftpMainFilter


// activate a new thread and pass control to it
static int TftpdChooseNewThread (SOCKET sListenerSocket)
{
    struct LL_TftpInfo *pTftp, *pTmp;
    int             fromlen;
    int             bNewThread;
    int             Rc;
    int             nThread=0;

    for (  pTmp = pTftpFirst ;  pTmp!=NULL ;  pTmp = pTmp->next)
        nThread++;
    // if max thread reach read datagram and quit
    if (nThread >= sSettings.dwMaxTftpTransfers)
    {char dummy_buf [PKTSIZE];
        SOCKADDR_STORAGE    from;
        fromlen = sizeof from;
        // Read the connect datagram to empty queue
        Rc = recvfrom (sListenerSocket, dummy_buf, sizeof dummy_buf, 0,
                       (struct sockaddr *) & from, & fromlen);
        if (Rc>0)
        {char szAddr[MAXLEN_IPv6];
            getnameinfo ( (LPSOCKADDR) & from, sizeof from,
                         szAddr, sizeof szAddr,
                         NULL, 0,
                         NI_NUMERICHOST );
            LOGD ("max number of threads reached, connection from %s dropped", szAddr );
        }
        return -1;
    }

    // search a permanent thread in waiting state
    for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
        if ( pTftp->tm.bPermanentThread  &&  ! pTftp->tm.bActive )  break;
    bNewThread = (pTftp==NULL);

    if (bNewThread)
    {
        // search for the last thread struct
        for ( pTftp=pTftpFirst ;  pTftp!=NULL && pTftp->next!=NULL ; pTftp=pTftp->next );
        if (pTftp==NULL)   pTftp=pTftpFirst = (LL_TftpInfo *)calloc (1, sizeof *pTftpFirst);
        else               pTftp=pTftp->next=(LL_TftpInfo *)calloc (1, sizeof *pTftpFirst);
        // note due the calloc if thread has just been created
        //   pTftp->tm.dwThreadHandle == NULL ;
        pTftp->next = NULL ;
    }

    PopulateTftpdStruct (pTftp);

    // Read the connect datagram (since this use a "global socket" port 69 its done here)
    fromlen = sizeof pTftp->b.cnx_frame;
    Rc = recvfrom (sListenerSocket, pTftp->b.cnx_frame, sizeof pTftp->b.cnx_frame, 0,
                   (struct sockaddr *)&pTftp->b.from, &fromlen);
    if (Rc < 0)
    {
        // the Tftp structure has been created --> suppress it
        LOGE ("Error : RecvFrom returns %d: <%s>", WSAGetLastError(), LastErrorText());
        if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
    }
    // should the message be silently dropped
    else if (TftpMainFilter (& pTftp->b.from, sizeof pTftp->b.from, pTftp->b.cnx_frame, Rc))
    {char szAddr[MAXLEN_IPv6];
        getnameinfo ( (LPSOCKADDR) & pTftp->b.from, sizeof pTftp->b.from,
                     szAddr, sizeof szAddr,
                     NULL, 0,
                     NI_NUMERICHOST | AI_NUMERICSERV );
        // If this is an IPv4-mapped IPv6 address; drop the leading part of the address string so we're left with the familiar IPv4 format.
        // Hack copied from the Apache source code
        if ( pTftp->b.from.ss_family == AF_INET6
            && IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & pTftp->b.from ).sin6_addr ) )
        {
            memmove (szAddr, szAddr + sizeof ("::ffff:") - 1, strlen (szAddr + sizeof ("::ffff:") -1) +1 );
        }
        LOGW("Warning : Unaccepted request received from %s", szAddr);
        // the Tftp structure has been created --> suppress it
        if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
    }
    else	// message is accepted
    {char szAddr[MAXLEN_IPv6], szServ[NI_MAXSERV];
        getnameinfo ( (LPSOCKADDR) & pTftp->b.from, sizeof pTftp->b.from,
                     szAddr, sizeof szAddr,
                     szServ, sizeof szServ,
                     NI_NUMERICHOST | AI_NUMERICSERV );
        // If this is an IPv4-mapped IPv6 address; drop the leading part of the address string so we're left with the familiar IPv4 format.
        if ( pTftp->b.from.ss_family == AF_INET6
            && IN6_IS_ADDR_V4MAPPED ( & (* (struct sockaddr_in6 *) & pTftp->b.from ).sin6_addr ) )
        {
            memmove (szAddr, szAddr + sizeof ("::ffff:") - 1, strlen (szAddr + sizeof ("::ffff:") -1) +1 );
        }
        LOGD ("Connection received from %s on port %s", szAddr, szServ);
#if (defined DEBUG || defined DEB_TEST)
        BinDump (pTftp->b.cnx_frame, Rc, "Connect:");
#endif

        // mark thread as started (will not be reused)
        pTftp->tm.bActive=TRUE ;

        // start new thread or wake up permanent one
        if (bNewThread)
        {
            // create the worker thread
            // pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 8192, (void *) pTftp);
            pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                     8192,
                                                     StartTftpTransfer,
                                                     pTftp,
                                                     0,
                                                     & pTftp->tm.dwThreadHandleId);
            LOGD ("Thread %d transfer %d started (records %p/%p)", pTftp->tm.dwThreadHandleId, pTftp->tm.dwTransferId, pTftpFirst, pTftp);
            LOGD("thread %d started", pTftp->tm.dwThreadHandle);
        }
        else                 // Start the thread
        {
            LOGD ("waking up thread %d for transfer %d",
                  pTftp->tm.dwThreadHandleId,
                  pTftp->tm.dwTransferId );
            if (pTftp->tm.hEvent!=NULL)       SetEvent (pTftp->tm.hEvent);
        }
        // Put the multicast hook which adds the new client if the same mcast transfer
        // is already in progress

    } // recv ok --> thread has been started

    return TRUE;
} // TftpdStartNewThread


static void SendStatsToGui (BOOL bFullStats)
{
    static struct S_TftpTrfStat sMsg;
    struct LL_TftpInfo         *pTftp;
    int                         Ark;

    // if full report should be sent, one message per transfer is generates
    if (bFullStats)
    {
        for ( pTftp=pTftpFirst ;  pTftp!=NULL ; pTftp=pTftp->next )
        {
            if (pTftp->tm.bActive) ReportNewTrf (pTftp);   // from tftp_thread !
        }
    }
    else
    {
        for ( Ark=0,  pTftp=pTftpFirst ;  Ark<SizeOfTab(sMsg.t)  &&  pTftp!=NULL ; pTftp=pTftp->next )
        {
            if (pTftp->tm.bActive )
            {
                sMsg.t[Ark].dwTransferId = pTftp->tm.dwTransferId;
                sMsg.t[Ark].stat = pTftp->st;
                Ark++ ;
            }
        }
        sMsg.nbTrf = Ark;
        time (& sMsg.dNow);
        //if (Ark>0)
        SendMsgRequest (C_TFTP_TRF_STAT, &sMsg);
//                        sMsg.nbTrf * sizeof (sMsg.t[0]) + offsetof (struct S_TftpTrfStat, t[0]));
    }
} // SendStatsToGui

int SendMsgRequest (int type,				// msg type
					const void *msg_stuff)	// data
					 {
	return SendMessage(gWndHandle, UI_MESSAGE_TFTPINFO, type, (LPARAM)msg_stuff);
}

////////////////////////////////////////////////////////////
// Init TFTP daemon
////////////////////////////////////////////////////////////

static int CreatePermanentThreads (void)
{
    int Ark;
    struct LL_TftpInfo *pTftp;


    // inits socket
    ////////////////////////////////////////////
    // Create the permanents threads
    for ( Ark=0  ; Ark < TFTP_PERMANENTTHREADS ; Ark++ )
    {
        if (pTftpFirst==NULL)  pTftp=pTftpFirst= (LL_TftpInfo *)calloc (1, sizeof *pTftpFirst);
        else                   pTftp=pTftp->next=(LL_TftpInfo *)calloc (1, sizeof *pTftpFirst);
        pTftp->next = NULL;
        pTftp->tm.bPermanentThread = TRUE;
        pTftp->tm.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        pTftp->tm.N = Ark+1 ;
        //pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 4096, (void *) pTftp);
        pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                 8192,
                                                 StartTftpTransfer,
                                                 pTftp,
                                                 0,
                                                 & pTftp->tm.dwThreadHandleId);
        pTftp->r.hFile=INVALID_HANDLE_VALUE ;
    }

    return TRUE;
}  // CreatePermanentThreads


static int TftpdCleanup (SOCKET sListenerSocket)
{
    struct LL_TftpInfo *pTftp, *pTmp;
    // suspend all threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) SuspendThread (pTftp->tm.dwThreadHandle);

    if (WSAIsBlocking ())  WSACancelBlockingCall ();   // the thread is probably blocked into a select

    // then frees resources
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTmp )
    {
        pTmp=pTftp->next ;

        if (pTftp->r.skt!=INVALID_SOCKET)          closesocket (pTftp->r.skt),   pTftp->r.skt=INVALID_SOCKET;
        if (pTftp->r.hFile!=INVALID_HANDLE_VALUE)  CloseHandle(pTftp->r.hFile),  pTftp->r.hFile!=INVALID_HANDLE_VALUE;
        if (pTftp->tm.hEvent!=NULL)                CloseHandle(pTftp->tm.hEvent),pTftp->tm.hEvent!=NULL;
        free (pTftp);
    }
    Sleep (100);
    // kill the threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) TerminateThread (pTftp->tm.dwThreadHandle, 0);

    // close main socket
    closesocket (sListenerSocket);
    return TRUE;
} // TftpdCleanup


// a watch dog which reset the socket event if data are available
static int ResetSockEvent (SOCKET s, HANDLE hEv)
{
    u_long dwData;
    int Rc;
    Rc = ioctlsocket ( s ,  FIONREAD, & dwData);
    if (dwData==0) ResetEvent (hEv);
    return Rc;
}


// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
void TftpdMain (void *param)
{
    int Rc;
//    int parse;
    HANDLE hSocketEvent = INVALID_HANDLE_VALUE;
    struct LL_TftpInfo *pTftp;
    // events : either socket event or wake up by another thread
    enum { E_TFTP_SOCK=0, E_TFTP_WAKE, E_TFTP_EV_NB };
//    HANDLE tObjects [E_TFTP_EV_NB];

    // creates socket and starts permanent threads
    if (pTftpFirst==NULL)  CreatePermanentThreads ();

    //tftpThreadMonitor.bInit = TRUE;  // inits OK

    // Socket was not opened at the start since we have to use interface
    // once an address as been assigned
    while ( tftpThreadMonitor.gRunning )
    {
        // if socket as not been created before
        if (tftpThreadMonitor.skt == INVALID_SOCKET)
        {
            tftpThreadMonitor.skt = TftpBindLocalInterface ();
        } // open the socket
        if (tftpThreadMonitor.skt == INVALID_SOCKET)
        {
            break;
        }

        // create event for the incoming Socket
        hSocketEvent = WSACreateEvent();
        Rc = WSAEventSelect (tftpThreadMonitor.skt, hSocketEvent, FD_READ);
        Rc = GetLastError ();
#if 0
        tObjects[E_TFTP_SOCK] = hSocketEvent;
        tObjects[E_TFTP_WAKE] = tftpThreadMonitor.hEv;

        // stop only when TFTP is stopped and all threads have returned

        // waits for either incoming connection or thread event
        Rc = WaitForMultipleObjects ( E_TFTP_EV_NB,
                                     tObjects,
                                     FALSE,
                                     sSettings.dwRefreshInterval );
#endif
        Rc = WaitForSingleObject(hSocketEvent, sSettings.dwRefreshInterval);
        if (! tftpThreadMonitor.gRunning ) break;

        switch (Rc)
        {
            // we have received a message on the port 69
        case WAIT_OBJECT_0:    // Socket Msg
            WSAEventSelect (tftpThreadMonitor.skt, 0, 0);
            TftpdChooseNewThread (tftpThreadMonitor.skt);
            ResetEvent( hSocketEvent );
            WSAEventSelect (tftpThreadMonitor.skt, hSocketEvent, FD_READ);
            // ResetSockEvent (sListenerSocket, hSocketEvent);
            break;

        case  WAIT_TIMEOUT :
			SendStatsToGui(gSendFullStat); // full stat flag may be set by console
			gSendFullStat = FALSE;         // reset full stat flag
            // ResetSockEvent (sListenerSocket, hSocketEvent);
            break;

        case WAIT_FAILED:
        case WAIT_ABANDONED:
            LOGE("WaitForSingleObject error %d", LastErrorText());
            break;
        }   // switch

        CloseHandle(hSocketEvent);
    } // endless loop

    // TftpdCleanup (sListenerSocket, hSemaphore);

    // Should the main thread kill other threads ?
    if ( tftpThreadMonitor.bSoftReset )
        LOGE ("do NOT signal worker threads");
    else
    {
        LOGE ("signalling worker threads");
        /////////////////////////////////
        // wait for end of worker threads
        for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
        {
            if (pTftp->tm.bActive)                nak (pTftp, ECANCELLED);
            else if (pTftp->tm.bPermanentThread)  SetEvent (pTftp->tm.hEvent);
        }
        LOGE ("waiting for worker threads");

        while ( pTftpFirst != NULL )
        {
            WaitForSingleObject (pTftpFirst->tm.dwThreadHandle, 10000);
            LOGE ("End of thread %d", pTftpFirst->tm.dwThreadHandleId);
            pTftpFirst->tm.bPermanentThread = FALSE;
            TftpdDestroyThreadItem (pTftpFirst);
        }
    } // Terminate sub threads

    Rc = closesocket (tftpThreadMonitor.skt);
    tftpThreadMonitor.skt=INVALID_SOCKET;
    WSACloseEvent (hSocketEvent);

    LOGE ("main TFTP thread ends here");
    _endthread ();
} // Tftpd main thread

#define TFTP_PORT     69

static void FreeThreadResources ()
{
    if (tftpThreadMonitor.skt != INVALID_SOCKET)
        closesocket (tftpThreadMonitor.skt);
    if (tftpThreadMonitor.hEv != INVALID_HANDLE_VALUE)
        CloseHandle (tftpThreadMonitor.hEv);
    tftpThreadMonitor.skt = INVALID_SOCKET;
    tftpThreadMonitor.hEv = INVALID_HANDLE_VALUE;
    tftpThreadMonitor.tTh = INVALID_HANDLE_VALUE;
    tftpThreadMonitor.bSoftReset = FALSE;
    tftpThreadMonitor.gRunning = FALSE;

}
int StartTftpdThread ()
{
    if (tftpThreadMonitor.gRunning)
        return 0;
    tftpThreadMonitor.skt = INVALID_SOCKET ;

    // Create the wake up event
    tftpThreadMonitor.hEv  = CreateEvent ( NULL, true, FALSE, NULL );
    if ( tftpThreadMonitor.hEv == INVALID_HANDLE_VALUE )
    {
        FreeThreadResources ();
        return FALSE;
    }

    tftpThreadMonitor.bSoftReset = FALSE;
    // now start the thread
    tftpThreadMonitor.tTh  = (HANDLE) _beginthread ( TftpdMain,
                                                    1024,
                                                    NULL );
    if (tftpThreadMonitor.tTh == INVALID_HANDLE_VALUE)
    {
        FreeThreadResources ();
        return FALSE;
    }
    else
    {
        // all resources have been allocated --> status OK
        tftpThreadMonitor.gRunning  = TRUE;

    } // service correctly started
    //if (Ark>TH_SCHEDULER)   SetEvent ( tThreads[TH_SCHEDULER].hEv );
    return TRUE;
} // StartSingleWorkerThread


void StartTftpd32Services (HWND param, DeviceCoordinator *coordinator)
{
#if 0
    char sz[_MAX_PATH];

    // read log level (env var TFTP_LOG)
    if (GetEnvironmentVariable (TFTP_LOG, sz, sizeof sz)!=0)
        sSettings.LogLvl = atoi (sz);
    else  sSettings.LogLvl = TFTPD32_DEF_LOG_LEVEL;

    // Get the path in order to find the help file
    if (GetEnvironmentVariable (TFTP_INI, sz, sizeof sz)!=0)
        SetIniFileName (sz, szTftpd32IniFile);
    else  SetIniFileName (INI_FILE, szTftpd32IniFile);

    // Read settings (tftpd32.ini)
    Tftpd32ReadSettings ();
    //	DHCPReadConfig ();
#endif
    gWndHandle = param;
    gDevCoordinator = coordinator;
    // starts worker threads
    StartTftpdThread ();
    LOGD("Worker threads started");
} // StartTftpd32Services

void StopTftpd32Services (void)
{
    FreeThreadResources ();
}
