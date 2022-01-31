#include "include/socket.hpp"

// #include "Poco/Net/NetException.h"
// #include "Poco/Net/StreamSocketImpl.h"
// #include "Poco/NumberFormatter.h"
// #include "Poco/Timestamp.h"
#include <cstring> // FD_SET needs memset on some platforms, so we can't use <cstring>


#include <sys/epoll.h>
#include <unistd.h>


namespace Bull {
namespace Net {

Socket::Socket()
	: sockfd_(BULL_INVALID_SOCKET)
	, blocking_(true)
{
}

Socket::Socket(bull_socket_t sockfd)
	: sockfd_(sockfd)
	, blocking_(true)
{
}

Socket::~Socket()
{
	close();
}

Socket* Socket::acceptConnection(SocketAddress& clientAddr)
{
	if(sockfd_ == BULL_INVALID_SOCKET) { 
        throw std::exception();
    }

	sockaddr_storage buffer;
	struct sockaddr* pSA = reinterpret_cast<struct sockaddr*>(&buffer);
	bull_socklen_t saLen = sizeof(buffer);
	bull_socket_t sd;

	do {
		sd = ::accept(sockfd_, pSA, &saLen);
	} while (sd == BULL_INVALID_SOCKET && lastError() == BULL_EINTR);

	if (sd != BULL_INVALID_SOCKET) {
		// clientAddr = SocketAddress(pSA, saLen);
		return new Socket(sockfd_); /// consider making this unique_ptr.
	}

	error(); // will throw

	return nullptr;
}


// void Socket::connect(const SocketAddress& address)
// {
// 	if (sockfd_ == BULL_INVALID_SOCKET) {
// 		// init(address.af());
// 	}

// 	int rc;
// 	do {
// 		rc = ::connect(sockfd_, address.addr(), address.length());
// 	} while (rc != 0 && lastError() == BULL_EINTR);

// 	if (rc != 0) {
// 		int err = lastError();
// 		error(err, address.toString());
// 	}
// }

// void Socket::connect(const SocketAddress& address, const Poco::Timespan& timeout)
// {
// 	if (sockfd_ == BULL_INVALID_SOCKET) {
// 		init(address.af());
// 	}
// 	setBlocking(false);
// 	try
// 	{
// #if defined(BULL_VXWORKS)
// 		int rc = ::connect(sockfd_, (sockaddr*) address.addr(), address.length());
// #else
// 		int rc = ::connect(sockfd_, address.addr(), address.length());
// #endif
// 		if (rc != 0)
// 		{
// 			int err = lastError();
// 			if (err != BULL_EINPROGRESS && err != BULL_EWOULDBLOCK)
// 				error(err, address.toString());
// 			if (!poll(timeout, SELECT_READ | SELECT_WRITE | SELECT_ERROR))
// 				throw Poco::TimeoutException("connect timed out", address.toString());
// 			err = socketError();
// 			if (err != 0) error(err);
// 		}
// 	}
// 	catch (Poco::Exception&)
// 	{
// 		setBlocking(true);
// 		throw;
// 	}
// 	setBlocking(true);
// }


// void Socket::connectNB(const SocketAddress& address)
// {
// 	if (sockfd_ == BULL_INVALID_SOCKET)
// 	{
// 		init(address.af());
// 	}
// 	setBlocking(false);
// #if defined(BULL_VXWORKS)
// 	int rc = ::connect(sockfd_, (sockaddr*) address.addr(), address.length());
// #else
// 	int rc = ::connect(sockfd_, address.addr(), address.length());
// #endif
// 	if (rc != 0)
// 	{
// 		int err = lastError();
// 		if (err != BULL_EINPROGRESS && err != BULL_EWOULDBLOCK)
// 			error(err, address.toString());
// 	}
// }


// void Socket::bind(const SocketAddress& address, bool reuseAddress)
// {
// 	bind(address, reuseAddress, reuseAddress);
// }


// void Socket::bind(const SocketAddress& address, bool reuseAddress, bool reusePort)
// {
// 	if (sockfd_ == BULL_INVALID_SOCKET)
// 	{
// 		init(address.af());
// 	}
// 	if (reuseAddress)
// 		setReuseAddress(true);
// 	if (reusePort)
// 		setReusePort(true);
// #if defined(BULL_VXWORKS)
// 	int rc = ::bind(sockfd_, (sockaddr*) address.addr(), address.length());
// #else
// 	int rc = ::bind(sockfd_, address.addr(), address.length());
// #endif
// 	if (rc != 0) error(address.toString());
// }

void Socket::listen(int backlog)
{
	if (sockfd_ == BULL_INVALID_SOCKET) {
        throw std::exception();
    }

	int rc = ::listen(sockfd_, backlog);
	if (rc != 0) {
        error();
    }
}

void Socket::close()
{
	if (sockfd_ != BULL_INVALID_SOCKET)
	{
		bull_closesocket(sockfd_);
		sockfd_ = BULL_INVALID_SOCKET;
	}
}

void Socket::shutdownReceive()
{
	if (sockfd_ == BULL_INVALID_SOCKET) {
        throw std::exception();
    }

	int rc = ::shutdown(sockfd_, 0);
	if (rc != 0) {
        error();
    }
}


void Socket::shutdownSend()
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	int rc = ::shutdown(sockfd_, 1);
	if (rc != 0) error();
}


void Socket::shutdown()
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	int rc = ::shutdown(sockfd_, 2);
	if (rc != 0) error();
}


