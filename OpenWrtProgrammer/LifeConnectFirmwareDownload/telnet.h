#ifndef __TELNET_H
#define __TELNET_H
#include <string>
#include <string.h>
using namespace std;

#define USE_WINSOCK
//#define HAVE_POLL_FINE
typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                    7.17.0, reused in April 2011 for 7.21.5] */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                    due to lack of access - when login fails
                                    this is not returned. */
  CURLE_OBSOLETE10,              /* 10 - NOT USED */
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_OBSOLETE12,              /* 12 - NOT USED */
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_OBSOLETE16,              /* 16 - NOT USED */
  CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_OBSOLETE20,              /* 20 - NOT USED */
  CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_OBSOLETE24,              /* 24 - NOT USED */
  CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
  CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  /* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
           instead of a memory allocation error if CURL_DOES_CONVERSIONS
           is defined
  */
  CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
  CURLE_OBSOLETE29,              /* 29 - NOT USED */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_OBSOLETE32,              /* 32 - NOT USED */
  CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_OBSOLETE40,              /* 40 - NOT USED */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_OBSOLETE44,              /* 44 - NOT USED */
  CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
  CURLE_OBSOLETE46,              /* 46 - NOT USED */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE50,              /* 50 - NOT USED */
  CURLE_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
                                     wasn't verified fine */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_RECV_TIMEOUT,            /* 57 - NOT IN USE */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
  CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
  CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
  CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
  CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                    that failed */
  CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
  CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                    accepted and we failed to login */
  CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
  CURLE_TFTP_PERM,               /* 69 - permission problem on server */
  CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
  CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
  CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
  CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
  CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
  CURLE_CONV_FAILED,             /* 75 - conversion failed */
  CURLE_CONV_REQD,               /* 76 - caller must register conversion
                                    callbacks using curl_easy_setopt options
                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                    CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
  CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                    or wrong format */
  CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
  CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                    generic so the error message will be of
                                    interest when this has happened */

  CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                    connection */
  CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                    wait till it's ready and try again (Added
                                    in 7.18.2) */
  CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                    wrong format (Added in 7.19.0) */
  CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                    7.19.0) */
  CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
  CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
  CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
  CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
  CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */

  CURL_LAST /* never use! */
} CURLcode;

typedef enum
{
   CURL_TS_DATA = 0,
   CURL_TS_IAC,
   CURL_TS_WILL,
   CURL_TS_WONT,
   CURL_TS_DO,
   CURL_TS_DONT,
   CURL_TS_CR,
   CURL_TS_SB,   /* sub-option collection */
   CURL_TS_SE   /* looking for sub-option end */
} TelnetReceive;

  /* Tests have proven that 20K is a very bad buffer size for uploads on
     Windows, while 16K for some odd reason performed a lot better.
     We do the ifndef check to allow this value to easier be changed at build
     time for those who feel adventurous. The practical minimum is about
     400 bytes since libcurl uses a buffer of this size as a scratch area
     (unrelated to network send operations). */
#define CURL_MAX_WRITE_SIZE 16384

#define BUFSIZE CURL_MAX_WRITE_SIZE

#define SUBBUFSIZE 512
//#define EINTR       WSAEINTR
#define error_not_EINTR (error != EINTR)

#ifdef USE_WINSOCK
#define SOCKERRNO         ((int)WSAGetLastError())
#define SET_SOCKERRNO(x)  (WSASetLastError((int)(x)))
#else
#define SOCKERRNO         (errno)
#define SET_SOCKERRNO(x)  (errno = (x))
#endif

#define ERRNO        ((int)GetLastError())

/* Some convenience macros to get the larger/smaller value out of two given.
   We prefix with CURL to prevent name collisions. */
#define CURLMAX(x,y) ((x)>(y)?(x):(y))
#define CURLMIN(x,y) ((x)<(y)?(x):(y))

/* For negotiation compliant to RFC 1143 */
#define CURL_NO          0
#define CURL_YES         1
#define CURL_WANTYES     2
#define CURL_WANTNO      3

