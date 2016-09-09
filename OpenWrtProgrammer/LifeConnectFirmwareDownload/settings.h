
//////////////////////////////////////////////////////
// Projet TFTPD32.  Mars 2000 Ph.jounin
//
// File settings.h
// settings structure declaration
//
// released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////

enum e_SecurityLevels { SECURITY_NONE, SECURITY_STD, SECURITY_HIGH, SECURITY_READONLY };

#define MAXLEN_IPv6 40

struct S_Tftpd32Settings
{
    char                  szBaseDirectory [_MAX_PATH];
    unsigned              Timeout;
    unsigned              Retransmit;
    unsigned              WinSize;
    enum e_SecurityLevels SecurityLvl;
    unsigned              Port;
    BOOL                  bNegociate;
    BOOL                  bPXECompatibility;
    BOOL                  bDirText;
    BOOL                  bMD5;
    BOOL                  bUnixStrings;
    BOOL                  bBeep;
    BOOL                  bVirtualRoot;
	// changed in release 4 : szTftpLocalIP is either an IP address or an interface descriptor
    char                  szTftpLocalIP [max (MAXLEN_IPv6, MAX_ADAPTER_DESCRIPTION_LENGTH+4)];

    unsigned              nTftpLowPort;
    unsigned              nTftpHighPort;
	BOOL                  bPortOption;			// experimental port option
	DWORD				  nGuiRemanence;
	BOOL                  bIgnoreLastBlockAck;
	BOOL                  bIPv4;
	BOOL                  bIPv6;

	// unsaved settings
	DWORD				  dwMaxTftpTransfers;
    char                  szWorkingDirectory [_MAX_PATH];
    DWORD                 dwRefreshInterval;
	BOOL                  bTftpOnPhysicalIf;
};

extern struct S_Tftpd32Settings sSettings;          // The settings,used anywhere in the code


#define  LOOPBACK_STRING        "127.0.0.1"
#define  DIR_TEXT_FILE          "dir.txt"

#  define TFTP_TIMEOUT            3
#  define TFTP_RETRANSMIT         6
#  define TFTPD32_DEF_LOG_LEVEL   8
#  define TFTP_DEFPORT           69

#define TFTP_MAXRETRIES          50 // do not resent same block more than # times
#define TIME_FOR_LONG_TRANSFER   10 // more than 10 seconds -> beep