int Socket::sendBytes(const void* buffer, int length, int flags)
{

	int rc;
	do {
		if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();
		rc = ::send(sockfd_, reinterpret_cast<const char*>(buffer), length, flags);
	} while (blocking_ && rc < 0 && lastError() == BULL_EINTR);

	if (rc < 0) error();
	return rc;
}

int Socket::receiveBytes(void* buffer, int length, int flags)
{
	checkBrokenTimeout(SELECT_READ);

	int rc;
	do {
		if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();
		rc = ::recv(sockfd_, reinterpret_cast<char*>(buffer), length, flags);
	} while (blocking_ && rc < 0 && lastError() == BULL_EINTR);

	if (rc < 0) {
		int err = lastError();
		if (err == BULL_EAGAIN && !blocking_)
			;
		else if (err == BULL_EAGAIN || err == BULL_ETIMEDOUT)
			throw std::exception();
		else
			error(err);
	}
	return rc;
}

int Socket::receiveFrom(void* buffer, int length, SocketAddress& address, int flags)
{
	sockaddr_storage abuffer;
	struct sockaddr* pSA = reinterpret_cast<struct sockaddr*>(&abuffer);
	bull_socklen_t saLen = sizeof(abuffer);
	bull_socklen_t* pSALen = &saLen;
	int rc = receiveFrom(buffer, length, &pSA, &pSALen, flags);
	if (rc >= 0)
	{
		address = SocketAddress(pSA, saLen);
	}
	return rc;
}

void Socket::sendUrgent(unsigned char data)
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	int rc = ::send(sockfd_, reinterpret_cast<const char*>(&data), sizeof(data), MSG_OOB);
	if (rc < 0) error();
}


int Socket::available()
{
	int result = 0;
	ioctl(FIONREAD, result);
	return result;
}

bool Socket::poll(const Poco::Timespan& timeout, int mode)
{
	// bull_socket_t sockfd = sockfd_;
	// if (sockfd == BULL_INVALID_SOCKET) throw std::exception();

	// int epollfd = epoll_create(1);
	// if (epollfd < 0) {
	// 	error("Can't create epoll queue");
	// }

	// struct epoll_event evin;
	// memset(&evin, 0, sizeof(evin));

	// if (mode & SELECT_READ)
	// 	evin.events |= EPOLLIN;
	// if (mode & SELECT_WRITE)
	// 	evin.events |= EPOLLOUT;
	// if (mode & SELECT_ERROR)
	// 	evin.events |= EPOLLERR;

	// if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &evin) < 0)
	// {
	// 	::close(epollfd);
	// 	error("Can't insert socket to epoll queue");
	// }

	// Poco::Timespan remainingTime(timeout);
	// int rc;
	// do
	// {
	// 	struct epoll_event evout;
	// 	memset(&evout, 0, sizeof(evout));

	// 	Poco::Timestamp start;
	// 	rc = epoll_wait(epollfd, &evout, 1, remainingTime.totalMilliseconds());
	// 	if (rc < 0 && lastError() == BULL_EINTR)
	// 	{
	// 		Poco::Timestamp end;
	// 		Poco::Timespan waited = end - start;
	// 		if (waited < remainingTime)
	// 			remainingTime -= waited;
	// 		else
	// 			remainingTime = 0;
	// 	}
	// }
	// while (rc < 0 && lastError() == BULL_EINTR);

	// ::close(epollfd);
	// if (rc < 0) error();
	// return rc > 0;
}