#define CURL_EMPTY       0
#define CURL_OPPOSITE    1

/*
struct timeval {
 long tv_sec;
 long tv_usec;
};
*/

//#define poll(x,y,z) WSAPoll((x),(y),(z))

#ifndef POLLRDNORM
#define POLLRDNORM POLLIN
#endif

#ifndef POLLWRNORM
#define POLLWRNORM POLLOUT
#endif

#ifndef POLLRDBAND
#define POLLRDBAND POLLPRI
#endif

#define WHILE_FALSE  while(0)
#define Curl_nop_stmt  do {} while(0)
typedef SOCKET curl_socket_t;
#define CURL_SOCKET_BAD INVALID_SOCKET
#define VALID_SOCK(x) 1
#define VERIFY_SOCK(x) Curl_nop_stmt

/* Define to the type of arg 1 for recv. */
#define RECV_TYPE_ARG1 SOCKET

/* Define to the type of arg 2 for recv. */
#define RECV_TYPE_ARG2 char *

/* Define to the type of arg 3 for recv. */
#define RECV_TYPE_ARG3 int

/* Define to the type of arg 4 for recv. */
#define RECV_TYPE_ARG4 int

/* Define to the function return type for recv. */
#define RECV_TYPE_RETV int

/* Define if you have the recvfrom function. */
#define HAVE_RECVFROM 1

/* Define to the type of arg 1 for recvfrom. */
#define RECVFROM_TYPE_ARG1 SOCKET

/* Define to the type pointed by arg 2 for recvfrom. */
#define RECVFROM_TYPE_ARG2 char

/* Define to the type of arg 3 for recvfrom. */
#define RECVFROM_TYPE_ARG3 int

/* Define to the type of arg 4 for recvfrom. */
#define RECVFROM_TYPE_ARG4 int

/* Define to the type pointed by arg 5 for recvfrom. */
#define RECVFROM_TYPE_ARG5 struct sockaddr

/* Define to the type pointed by arg 6 for recvfrom. */
#define RECVFROM_TYPE_ARG6 int

/* Define to the function return type for recvfrom. */
#define RECVFROM_TYPE_RETV int

/* Define to the type of arg 1 for send. */
#define SEND_TYPE_ARG1 SOCKET

/* Define to the type qualifier of arg 2 for send. */
#define SEND_QUAL_ARG2 const

/* Define to the type of arg 2 for send. */
#define SEND_TYPE_ARG2 char *

/* Define to the type of arg 3 for send. */
#define SEND_TYPE_ARG3 int

/* Define to the type of arg 4 for send. */
#define SEND_TYPE_ARG4 int

/* Define to the function return type for send. */
#define SEND_TYPE_RETV int
#define swrite(x,y,z) (ssize_t)send((SEND_TYPE_ARG1)(x), \
                                    (SEND_TYPE_ARG2)(y), \
                                    (SEND_TYPE_ARG3)(z), \
                                    (SEND_TYPE_ARG4)(0))

#define sread(x,y,z) (ssize_t)recv((RECV_TYPE_ARG1)(x), \
                                   (RECV_TYPE_ARG2)(y), \
                                   (RECV_TYPE_ARG3)(z), \
                                   (RECV_TYPE_ARG4)(0))

/* This is a return code for the read callback that, when returned, will
   signal libcurl to immediately abort the current transfer. */
#define CURL_READFUNC_ABORT 0x10000000
/* This is a return code for the read callback that, when returned, will
   signal libcurl to pause sending data on the current transfer. */
#define CURL_READFUNC_PAUSE 0x10000001

