/*
 * UdpClient.h
 *
 *  Created on: Aug 27, 2015
 *      Author: ivp
 */

#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include <netdb.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>

namespace tmx {
namespace plugin {
namespace utils {
namespace net {

class UdpClientRuntimeError : public std::runtime_error {
public:
    UdpClientRuntimeError(const char *w) : std::runtime_error(w) { }
};

class UdpClient {
public:
    UdpClient(const std::string &address, int port);

    ~UdpClient();

    int GetSocket() const;

    int GetPort() const;

    std::string GetAddress() const;

    int Send(const std::string &message);

    int Send(const void *buffer, size_t size);

private:
    int _socket;
    int _port;
    std::string _address;
    struct addrinfo *_addrInfo;
};

}}}} // namespace tmx::plugin::utils::net

#endif /* UDPCLIENT_H_ */