void Socket::setSendBufferSize(int size)
{
	setOption(SOL_SOCKET, SO_SNDBUF, size);
}

int Socket::getSendBufferSize()
{
	int result;
	getOption(SOL_SOCKET, SO_SNDBUF, result);
	return result;
}

void Socket::setReceiveBufferSize(int size)
{
	setOption(SOL_SOCKET, SO_RCVBUF, size);
}

int Socket::getReceiveBufferSize()
{
	int result;
	getOption(SOL_SOCKET, SO_RCVBUF, result);
	return result;
}

SocketAddress Socket::address()
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	sockaddr_storage buffer;
	struct sockaddr* pSA = reinterpret_cast<struct sockaddr*>(&buffer);
	bull_socklen_t saLen = sizeof(buffer);
	int rc = ::getsockname(sockfd_, pSA, &saLen);
	if (rc == 0)
		return SocketAddress(pSA, saLen);
	else
		error();
	return SocketAddress();
}


SocketAddress Socket::peerAddress()
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	sockaddr_storage buffer;
	struct sockaddr* pSA = reinterpret_cast<struct sockaddr*>(&buffer);
	bull_socklen_t saLen = sizeof(buffer);
	int rc = ::getpeername(sockfd_, pSA, &saLen);
	if (rc == 0)
		return SocketAddress(pSA, saLen);
	else
		error();
	return SocketAddress();
}


void Socket::setOption(int level, int option, int value)
{
	setRawOption(level, option, &value, sizeof(value));
}


void Socket::setOption(int level, int option, unsigned value)
{
	setRawOption(level, option, &value, sizeof(value));
}


void Socket::setOption(int level, int option, unsigned char value)
{
	setRawOption(level, option, &value, sizeof(value));
}


void Socket::setOption(int level, int option, const IPAddress& value)
{
	setRawOption(level, option, value.addr(), value.length());
}


void Socket::setOption(int level, int option, const Poco::Timespan& value)
{
	struct timeval tv;
	tv.tv_sec  = (long) value.totalSeconds();
	tv.tv_usec = (long) value.useconds();

	setRawOption(level, option, &tv, sizeof(tv));
}


void Socket::setRawOption(int level, int option, const void* value, bull_socklen_t length)
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

#if defined(BULL_VXWORKS)
	int rc = ::setsockopt(sockfd_, level, option, (char*) value, length);
#else
	int rc = ::setsockopt(sockfd_, level, option, reinterpret_cast<const char*>(value), length);
#endif
	if (rc == -1) error();
}


void Socket::getOption(int level, int option, int& value)
{
	bull_socklen_t len = sizeof(value);
	getRawOption(level, option, &value, len);
}


void Socket::getOption(int level, int option, unsigned& value)
{
	bull_socklen_t len = sizeof(value);
	getRawOption(level, option, &value, len);
}


void Socket::getOption(int level, int option, unsigned char& value)
{
	bull_socklen_t len = sizeof(value);
	getRawOption(level, option, &value, len);
}


void Socket::getOption(int level, int option, Poco::Timespan& value)
{
	struct timeval tv;
	bull_socklen_t len = sizeof(tv);
	getRawOption(level, option, &tv, len);
	value.assign(tv.tv_sec, tv.tv_usec);
}


void Socket::getOption(int level, int option, IPAddress& value)
{
	char buffer[IPAddress::MAX_ADDRESS_LENGTH];
	bull_socklen_t len = sizeof(buffer);
	getRawOption(level, option, buffer, len);
	value = IPAddress(buffer, len);
}


void Socket::getRawOption(int level, int option, void* value, bull_socklen_t& length)
{
	if (sockfd_ == BULL_INVALID_SOCKET) throw std::exception();

	int rc = ::getsockopt(sockfd_, level, option, reinterpret_cast<char*>(value), &length);
	if (rc == -1) error();
}


void Socket::setLinger(bool on, int seconds)
{
	struct linger l;
	l.l_onoff  = on ? 1 : 0;
	l.l_linger = seconds;
	setRawOption(SOL_SOCKET, SO_LINGER, &l, sizeof(l));
}


void Socket::getLinger(bool& on, int& seconds)
{
	struct linger l;
	bull_socklen_t len = sizeof(l);
	getRawOption(SOL_SOCKET, SO_LINGER, &l, len);
	on      = l.l_onoff != 0;
	seconds = l.l_linger;
}