#ifdef USE_WINSOCK
#undef  EBADF            /* override definition in errno.h */
#define EBADF            WSAEBADF
#undef  EINTR            /* override definition in errno.h */
#define EINTR            WSAEINTR
#undef  EINVAL           /* override definition in errno.h */
#define EINVAL           WSAEINVAL
#undef  EWOULDBLOCK      /* override definition in errno.h */
#define EWOULDBLOCK      WSAEWOULDBLOCK
#undef  EINPROGRESS      /* override definition in errno.h */
#define EINPROGRESS      WSAEINPROGRESS
#undef  EALREADY         /* override definition in errno.h */
#define EALREADY         WSAEALREADY
#undef  ENOTSOCK         /* override definition in errno.h */
#define ENOTSOCK         WSAENOTSOCK
#undef  EDESTADDRREQ     /* override definition in errno.h */
#define EDESTADDRREQ     WSAEDESTADDRREQ
#undef  EMSGSIZE         /* override definition in errno.h */
#define EMSGSIZE         WSAEMSGSIZE
#undef  EPROTOTYPE       /* override definition in errno.h */
#define EPROTOTYPE       WSAEPROTOTYPE
#undef  ENOPROTOOPT      /* override definition in errno.h */
#define ENOPROTOOPT      WSAENOPROTOOPT
#undef  EPROTONOSUPPORT  /* override definition in errno.h */
#define EPROTONOSUPPORT  WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT  WSAESOCKTNOSUPPORT
#undef  EOPNOTSUPP       /* override definition in errno.h */
#define EOPNOTSUPP       WSAEOPNOTSUPP
#define EPFNOSUPPORT     WSAEPFNOSUPPORT
#undef  EAFNOSUPPORT     /* override definition in errno.h */
#define EAFNOSUPPORT     WSAEAFNOSUPPORT
#undef  EADDRINUSE       /* override definition in errno.h */
#define EADDRINUSE       WSAEADDRINUSE
#undef  EADDRNOTAVAIL    /* override definition in errno.h */
#define EADDRNOTAVAIL    WSAEADDRNOTAVAIL
#undef  ENETDOWN         /* override definition in errno.h */
#define ENETDOWN         WSAENETDOWN
#undef  ENETUNREACH      /* override definition in errno.h */
#define ENETUNREACH      WSAENETUNREACH
#undef  ENETRESET        /* override definition in errno.h */
#define ENETRESET        WSAENETRESET
#undef  ECONNABORTED     /* override definition in errno.h */
#define ECONNABORTED     WSAECONNABORTED
#undef  ECONNRESET       /* override definition in errno.h */
#define ECONNRESET       WSAECONNRESET
#undef  ENOBUFS          /* override definition in errno.h */
#define ENOBUFS          WSAENOBUFS
#undef  EISCONN          /* override definition in errno.h */
#define EISCONN          WSAEISCONN
#undef  ENOTCONN         /* override definition in errno.h */
#define ENOTCONN         WSAENOTCONN
#define ESHUTDOWN        WSAESHUTDOWN
#define ETOOMANYREFS     WSAETOOMANYREFS
#undef  ETIMEDOUT        /* override definition in errno.h */
#define ETIMEDOUT        WSAETIMEDOUT
#undef  ECONNREFUSED     /* override definition in errno.h */
#define ECONNREFUSED     WSAECONNREFUSED
#undef  ELOOP            /* override definition in errno.h */
#define ELOOP            WSAELOOP
#ifndef ENAMETOOLONG     /* possible previous definition in errno.h */
#define ENAMETOOLONG     WSAENAMETOOLONG
#endif
#define EHOSTDOWN        WSAEHOSTDOWN
#undef  EHOSTUNREACH     /* override definition in errno.h */
#define EHOSTUNREACH     WSAEHOSTUNREACH
#ifndef ENOTEMPTY        /* possible previous definition in errno.h */
#define ENOTEMPTY        WSAENOTEMPTY
#endif
#define EPROCLIM         WSAEPROCLIM
#define EUSERS           WSAEUSERS
#define EDQUOT           WSAEDQUOT
#define ESTALE           WSAESTALE
#define EREMOTE          WSAEREMOTE
#endif

