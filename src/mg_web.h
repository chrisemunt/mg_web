/*
   ----------------------------------------------------------------------------
   | mg_dba.so|dll                                                            |
   | Description: An abstraction of the InterSystems Cache/IRIS API           |
   |              and YottaDB API                                             |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2024 MGateway Ltd                                     |
   | Surrey UK.                                                               |
   | All rights reserved.                                                     |
   |                                                                          |
   | http://www.mgateway.com                                                  |
   |                                                                          |
   | Licensed under the Apache License, Version 2.0 (the "License"); you may  |
   | not use this file except in compliance with the License.                 |
   | You may obtain a copy of the License at                                  |
   |                                                                          |
   | http://www.apache.org/licenses/LICENSE-2.0                               |
   |                                                                          |
   | Unless required by applicable law or agreed to in writing, software      |
   | distributed under the License is distributed on an "AS IS" BASIS,        |
   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
   | See the License for the specific language governing permissions and      |
   | limitations under the License.                                           |
   |                                                                          |
   ----------------------------------------------------------------------------
*/

#ifndef MG_WEB_H
#define MG_WEB_H

#if defined(_WIN32)

#if defined(_MSC_VER)
/* Check for MS compiler later than VC6 */
#if (_MSC_VER >= 1400)

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE    1
#endif

#if !defined(_CRT_NONSTDC_NO_DEPRECATE)
#define _CRT_NONSTDC_NO_DEPRECATE   1
#endif

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS   1
#endif

#endif
#endif

#elif defined(__linux__) || defined(__linux) || defined(linux)

#if !defined(LINUX)
#define LINUX                       1
#endif

#elif defined(__APPLE__)

#if !defined(MACOSX)
#define MACOSX                      1
#endif

#endif

#if defined(SOLARIS)
#ifndef __GNUC__
#  define  __attribute__(x)
#endif
#endif

#if defined(_WIN32)

#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__MINGW32__)
#include <math.h>
#endif

#else

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#if !defined(HPUX) && !defined(HPUX10) && !defined(HPUX11)
#include <sys/select.h>
#endif
#if defined(SOLARIS)
#include <sys/filio.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <dlfcn.h>
#include <math.h>
#include <pwd.h> /* v2.7.36 */
#include <grp.h>

#endif


#ifndef INCL_WINSOCK_API_TYPEDEFS
#define MG_USE_MS_TYPEDEFS 1
#endif

/* Cache/IRIS */

#define CACHE_MAXSTRLEN	32767
#define CACHE_MAXLOSTSZ	3641144

typedef char		Callin_char_t;
#define CACHE_INT64 long long
#define CACHESTR	CACHE_ASTR

typedef struct {
   unsigned short len;
   Callin_char_t  str[CACHE_MAXSTRLEN];
} CACHE_ASTR, *CACHE_ASTRP;

typedef struct {
   unsigned int	len;
   union {
      Callin_char_t * ch;
      unsigned short *wch;
      unsigned short *lch;
   } str;
} CACHE_EXSTR, *CACHE_EXSTRP;

#define CACHE_TTALL     1
#define CACHE_TTNEVER   8
#define CACHE_PROGMODE  32

#define CACHE_INT	      1
#define CACHE_DOUBLE	   2
#define CACHE_ASTRING   3

#define CACHE_CHAR      4
#define CACHE_INT2      5
#define CACHE_INT4      6
#define CACHE_INT8      7
#define CACHE_UCHAR     8
#define CACHE_UINT2     9
#define CACHE_UINT4     10
#define CACHE_UINT8     11
#define CACHE_FLOAT     12
#define CACHE_HFLOAT    13
#define CACHE_UINT      14
#define CACHE_WSTRING   15
#define CACHE_OREF      16
#define CACHE_LASTRING  17
#define CACHE_LWSTRING  18
#define CACHE_IEEE_DBL  19
#define CACHE_HSTRING   20
#define CACHE_UNDEF     21

#define CACHE_CHANGEPASSWORD  -16
#define CACHE_ACCESSDENIED    -15
#define CACHE_EXSTR_INUSE     -14
#define CACHE_NORES	         -13
#define CACHE_BADARG	         -12
#define CACHE_NOTINCACHE      -11
#define CACHE_RETTRUNC 	      -10
#define CACHE_ERUNKNOWN	      -9	
#define CACHE_RETTOOSMALL     -8	
#define CACHE_NOCON 	         -7
#define CACHE_INTERRUPT       -6
#define CACHE_CONBROKEN       -4
#define CACHE_STRTOOLONG      -3
#define CACHE_ALREADYCON      -2
#define CACHE_FAILURE	      -1
#define CACHE_SUCCESS 	      0

#define CACHE_ERMXSTR         5
#define CACHE_ERNOLINE        8
#define CACHE_ERUNDEF         9
#define CACHE_ERSYSTEM        10
#define CACHE_ERSUBSCR        16
#define CACHE_ERNOROUTINE     17
#define CACHE_ERSTRINGSTACK   20
#define CACHE_ERUNIMPLEMENTED 22
#define CACHE_ERARGSTACK      25
#define CACHE_ERPROTECT       27
#define CACHE_ERPARAMETER     40
#define CACHE_ERNAMSP         83
#define CACHE_ERWIDECHAR      89
#define CACHE_ERNOCLASS       122
#define CACHE_ERBADOREF       119
#define CACHE_ERNOMETHOD      120
#define CACHE_ERNOPROPERTY    121

#define CACHE_ETIMEOUT        -100
#define CACHE_BAD_STRING      -101
#define CACHE_BAD_NAMESPACE   -102
#define CACHE_BAD_GLOBAL      -103
#define CACHE_BAD_FUNCTION    -104
#define CACHE_BAD_CLASS       -105
#define CACHE_BAD_METHOD      -106

#define CACHE_MAXCON          -120

#define CACHE_INCREMENTAL_LOCK   1

/* End of Cache/IRIS */


/* YottaDB */

#define YDB_OK       0
#define YDB_DEL_TREE 1

typedef struct {
   unsigned int   len_alloc;
   unsigned int   len_used;
   char		      *buf_addr;
} ydb_buffer_t;

typedef struct {
   unsigned long  length;
   char		      *address;
} ydb_string_t;

typedef struct {
   ydb_string_t   rtn_name;
   void		      *handle;
} ci_name_descriptor;

typedef ydb_buffer_t DBXSTR;
typedef char            ydb_char_t;
typedef long            ydb_long_t;


/* End of YottaDB */

/* GT.M call-in interface */

typedef int    gtm_status_t;
typedef char   gtm_char_t;
typedef int    xc_status_t;

/* End of GT.M call-in interface */

#define MG_DEFAULT_CONFIG        "timeout 30\n<server local>\ntype IRIS\nhost localhost\ntcp_port 7041\nusername _SYSTEM\npassword SYS\nnamespace USER\n</server>\n<location />\nfunction mgweb^%zmgsis\nservers local\n</location>\n"
#define MG_DEFAULT_HEADER        "HTTP/1.1 200 OK\r\nContent-type: text/html"
#define DBX_CGI_BASE             "REQUEST_METHOD\nSCRIPT_NAME\nQUERY_STRING\nSERVER_PROTOCOL\n\n"

#define DBX_CGI_REQUEST_METHOD   0
#define DBX_CGI_SCRIPT_NAME      1
#define DBX_CGI_QUERY_STRING     2

#define DBX_DBTYPE_CACHE         1
#define DBX_DBTYPE_IRIS          2
#define DBX_DBTYPE_YOTTADB       5
#define DBX_DBTYPE_GTM           11
#define DBX_DBTYPE_NODEJS        21
#define DBX_DBTYPE_BUN           22

#define DBX_MAXCONS              8192
#define DBX_MAXARGS              64
#define DBX_HEADER_SIZE          2048

#define DBX_ERROR_SIZE           512

#define DBX_DSORT_INVALID        0
#define DBX_DSORT_DATA           1
#define DBX_DSORT_SUBSCRIPT      2
#define DBX_DSORT_GLOBAL         3
#define DBX_DSORT_EOD            9
#define DBX_DSORT_STATUS         10
#define DBX_DSORT_ERROR          11

#define DBX_DSORT_WEBCGI         5
#define DBX_DSORT_WEBCONTENT     6
#define DBX_DSORT_WEBNV          7
#define DBX_DSORT_WEBSYS         8

