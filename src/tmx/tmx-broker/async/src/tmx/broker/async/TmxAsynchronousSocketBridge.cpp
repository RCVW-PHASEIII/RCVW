/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsynchronousSocketBridge.hpp
 *
 *  Created on: May 31, 2024
 *      @author: Gregory M. Baumgardner
 */
#include <tmx/broker/async/TmxAsynchronousIOBroker.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <thread>

using namespace tmx::common;
using namespace tmx::message;
using namespace boost;

namespace tmx {
namespace broker {
namespace async {

static auto _conn_reset = boost::system::errc::make_error_code(boost::system::errc::errc_t::connection_reset);

/*!
 * @brief A broker that bridges generic socket input/output to TMX messages
 *
 * TODO: Support SSL
 *
 * @tparam _InternetProtocol The Boost ASIO internet transport protocol to use,
 * i.e. boost::asio::tcp or boost::asio::udp
 */
template <typename _InternetProtocol>
class TmxAsynchronousSocketBridge: public TmxAsynchronousIOBrokerClient {
    typedef TmxAsynchronousSocketBridge<_InternetProtocol> self_type;

public:
    using error_code = boost::system::error_code;

    typedef _InternetProtocol ip_type;
    typedef typename ip_type::endpoint endpoint;
    typedef typename ip_type::resolver resolver;
    typedef typename ip_type::socket socket;
    typedef typename boost::asio::streambuf streambuf;

    typename types::Properties_::key_t _sock { type_short_name<socket>().data() };
    typename types::Properties_::key_t _buffer { type_short_name<streambuf>().data() };

    TmxAsynchronousSocketBridge() {
        std::string ip = type_short_name<ip_type>().data();

        // Register a client connection
        this->register_broker(ip);

        // Register a server
        this->register_broker(ip + "-d");
    }

    TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_desc = TmxBrokerClient::get_descriptor();
        return { _desc.get_instance(), typeid(self_type), type_fqname(*this).data() };
    }

    std::string endpoint_info(endpoint const &ep) const {
        return std::string(type_short_name<ip_type>()) + "://" +
                    ep.address().to_string() + ":" + std::to_string(ep.port());
    }

    types::Any get_broker_info(TmxBrokerContext &ctx) const noexcept override {
        TmxData info { TmxAsynchronousIOBrokerClient::get_broker_info(ctx) };
        info["socket"]["open"] = false;

        std::shared_ptr<socket> sock;
        if (ctx.count(_sock))
            sock = types::as<socket>(ctx.at(_sock));

        if (sock) {
            info["socket"]["open"] = sock->is_open();
            info["socket"]["non-blocking"] = sock->non_blocking();
            info["socket"]["local-endpoint"] = this->endpoint_info(sock->local_endpoint());

            if (sock->is_open())
                info["socket"]["remote-endpoint"] = this->endpoint_info(sock->remote_endpoint());
        }

        return std::move(info.get_container());
    }