#if defined(_WIN64)
#define ssize_t __int64
#else
#define ssize_t int
#endif
/* linked-list structure for the CURLOPT_QUOTE option (and other) */
struct curl_slist {
  char *data;
  struct curl_slist *next;
};

class telnet {
protected:
  bool verbose;
  curl_socket_t sockfd;
  int timeout;
  int please_negotiate;
  int already_negotiated;
  int telnet_cmd_negotiate;
  int us[256];
  int usq[256];
  int us_preferred[256];
  int him[256];
  int himq[256];
  int him_preferred[256];
  char subopt_ttype[32];             /* Set with suboption TTYPE */
  char subopt_xdisploc[128];          /* Set with suboption XDISPLOC */
  struct curl_slist *telnet_vars; /* Environment variables */

  /* suboptions */
  unsigned char subbuffer[SUBBUFSIZE];
  unsigned char *subpointer, *subend;      /* buffer for sub-options */

  TelnetReceive telrcv_state;

  public:
     telnet(curl_socket_t sock, int to, bool verb);
     ~telnet();

     int receive_telnet_data(char *buffer, ssize_t len);

     void negotiate();
     void send_negotiation( int cmd, int option);
     void set_local_option( int cmd, int option);
     void rec_do( int option);
     void rec_dont( int option);
     void set_remote_option( int cmd, int option);
     void rec_will( int option);
     void rec_wont( int option);

     void printsub(int direction, unsigned char *pointer,   size_t length);
     void printoption(const char *direction, int cmd, int option);
     CURLcode check_telnet_options();
     void suboption();
     CURLcode telrcv(const unsigned char *inbuf, /* Data received from socket */
                    ssize_t count);              /* Number of bytes received */
     CURLcode send_telnet_data(const char *buffer, ssize_t len);

     int receive_telnet_cmd(char *buffer, ssize_t len);
     int send_command(const char *buffer, string &result, bool trim = true);
};

int Curl_poll(struct pollfd ufds[], unsigned int nfds, int timeout_ms);
CURLcode Curl_write(curl_socket_t sockfd,
                    const void *mem,
                    size_t len,
                    ssize_t *written);
CURLcode Curl_read( curl_socket_t sockfd,     /* read from this socket */
                   char *buf,                /* store read data here */
                   size_t sizerequested,     /* max amount to read */
                   ssize_t *n);               /* amount bytes read */
int Curl_raw_equal(const char *first, const char *second);

const char *Curl_strerror( int err);

struct timeval curlx_tvnow(void);

/*
 * Make sure that the first argument (t1) is the more recent time and t2 is
 * the older time, as otherwise you get a weird negative time-diff back...
 *
 * Returns: the time difference in number of milliseconds.
 */
long curlx_tvdiff(struct timeval t1, struct timeval t2);

/*
 * Same as curlx_tvdiff but with full usec resolution.
 *
 * Returns: the time difference in seconds with subsecond resolution.
 */
double curlx_tvdiff_secs(struct timeval t1, struct timeval t2);

long Curl_tvlong(struct timeval t1);

int Curl_wait_ms(int timeout_ms);

/* These two defines below exist to provide the older API for library
   internals only. */
#define Curl_tvnow() curlx_tvnow()
#define Curl_tvdiff(x,y) curlx_tvdiff(x,y)
#define Curl_tvdiff_secs(x,y) curlx_tvdiff_secs(x,y)


/*
 * NAME curl_slist_append()
 *
 * DESCRIPTION
 *
 * Appends a string to a linked list. If no list exists, it will be created
 * first. Returns the new list, after appending.
 */
struct curl_slist *curl_slist_append(struct curl_slist *,
                                                 const char *);

/*
 * NAME curl_slist_free_all()
 *
 * DESCRIPTION
 *
 * free a previously built curl_slist.
 */
void curl_slist_free_all(struct curl_slist *);

#define Curl_safefree(ptr) \
  do {if((ptr)) {free((ptr)); (ptr) = NULL;}} WHILE_FALSE
#endif