#define DBX_DSORT_ISVALID(a)     ((a > 0) && (a < 12))

#define DBX_DTYPE_NONE           0
#define DBX_DTYPE_STR            1
#define DBX_DTYPE_STR8           2
#define DBX_DTYPE_STR16          3
#define DBX_DTYPE_INT            4
#define DBX_DTYPE_INT64          5
#define DBX_DTYPE_DOUBLE         6
#define DBX_DTYPE_OREF           7
#define DBX_DTYPE_NULL           10

#define DBX_CMND_FUNCTION        31
#define DBX_CMND_PING            90

#define DBX_IBUFFER_OFFSET       15

#define DBX_MAXSIZE              32767
#define DBX_BUFFER               32768

#define DBX_LS_MAXSIZE_ISC       3641144
#define DBX_LS_BUFFER_ISC        3641145

#define DBX_LS_MAXSIZE_YDB       1048576
#define DBX_LS_BUFFER_YDB        1048577

#if defined(MAX_PATH) && (MAX_PATH>511)
#define DBX_MAX_PATH             MAX_PATH
#else
#define DBX_MAX_PATH             512
#endif

#define DBX_WEB_ROUTINE          "dbxweb^%zmgsis"
#define DBX_PING_ROUTINE         "dbxping^%zmgsis"

#if defined(_WIN32)
#define DBX_NULL_DEVICE          "//./nul"
#else
#define DBX_NULL_DEVICE          "/dev/null/"
#endif

#if defined(_WIN32)
#define DBX_CACHE_DLL            "cache.dll"
#define DBX_IRIS_DLL             "irisdb.dll"
#define DBX_YDB_DLL              "yottadb.dll"
#define DBX_GTM_DLL              "gtmshr.dll"
#else
#define DBX_CACHE_SO             "libcache.so"
#define DBX_CACHE_DYLIB          "libcache.dylib"
#define DBX_ISCCACHE_SO          "libisccache.so"
#define DBX_ISCCACHE_DYLIB       "libisccache.dylib"
#define DBX_IRIS_SO              "libirisdb.so"
#define DBX_IRIS_DYLIB           "libirisdb.dylib"
#define DBX_ISCIRIS_SO           "libiscirisdb.so"
#define DBX_ISCIRIS_DYLIB        "libiscirisdb.dylib"
#define DBX_YDB_SO               "libyottadb.so"
#define DBX_YDB_DYLIB            "libyottadb.dylib"
#define DBX_GTM_SO               "libgtmshr.so"
#define DBX_GTM_DYLIB            "libgtmshr.dylib"
#endif

#if defined(__linux__) || defined(linux) || defined(LINUX)
#define DBX_MEMCPY(a,b,c)           memmove(a,b,c)
#else
#define DBX_MEMCPY(a,b,c)           memcpy(a,b,c)
#endif

/* v2.4.24 */
#if defined(_WIN32)
#define DBX_TRACE_INIT(a)           short dbx_trace = a;
#define DBX_TRACE(a)                dbx_trace = a;
#define DBX_TRACE_VAR               dbx_trace
#else
#define DBX_TRACE_INIT(a)
#define DBX_TRACE(a)
#define DBX_TRACE_VAR               0
#endif

#define DBX_LOCK(TIMEOUT) \
   if (pcon->use_db_mutex) { \
      mg_mutex_lock(pcon->p_db_mutex, TIMEOUT); \
   } \

#define DBX_LOCK_EX(RC, TIMEOUT) \
   if (pcon->use_db_mutex) { \
      RC = mg_mutex_lock(pcon->p_db_mutex, TIMEOUT); \
   } \

#define DBX_UNLOCK() \
   if (pcon->use_db_mutex) { \
      mg_mutex_unlock(pcon->p_db_mutex); \
   } \

#define DBX_UNLOCK_EX(RC) \
   if (pcon->use_db_mutex) { \
      RC = mg_mutex_unlock(pcon->p_db_mutex); \
   } \

#define MG_LOG_REQUEST_FRAME(PWEB, HEAD, SIZE) \
   if (PWEB->plog->log_frames) { \
      char bufferx[256]; \
      sprintf(bufferx, "Request to DB Server: 0x%02x%02x%02x%02x (%lu Bytes)", (unsigned char) HEAD[3], (unsigned char) HEAD[2], (unsigned char) HEAD[1], (unsigned char) HEAD[0], (unsigned long) SIZE); \
      mg_log_event(PWEB->plog, PWEB, bufferx, "mg_web: Send request", 0); \
   } \

#define MG_LOG_RESPONSE_FRAME(PWEB, HEAD, SIZE) \
   if (PWEB->plog->log_frames) { \
      char bufferx[256]; \
      if (SIZE > 0) \
         sprintf(bufferx, "%sResponse from DB Server: 0x%02x%02x%02x%02x (%lu Bytes; sort=%d; type=%d)", PWEB->response_streamed ? "Chunked " : "", (unsigned char) HEAD[3], (unsigned char) HEAD[2], (unsigned char) HEAD[1], (unsigned char) HEAD[0], (unsigned long) SIZE, pweb->output_val.sort, pweb->output_val.type); \
      else \
         sprintf(bufferx, "%sResponse from DB Server: 0x%02x%02x%02x%02x (EOF)", PWEB->response_streamed ? "Chunked " : "", (unsigned char) HEAD[3], (unsigned char) HEAD[2], (unsigned char) HEAD[1], (unsigned char) HEAD[0]); \
      mg_log_event(PWEB->plog, PWEB, bufferx, "mg_web: Read response", 0); \
   } \

#define MG_LOG_REQUEST_BUFFER(PWEB, BUFFER, SIZE) \
   if (PWEB->plog->log_transmissions) { \
      char bufferx[256]; \
      sprintf(bufferx, "mg_web: Request to DB Server: (%lu Bytes)", (unsigned long) SIZE); \
      mg_log_buffer(pweb->plog, pweb, (char *) BUFFER, (int) SIZE, bufferx, 0); \
   } \

#define MG_LOG_RESPONSE_BUFFER(PWEB, HEAD, BUFFER, SIZE) \
   if (PWEB->plog->log_transmissions) { \
      char bufferx[256]; \
      if (SIZE > 0) \
         sprintf(bufferx, "mg_web: %sResponse from DB Server: 0x%02x%02x%02x%02x (%lu Bytes; sort=%d; type=%d)", PWEB->response_streamed ? "Chunked " : "", (unsigned char) HEAD[3], (unsigned char) HEAD[2], (unsigned char) HEAD[1], (unsigned char) HEAD[0], (unsigned long) SIZE, pweb->output_val.sort, pweb->output_val.type); \
      else \
         sprintf(bufferx, "mg_web: %sResponse from DB Server: 0x%02x%02x%02x%02x (EOF)", PWEB->response_streamed ? "Chunked " : "", (unsigned char) HEAD[3], (unsigned char) HEAD[2], (unsigned char) HEAD[1], (unsigned char) HEAD[0]); \
      mg_log_buffer(PWEB->plog, PWEB, BUFFER, SIZE, bufferx, 0); \
   } \

#define MG_LOG_RESPONSE_BUFFER_TO_WEBSERVER(PWEB, BUFFER, SIZE) \
   if (PWEB->plog->log_transmissions_to_webserver) { \
      char bufferx[256]; \
      sprintf(bufferx, "mg_web: Response to Web Server: (%lu Bytes)", (unsigned long) SIZE); \
      mg_log_buffer(pweb->plog, pweb, (char *) BUFFER, (int) SIZE, bufferx, 0); \
   } \

#define MG_LOG_RESPONSE_HEADER(PWEB) \
   if (PWEB->plog->log_frames || PWEB->plog->log_transmissions || PWEB->plog->log_transmissions_to_webserver) { \
      char bufferx[256]; \
      sprintf(bufferx, "mg_web: Response HTTP Header: (%d Bytes)", PWEB->response_headers_len); \
      mg_log_buffer(pweb->plog, pweb, (char *) PWEB->response_headers, (int) PWEB->response_headers_len, bufferx, 0); \
   } \


#define NETX_TIMEOUT             30
#define NETX_QUEUE_TIMEOUT       60
#define NETX_IPV6                1
#define NETX_READ_EOF            -9
#define NETX_READ_NOCON          -1
#define NETX_READ_ERROR          -2
#define NETX_READ_TIMEOUT        -3
#define NETX_RECV_BUFFER         32768

