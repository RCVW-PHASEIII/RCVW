///*!
// * Copyright (c) 2024 Battelle Memorial Institute
// *
// * All Rights Reserved.
// *
// * @file TmxHttpBrokerClient.cpp
// *
// * @see https://www.boost.org/doc/libs/1_76_0/libs/beast/example/http/client/async/http_client_async.cpp
// *
// *  Created on: Apr 29, 2024
// *      @author: Gregory M. Baumgardner
// */
//
//#include <tmx/broker/TmxBrokerClient.hpp>
//#include <tmx/broker/TmxBrokerContext.hpp>
//#include <tmx/common/TmxError.hpp>
//#include <tmx/common/TmxFunctor.hpp>
//#include <tmx/common/TmxTypeDescriptor.hpp>
//#include <tmx/common/TmxTypeRegistrar.hpp>
//#include <tmx/common/TmxTypeRegistry.hpp>
//#include <tmx/message/codec/TmxCodec.hpp>
//#include <tmx/message/TmxMessage.hpp>
//#include <tmx/message/TmxData.hpp>
////#include <tmx/message/v2x/NmeaMessage.hpp>
////#include <tmx/message/v2x/rtcm/RtcmMessage.hpp>
//
//#include <arpa/inet.h>
//#include <boost/beast.hpp>
//#include <chrono>
//#include <netdb.h>
//#include <sstream>
//#include <sys/socket.h>
//
//using namespace tmx::common;
//using namespace tmx::message;
//using namespace boost;
//
//using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
//
//namespace tmx {
//namespace broker {
//namespace http {
//
//typedef typename common::TmxFunctor<common::types::Any const, message::TmxMessage const>::type cb_type;
//
//struct callbacks { };
//
//TmxTypeRegistry get_callbacks(std::string const &id, std::string const &topic) {
//    static TmxTypeRegistrar<callbacks> cbs;
//    return cbs.get_registry() / id / topic;
//}
//
//typename common::types::Properties<TmxBrokerContext *> _ctx_map;
//
//static const types::Properties_::key_t _ioc { "ioc" };
//
//static const types::Properties_::key_t _socket { "socket" };
//static const types::Properties_::key_t _nmea { "nmea" };
//static const types::Properties_::key_t _thread { "thread" };
//
//template <typename _IP>
//std::string to_string(asio::ip::basic_endpoint<_IP> const &ep) noexcept {
//    std::stringstream ss;
//    ss << ep;
//    return ss.str();
//}
//
//template <typename _IP = asio::ip::tcp>
//asio::ip::basic_endpoint<_IP> get_endpoint(TmxBrokerContext &ctx) noexcept {
//    const TmxData port { ctx.get_port() };
//
//    std::string svc = ctx.get_scheme();
//    if (svc == "ntrip")
//        svc = "rtcm-sc104";
//    if (port && port.to_int() == 0)
//        svc = port.to_string();
//
////    auto ent = getservbyname(svc.c_str(), "tcp");
////    if (ent)
////        addr.sin_port = htons(ent->s_port);
////    else
////        addr.sin_port = htons(port.to_int());
////    return { ctx.get_host() }
//}
//
//class TmxHttpBroker: public TmxBrokerClient {
//public:
//    TmxHttpBroker() {
//        this->register_broker("http");
//        this->register_broker("https");
//        this->register_broker("ntrip");
//    }
//
//    TmxTypeDescriptor get_descriptor() const noexcept override {
//        static const auto &desc = TmxBrokerClient::get_descriptor();
//        return { desc.get_instance(), typeid(*this), "boost-beast" };
//    }
//
//    int get_socket(TmxBrokerContext const &ctx) const noexcept {
//        if (ctx.count(_socket)) {
//            auto fd = types::as<int>(ctx.at(_socket));
//            if (fd) return *fd;
//        }
//
//        return 0;
//    }
//
//    bool is_subscribed(TmxBrokerContext const &ctx, const_string topicName) const noexcept override {
//        return get_callbacks(ctx.get_id(), topicName.data()).get_all().size() > 0;
//    }
//
//    void initialize(TmxBrokerContext &ctx) noexcept override {
//    }
//
//
//    void connect(TmxBrokerContext &ctx, types::Any const &) noexcept override {
//        struct sockaddr_in addr;
//        memset(&addr, 0, sizeof(addr));
//
//        addr.sin_family = AF_INET;
//        addr.sin_addr.s_addr = inet_addr(ctx.get_host().c_str());
//
//
//
//        int ret = -1;
//        auto &fd = ctx[_socket].emplace<int>(::socket(AF_INET, SOCK_STREAM, 0));
//        if (fd > 0)
//            ret = ::connect(fd, (struct sockaddr *) &addr, sizeof(addr));
//
//
//    }
//
//    void disconnect(TmxBrokerContext &ctx) noexcept override {
//        shutdown(ctx);
//        TmxBrokerClient::disconnect(ctx);
//    }
//
//    void publish(TmxBrokerContext &ctx, message::TmxMessage const &msg) noexcept {
////        if (!this->is_socket_connected(ctx)) {
////            this->on_published(ctx, { EBADF, "Broker context " + ctx.get_id() + " socket is not connected."}, msg);
////            return;
////        }
////
////        // Need a GGA message
////        message::v2x::NmeaMessage _tmp;
////        if (msg.get_length())
////            _tmp.set_sentence(msg.get_payload_string());
////        else if (ctx.count(_nmea))
////            _tmp.set_sentence(TmxData(ctx.at(_nmea)).to_string());
////
////        if (_tmp.get_sentence_type() != "GGA") {
////            this->on_published(ctx, { EINVAL, "Invalid GGA string: " + _tmp.get_sentence() }, msg);
////            return;
////        }
////
////        // Save this GGA string for next iteration
////        std::string send = ctx[_nmea].emplace<std::string>(_tmp.get_sentence());
////
////        // Kick off the request if a connection is still pending
////        if (TmxBrokerClient::is_connected(ctx)) {
////            std::stringstream header;
////            header << "GET /" << ctx.get_path() << " HTTP/1.1\r\n";
////            header << "User-Agent: TMX NTRIP " << this->get_descriptor().get_type_short_name() << "/20\r\n";
////            header << "Accept: */*\r\n";
////            header << "Ntrip-Version: Ntrip/2.0\r\n";
////            header << "Ntrip-GGA: " << _tmp.get_sentence() << "\r\n";
////            header << "Connection: close\r\n";
////
////            std::string userpass = ctx.get_user() + std::string(":") + ctx.get_secret();
////            header << "Authorization: Basic "
////                   << byte_string_encode(to_byte_sequence(userpass.data()), tmx::common::base64::value)
////                   << "\r\n\r\n";
////
////            send = header.str();
////        }
////
////        if (!send.empty()) {
////            TLOG(DEBUG) << "Sending message to socket: \n" << send;
////
////            if (::send(this->get_socket(ctx), send.c_str(), send.length(), 0) < 0) {
////                std::string err{ "Unable to write bytes to socket: " };
////                err.append(strerror(errno));
////
////                this->on_published(ctx, { errno, err }, msg);
////                return;
////            }
////        }
////
//        TmxBrokerClient::publish(ctx, msg);
//    }
//
//    void subscribe(TmxBrokerContext &ctx, const_string topicName, TmxTypeDescriptor const &cb) noexcept override {
//        if (!cb) {
//            std::string err{ "Callback " };
//            err.append(cb.get_type_name());
//            err.append(" is not valid.");
//
//            this->on_subscribed(ctx, { 20, err }, topicName, cb);
//            return;
//        }
//
//        auto callback = cb.as_instance<cb_type>();
//        if (!callback) {
//            std::string err{ "Callback " };
//            err.append(cb.get_type_name());
//            err.append(" is not the correct signature. Expecting ");
//            err.append(type_fqname<cb_type>());
//
//            this->on_subscribed(ctx, { 30, err }, topicName, cb);
//            return;
//        }
//
//        // Register the handler
//        get_callbacks(ctx.get_id(), topicName.data()).register_handler(*callback, cb.get_typeid(),
//                                                                       cb.get_type_short_name());
//
//        TmxBrokerClient::on_subscribed(ctx, { }, topicName, cb);
//    }
//
//    void unsubscribe(TmxBrokerContext &ctx, const_string topicName, TmxTypeDescriptor const &cb) noexcept override {
//        // Unregister the handler
//        get_callbacks(ctx.get_id(), topicName.data()).unregister(cb.get_typeid());
//        TmxBrokerClient::on_unsubscribed(ctx, { }, topicName, cb);
//    }
//
//private:
//    bool is_socket_connected(TmxBrokerContext &ctx) {
//        return this->get_socket(ctx) > 0;
//    }
//
//    void shutdown(TmxBrokerContext &ctx) {
//        int fd = this->get_socket(ctx);
//        if (fd)
//            ::close(fd);
//
//        ctx.erase(_socket);
//
//        // Wait for the thread to end
//        if (ctx.count(_thread)) {
//            auto thread = types::as<std::thread>(ctx.at(_thread));
//            if (thread && thread->joinable())
//                thread->join();
//        }
//    }
//
//    void ntrip(TmxBrokerContext &ctx) {
//        // Speak NTRIP 2.0 over HTTP 1.1
//        const TmxData params { ctx.get_parameters() };
//        TmxTypeRegistry topicReg { params["topic"] ? params["topic"].to_string().c_str() : "V2X/RTCM" };
//
//        while (this->is_socket_connected(ctx)) {
//            int fd = this->get_socket(ctx);
//
//            std::array<char, 4096> inBytes;
//            int recv = ::recv(fd, inBytes.data(), inBytes.size(), 0);
//            if (recv < 0) {
//                std::string err { "Unable to receive bytes from socket: " };
//                err.append(strerror(errno));
//
//                this->on_disconnected(ctx, { errno, err });
//                this->shutdown(ctx);
//                continue;
//            }
//
//            if (!TmxBrokerClient::is_connected(ctx)) {
//                common::types::Array<std::string> response;
//
//                // Read incoming message by line
//                std::istringstream inStream(std::string((const char *) inBytes.data(), recv));
//                std::string line;
//                while (getline(inStream, line)) {
//                    if (line[line.length() - 1] == '\r') line.erase(line.length() - 1);
//                    response.push_back(line);
//                }
//
//                // Check to see if the connection to the castor was successful
//                if (response.size() > 1) {
//                    if (::strncmp("200 OK", response[0].substr(response[0].length() - 6).c_str(), 6) == 0) {
//                        std::string msg{ "Successfully connected to NTRIP " };
//
//                        bool sendGGA = false;
//                        if (::strncmp("ICY", response[0].c_str(), 3) == 0) {
//                            msg.append("1.0 server.");
//
//                            // Need to send the GGA string
//                            sendGGA = true;
//                        } else {
//                            msg.append("2.0 server.");
//                        }
//
//                        this->on_connected(ctx, { 0, msg });
//                        if (sendGGA)
//                            this->publish(ctx, { });
//                    } else {
//                        std::string err{ "Invalid response: " };
//                        err.append(response[0]);
//
//                        this->on_connected(ctx, { EBADMSG, err });
//                    }
//                } else {
//                    // Convert the bytes to a set of new message
//                    common::types::Any _tmp;
//                    codec::TmxCodec codec;
//                    codec.get_message().set_timepoint();
//                    codec.get_message().set_source(ctx.to_string());
//                    codec.get_message().set_payload(byte_string_encode(to_byte_sequence(inBytes.data(), recv)));
//
//                    types::Any _id;
//                    _id.emplace<std::string>(ctx.get_id());
//
//                    // Try to encode each RTCM version until one is correct
//                    for (auto val: {
//#ifndef IGNORE_RTCM3
//                            v2x::rtcm::RTCM_VERSION::SC10403_3,
//#endif
//#ifndef IGNORE_RTCM21
//                            v2x::rtcm::RTCM_VERSION::SC10402_3
//#endif
//                        }) {
//                        codec.get_message().set_encoding(RtcmVersionName(val));
//                        codec.get_message().set_topic((topicReg / enums::enum_name(val).data()).get_namespace().data());
//
//                        auto ret = codec.decode(_tmp);
//                        if (!ret) {
//                            // Execute the callbacks
//                            const auto &callbacks = get_callbacks(ctx.get_id(), codec.get_message().get_topic()).get_all();
//                            for (const auto &cb: callbacks) {
//                                auto ret = dispatch(cb, _id, codec.get_message());
//                                if (ret)
//                                    TmxBrokerClient::on_error(ctx, ret);
//                            }
//
//                            return;
//                        }
//                    }
//
//                    this->on_error(ctx, { EILSEQ, "Unknown RTCM message " + codec.get_message().get_payload_string() });
//                }
//            }
//        }
//    }
//};
//
//static common::TmxTypeRegistrar<TmxNtripBroker> _registrar;
//
//} /* End namespace http */
//} /* End namespace broker */
//} /* End namespace tmx */