    void initialize(TmxBrokerContext &ctx) noexcept override {
        std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
        if (ctx.get_state() > TmxBrokerState::uninitialized)
            return;

        // Add a thread to the thread pool to ensure parallel processing
        // between reads, writes and callbacks
        std::thread newThread {[this, &ctx]() -> void {
            this->get_context(ctx).attach();
        }};

        do {
            std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while (!newThread.joinable());

        newThread.detach();

        // Add a socket to use, with a strand for synchronizing when necessary
        ctx[_sock].emplace<std::shared_ptr<socket> >(new socket(this->make_strand(ctx)));
        TmxAsynchronousIOBrokerClient::initialize(ctx);
    }

    void destroy(TmxBrokerContext &ctx) noexcept override {
        this->get_context(ctx).stop();

        ctx.erase(_sock);
        TmxAsynchronousIOBrokerClient::destroy(ctx);
    }

    virtual void on_socket_accepted(error_code const &ec, std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

        if (ec.value() == boost::asio::error::operation_aborted)
            return;

        // Wait for more to accept as long as the socket stays open
        std::shared_ptr<socket> sock;
        if (ctx.get().count(_sock))
            sock = types::as<socket>(ctx.get().at(_sock));

        if (sock && sock->is_open()) {
            // TODO: Spawn a new socket for read/write?

            // Start the read cycle with a new socket
            this->read_next_message(*sock, std::make_shared<streambuf>(), ctx.get());

            typename ip_type::acceptor acceptor { this->get_context(ctx.get()) };
            acceptor.async_accept(*sock, std::bind(&self_type::on_socket_accepted, this,
                                                    std::placeholders::_1, ctx));
        } else {
            this->on_disconnected(ctx.get(), { _conn_reset.value(), _conn_reset.message() });
        }
    }

    virtual void on_socket_bound(error_code const &ec, std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

        std::shared_ptr<socket> sock;
        if (ctx.get().count(_sock))
            sock = types::as<socket>(ctx.get().at(_sock));

        if (sock && sock->is_open()) {
            // After binding the server socket, the system is considered connected
            this->on_connected(ctx, { ec.value(), ec.message() });

            // Start accepting connections
            if (sock) {
                typename ip_type::acceptor acceptor{ this->get_context(ctx.get()) };
                acceptor.async_accept(*sock, std::bind(&self_type::on_socket_accepted, this,
                                                       std::placeholders::_1, ctx));
            }
        } else {
            this->on_connected(ctx, { _conn_reset.value(), _conn_reset.message() });
        }
    }

    virtual void on_socket_connected(error_code const &ec, std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();
        this->on_connected(ctx.get(), { ec.value(), ec.message() });
    }

    virtual void connect(TmxBrokerContext &ctx, types::Any const &params) noexcept override {
        if (ctx.get_state() != TmxBrokerState::initialized && ctx.get_state() != TmxBrokerState::disconnected)
            return;

        std::shared_ptr<socket> sock;
        {
            std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
            if (ctx.count(_sock))
                sock = types::as<socket>(ctx.at(_sock));

            if (!sock) {
                this->on_connected(ctx,
                                   { EINVAL, "Broker context " + ctx.to_string() + " was not initialized properly." });
                return;
            }
        }

        // Resolve the host name
        boost::system::error_code ec;
        resolver _resolver { sock->get_executor() };
        auto endpoints = _resolver.resolve(ctx.get_host(), ctx.get_port().empty() ? ctx.get_scheme() : ctx.get_port(), ec);
        if (ec) {
            this->on_connected(ctx, { ec.value(), ec.message() });
            return;
        }

        auto ep = endpoints.begin();

        // Open this socket
        if (!sock->is_open()) {
            error_code ec;
            sock->open(ep->endpoint().protocol(), ec);

            if (ec) {
                this->on_connected(ctx, { ec.value(), ec.message() });
                return;
            }
        }

        // Bind the server
        std::size_t len = ctx.get_scheme().length();
        if (len > 2 && ctx.get_scheme().substr(len - 2, 2) == "-d") {
            boost::asio::post(sock->get_executor(), [this, sock, ep, &ctx]() -> void {
                error_code ec;
                endpoint srv { ep->endpoint().address(), ep->endpoint().port() };

                TLOG(DEBUG) << ctx.get_id() << ": Binding to " << this->endpoint_info(srv);

                if (sock && sock->is_open())
                    sock->bind(srv, ec);
                else
                    ec = _conn_reset;

                this->on_socket_bound(ec, ctx);
            });
        } else {
            TLOG(DEBUG) << ctx.get_id() << ": Connecting to " << this->endpoint_info(ep->endpoint());

            sock->async_connect(endpoint(ep->endpoint().address(), ep->endpoint().port()),
                                std::bind(&self_type::on_socket_connected, this,
                                          std::placeholders::_1, std::ref(ctx)));
        }
    }

    void disconnect(TmxBrokerContext &ctx) noexcept override {
        if (ctx.count(_sock)) {
            auto sock = types::as<socket>(ctx.at(_sock));
            if (sock) {
                boost::asio::post(sock->get_executor(), [this, sock, &ctx]() {
                    error_code ec;
                    sock->cancel(ec);
                    sock->close(ec);

                    this->on_disconnected(ctx, { ec.value(), ec.message() });
                });
            }
        }
    }

    virtual void publish(TmxBrokerContext &ctx, TmxMessage const &msg) noexcept override {
        std::shared_ptr<socket> sock;
        if (ctx.count(_sock))
            sock = types::as<socket>(ctx.at(_sock));

        if (sock && sock->is_open()) {
            codec::TmxCodec codec { msg };

            auto bytes = codec.get_payload_bytes();
            TLOG(DEBUG1) << ctx.get_id() << ": " << "Writing " << bytes.length() << " bytes to the socket.";
            sock->async_send(boost::asio::const_buffer(bytes.data(), bytes.length()),
                             std::bind(&self_type ::on_write, this,
                                       std::placeholders::_1, std::placeholders::_2, std::ref(ctx), msg));
        } else {
            this->on_published(ctx, { ENOTCONN, std::strerror(ENOTCONN) }, msg);
        }
    }

    void on_read(error_code const &ec, std::size_t bytes,
                 std::shared_ptr<streambuf> buffer, std::reference_wrapper<TmxBrokerContext> ctx) noexcept override {

        if (ec.value() == boost::asio::error::operation_aborted)
            return;

        // Check the results, and invoke the callbacks
        TmxAsynchronousIOBrokerClient::on_read(ec, bytes, buffer, ctx);

        std::shared_ptr<socket> sock;
        if (ctx.get().count(_sock))
            sock = types::as<socket>(ctx.get().at(_sock));

        if (sock)
            this->read_next_message(*sock, buffer, ctx.get());
    }

    void read_next_message(socket &sock, std::shared_ptr<streambuf> buffer, TmxBrokerContext &ctx) noexcept {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

        if (!buffer) {
            this->on_error(ctx, { ENOMEM, "Could not create buffer for context " + ctx.get_id() });
            return;
        }

        // Check to see if there is a message break to use
        // TODO: Build a custom MatchCondition to use as std::regex. Supports boost::regex, but that is not header-only
        const TmxData params { ctx.get_parameters() };
        if (params["message-break"])
            boost::asio::async_read_until(sock, *buffer, params["message-break"].to_string(),
                                          std::bind(&self_type::on_read, this,
                                                    std::placeholders::_1, std::placeholders::_2, buffer, std::ref(ctx)));
        else
            boost::asio::async_read(sock, *buffer, boost::asio::transfer_at_least(1),
                                    std::bind(&self_type::on_read, this,
                                              std::placeholders::_1, std::placeholders::_2, buffer, std::ref(ctx)));
    }
};

template <>
void TmxAsynchronousSocketBridge<boost::asio::ip::udp>::read_next_message(socket &sock,
                                                                          std::shared_ptr<streambuf> buffer,
                                                                          TmxBrokerContext &ctx) noexcept {
    if (!buffer) {
        this->on_error(ctx, { ENOMEM, "Could not create buffer for context " + ctx.get_id() });
        return;
    }

    TLOG(DEBUG2) << ctx.get_id() << ": Awaiting incoming UDP message";

    const TmxData params { ctx.get_parameters() };
    sock.async_receive(buffer->prepare(65535), std::bind(&self_type::on_read, this,
                                                          std::placeholders::_1, std::placeholders::_2,
                                                          buffer, std::ref(ctx)));
}

template <>
void TmxAsynchronousSocketBridge<boost::asio::ip::udp>::on_socket_accepted(error_code const &ec,
                                                                           std::reference_wrapper<TmxBrokerContext> ctx) noexcept { }

template <>
void TmxAsynchronousSocketBridge<boost::asio::ip::udp>::on_socket_bound(const error_code &ec,
                                                                        std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

    std::shared_ptr<socket> sock;
    if (ctx.get().count(_sock))
        sock = types::as<socket>(ctx.get().at(_sock));

    if (sock && sock->is_open())
        this->on_connected(ctx, { ec.value(), ec.message() });
    else
        this->on_connected(ctx, { _conn_reset.value(), _conn_reset.message() });

    // If no errors, simply wait for any incoming messages
    if (!ec && sock)
        this->read_next_message(*sock, std::make_shared<streambuf>(), ctx);
}

/*!
 * @brief An HTTP client class to bridge remote files to TMX messages
 *
 * @TODO: Support https
 */
class TmxAsynchronousHTTPBridge: public TmxAsynchronousSocketBridge<boost::asio::ip::tcp> {
public:
    typedef TmxAsynchronousHTTPBridge self_type;
    typedef TmxAsynchronousSocketBridge<boost::asio::ip::tcp> super;