#if defined(LINUX)
#define NETX_MEMCPY(a,b,c)       memmove(a,b,c)
#else
#define NETX_MEMCPY(a,b,c)       memcpy(a,b,c)
#endif

#define MG_CGI_SUCCESS           0
#define MG_CGI_LIST              1
#define MG_CGI_UNDEFINED         -1
#define MG_CGI_TOOLONG           -2

#define MG_WS_IIS                1
#define MG_WS_APACHE             2
#define MG_WS_NGINX              3

#if defined(_WIN32)
/*
#if defined(MG_DBA_DSO)
#define DBX_EXTFUN(a)    __declspec(dllexport) a __cdecl
#else
#define DBX_EXTFUN(a)    a
#endif
*/

#define NETX_WSASOCKET               netx_so.p_WSASocket
#define NETX_WSAGETLASTERROR         netx_so.p_WSAGetLastError
#define NETX_WSASTARTUP              netx_so.p_WSAStartup
#define NETX_WSACLEANUP              netx_so.p_WSACleanup
#define NETX_WSAFDISET               netx_so.p_WSAFDIsSet
#define NETX_WSARECV                 netx_so.p_WSARecv
#define NETX_WSASEND                 netx_so.p_WSASend

#define NETX_WSASTRINGTOADDRESS      netx_so.p_WSAStringToAddress
#define NETX_WSAADDRESSTOSTRING      netx_so.p_WSAAddressToString
#define NETX_GETADDRINFO             netx_so.p_getaddrinfo
#define NETX_FREEADDRINFO            netx_so.p_freeaddrinfo
#define NETX_GETNAMEINFO             netx_so.p_getnameinfo
#define NETX_GETPEERNAME             netx_so.p_getpeername
#define NETX_INET_NTOP               netx_so.p_inet_ntop
#define NETX_INET_PTON               netx_so.p_inet_pton

#define NETX_CLOSESOCKET             netx_so.p_closesocket
#define NETX_GETHOSTNAME             netx_so.p_gethostname
#define NETX_GETHOSTBYNAME           netx_so.p_gethostbyname
#define NETX_SETSERVBYNAME           netx_so.p_getservbyname
#define NETX_GETHOSTBYADDR           netx_so.p_gethostbyaddr
#define NETX_HTONS                   netx_so.p_htons
#define NETX_HTONL                   netx_so.p_htonl
#define NETX_NTOHL                   netx_so.p_ntohl
#define NETX_NTOHS                   netx_so.p_ntohs
#define NETX_CONNECT                 netx_so.p_connect
#define NETX_INET_ADDR               netx_so.p_inet_addr
#define NETX_INET_NTOA               netx_so.p_inet_ntoa
#define NETX_SOCKET                  netx_so.p_socket
#define NETX_SETSOCKOPT              netx_so.p_setsockopt
#define NETX_GETSOCKOPT              netx_so.p_getsockopt
#define NETX_GETSOCKNAME             netx_so.p_getsockname
#define NETX_SELECT                  netx_so.p_select
#define NETX_RECV                    netx_so.p_recv
#define NETX_SEND                    netx_so.p_send
#define NETX_SHUTDOWN                netx_so.p_shutdown
#define NETX_BIND                    netx_so.p_bind
#define NETX_LISTEN                  netx_so.p_listen
#define NETX_ACCEPT                  netx_so.p_accept

#define  NETX_FD_ISSET(fd, set)              netx_so.p_WSAFDIsSet((SOCKET)(fd), (fd_set *)(set))

typedef int (WINAPI * MG_LPFN_WSAFDISSET)       (SOCKET, fd_set *);

typedef LPTHREAD_START_ROUTINE   DBX_THR_FUNCTION;
typedef DWORD           DBXTHID;
#define DBX_THR_TYPE    DWORD WINAPI
#define DBX_THR_RETURN  ((DWORD) rc)

typedef HINSTANCE       DBXPLIB;
typedef FARPROC         DBXPROC;

typedef LPSOCKADDR      xLPSOCKADDR;
typedef u_long          *xLPIOCTL;
typedef const char      *xLPSENDBUF;
typedef char            *xLPRECVBUF;

#ifdef _WIN64
typedef int             socklen_netx;
#else
typedef size_t          socklen_netx;
#endif

#define SOCK_ERROR(n)   (n == SOCKET_ERROR)
#define INVALID_SOCK(n) (n == INVALID_SOCKET)
#define NOT_BLOCKING(n) (n != WSAEWOULDBLOCK)

#define BZERO(b,len) (memset((b), '\0', (len)), (void) 0)

#else /* #if defined(_WIN32) */

#define DBX_EXTFUN(a)    a

#define NETX_WSASOCKET               WSASocket
#define NETX_WSAGETLASTERROR         WSAGetLastError
#define NETX_WSASTARTUP              WSAStartup
#define NETX_WSACLEANUP              WSACleanup
#define NETX_WSAFDIsSet              WSAFDIsSet
#define NETX_WSARECV                 WSARecv
#define NETX_WSASEND                 WSASend

#define NETX_WSASTRINGTOADDRESS      WSAStringToAddress
#define NETX_WSAADDRESSTOSTRING      WSAAddressToString
#define NETX_GETADDRINFO             getaddrinfo
#define NETX_FREEADDRINFO            freeaddrinfo
#define NETX_GETNAMEINFO             getnameinfo
#define NETX_GETPEERNAME             getpeername
#define NETX_INET_NTOP               inet_ntop
#define NETX_INET_PTON               inet_pton

#define NETX_CLOSESOCKET             closesocket
#define NETX_GETHOSTNAME             gethostname
#define NETX_GETHOSTBYNAME           gethostbyname
#define NETX_SETSERVBYNAME           getservbyname
#define NETX_GETHOSTBYADDR           gethostbyaddr
#define NETX_HTONS                   htons
#define NETX_HTONL                   htonl
#define NETX_NTOHL                   ntohl
#define NETX_NTOHS                   ntohs
#define NETX_CONNECT                 connect
#define NETX_INET_ADDR               inet_addr
#define NETX_INET_NTOA               inet_ntoa
#define NETX_SOCKET                  socket
#define NETX_SETSOCKOPT              setsockopt
#define NETX_GETSOCKOPT              getsockopt
#define NETX_GETSOCKNAME             getsockname
#define NETX_SELECT                  select
#define NETX_RECV                    recv
#define NETX_SEND                    send
#define NETX_SHUTDOWN                shutdown
#define NETX_BIND                    bind
#define NETX_LISTEN                  listen
#define NETX_ACCEPT                  accept

#define NETX_FD_ISSET(fd, set) FD_ISSET(fd, set)

typedef void  *(*DBX_THR_FUNCTION) (void * arg);

typedef pthread_t       DBXTHID;
#define DBX_THR_TYPE    void *
#define DBX_THR_RETURN  NULL
typedef void            *DBXPLIB;
typedef void            *DBXPROC;

typedef unsigned long   DWORD;
typedef unsigned long   WORD;
typedef int             WSADATA;
typedef int             SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr * LPSOCKADDR;
typedef struct hostent  HOSTENT;
typedef struct hostent  * LPHOSTENT;
typedef struct servent  SERVENT;
typedef struct servent  * LPSERVENT;

#ifdef NETX_BS_GEN_PTR
typedef const void      * xLPSOCKADDR;
typedef void            * xLPIOCTL;
typedef const void      * xLPSENDBUF;
typedef void            * xLPRECVBUF;
#else
typedef LPSOCKADDR      xLPSOCKADDR;
typedef char            * xLPIOCTL;
typedef const char      * xLPSENDBUF;
typedef char            * xLPRECVBUF;
#endif /* #ifdef NETX_BS_GEN_PTR */

#if defined(OSF1) || defined(HPUX) || defined(HPUX10) || defined(HPUX11)
typedef int             socklen_netx;
#elif defined(LINUX) || defined(AIX) || defined(AIX5) || defined(MACOSX)
typedef socklen_t       socklen_netx;
#else
typedef size_t          socklen_netx;
#endif

#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif

#define SOCK_ERROR(n)   (n < 0)
#define INVALID_SOCK(n) (n < 0)
#define NOT_BLOCKING(n) (n != EWOULDBLOCK && n != 2)

