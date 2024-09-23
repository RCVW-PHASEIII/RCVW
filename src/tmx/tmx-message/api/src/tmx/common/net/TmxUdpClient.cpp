/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxUdpClient.cpp
 *
 *  Created on: June 16, 2021
 *  	@author: Gregory M. Baumgardner
 *      @author: Dave Charlson
 */
#include <tmx/common/net/TmxUdpClient.hpp>


namespace tmx {
namespace common {
namespace net {

TmxUdpClient::TmxUdpClient(std::string_view address, int port)
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
	if (r != 0 || _addrInfo == NULL) {
		throw TmxUdpClientRuntimeError(std::string("invalid address or port: \""));// + address + ":" + decimalPort + "\"");
	}

	_socket = socket(_addrInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
	if (_socket == -1) {
		freeaddrinfo(_addrInfo);
		throw TmxUdpClientRuntimeError(std::string("could not create UDP socket for: \""));// + address + ":" + decimalPort + "\"");
	}
}

TmxUdpClient::~TmxUdpClient()
{
    freeaddrinfo(_addrInfo);
    close(_socket);
}

int TmxUdpClient::get_socket() const
{
    return _socket;
}

int TmxUdpClient::get_port() const
{
    return _port;
}

std::string_view TmxUdpClient::get_address() const
{
    return _address;
}

int TmxUdpClient::send(void *buffer, size_t size)
{
    return sendto(_socket, buffer, size, 0, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
}

int TmxUdpClient::send(std::string_view message)
{
    return ::sendto(_socket, message.data(), message.length(), 0, _addrInfo->ai_addr, _addrInfo->ai_addrlen);
}

} /* End namespace net */
} /* End namespace common */
} /* End namespace tmx */

