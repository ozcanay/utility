#pragma once

#include "../net_common.hpp"
#include "include/socket_address.hpp"
#include "../core/noncopyable.hpp"

#include <string>

namespace Bull {
namespace Net {

class Socket : private Noncopyable {
public:
	enum SelectMode
	{
		SELECT_READ  = 1,
		SELECT_WRITE = 2,
		SELECT_ERROR = 4
	};

    Socket();
	Socket(bull_socket_t sockfd);
	~Socket();

	Socket* acceptConnection(SocketAddress& clientAddr);
	void connect(const SocketAddress& address);

	// void connect(const SocketAddress& address, const Poco::Timespan& timeout);

	void connectNB(const SocketAddress& address);

	void bind(const SocketAddress& address, bool reuseAddress = false);

	void bind(const SocketAddress& address, bool reuseAddress, bool reusePort);

	void listen(int backlog = 64);
	void close();

	void shutdownReceive();
	void shutdownSend();
	void shutdown();

	int sendBytes(const void* buffer, int length, int flags = 0);
	int receiveBytes(void* buffer, int length, int flags = 0);



	int receiveFrom(void* buffer, int length, struct sockaddr** ppSA, bull_socklen_t** ppSALen, int flags = 0);

	int receiveFrom(void* buffer, int length, SocketAddress& address, int flags = 0);

	void sendUrgent(unsigned char data);
	int available();
	// bool poll(const Poco::Timespan& timeout, int mode);
	void setSendBufferSize(int size);

	int getSendBufferSize();
	void setReceiveBufferSize(int size);
	int getReceiveBufferSize();
	// void setSendTimeout(const Poco::Timespan& timeout);

	// Poco::Timespan getSendTimeout();
	// void setReceiveTimeout(const Poco::Timespan& timeout);
	// Poco::Timespan getReceiveTimeout();
	SocketAddress address();
	SocketAddress peerAddress();

	void setOption(int level, int option, int value);
	void setOption(int level, int option, unsigned value);
	void setOption(int level, int option, unsigned char value);
	// void setOption(int level, int option, const Poco::Timespan& value);
	// void setOption(int level, int option, const IPAddress& value);
	void setRawOption(int level, int option, const void* value, bull_socklen_t length);
	void getOption(int level, int option, int& value);
	void getOption(int level, int option, unsigned& value);
	void getOption(int level, int option, unsigned char& value);
	// void getOption(int level, int option, Poco::Timespan& value);
	// void getOption(int level, int option, IPAddress& value);
	void getRawOption(int level, int option, void* value, bull_socklen_t& length);
	void setLinger(bool on, int seconds);
	void getLinger(bool& on, int& seconds);
	void setNoDelay(bool flag);
	bool getNoDelay();
	void setKeepAlive(bool flag);
	bool getKeepAlive();
	void setReuseAddress(bool flag);
	bool getReuseAddress();
	void setReusePort(bool flag);
	bool getReusePort();
	void setOOBInline(bool flag);
	bool getOOBInline();
	void setBroadcast(bool flag);
	bool getBroadcast();
	void setBlocking(bool flag);
	bool getBlocking() const;

	int socketError();
	bull_socket_t sockfd() const;

	int fcntl(bull_fcntl_request_t request);
	int fcntl(bull_fcntl_request_t request, long arg);

	bool initialized() const;
		/// Returns true iff the underlying socket is initialized.

protected:


	void init(int af);
		/// Creates the underlying native socket.
		///
		/// Subclasses must implement this method so
		/// that it calls initSocket() with the
		/// appropriate arguments.
		///
		/// The default implementation creates a
		/// stream socket.

	void initSocket(int af, int type, int proto = 0);
		/// Creates the underlying native socket.
		///
		/// The first argument, af, specifies the address family
		/// used by the socket, which should be either AF_INET or
		/// AF_INET6.
		///
		/// The second argument, type, specifies the type of the
		/// socket, which can be one of SOCK_STREAM, SOCK_DGRAM
		/// or SOCK_RAW.
		///
		/// The third argument, proto, is normally set to 0,
		/// except for raw sockets.

	void reset(bull_socket_t fd = BULL_INVALID_SOCKET);
		/// Allows subclasses to set the socket manually, iff no valid socket is set yet.

	void checkBrokenTimeout(SelectMode mode);

	static int lastError();
		/// Returns the last error code.

	static void error();
		/// Throws an appropriate exception for the last error.

	static void error(const std::string& arg);
		/// Throws an appropriate exception for the last error.

	static void error(int code);
		/// Throws an appropriate exception for the given error code.

	static void error(int code, const std::string& arg);
		/// Throws an appropriate exception for the given error code.

private:

	bull_socket_t  sockfd_;
	// Poco::Timespan _recvTimeout;
	// Poco::Timespan _sndTimeout;
	bool blocking_;
	// bool           _isBrokenTimeout;

};

inline bull_socket_t Socket::sockfd() const
{
	return sockfd_;
}

inline bool Socket::initialized() const
{
	return sockfd_ != BULL_INVALID_SOCKET;
}

inline int Socket::lastError()
{
	return errno;
}

inline bool Socket::getBlocking() const
{
	return blocking_;
}

}}