#define BZERO(b, len)   (bzero(b, len))

#endif /* #if defined(_WIN32) */

#if defined(__MINGW32__)

typedef
INT
(WSAAPI * LPFN_INET_PTONA)(
    __in                                INT             Family,
    __in                                PCSTR           pszAddrString,
    __out_bcount(sizeof(IN6_ADDR))      PVOID           pAddrBuf
    );

#define LPFN_INET_PTON          LPFN_INET_PTONA

typedef
PCSTR
(WSAAPI * LPFN_INET_NTOPA)(
    __in                                INT             Family,
    __in                                PVOID           pAddr,
    __out_ecount(StringBufSize)         PSTR            pStringBuf,
    __in                                size_t          StringBufSize
    );

#define LPFN_INET_NTOP          LPFN_INET_NTOPA

#endif


#if defined(_WIN32)
#if defined(MG_USE_MS_TYPEDEFS)

typedef LPFN_WSASOCKET                MG_LPFN_WSASOCKET;
typedef LPFN_WSAGETLASTERROR          MG_LPFN_WSAGETLASTERROR; 
typedef LPFN_WSASTARTUP               MG_LPFN_WSASTARTUP;
typedef LPFN_WSACLEANUP               MG_LPFN_WSACLEANUP;
typedef LPFN_WSARECV                  MG_LPFN_WSARECV;
typedef LPFN_WSASEND                  MG_LPFN_WSASEND;

#if defined(NETX_IPV6)
typedef LPFN_WSASTRINGTOADDRESS       MG_LPFN_WSASTRINGTOADDRESS;
typedef LPFN_WSAADDRESSTOSTRING       MG_LPFN_WSAADDRESSTOSTRING;
typedef LPFN_GETADDRINFO              MG_LPFN_GETADDRINFO;
typedef LPFN_FREEADDRINFO             MG_LPFN_FREEADDRINFO;
typedef LPFN_GETNAMEINFO              MG_LPFN_GETNAMEINFO;
typedef LPFN_GETPEERNAME              MG_LPFN_GETPEERNAME;
typedef LPFN_INET_NTOP                MG_LPFN_INET_NTOP;
typedef LPFN_INET_PTON                MG_LPFN_INET_PTON;
#endif

typedef LPFN_CLOSESOCKET              MG_LPFN_CLOSESOCKET;
typedef LPFN_GETHOSTNAME              MG_LPFN_GETHOSTNAME;
typedef LPFN_GETHOSTBYNAME            MG_LPFN_GETHOSTBYNAME;
typedef LPFN_GETHOSTBYADDR            MG_LPFN_GETHOSTBYADDR;
typedef LPFN_GETSERVBYNAME            MG_LPFN_GETSERVBYNAME;

typedef LPFN_HTONS                    MG_LPFN_HTONS;
typedef LPFN_HTONL                    MG_LPFN_HTONL;
typedef LPFN_NTOHL                    MG_LPFN_NTOHL;
typedef LPFN_NTOHS                    MG_LPFN_NTOHS;
typedef LPFN_CONNECT                  MG_LPFN_CONNECT;
typedef LPFN_INET_ADDR                MG_LPFN_INET_ADDR;
typedef LPFN_INET_NTOA                MG_LPFN_INET_NTOA;

typedef LPFN_SOCKET                   MG_LPFN_SOCKET;
typedef LPFN_SETSOCKOPT               MG_LPFN_SETSOCKOPT;
typedef LPFN_GETSOCKOPT               MG_LPFN_GETSOCKOPT;
typedef LPFN_GETSOCKNAME              MG_LPFN_GETSOCKNAME;
typedef LPFN_SELECT                   MG_LPFN_SELECT;
typedef LPFN_RECV                     MG_LPFN_RECV;
typedef LPFN_SEND                     MG_LPFN_SEND;
typedef LPFN_SHUTDOWN                 MG_LPFN_SHUTDOWN;
typedef LPFN_BIND                     MG_LPFN_BIND;
typedef LPFN_LISTEN                   MG_LPFN_LISTEN;
typedef LPFN_ACCEPT                   MG_LPFN_ACCEPT;

#else

