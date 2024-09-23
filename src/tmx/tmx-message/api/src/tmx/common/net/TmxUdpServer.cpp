/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxUdpServer.cpp
 *
 *  Created on: June 17, 2021
 *  	@author: Gregory M. Baumgardner
 *      @author: Dave Charlson
 */

#include <tmx/common/net/TmxUdpServer.hpp>

#include <cerrno>

using namespace std;

namespace tmx {
namespace common {
namespace net {

TmxUdpServer::TmxUdpServer(std::string_view address, int port)
    : _port(port)
    , _address(address)
{
	char decimalPort[16];
	snprintf(decimalPort, sizeof(decimalPort), "%d", _port);
	decimalPort[sizeof(decimalPort) / sizeof(decimalPort[0]) - 1] = '\0';

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int r(getaddrinfo(address.data(), decimalPort, &hints, &_addrInfo));
	if (r != 0 || _addrInfo == NULL)
	{
		throw UdpServerRuntimeError("invalid address or port for UDP socket: \"");// + address + ":" + decimalPort + "\""));
	}

	_socket = socket(_addrInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
	if (_socket == -1)
	{
		freeaddrinfo(_addrInfo);
		throw UdpServerRuntimeError("could not create UDP socket for: \"");// + address + ":" + decimalPort + "\""));
	}

	r = bind(_socket, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
	if (r != 0)
	{
		freeaddrinfo(_addrInfo);
		close(_socket);
		throw UdpServerRuntimeError("could not bind UDP socket with: \"");// + address + ":" + decimalPort + "\""));
	}
}

TmxUdpServer::~TmxUdpServer()
{
    freeaddrinfo(_addrInfo);
    close(_socket);
}

int TmxUdpServer::get_socket() const
{
    return _socket;
}

int TmxUdpServer::get_port() const
{
    return _port;
}

std::string_view TmxUdpServer::get_address() const
{
    return _address;
}

int TmxUdpServer::receive(char *msg, size_t maxSize)
{
    return ::recv(_socket, msg, maxSize, 0);
}

int TmxUdpServer::timed_receive(char *msg, size_t maxSize, int maxWait_ms)
{
    fd_set s;
    FD_ZERO(&s);
    FD_SET(_socket, &s);
    struct timeval timeout;
    timeout.tv_sec = maxWait_ms / 1000;
    timeout.tv_usec = (maxWait_ms % 1000) * 1000;

    int retval = select(_socket + 1, &s, &s, &s, &timeout);
    if (retval == -1)
    {
        // select() set errno accordingly
        return -1;
    }
    if (retval > 0)
    {
        // The socket has data.
        return ::recv(_socket, msg, maxSize, 0);
    }

    // The socket has no data.
    errno = EAGAIN;
    return -1;
}

} /* End namespace net */
} /* End namespace common */
} /* End namespace tmx */