void Socket::setNoDelay(bool flag)
{
	int value = flag ? 1 : 0;
	setOption(IPPROTO_TCP, TCP_NODELAY, value);
}


bool Socket::getNoDelay()
{
	int value(0);
	getOption(IPPROTO_TCP, TCP_NODELAY, value);
	return value != 0;
}


void Socket::setKeepAlive(bool flag)
{
	int value = flag ? 1 : 0;
	setOption(SOL_SOCKET, SO_KEEPALIVE, value);
}


bool Socket::getKeepAlive()
{
	int value(0);
	getOption(SOL_SOCKET, SO_KEEPALIVE, value);
	return value != 0;
}


void Socket::setReuseAddress(bool flag)
{
	int value = flag ? 1 : 0;
	setOption(SOL_SOCKET, SO_REUSEADDR, value);
}


bool Socket::getReuseAddress()
{
	int value(0);
	getOption(SOL_SOCKET, SO_REUSEADDR, value);
	return value != 0;
}


void Socket::setReusePort(bool flag)
{
#ifdef SO_REUSEPORT
	try
	{
		int value = flag ? 1 : 0;
		setOption(SOL_SOCKET, SO_REUSEPORT, value);
	}
	catch (IOException&)
	{
		// ignore error, since not all implementations
		// support SO_REUSEPORT, even if the macro
		// is defined.
	}
#endif
}


bool Socket::getReusePort()
{
#ifdef SO_REUSEPORT
	int value(0);
	getOption(SOL_SOCKET, SO_REUSEPORT, value);
	return value != 0;
#else
	return false;
#endif
}


void Socket::setOOBInline(bool flag)
{
	int value = flag ? 1 : 0;
	setOption(SOL_SOCKET, SO_OOBINLINE, value);
}


bool Socket::getOOBInline()
{
	int value(0);
	getOption(SOL_SOCKET, SO_OOBINLINE, value);
	return value != 0;
}


void Socket::setBroadcast(bool flag)
{
	int value = flag ? 1 : 0;
	setOption(SOL_SOCKET, SO_BROADCAST, value);
}


bool Socket::getBroadcast()
{
	int value(0);
	getOption(SOL_SOCKET, SO_BROADCAST, value);
	return value != 0;
}


void Socket::setBlocking(bool flag)
{
#if !defined(BULL_OS_FAMILY_UNIX)
	int arg = flag ? 0 : 1;
	ioctl(FIONBIO, arg);
#else
	int arg = fcntl(F_GETFL);
	long flags = arg & ~O_NONBLOCK;
	if (!flag) flags |= O_NONBLOCK;
	(void) fcntl(F_SETFL, flags);
#endif
	_blocking = flag;
}


int Socket::socketError()
{
	int result(0);
	getOption(SOL_SOCKET, SO_ERROR, result);
	return result;
}


void Socket::init(int af)
{
	initSocket(af, SOCK_STREAM);
}


void Socket::initSocket(int af, int type, int proto)
{
	bull_assert (sockfd_ == BULL_INVALID_SOCKET);

	sockfd_ = ::socket(af, type, proto);
	if (sockfd_ == BULL_INVALID_SOCKET)
		error();

#if defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
	// SIGPIPE sends a signal that if unhandled (which is the default)
	// will crash the process. This only happens on UNIX, and not Linux.
	//
	// In order to have POCO sockets behave the same across platforms, it is
	// best to just ignore SIGPIPE altogether.
	setOption(SOL_SOCKET, SO_NOSIGPIPE, 1);
#endif
}


void Socket::ioctl(bull_ioctl_request_t request, int& arg)
{
#if defined(_WIN32)
	int rc = ioctlsocket(sockfd_, request, reinterpret_cast<u_long*>(&arg));
#elif defined(BULL_VXWORKS)
	int rc = ::ioctl(sockfd_, request, (int) &arg);
#else
	int rc = ::ioctl(sockfd_, request, &arg);
#endif
	if (rc != 0) error();
}


void Socket::ioctl(bull_ioctl_request_t request, void* arg)
{
#if defined(_WIN32)
	int rc = ioctlsocket(sockfd_, request, reinterpret_cast<u_long*>(arg));
#elif defined(BULL_VXWORKS)
	int rc = ::ioctl(sockfd_, request, (int) arg);
#else
	int rc = ::ioctl(sockfd_, request, arg);
#endif
	if (rc != 0) error();
}


