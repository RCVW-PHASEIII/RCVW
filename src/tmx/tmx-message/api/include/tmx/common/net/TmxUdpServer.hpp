/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxUdpServer.hpp
 *
 *  Created on: June 17, 2021
 *  	@author: Gregory M. Baumgardner
 *      @author: Dave Charlson
 */

#ifndef SRC_UDPSERVER_H_
#define SRC_UDPSERVER_H_

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

class UdpServerRuntimeError : public tmx::common::TmxException
{
public:
	UdpServerRuntimeError(const char *w) : tmx::common::TmxException(w) {}
};

class TmxUdpServer
{
public:
	/*!
	 * @brief Initialize a UDP server object.
	 *
	 * This function initializes a UDP server object making it ready to
	 * receive messages.
	 *
	 * The server address and port are specified in the constructor so
	 * if you need to receive messages from several different addresses
	 * and/or port, you'll have to create a server for each.
	 *
	 * The address is a string and it can represent an IPv4 or IPv6
	 * address.
	 *
	 * Note that this function calls connect() to connect the socket
	 * to the specified address. To accept data on different UDP addresses
	 * and ports, multiple UDP servers must be created.
	 *
	 * @note
	 * The socket is open in this process. If you fork() or exec() then the
	 * socket will be closed by the operating system.
	 *
	 * @warning
	 * We only make use of the first address found by getaddrinfo(). All
	 * the other addresses are ignored.
	 *
	 * @exception udp_client_server_runtime_error
	 * The udp_client_server_runtime_error exception is raised when the address
	 * and port combination cannot be resolved or if the socket cannot be
	 * opened.
	 *
	 * @param[in] address  The address we receive on.
	 * @param[in] port  The port we receive from.
	 */
	TmxUdpServer(std::string_view address, int port);

	/*!
	 * @brief Clean up the UDP server.
	 *
	 * This function frees the address info structures and close the socket.
	 */
	~TmxUdpServer();

	/*!
	 * @brief The socket used by this UDP server.
	 *
	 * This function returns the socket identifier. It can be useful if you are
	 * doing a select() on many sockets.
	 *
	 * @return The socket of this UDP server.
	 */
	int get_socket() const;


	/*!
	 * @brief The port used by this UDP server.
	 *
	 * This function returns the port attached to the UDP server. It is a copy
	 * of the port specified in the constructor.
	 *
	 * @return The port of the UDP server.
	 */
	int get_port() const;

	/*!
	 * @brief Return the address of this UDP server.
	 *
	 * This function returns a verbatim copy of the address as passed to the
	 * constructor of the UDP server (i.e. it does not return the canonalized
	 * version of the address.)
	 *
	 * @return The address as passed to the constructor.
	 */
	std::string_view get_address() const;

	/*!
	 * @brief Wait on a message.
	 *
	 * This function waits until a message is received on this UDP server.
	 * There are no means to return from this function except by receiving
	 * a message. Remember that UDP does not have a connect state so whether
	 * another process quits does not change the status of this UDP server
	 * and thus it continues to wait forever.
	 *
	 * Note that you may change the type of socket by making it non-blocking
	 * (use the get_socket() to retrieve the socket identifier) in which
	 * case this function will not block if no message is available. Instead
	 * it returns immediately.
	 *
	 * @param[in] msg  The buffer where the message is saved.
	 * @param[in] maxSize  The maximum size the message (i.e. size of the \p msg buffer.)
	 *
	 * @return The number of bytes read or -1 if an error occurs.
	 */
	int receive(char *msg, size_t maxSize);

	/*!
	 * @brief Wait for data to come in.
	 *
	 * This function waits for a given amount of time for data to come in. If
	 * no data comes in after max_wait_ms, the function returns with -1 and
	 * errno set to EAGAIN.
	 *
	 * The socket is expected to be a blocking socket (the default,) although
	 * it is possible to setup the socket as non-blocking if necessary for
	 * some other reason.
	 *
	 * This function blocks for a maximum amount of time as defined by
	 * max_wait_ms. It may return sooner with an error or a message.
	 *
	 * @param[in] msg  The buffer where the message will be saved.
	 * @param[in] maxSize  The size of the \p msg buffer in bytes.
	 * @param[in] maxWait_ms  The maximum number of milliseconds to wait for a message.
	 *
	 * @return -1 if an error occurs or the function timed out, the number of bytes received otherwise.
	 */
	int timed_receive(char *msg, size_t maxSize, int maxWait_ms);

private:
	int _socket;
	int _port;
	std::string_view _address;
	struct addrinfo *_addrInfo;
};

} /* End namespace net */
} /* End namespace common */
} /* End namespace tmx */

#endif /* SRC_UDPSERVER_H_ */