    TmxAsynchronousHTTPBridge() {
        this->register_broker("http");
//        this->register_broker("https");
    }

    TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_desc = TmxBrokerClient::get_descriptor();
        return { _desc.get_instance(), typeid(self_type), type_fqname(*this).data() };
    }

    void on_read_header(error_code const &ec, std::size_t bytes, std::shared_ptr<streambuf> buffer,
                         std::reference_wrapper<TmxBrokerContext> ctx) {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

        TLOG(DEBUG1) << ctx.get().get_id() << ": " << bytes << " bytes of HTTP header read from the socket.";

        if (ec) {
            this->on_disconnected(ctx, { ec.value(), ec.message() });
            return;
        }

        // Process the response header
        std::string header;
        std::istream is { buffer.get() };
        std::getline(is, header);

        header = types::String_(header.c_str()).trim();

        // Save the headers in case they are needed later
        if (!header.empty()) {
            TLOG(DEBUG1) << ctx.get().get_id() << ": " << header;

            if (!ctx.get().count("headers"))
                ctx.get()[types::Properties_::key_t("headers")].emplace<types::Null>();

            TmxData headers { ctx.get().at("headers") };
            auto idx = header.find_first_of(':');
            if (idx != header.npos)
                headers[std::string(header.substr(0, idx))] = types::String_(header.substr(idx+1)).trim();
        }

        std::shared_ptr<socket> sock;
        if (ctx.get().count(_sock))
            sock = types::as<socket>(ctx.get().at(_sock));

        if (sock) {
            if (!header.empty()) {
                // Read the next header line
                boost::asio::async_read_until(*sock, *buffer, "\r\n",
                                              std::bind(&self_type::on_read_header, this,
                                                        std::placeholders::_1, std::placeholders::_2, buffer, ctx));
            } else {
                // Read the next message
                this->read_next_message(*sock, buffer, ctx.get());
            }
        }
    }