#if defined(BULL_OS_FAMILY_UNIX)
int Socket::fcntl(bull_fcntl_request_t request)
{
	int rc = ::fcntl(sockfd_, request);
	if (rc == -1) error();
	return rc;
}


int Socket::fcntl(bull_fcntl_request_t request, long arg)
{
	int rc = ::fcntl(sockfd_, request, arg);
	if (rc == -1) error();
	return rc;
}
#endif


void Socket::reset(bull_socket_t aSocket)
{
	sockfd_ = aSocket;
}


void Socket::error()
{
	int err = lastError();
	std::string empty;
	error(err, empty);
}


void Socket::error(const std::string& arg)
{
	error(lastError(), arg);
}


void Socket::error(int code)
{
	std::string arg;
	error(code, arg);
}


void Socket::error(int code, const std::string& arg)
{
	switch (code)
	{
	case BULL_ENOERR: return;
	case BULL_ESYSNOTREADY:
		throw NetException("Net subsystem not ready", code);
	case BULL_ENOTINIT:
		throw NetException("Net subsystem not initialized", code);
	case BULL_EINTR:
		throw IOException("Interrupted", code);
	case BULL_EACCES:
		throw IOException("Permission denied", code);
	case BULL_EFAULT:
		throw IOException("Bad address", code);
	case BULL_EINVAL:
		throw InvalidArgumentException(code);
	case BULL_EMFILE:
		throw IOException("Too many open files", code);
	case BULL_EWOULDBLOCK:
		throw IOException("Operation would block", code);
	case BULL_EINPROGRESS:
		throw IOException("Operation now in progress", code);
	case BULL_EALREADY:
		throw IOException("Operation already in progress", code);
	case BULL_ENOTSOCK:
		throw IOException("Socket operation attempted on non-socket", code);
	case BULL_EDESTADDRREQ:
		throw NetException("Destination address required", code);
	case BULL_EMSGSIZE:
		throw NetException("Message too long", code);
	case BULL_EPROTOTYPE:
		throw NetException("Wrong protocol type", code);
	case BULL_ENOPROTOOPT:
		throw NetException("Protocol not available", code);
	case BULL_EPROTONOSUPPORT:
		throw NetException("Protocol not supported", code);
	case BULL_ESOCKTNOSUPPORT:
		throw NetException("Socket type not supported", code);
	case BULL_ENOTSUP:
		throw NetException("Operation not supported", code);
	case BULL_EPFNOSUPPORT:
		throw NetException("Protocol family not supported", code);
	case BULL_EAFNOSUPPORT:
		throw NetException("Address family not supported", code);
	case BULL_EADDRINUSE:
		throw NetException("Address already in use", arg, code);
	case BULL_EADDRNOTAVAIL:
		throw NetException("Cannot assign requested address", arg, code);
	case BULL_ENETDOWN:
		throw NetException("Network is down", code);
	case BULL_ENETUNREACH:
		throw NetException("Network is unreachable", code);
	case BULL_ENETRESET:
		throw NetException("Network dropped connection on reset", code);
	case BULL_ECONNABORTED:
		throw ConnectionAbortedException(code);
	case BULL_ECONNRESET:
		throw ConnectionResetException(code);
	case BULL_ENOBUFS:
		throw IOException("No buffer space available", code);
	case BULL_EISCONN:
		throw NetException("Socket is already connected", code);
	case BULL_ENOTCONN:
		throw NetException("Socket is not connected", code);
	case BULL_ESHUTDOWN:
		throw NetException("Cannot send after socket shutdown", code);
	case BULL_ETIMEDOUT:
		throw TimeoutException(code);
	case BULL_ECONNREFUSED:
		throw ConnectionRefusedException(arg, code);
	case BULL_EHOSTDOWN:
		throw NetException("Host is down", arg, code);
	case BULL_EHOSTUNREACH:
		throw NetException("No route to host", arg, code);
#if defined(BULL_OS_FAMILY_UNIX)
	case EPIPE:
		throw IOException("Broken pipe", code);
	case EBADF:
		throw IOException("Bad socket descriptor", code);
	case ENOENT:
		throw IOException("Not found", arg, code);
#endif
	default:
		throw IOException(NumberFormatter::format(code), arg, code);
	}
}


}}
