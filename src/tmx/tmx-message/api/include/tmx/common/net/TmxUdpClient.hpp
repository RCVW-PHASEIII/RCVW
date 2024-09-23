/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxUdpClient.hpp
 *
 *  Created on: June 16, 2021
 *  	@author: Gregory M. Baumgardner
 *      @author: Dave Charlson
 */
#ifndef INCLUDE_TMX_COMMON_NET_TMXUDPCLIENT_HPP_
#define INCLUDE_TMX_COMMON_NET_TMXUDPCLIENT_HPP_

#include <tmx/common/TmxException.hpp>

#ifdef _MSC_VER
#else
#include <netdb.h>
#include <sys/socket.h>
#endif
#include <sys/types.h>
#include <string_view>

namespace tmx {
namespace common {
namespace net {

class TmxUdpClientRuntimeError : public tmx::common::TmxException
{
public:
	TmxUdpClientRuntimeError(std::string_view w) : tmx::common::TmxException(w) {}
};

class TmxUdpClient
{
public:
	/*!
	 * @brief Initialize a UDP client object.
	 *
	 * This function initializes the UDP client object using the address and the
	 * port as specified.
	 *
	 * The port is expected to be a host side port number (i.e. 59200).
	 *
	 * The \p address parameter is a textual address. It may be an IPv4 or IPv6
	 * address and it can represent a host name or an address defined with
	 * just numbers. If the address cannot be resolved then an error occurs
	 * and constructor throws.
	 *
	 * @note
	 * The socket is open in this process. If you fork() or exec() then the
	 * socket will be closed by the operating system.
	 *
	 * @warning
	 * We only make use of the first address found by getaddrinfo(). All
	 * the other addresses are ignored.
	 *
	 * @exception UdpClientRuntimeError
	 * The server could not be initialized properly. Either the address cannot be
	 * resolved, the port is incompatible or not available, or the socket could
	 * not be created.
	 *
	 * @param[in] address  The address to send to.
	 * @param[in] port  The port to send to.
	 */
	TmxUdpClient(const std::string_view address, int port);

	/*!
	 * @brief Clean up the UDP client.
	 *
	 * This function frees the address info structures and close the socket.
	 */
	~TmxUdpClient();

	/*!
	 * @brief Retrieve a copy of the socket identifier.
	 *
	 * This function return the socket identifier as returned by the socket()
	 * function. This can be used to change some flags.
	 *
	 * @return The socket used by this UDP client.
	 */
	int get_socket() const;

	/*!
	 * @brief Retrieve the port used by this UDP client.
	 *
	 * This function returns the port used by this UDP client. The port is
	 * defined as an integer, host side.
	 *
	 * @return The port as expected in a host integer.
	 */
	int get_port() const;


	/*!
	 * @brief Retrieve a copy of the address.
	 *
	 * This function returns a copy of the address as it was specified in the
	 * constructor. This does not return a canonalized version of the address.
	 *
	 * The address cannot be modified. If you need to send data on a different
	 * address, create a new UDP client.
	 *
	 * @return A string with a copy of the constructor input address.
	 */
	std::string_view get_address() const;

	/*!
	 * @brief Send a message through this UDP client.
	 *
	 * This function sends the \p buffer data through the UDP client socket.
	 * The function cannot be used to change the destination as it was defined
	 * when creating the UdpClient object.
	 *
	 * The size must be small enough for the message to fit.
	 *
	 * @param[in] buffer  The buffer containing the data to send.
	 * @param[in] size  The number of bytes contained within the buffer to send.
	 *
	 * @return -1 if an error occurs, otherwise the number of bytes sent. errno
	 * is set accordingly on error.
	 */
	int send(std::string_view message);


	int send(void *buffer, size_t size);
private:
	int _socket;
	int _port;
	std::string_view _address;
	struct addrinfo *_addrInfo;
};

} /* End namespace net */
} /* End namespace common */
} /* End namespace tmx */

#endif /* INCLUDE_TMX_COMMON_NET_TMXUDPCLIENT_HPP_ */