    void on_read_response(boost::system::error_code const &ec, std::size_t bytes, std::shared_ptr<streambuf> buffer,
                          std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

        TLOG(DEBUG1) << ctx.get().get_id() << ": " << bytes << " bytes of HTTP response read from the socket.";

        if (ec && ec ) {
            this->on_disconnected(ctx, { ec.value(), ec.message() });
            return;
        }

        // Check that response is OK.
        std::istream is { buffer.get() };

        std::string http_version;
        unsigned int status_code;
        std::string status_message;

        is >> http_version;
        is >> status_code;
        std::getline(is, status_message);

        status_message = types::String_(status_message.c_str()).trim();

        TLOG(DEBUG2) << ctx.get().get_id() << ": " << http_version << " " << status_code  << " " << status_message;

        if (!is || http_version.empty()) {
            this->on_disconnected(ctx, { EBADMSG, "Incorrect response to HTTP request: " +
                                         http_version.empty() ? std::strerror(errno) : http_version });
            return;
        }

        if (status_code != 200) {
            this->on_disconnected(ctx.get(), { status_code, status_message });
            return;
        }

        std::shared_ptr<socket> sock;
        if (ctx.get().count(_sock))
            sock = types::as<socket>(ctx.get().at(_sock));

        if (sock && sock->is_open()) {
            // Read the response headers, line by line
            boost::asio::async_read_until(*sock, *buffer, "\r\n",
                                          std::bind(&self_type::on_read_header, this,
                                                    std::placeholders::_1, std::placeholders::_2, buffer, ctx));
        } else {
            this->on_disconnected(ctx, { _conn_reset.value(), _conn_reset.message() });
        }
     }

#define HTTP_HEADER_SRC 0x1eaddada