typedef int                   (WSAAPI * MG_LPFN_WSASTARTUP)          (WORD wVersionRequested, LPWSADATA lpWSAData);
typedef int                   (WSAAPI * MG_LPFN_WSACLEANUP)          (void);
typedef int                   (WSAAPI * MG_LPFN_WSAGETLASTERROR)     (void);
typedef SOCKET                (WSAAPI * MG_LPFN_WSASOCKET)           (int af, int type, int protocol, LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags);
typedef int                   (WSAAPI * MG_LPFN_WSARECV)             (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef int                   (WSAAPI * MG_LPFN_WSASEND)             (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
typedef INT                   (WSAAPI * MG_LPFN_WSASTRINGTOADDRESS)  (LPSTR AddressString, INT AddressFamily, LPWSAPROTOCOL_INFOA lpProtocolInfo, LPSOCKADDR lpAddress, LPINT lpAddressLength);
typedef INT                   (WSAAPI * MG_LPFN_WSAADDRESSTOSTRING)  (LPSOCKADDR lpsaAddress, DWORD dwAddressLength, LPWSAPROTOCOL_INFOA lpProtocolInfo, LPSTR lpszAddressString, LPDWORD lpdwAddressStringLength);
typedef INT                   (WSAAPI * MG_LPFN_GETADDRINFO)         (PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA * pHints, PADDRINFOA * ppResult);
typedef VOID                  (WSAAPI * MG_LPFN_FREEADDRINFO)        (PADDRINFOA pAddrInfo);
typedef int                   (WSAAPI * MG_LPFN_GETNAMEINFO)         (const SOCKADDR * pSockaddr, socklen_t SockaddrLength, PCHAR pNodeBuffer, DWORD NodeBufferSize, PCHAR pServiceBuffer, DWORD ServiceBufferSize, INT Flags);
typedef int                   (WSAAPI * MG_LPFN_GETPEERNAME)         (SOCKET s, struct sockaddr FAR * name, int FAR * namelen);
typedef PCSTR                 (WSAAPI * MG_LPFN_INET_NTOP)           (INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize);
typedef INT                   (WSAAPI * MG_LPFN_INET_PTON)           (INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
typedef int                   (WSAAPI * MG_LPFN_CLOSESOCKET)         (SOCKET s);
typedef int                   (WSAAPI * MG_LPFN_GETHOSTNAME)         (char FAR * name, int namelen);
typedef struct hostent FAR *  (WSAAPI * MG_LPFN_GETHOSTBYNAME)       (const char FAR * name);
typedef struct hostent FAR *  (WSAAPI * MG_LPFN_GETHOSTBYADDR)       (const char FAR * addr, int len, int type);
typedef struct servent FAR *  (WSAAPI * MG_LPFN_GETSERVBYNAME)       (const char FAR * name, const char FAR * proto);
typedef u_short               (WSAAPI * MG_LPFN_HTONS)               (u_short hostshort);
typedef u_long                (WSAAPI * MG_LPFN_HTONL)               (u_long hostlong);
typedef u_long                (WSAAPI * MG_LPFN_NTOHL)               (u_long netlong);
typedef u_short               (WSAAPI * MG_LPFN_NTOHS)               (u_short netshort);
typedef char FAR *            (WSAAPI * MG_LPFN_INET_NTOA)           (struct in_addr in);
typedef int                   (WSAAPI * MG_LPFN_CONNECT)             (SOCKET s, const struct sockaddr FAR * name, int namelen);
typedef unsigned long         (WSAAPI * MG_LPFN_INET_ADDR)           (const char FAR * cp);
typedef SOCKET                (WSAAPI * MG_LPFN_SOCKET)              (int af, int type, int protocol);
typedef int                   (WSAAPI * MG_LPFN_SETSOCKOPT)          (SOCKET s, int level, int optname, const char FAR * optval, int optlen);
typedef int                   (WSAAPI * MG_LPFN_GETSOCKOPT)          (SOCKET s, int level, int optname, char FAR * optval, int FAR * optlen);
typedef int                   (WSAAPI * MG_LPFN_GETSOCKNAME)         (SOCKET s, struct sockaddr FAR * name, int FAR * namelen);
typedef int                   (WSAAPI * MG_LPFN_SELECT)              (int nfds, fd_set FAR * readfds, fd_set FAR * writefds, fd_set FAR *exceptfds, const struct timeval FAR * timeout);
typedef int                   (WSAAPI * MG_LPFN_RECV)                (SOCKET s, char FAR * buf, int len, int flags);
typedef int                   (WSAAPI * MG_LPFN_SEND)                (SOCKET s, const char FAR * buf, int len, int flags);
typedef int                   (WSAAPI * MG_LPFN_SHUTDOWN)            (SOCKET s, int how);
typedef int                   (WSAAPI * MG_LPFN_BIND)                (SOCKET s, const struct sockaddr FAR * name, int namelen);
typedef int                   (WSAAPI * MG_LPFN_LISTEN)              (SOCKET s, int backlog);
typedef SOCKET                (WSAAPI * MG_LPFN_ACCEPT)              (SOCKET s, struct sockaddr FAR * addr, int FAR * addrlen);

#endif
#endif /* #if defined(_WIN32) */

typedef struct tagNETXSOCK {

   unsigned char                 winsock_ready;
   short                         sock;
   short                         load_attempted;
   short                         nagle_algorithm;
   short                         winsock;
   short                         ipv6;
   DBXPLIB                       plibrary;

   char                          libnam[256];

#if defined(_WIN32)
   WSADATA                       wsadata;
   int                           wsastartup;
   WORD                          version_requested;
   MG_LPFN_WSASOCKET                p_WSASocket;
   MG_LPFN_WSAGETLASTERROR          p_WSAGetLastError; 
   MG_LPFN_WSASTARTUP               p_WSAStartup;
   MG_LPFN_WSACLEANUP               p_WSACleanup;
   MG_LPFN_WSAFDISSET               p_WSAFDIsSet;
   MG_LPFN_WSARECV                  p_WSARecv;
   MG_LPFN_WSASEND                  p_WSASend;

#if defined(NETX_IPV6)
   MG_LPFN_WSASTRINGTOADDRESS       p_WSAStringToAddress;
   MG_LPFN_WSAADDRESSTOSTRING       p_WSAAddressToString;
   MG_LPFN_GETADDRINFO              p_getaddrinfo;
   MG_LPFN_FREEADDRINFO             p_freeaddrinfo;
   MG_LPFN_GETNAMEINFO              p_getnameinfo;
   MG_LPFN_GETPEERNAME              p_getpeername;
   MG_LPFN_INET_NTOP                p_inet_ntop;
   MG_LPFN_INET_PTON                p_inet_pton;
#else
   LPVOID                        p_WSAStringToAddress;
   LPVOID                        p_WSAAddressToString;
   LPVOID                        p_getaddrinfo;
   LPVOID                        p_freeaddrinfo;
   LPVOID                        p_getnameinfo;
   LPVOID                        p_getpeername;
   LPVOID                        p_inet_ntop;
   LPVOID                        p_inet_pton;
#endif

   MG_LPFN_CLOSESOCKET              p_closesocket;
   MG_LPFN_GETHOSTNAME              p_gethostname;
   MG_LPFN_GETHOSTBYNAME            p_gethostbyname;
   MG_LPFN_GETHOSTBYADDR            p_gethostbyaddr;
   MG_LPFN_GETSERVBYNAME            p_getservbyname;

   MG_LPFN_HTONS                    p_htons;
   MG_LPFN_HTONL                    p_htonl;
   MG_LPFN_NTOHL                    p_ntohl;
   MG_LPFN_NTOHS                    p_ntohs;
   MG_LPFN_CONNECT                  p_connect;
   MG_LPFN_INET_ADDR                p_inet_addr;
   MG_LPFN_INET_NTOA                p_inet_ntoa;

   MG_LPFN_SOCKET                   p_socket;
   MG_LPFN_SETSOCKOPT               p_setsockopt;
   MG_LPFN_GETSOCKOPT               p_getsockopt;
   MG_LPFN_GETSOCKNAME              p_getsockname;
   MG_LPFN_SELECT                   p_select;
   MG_LPFN_RECV                     p_recv;
   MG_LPFN_SEND                     p_send;
   MG_LPFN_SHUTDOWN                 p_shutdown;
   MG_LPFN_BIND                     p_bind;
   MG_LPFN_LISTEN                   p_listen;
   MG_LPFN_ACCEPT                   p_accept;
#endif /* #if defined(_WIN32) */

} NETXSOCK, *PNETXSOCK;


typedef struct tagDBXLOG {
   char log_file[256];
   char log_level[8];
   char log_filter[64];
   short log_errors;
   short log_verbose;
   short log_frames;
   short log_transmissions;
   short log_transmissions_to_webserver;
   short log_tls;
   unsigned long req_no;
   unsigned long fun_no;
   char product[4];
   char product_version[16];
} DBXLOG, *PDBXLOG;


typedef struct tagDBXZV {
   unsigned char  product;
   double         mg_version;
   int            majorversion;
   int            minorversion;
   int            mg_build;
   unsigned long  vnumber; /* yymbbbb */
   char           version[64];
} DBXZV, *PDBXZV;

typedef struct tagDBXMUTEX {
   unsigned char     created;
   int               stack;
#if defined(_WIN32)
   HANDLE            h_mutex;
#else
   pthread_mutex_t   h_mutex;
#endif /* #if defined(_WIN32) */
   DBXTHID           thid;
} DBXMUTEX, *PDBXMUTEX;


typedef struct tagDBXTHR {
   DBXTHID           thread_id;
#if defined(_WIN32)
   DWORD             stack_size;
   HANDLE            thread_handle;
#else
   int               stack_size;
#endif
} DBXTHR, *PDBXTHR;


typedef struct tagDBXCVAL {
   void           *pstr;
   CACHE_EXSTR    zstr;
} DBXCVAL, *PDBXCVAL;


typedef struct tagDBXVAL {
   int               type;
   int               sort;
   union {
      int            int32;
      long long      int64;
      double         real;
      unsigned int   oref;
      char           *str;
   } num;
   unsigned int      api_size;
   DBXSTR            svalue;
   DBXCVAL           cvalue;
   struct tagDBXVAL  *pnext;
} DBXVAL, *PDBXVAL;


typedef struct tagMGBUF {
   unsigned long     size;
   unsigned long     data_size;
   unsigned long     increment_size;
   unsigned char *   p_buffer;
} MGBUF, *LPMGBUF;


typedef struct tagDBXFUN {
   unsigned int   rflag;
   int            label_len;
   char *         label;
   int            routine_len;
   char *         routine;
   char           buffer[128];
} DBXFUN, *PDBXFUN;


typedef struct tagDBXISCSO {

   short             loaded;
   short             iris;
   short             merge_enabled;
   char              funprfx[8];
   char              libdir[256];
   char              libnam[256];
   char              dbname[32];
   DBXPLIB           p_library;

   int               (* p_CacheSetDir)                   (char * dir);
   int               (* p_CacheSecureStartA)             (CACHE_ASTRP username, CACHE_ASTRP password, CACHE_ASTRP exename, unsigned long flags, int tout, CACHE_ASTRP prinp, CACHE_ASTRP prout);
   int               (* p_CacheEnd)                      (void);

   unsigned char *   (* p_CacheExStrNew)                 (CACHE_EXSTRP zstr, int size);
   unsigned short *  (* p_CacheExStrNewW)                (CACHE_EXSTRP zstr, int size);
   wchar_t *         (* p_CacheExStrNewH)                (CACHE_EXSTRP zstr, int size);
   int               (* p_CachePushExStr)                (CACHE_EXSTRP sptr);
   int               (* p_CachePushExStrW)               (CACHE_EXSTRP sptr);
   int               (* p_CachePushExStrH)               (CACHE_EXSTRP sptr);
   int               (* p_CachePopExStr)                 (CACHE_EXSTRP sstrp);
   int               (* p_CachePopExStrW)                (CACHE_EXSTRP sstrp);
   int               (* p_CachePopExStrH)                (CACHE_EXSTRP sstrp);
   int               (* p_CacheExStrKill)                (CACHE_EXSTRP obj);
   int               (* p_CachePushStr)                  (int len, Callin_char_t * ptr);
   int               (* p_CachePushStrW)                 (int len, short * ptr);
   int               (* p_CachePushStrH)                 (int len, wchar_t * ptr);
   int               (* p_CachePopStr)                   (int * lenp, Callin_char_t ** strp);
   int               (* p_CachePopStrW)                  (int * lenp, short ** strp);
   int               (* p_CachePopStrH)                  (int * lenp, wchar_t ** strp);
   int               (* p_CachePushDbl)                  (double num);
   int               (* p_CachePushIEEEDbl)              (double num);
   int               (* p_CachePopDbl)                   (double * nump);
   int               (* p_CachePushInt)                  (int num);
   int               (* p_CachePopInt)                   (int * nump);
   int               (* p_CachePushInt64)                (CACHE_INT64 num);
   int               (* p_CachePopInt64)                 (CACHE_INT64 * nump);

   int               (* p_CachePushGlobal)               (int nlen, const Callin_char_t * nptr);
   int               (* p_CachePushGlobalX)              (int nlen, const Callin_char_t * nptr, int elen, const Callin_char_t * eptr);
   int               (* p_CacheGlobalGet)                (int narg, int flag);
   int               (* p_CacheGlobalSet)                (int narg);
   int               (* p_CacheGlobalData)               (int narg, int valueflag);
   int               (* p_CacheGlobalKill)               (int narg, int nodeonly);
   int               (* p_CacheGlobalOrder)              (int narg, int dir, int valueflag);
   int               (* p_CacheGlobalQuery)              (int narg, int dir, int valueflag);
   int               (* p_CacheGlobalIncrement)          (int narg);
   int               (* p_CacheGlobalRelease)            (void);

   int               (* p_CacheAcquireLock)              (int nsub, int flg, int tout, int * rval);
   int               (* p_CacheReleaseAllLocks)          (void);
   int               (* p_CacheReleaseLock)              (int nsub, int flg);
   int               (* p_CachePushLock)                 (int nlen, const Callin_char_t * nptr);

   int               (* p_CacheAddGlobal)                (int num, const Callin_char_t * nptr);
   int               (* p_CacheAddGlobalDescriptor)      (int num);
   int               (* p_CacheAddSSVN)                  (int num, const Callin_char_t * nptr);
   int               (* p_CacheAddSSVNDescriptor)        (int num);
   int               (* p_CacheMerge)                    (void);

   int               (* p_CachePushFunc)                 (unsigned int * rflag, int tlen, const Callin_char_t * tptr, int nlen, const Callin_char_t * nptr);
   int               (* p_CacheExtFun)                   (unsigned int flags, int narg);
   int               (* p_CachePushRtn)                  (unsigned int * rflag, int tlen, const Callin_char_t * tptr, int nlen, const Callin_char_t * nptr);
   int               (* p_CacheDoFun)                    (unsigned int flags, int narg);
   int               (* p_CacheDoRtn)                    (unsigned int flags, int narg);

   int               (* p_CacheCloseOref)                (unsigned int oref);
   int               (* p_CacheIncrementCountOref)       (unsigned int oref);
   int               (* p_CachePopOref)                  (unsigned int * orefp);
   int               (* p_CachePushOref)                 (unsigned int oref);
   int               (* p_CacheInvokeMethod)             (int narg);
   int               (* p_CachePushMethod)               (unsigned int oref, int mlen, const Callin_char_t * mptr, int flg);
   int               (* p_CacheInvokeClassMethod)        (int narg);
   int               (* p_CachePushClassMethod)          (int clen, const Callin_char_t * cptr, int mlen, const Callin_char_t * mptr, int flg);
   int               (* p_CacheGetProperty)              (void);
   int               (* p_CacheSetProperty)              (void);
   int               (* p_CachePushProperty)             (unsigned int oref, int plen, const Callin_char_t * pptr);

   int               (* p_CacheType)                     (void);

   int               (* p_CacheEvalA)                    (CACHE_ASTRP volatile expr);
   int               (* p_CacheExecuteA)                 (CACHE_ASTRP volatile cmd);
   int               (* p_CacheConvert)                  (unsigned long type, void * rbuf);

   int               (* p_CacheErrorA)                   (CACHE_ASTRP, CACHE_ASTRP, int *);
   int               (* p_CacheErrxlateA)                (int, CACHE_ASTRP);

   int               (* p_CacheEnableMultiThread)        (void);

} DBXISCSO, *PDBXISCSO;


typedef struct tagDBXYDBSO {
   short             loaded;
   char              libdir[256];
   char              libnam[256];
   char              funprfx[8];
   char              dbname[32];
   DBXPLIB           p_library;

   int               (* p_ydb_init)                      (void);
   int               (* p_ydb_exit)                      (void);
   int               (* p_ydb_malloc)                    (size_t size);
   int               (* p_ydb_free)                      (void *ptr);
   int               (* p_ydb_data_s)                    (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, unsigned int *ret_value);
   int               (* p_ydb_delete_s)                  (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int deltype);
   int               (* p_ydb_set_s)                     (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *value);
   int               (* p_ydb_get_s)                     (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_subscript_next_s)          (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_subscript_previous_s)      (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_node_next_s)               (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int *ret_subs_used, ydb_buffer_t *ret_subsarray);
   int               (* p_ydb_node_previous_s)           (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int *ret_subs_used, ydb_buffer_t *ret_subsarray);
   int               (* p_ydb_incr_s)                    (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *increment, ydb_buffer_t *ret_value);
   int               (* p_ydb_ci)                        (const char *c_rtn_name, ...);
   int               (* p_ydb_cip)                       (ci_name_descriptor *ci_info, ...);
   void              (*p_ydb_zstatus)                    (ydb_char_t* msg_buffer, ydb_long_t buf_len);
} DBXYDBSO, *PDBXYDBSO;


typedef struct tagDBXGTMSO {
   short             loaded;
   char              libdir[256];
   char              libnam[256];
   char              funprfx[8];
   char              dbname[32];
   DBXPLIB           p_library;
   xc_status_t       (* p_gtm_ci)       (const char *c_rtn_name, ...);
   xc_status_t       (* p_gtm_init)     (void);
   xc_status_t       (* p_gtm_exit)     (void);
   void              (* p_gtm_zstatus)  (char* msg, int len);
} DBXGTMSO, *PDBXGTMSO;


/* v2.3.21 */
typedef struct tagMGTLS {
   char              *name;
   char              *libpath;
   char              *cert_file;
   char              *key_file;
   char              *password;
   char              *ca_file;
   char              *ca_path;
   char              *cipher_list;
   char              *protocols[8];
   int               verify_peer;
   int               key_type;
   struct tagMGTLS   *pnext;
} MGTLS, *LPMGTLS;


typedef struct tagMGSRV {
   short             dbtype;
   short             offline;
   unsigned long     no_requests;
   time_t            time_offline; /* v2.2.20 */
   int               idle_timeout; /* v2.4.26 */
   int               health_check;  /* v2.2.20 */
   int               con_retry_time; /* v2.4.25 */
   int               con_retry_no;
   int               max_connections; /* v2.5.30 */
   char              *name;
   char              lcname[64]; /* v2.1.17 */
   int               name_len;
   char              *uci;
   char              *shdir;
   char              *ip_address;
   int               port;
   int               timeout;
   int               nagle_algorithm;
   unsigned long     max_string_size; /* v2.2.18 */
   char              *username;
   char              *password;
   char              *input_device;
   char              *output_device;
   char              *dbtype_name;
   MGBUF             *penv;
   int               net_connection;
   char              *tls_name; /* v2.3.21 */
   MGTLS             *ptls;
   struct tagMGSRV   *pnext;
} MGSRV, *LPMGSRV;

/* v2.1.16 */
typedef struct tagMGPSRV {
   char        *name;
   char        lcname[64]; /* v2.1.17 */
   MGSRV       *psrv;
   short       exclusive;
} MGPSRV, *LPMGPSRV;

/* v2.6.32 */
typedef struct tagMGWSMAP {
   int         name_len;
   char        *name;
   int         function_len;
   char        *function;
   struct tagMGWSMAP  *pnext;
} MGWSMAP, *LPMGWSMAP;

typedef struct tagMGPATH {
   int         cgi_max;
   int         srv_max;
   char        *name;
   int         name_len;
   char        *function;
   MGWSMAP     *pwsmap; /* v2.6.32 */
   int         load_balancing;
   int         server_no;
   int         sa_order;
   char        *sa_cookie;
   char        *sa_variables[4];
/*
   char        *servers[32];
   MGSRV       *psrv[32];
*/
   MGPSRV      servers[32]; /* v2.1.16 */
   char        *cgi[128];
   int         admin; /* v2.4.24 */
   struct tagMGPATH  *pnext;
} MGPATH, *LPMGPATH;


typedef struct tagDBXCON {
   int               alloc;
   int               inuse;
   int               argc;
   unsigned long     no_requests;
   unsigned long     pid;
   short             use_db_mutex;
   DBXMUTEX          *p_db_mutex;
   DBXMUTEX          db_mutex;
   DBXZV             *p_zv;
   DBXZV             zv;
   DBXISCSO          *p_isc_so;
   DBXYDBSO          *p_ydb_so;
   DBXGTMSO          *p_gtm_so;
   short             increment;
   int               net_connection;
   int               closed;
   int               timeout;
   int               current_timeout;
   int               eof;
   time_t            time_request; /* v2.4.26 */
   SOCKET            cli_socket;
   int               int_pipe[2];
   int               stream_tail_len;
   unsigned char     stream_tail[8];
   MGSRV             *psrv;
   void              *ptlscon;
   struct tagDBXCON  *pnext;
} DBXCON, *PDBXCON;


#define MG_CUSTOMPAGE_DBSERVER_NONE          0
#define MG_CUSTOMPAGE_DBSERVER_UNAVAILABLE   1
#define MG_CUSTOMPAGE_DBSERVER_BUSY          2
#define MG_CUSTOMPAGE_DBSERVER_DISABLED      3
#define MG_CUSTOMPAGE_DBSERVER_TIMEOUT       4

typedef struct tagMGSYS {
   int            config_size;
   int            cgi_max;
   int            timeout;
   unsigned long  requestno;
   unsigned long  chunking;
   unsigned long  request_buffer_size;
   char           module_file[256];
   char           config_file[256];
   char           config_error[512];
   char           info[256];
   char           *config;
   char           *custompage_dbserver_unavailable;
   char           *custompage_dbserver_busy;
   char           *custompage_dbserver_disabled;
   char           *custompage_dbserver_timeout; /* v2.7.33 */
   DBXLOG         *plog;
   char           cgi_base[64];
   char           *cgi[128];
   DBXLOG         log;
} MGSYS, *LPMGSYS;


#define MG_WS_BLOCK_DATA_SIZE              4096

typedef unsigned long long    mg_uint64_t;
typedef long long             mg_int64_t;

typedef struct tagMGWSMESS {
    int              type;
    unsigned char *  buffer;
    size_t           buffer_size;
    int              done;
    size_t           written;
} MGWSMESS, *PMGWSMESS;


typedef struct tagMGWSFDATA {
   mg_uint64_t       application_data_offset;
   unsigned char *   application_data;
   unsigned char     fin;
   unsigned char     opcode;
   unsigned int      utf8_state;
   mg_int64_t        message_length;
} MGWSFDATA, *PMGWSFDATA;


typedef struct tagMGWSRSTATE {
   int               framing_state;
   int               closing;
   unsigned short    status_code;
   unsigned char     fin;
   unsigned char     opcode;
   MGWSFDATA         control_frame;
   MGWSFDATA         message_frame;
   MGWSFDATA *       frame;
   mg_int64_t        payload_length;
   mg_int64_t        mask_offset;
   mg_int64_t        extension_bytes_remaining;
   int               payload_length_bytes_remaining;
   int               masking;
   int               mask_index;
   unsigned char     mask[4];
} MGWSRSTATE, *PMGWSRSTATE;

#define MG_WEBSOCKET_NOCON             0
#define MG_WEBSOCKET_HEADERS_SENT      10
#define MG_WEBSOCKET_CONNECTED         20
#define MG_WEBSOCKET_CLOSING           30
#define MG_WEBSOCKET_CLOSED            40
#define MG_WEBSOCKET_CLOSED_BYSERVER   50
#define MG_WEBSOCKET_CLOSED_BYCLIENT   60

typedef struct tagMGWEBSOCK {
   short             status;
   short             binary;
   int               closing;
   int               protocol_version;
   char              sec_websocket_key[256];
   char              sec_websocket_protocol[32];
   unsigned char     block[MG_WS_BLOCK_DATA_SIZE];
   mg_int64_t        block_size;
   unsigned char     status_code_buffer[2];
   size_t            remaining_length;
   DBXTHR            db_read_thread;
} MGWEBSOCK, *LPMGWEBSOCK;


typedef struct tagMGWEB {
   int            wstype; /* v2.7.33 web server type */
   int            evented;
   int            tls;
   int            sse; /* v2.7.33 Server Sent Events */
   int            http_version_major;
   int            http_version_minor;
   int            wserver_chunks_response;
   int            mg_connect_failed;
   int            failover_possible;
   int            request_long; /* v2.2.18 */
   int            request_clen;
   int            request_clen_remaining; /* v2.2.18 */
   int            request_csize; /* v2.2.18 */
   int            request_bsize; /* v2.2.18 */
   char           *request_content;
   char           *script_name;
   int            script_name_len;
   char           script_name_lc[256];
   char           *request_method;
   int            request_method_len;
   char           *query_string;
   int            query_string_len;
   char           request_content_type[1024];
   char           boundary[256]; /* v2.1.15 */
   char           *request_cookie;
   int            response_clen_server;
   int            response_streamed;
   int            response_chunked;
   int            response_clen;
   unsigned int   response_size;
   unsigned int   response_remaining;
   int            offs_content;
   char           *response_content;
   char           *response_headers;
   char           *response_content_type; /* v2.7.33 */
   char           *response_cache_control;
   char           *response_connection;
   int            response_headers_len;
   unsigned long  requestno_in;
   unsigned long  requestno_out;
   char           *requestno;
   char           *server;
   char           *serverno;
   char           *requestkey;
   char           *mode;
   char           key[32];
   unsigned char  db_chunk_head[8];
   DBXLOG         *plog;
   DBXSTR         input_buf;
   DBXVAL         output_val;
   DBXVAL         *poutput_val_last;
   int            offset;
   DBXVAL         args[DBX_MAXARGS];
   ydb_buffer_t   yargs[DBX_MAXARGS];

   int            error_no;
   int            error_code;
   char           error[DBX_ERROR_SIZE];

   MGPATH         *ppath;
   int            server_no;
   MGSRV          *psrv;
   DBXCON         *pcon;
   MGWEBSOCK      *pwsock;
   void           *pweb_server;
} MGWEB, *LPMGWEB;


#ifdef __cplusplus
extern "C" {
#endif

typedef void * (* MG_MALLOC)     (void *pweb_server, unsigned long size);
typedef void * (* MG_REALLOC)    (void *pweb_server, void *p, unsigned long size);
typedef int    (* MG_FREE)       (void *pweb_server, void *p);


#if defined(_WIN32)
extern CRITICAL_SECTION  mg_global_mutex;
#else
extern pthread_mutex_t   mg_global_mutex;
#endif

extern MGSYS         mg_system;
extern DBXCON *      mg_connection;
extern MGSRV *       mg_server;
extern MGPATH *      mg_path;

extern MG_MALLOC     mg_ext_malloc;
extern MG_REALLOC    mg_ext_realloc;
extern MG_FREE       mg_ext_free;


/* From web server interface code page */
int                     mg_get_cgi_variable           (MGWEB *pweb, char *name, char *pbuffer, int *pbuffer_size);
int                     mg_client_write               (MGWEB *pweb, unsigned char *pbuffer, int buffer_size);
int                     mg_client_write_now           (MGWEB *pweb, unsigned char *pbuffer, int buffer_size);
int                     mg_client_read                (MGWEB *pweb, unsigned char *pbuffer, int buffer_size);
int                     mg_suppress_headers           (MGWEB *pweb);
int                     mg_submit_headers             (MGWEB *pweb);

int                     mg_websocket_init             (MGWEB *pweb);
int                     mg_websocket_create_lock      (MGWEB *pweb);
int                     mg_websocket_destroy_lock     (MGWEB *pweb);
int                     mg_websocket_lock             (MGWEB *pweb);
int                     mg_websocket_unlock           (MGWEB *pweb);
int                     mg_websocket_write            (MGWEB *pweb, char *buffer, int len);
int                     mg_websocket_frame_init       (MGWEB *pweb);
int                     mg_websocket_frame_read       (MGWEB *pweb, MGWSRSTATE *pread_state);
int                     mg_websocket_frame_exit       (MGWEB *pweb);
size_t                  mg_websocket_read_block       (MGWEB *pweb, char *buffer, size_t bufsiz);
size_t                  mg_websocket_read             (MGWEB *pweb, char *buffer, size_t bufsiz);
size_t                  mg_websocket_queue_block      (MGWEB *pweb, int type, unsigned char *buffer, size_t buffer_size, short locked);
size_t                  mg_websocket_write_block      (MGWEB *pweb, int type, unsigned char *buffer, size_t buffer_size);
int                     mg_websocket_write            (MGWEB *pweb, char *buffer, int len);
int                     mg_websocket_exit             (MGWEB *pweb);


/* Core code page */
int                     mg_web                        (MGWEB *pweb);
int                     mg_web_process                (MGWEB *pweb);
int                     mg_parse_headers              (MGWEB *pweb);
int                     mg_web_execute                (MGWEB *pweb);
int                     mg_execute_request_long       (MGWEB *pweb, int (*p_write_chunk) (MGWEB *, unsigned char *, unsigned int, int));
int                     mg_write_chunk_tcp            (MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no);
int                     mg_write_chunk_isc            (MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no);
int                     mg_write_chunk_ydb            (MGWEB *pweb, unsigned char *netbuf, unsigned int netbuf_used, int chunk_no);
int                     mg_web_http_error             (MGWEB *pweb, int http_status_code, int custompage);
int                     mg_web_simple_response        (MGWEB *pweb, char *text, char *error, int context);
int                     mg_get_all_cgi_variables      (MGWEB *pweb);
int                     mg_get_path_configuration     (MGWEB *pweb);
int                     mg_add_cgi_variable           (MGWEB *pweb, char *name, int name_len, char *value, int value_len);
int                     mg_obtain_connection          (MGWEB *pweb);
int                     mg_obtain_server              (MGWEB *pweb, char *info, int context);
int                     mg_server_offline             (MGWEB *pweb, MGSRV *psrv, char *info, int context);
int                     mg_server_online              (MGWEB *pweb, MGSRV *psrv, char *info, int context);
int                     mg_connect                    (MGWEB *pweb, int context);
int                     mg_release_connection         (MGWEB *pweb, int close_connection);
MGWEB *                 mg_obtain_request_memory      (void *pweb_server, unsigned long request_clen, int wstype);
DBXVAL *                mg_extend_response_memory     (MGWEB *pweb);
int                     mg_release_request_memory     (MGWEB *pweb);
int                     mg_find_sa_variable           (MGWEB *pweb);
int                     mg_find_sa_variable_ex        (MGWEB *pweb, char *name, int name_len, unsigned char *nvpairs, int nvpairs_len);
int                     mg_find_sa_variable_ex_mp     (MGWEB *pweb, char *name, int name_len, unsigned char *content, int content_len); /* v2.1.15 */
int                     mg_find_sa_cookie             (MGWEB *pweb);
int                     mg_worker_init                ();
int                     mg_worker_exit                ();
int                     mg_parse_config               ();
int                     mg_set_log_level              (DBXLOG *plog, char *word, int wn);
int                     mg_verify_config              ();

int                     isc_load_library              (MGWEB *pweb);
int                     isc_authenticate              (MGWEB *pweb);
int                     isc_open                      (MGWEB *pweb);
int                     isc_parse_zv                  (char *zv, DBXZV * p_isc_sv);
int                     isc_change_namespace          (DBXCON *pcon, char *nspace);
int                     isc_pop_value                 (DBXCON *pcon, DBXVAL *value, int required_type);
int                     isc_error_message             (MGWEB *pweb, int error_code);

int                     ydb_load_library              (MGWEB *pweb);
int                     ydb_open                      (MGWEB *pweb);
int                     ydb_parse_zv                  (char *zv, DBXZV * p_ydb_sv);
int                     ydb_error_message             (MGWEB *pweb, int error_code);

int                     gtm_load_library              (MGWEB *pweb);
int                     gtm_open                      (MGWEB *pweb);
int                     gtm_parse_zv                  (char *zv, DBXZV * p_gtm_sv);
int                     gtm_error_message             (MGWEB *pweb, int error_code);

int                     mg_add_block_size             (unsigned char *block, unsigned long offset, unsigned long data_len, int dsort, int dtype);
unsigned long           mg_get_block_size             (unsigned char *block, unsigned long offset, int *dsort, int *dtype);
int                     mg_set_size                   (unsigned char *str, unsigned long data_len);
unsigned long           mg_get_size                   (unsigned char *str);

int                     mg_buf_init                   (MGBUF *p_buf, int size, int increment_size);
int                     mg_buf_resize                 (MGBUF *p_buf, unsigned long size);
int                     mg_buf_free                   (MGBUF *p_buf);
int                     mg_buf_cpy                    (MGBUF *p_buf, char *buffer, unsigned long size);
int                     mg_buf_cat                    (MGBUF *p_buf, char * uffer, unsigned long size);
void *                  mg_malloc                     (void *pweb_server, int size, short id);
void *                  mg_realloc                    (void *pweb_server, void *p, int curr_size, int new_size, short id);
int                     mg_free                       (void *pweb_server, void *p, short id);
int                     mg_memcpy                     (void * to, void *from, size_t size);

int                     mg_ucase                      (char *string);
int                     mg_lcase                      (char *string);
int                     mg_ccase                      (char *string);
int                     mg_log_init                   (DBXLOG *plog);
int                     mg_log_event                  (DBXLOG *plog, MGWEB *pweb, char *message, char *title, int level);
int                     mg_log_buffer                 (DBXLOG *plog, MGWEB *pweb, char *buffer, int buffer_len, char *title, int level);
DBXPLIB                 mg_dso_load                   (char *library);
DBXPROC                 mg_dso_sym                    (DBXPLIB p_library, char *symbol);
int                     mg_dso_unload                 (DBXPLIB p_library);
int                     mg_thread_create              (DBXTHR *pthr, DBX_THR_FUNCTION function, void * arg);
int                     mg_thread_terminate           (DBXTHR *pthr);
int                     mg_thread_join                (DBXTHR *pthr);
int                     mg_thread_exit                (void);
DBXTHID                 mg_current_thread_id          (void);
unsigned long           mg_current_process_id         (void);
int                     mg_error_message              (MGWEB *pweb, int error_code);
int                     mg_cleanup                    (MGWEB *pweb);

int                     mg_mutex_create               (DBXMUTEX *p_mutex);
int                     mg_mutex_lock                 (DBXMUTEX *p_mutex, int timeout);
int                     mg_mutex_unlock               (DBXMUTEX *p_mutex);
int                     mg_mutex_destroy              (DBXMUTEX *p_mutex);
int                     mg_init_critical_section      (void *p_crit);
int                     mg_delete_critical_section    (void *p_crit);
int                     mg_enter_critical_section     (void *p_crit);
int                     mg_leave_critical_section     (void *p_crit);
int                     mg_sleep                      (unsigned long msecs);
unsigned int            mg_file_size                  (char *file);

int                     netx_load_winsock             (MGWEB *pweb, int context);
int                     netx_tcp_connect              (MGWEB *pweb, int context);
int                     netx_tcp_handshake            (MGWEB *pweb, int context);
int                     netx_tcp_ping                 (MGWEB *pweb, int context);
int                     netx_tcp_command              (MGWEB *pweb, int command, int context);
int                     netx_tcp_read_stream          (MGWEB *pweb);
int                     netx_tcp_connect_ex           (MGWEB *pweb, xLPSOCKADDR p_srv_addr, socklen_netx srv_addr_len, int timeout);
int                     netx_tcp_disconnect           (MGWEB *pweb, int context);
int                     netx_tcp_write                (MGWEB *pweb, unsigned char *data, int size);
int                     netx_tcp_read                 (MGWEB *pweb, unsigned char *data, int size, int timeout, int context);
int                     netx_get_last_error           (int context);
int                     netx_get_error_message        (int error_code, char *message, int size, int context);
int                     netx_get_std_error_message    (int error_code, char *message, int size, int context);

#ifdef __cplusplus
}
#endif


#endif


