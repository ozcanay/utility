#pragma once

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>


#define BULL_INVALID_SOCKET  -1
#define bull_socket_t        int
#define bull_socklen_t       socklen_t
#define bull_fcntl_request_t int
#define bull_closesocket(s)  ::close(s)
#define BULL_EINTR           EINTR
#define BULL_EACCES          EACCES
#define BULL_EFAULT          EFAULT
#define BULL_EINVAL          EINVAL
#define BULL_EMFILE          EMFILE
#define BULL_EAGAIN          EAGAIN
#define BULL_EWOULDBLOCK     EWOULDBLOCK
#define BULL_EINPROGRESS     EINPROGRESS
#define BULL_EALREADY        EALREADY
#define BULL_ENOTSOCK        ENOTSOCK
#define BULL_EDESTADDRREQ    EDESTADDRREQ
#define BULL_EMSGSIZE        EMSGSIZE
#define BULL_EPROTOTYPE      EPROTOTYPE
#define BULL_ENOPROTOOPT     ENOPROTOOPT
#define BULL_EPROTONOSUPPORT EPROTONOSUPPORT
#if defined(ESOCKTNOSUPPORT)
    #define BULL_ESOCKTNOSUPPORT ESOCKTNOSUPPORT
#else
    #define BULL_ESOCKTNOSUPPORT -1
#endif
#define BULL_ENOTSUP         ENOTSUP
#define BULL_EPFNOSUPPORT    EPFNOSUPPORT
#define BULL_EAFNOSUPPORT    EAFNOSUPPORT
#define BULL_EADDRINUSE      EADDRINUSE
#define BULL_EADDRNOTAVAIL   EADDRNOTAVAIL
#define BULL_ENETDOWN        ENETDOWN
#define BULL_ENETUNREACH     ENETUNREACH
#define BULL_ENETRESET       ENETRESET
#define BULL_ECONNABORTED    ECONNABORTED
#define BULL_ECONNRESET      ECONNRESET
#define BULL_ENOBUFS         ENOBUFS
#define BULL_EISCONN         EISCONN
#define BULL_ENOTCONN        ENOTCONN
#if defined(ESHUTDOWN)
    #define BULL_ESHUTDOWN   ESHUTDOWN
#else
    #define BULL_ESHUTDOWN   -2
#endif
#define BULL_ETIMEDOUT       ETIMEDOUT
#define BULL_ECONNREFUSED    ECONNREFUSED
#if defined(EHOSTDOWN)
    #define BULL_EHOSTDOWN   EHOSTDOWN
#else
    #define BULL_EHOSTDOWN   -3
#endif
#define BULL_EHOSTUNREACH    EHOSTUNREACH
#define BULL_ESYSNOTREADY    -4
#define BULL_ENOTINIT        -5
#define BULL_HOST_NOT_FOUND  HOST_NOT_FOUND
#define BULL_TRY_AGAIN       TRY_AGAIN
#define BULL_NO_RECOVERY     NO_RECOVERY
#define BULL_NO_DATA         NO_DATA