    void on_write(error_code const &ec, std::size_t bytes,
                  std::reference_wrapper<TmxBrokerContext> ctx, TmxMessage const &msg) noexcept override {
        super::on_write(ec, bytes, ctx, msg);

        // Read the incoming status from the request
        auto buffer = std::make_shared<streambuf>();
        if (msg.get_programmable_metadata() == HTTP_HEADER_SRC) {
            std::shared_ptr<socket> sock;
            if (ctx.get().count(_sock))
                sock = types::as<socket>(ctx.get().at(_sock));

            if (sock && sock->is_open()) {
                boost::asio::async_read_until(*sock, *buffer, "\r\n",
                                              std::bind(&self_type::on_read_response, this,
                                                        std::placeholders::_1, std::placeholders::_2,
                                                        buffer, ctx));
            } else {
                this->on_disconnected(ctx.get(), { _conn_reset.value(), _conn_reset.message() });
            }
        }
    }

    void subscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &cb) noexcept override {
        TmxAsynchronousIOBrokerClient::subscribe(ctx, topic, cb);

        // Now read the web page and forward to subscribers
        const TmxData params{ ctx.get_parameters() };

        std::stringstream ss;
        ss << "GET " << std::string(ctx.get_path().c_str()) << " HTTP/";
        ss << (params["http-version"] ? params["http-version"].to_string() : "1.1") << "\r\n";
        ss << "Host: ";
        ss << (params["hostname"] ? params["hostname"].to_string() : boost::asio::ip::host_name()) << "\r\n";
        ss << "User-Agent: ";
        ss << (params["user-agent"] ? params["user-agent"].to_string() : this->get_descriptor().get_type_name())
           << "\r\n";

        for (auto &header: params["additional-headers"].to_map())
            ss << header.first << ": " << TmxData(header.second).to_string() << "\r\n";

        if (!ctx.get_user().empty() || !ctx.get_secret().empty()) {
            auto userpass = std::string(ctx.get_user().c_str()) + ":" + ctx.get_secret().c_str();
            ss << "Authorization: Basic ";
            ss << byte_string_encode(to_byte_sequence(userpass.c_str(), userpass.length()), base64::value) << "\r\n";
        }

        ss << "Accept: */*\r\n";
        ss << "Connection: close\r\n\r\n";

        TLOG(DEBUG2) << "Sending headers: " << std::regex_replace(ss.str(), std::regex("\\r"), "\\\\r");
        ctx[_buffer].emplace<std::shared_ptr<streambuf> >(new streambuf());

        TmxMessage msg;
        msg.set_programmable_metadata(HTTP_HEADER_SRC);
        msg.set_timepoint();
        msg.set_payload(ss.str());

        this->publish(ctx, msg);
    }

    TmxError to_message(boost::asio::const_buffer const &buffer, TmxBrokerContext &ctx,
                        TmxMessage &msg) const noexcept override {
        auto err = TmxAsynchronousIOBrokerClient::to_message(buffer, ctx, msg);
        if (!err) {
            if (!ctx.get_path().empty())
                msg.set_topic(ctx.get_path().substr(1));

            if (ctx.count("headers")) {
                const TmxData headers{ ctx.at("headers") };

                if (headers["Server"])
                    msg.set_source(headers["Server"]);
                if (headers["Last-Modified"]) {
                    // Convert to a timestamp from (e.g.) Tue, 28 May 2024 13:22:30 GMT
                    static auto fmt = "%a, %d %b %Y %H:%M:%S";
                    std::istringstream is{ headers["Last-Modified"].to_string() };

                    std::tm time;
                    is >> std::get_time(&time, fmt);
                    if (is)
                        msg.set_timepoint(std::chrono::system_clock::from_time_t(std::mktime(&time)));
                }
            }
        }

        return err;
    }
};

static TmxAsynchronousSocketBridge<boost::asio::ip::udp> _udp_bridge_broker;
static TmxAsynchronousSocketBridge<boost::asio::ip::tcp> _tcp_bridge_broker;
static TmxAsynchronousHTTPBridge _http_bridge_broker;

} /* End namespace async */
} /* End namespace broker */
} /* End namespace tmx */
