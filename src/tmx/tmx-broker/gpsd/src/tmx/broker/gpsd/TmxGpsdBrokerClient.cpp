/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GPSDBrokerClient.hpp
 *
 *  Created on: Jun 14, 2023
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 7, 2024
 *      @author: Noah Limes
 */

#include <tmx/broker/gpsd/TmxGpsdBrokerClient.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxLogger.hpp>

#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <gps.h>
#include <istream>
#include <regex>
#include <sstream>

using namespace tmx::common;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace gpsd {

using boost::system::error_code;
typedef typename boost::asio::ip::tcp::socket socket;

static typename types::Properties_::key_t _gps { type_short_name<struct gps_data_t>().data() };
static typename types::Properties_::key_t _sock { type_short_name<socket>().data() };

TmxGpsdBrokerClient::TmxGpsdBrokerClient() noexcept {
    this->register_broker("gpsd");      // Basic location information
    this->register_broker("gnss");      // Raw bytes from the GNSS receiver
    this->register_broker("nmea");      // (pseudo) NMEA messages from the GNSS reciver
}

TmxTypeDescriptor TmxGpsdBrokerClient::get_descriptor() const noexcept {
    auto _desc = TmxBrokerClient::get_descriptor();
    return { _desc.get_instance(), typeid(*this), type_fqname(*this).data() };
}

common::types::Any TmxGpsdBrokerClient::get_broker_info(TmxBrokerContext &ctx) const noexcept {
    message::TmxData info = super::get_broker_info(ctx);

    // Make copies of the context data
    // TODO: Add more broker information besides version?
    for (auto &key: { typename types::Properties_::key_t("version") }) {
        if (ctx.count(key)) {
            TLOG(INFO) << "Got " << key << ": " << TmxData(ctx[key]).to_string();
            info["gpsd"][key] = types::Any(static_cast<types::Any const &>(ctx[key]));
        }
    }

    return std::move(info.get_container());
}

void TmxGpsdBrokerClient::initialize(TmxBrokerContext &ctx) noexcept {
    if (ctx.get_state() == TmxBrokerState::connected) {
        this->on_destroyed(ctx, {1, "Broker context " + ctx.to_string() + " must first be disconnected"});
        return;
    }

    // Insert a null GPSD data structure
    ctx[_gps].emplace<struct gps_data_t>().gps_fd = 0;

    // Insert an empty socket within a strand to use
    ctx[_sock].emplace<std::shared_ptr<socket> >(std::make_shared<socket>(this->make_strand(ctx)));

    super::initialize(ctx);
}

void TmxGpsdBrokerClient::destroy(TmxBrokerContext &ctx) noexcept {
    if (this->is_connected(ctx))
        this->disconnect(ctx);

    ctx.erase(_gps);
    ctx.erase(_sock);
    TmxBrokerClient::destroy(ctx);
}

auto to_seconds(timespec &tm) {
    std::chrono::duration<double, std::chrono::seconds::period> sec { tm.tv_sec };
    sec += std::chrono::nanoseconds(tm.tv_nsec);
    return sec.count();
}

int read_gps_message(TmxBrokerContext &ctx, std::shared_ptr<boost::asio::streambuf> buffer) noexcept {
    if (!ctx.count(_gps) || !ctx.count(_sock))
        return ENOTCONN;

    auto gps = types::as<struct gps_data_t>(ctx.at(_gps));
    if (!gps || !gps->gps_fd)
        return ENOTCONN;

    std::size_t bufSize = 64 * 1024;
    const TmxData params { ctx.get_parameters() };
    if (params["max-buffer-size"])
        bufSize = params["max-buffer-size"];

    auto buf = buffer->prepare(bufSize);
    int n = gps_read(gps.get(), (char *) buf.data(), buf.size());
    if (n <= 0)
        return n;

    // Need to cache some things
    if (gps->set & VERSION_SET) {
        TmxData ver;
        if (std::strlen(gps->version.release))
            ver["release"] = std::string(gps->version.release);
        if (std::strlen(gps->version.rev))
            ver["revision"] = std::string(gps->version.rev);
        if (gps->version.proto_major) {
            ver["protocol"]["major"] = gps->version.proto_major;
            ver["protocol"]["minor"] = gps->version.proto_minor;
        }

        ctx[typename types::Properties_::key_t("version")] = std::move(ver.get_container());
    }

    auto chars = to_char_sequence((char *) buf.data(), n);
    int idx = chars.find_first_of('\n');
    chars = chars.substr(0, idx);

    TLOG(DEBUG1) << "Read from GPSD: " << chars;

    return chars.length();
}

void TmxGpsdBrokerClient::on_read(const boost::system::error_code &ec, std::size_t bytes,
                                  std::shared_ptr<boost::asio::streambuf> buffer,
                                  std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

    if (ec.value() == boost::asio::error::operation_aborted)
        return;

    if (ec) {
        this->on_error(ctx, { ec.value(), ec.message() });
        return;
    }

    error_code err;
    int n = read_gps_message(ctx.get(), buffer);
    if (n < 0)
        err = boost::system::errc::make_error_code(static_cast<boost::system::errc::errc_t>(errno));
    else
        err = boost::system::errc::make_error_code((boost::system::errc::errc_t::success));

    // Process the incoming GPSD message, reading one line at a time
    if (n < 0) {
        this->on_disconnected(ctx, { err.value(), err.message() });
        return;
    }

    if (n > 0) {
        super::on_read(err, n - 1, buffer, ctx);
        buffer->consume(1);
    }

    // Schedule the next read
    if (ctx.get().count(_sock)) {
        auto sock = types::as<socket>(ctx.get().at(_sock));

        // Wait for the next data to be available
        if (sock && sock->is_open())
            sock->async_wait(boost::asio::ip::tcp::socket::wait_read, std::bind(&self_type::on_read, this,
                                                                                std::placeholders::_1, 0, buffer, ctx));

        // Consider the broker connected once the first successful read (VERSION) is complete
        // But, post the callback to the strand so that additional reads may occur to get more information
        if (!err && !this->is_connected(ctx.get())) {
            boost::asio::post(sock->get_executor(), [this, ctx]() {
                this->on_connected(ctx.get(), { EXIT_SUCCESS, std::strerror(EXIT_SUCCESS) });
            });
        }
    }

}

void TmxGpsdBrokerClient::on_gps_stream(error_code const &ec, std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

    auto gps = types::as<struct gps_data_t>(ctx.get().at(_gps));
    if (!gps) {
        this->on_connected(ctx, {2, "Context not properly initialized for GPS use."});
        return;
    }

    auto sock = types::as<socket>(ctx.get().at(_sock));
    if (!sock) {
        this->on_connected(ctx, {3, "Context not properly initialized for GPS use."});
        return;
    }

    //    super::on_connected(ctx.get(), { ec.value(), ec.message() });

    if (!ec && gps->gps_fd > 0) {
        error_code err;

        // Set the socket up
        sock->assign(socket::protocol_type::v4(), gps->gps_fd, err);
        if (err) {
            this->on_connected(ctx, { err.value(), err.message() });
            return;
        }

        if (sock->is_open()) {
            // Automatically query for devices
            gps_send(gps.get(), "?DEVICES;\n");
            sock->async_wait(socket::wait_read, std::bind(&self_type::on_read, this,
                                                          std::placeholders::_1, 0,
                                                          std::make_shared<boost::asio::streambuf>(), ctx));
        }
    }
}

void TmxGpsdBrokerClient::on_gps_open(error_code const &ec, std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

    if (ec) {
        this->on_connected(ctx.get(), { ec.value(), "Failed to open connection to GPSD: " + ec.message() });
        return;
    }

    auto gps = types::as<struct gps_data_t>(ctx.get().at(_gps));
    if (!gps) {
        this->on_connected(ctx, {2, "Context not properly initialized for GPS use."});
        return;
    }

    // Start the streaming
    boost::asio::post(this->get_context(ctx.get()), [this, ctx, gps] {
        int watch = WATCH_ENABLE;
        if (!ctx.get().get_path().empty())
            watch |= WATCH_DEVICE;

        if (std::strcmp("gnss", ctx.get().get_scheme().c_str()) == 0)
            watch |= WATCH_RAW;
        else if (std::strcmp("nmea", ctx.get().get_scheme().c_str()) == 0)
            watch |= WATCH_NMEA;
        else
            watch |= WATCH_JSON;

        TLOG(DEBUG) << "Opening GPSD stream with watch " << watch;

        int ret = gps_stream(gps.get(), watch, (void *)ctx.get().get_path().c_str());

        boost::system::error_code ec;
        if (ret)
            ec = boost::system::errc::make_error_code(static_cast<boost::system::errc::errc_t>(errno));
        else
            ec = boost::system::errc::make_error_code((boost::system::errc::errc_t::success));

        this->on_gps_stream(ec, ctx);
    });
}

void TmxGpsdBrokerClient::connect(TmxBrokerContext &ctx, const types::Any &args) noexcept {
    if (ctx.get_state() != TmxBrokerState::initialized && ctx.get_state() != TmxBrokerState::disconnected)
        return;

    if (!ctx.count(_gps)) {
        this->on_connected(ctx, {1, "Invalid context"});
        return;
    }

    auto gps = types::as<struct gps_data_t>(ctx.at(_gps));
    if (!gps) {
        this->on_connected(ctx, {2, "Context not properly initialized for GPS use."});
        return;
    }

    // Resolve the hostname
    // Resolve the host name, but only the service name if it is not already a port number
    boost::system::error_code ec;
    typename boost::asio::ip::tcp::resolver _resolver { this->get_context(ctx) };
    auto endpoints = _resolver.resolve(ctx.get_host(),
                                       ctx.get_port().empty() ? "gpsd" : ctx.get_port().c_str(), ec);
    if (ec) {
        this->on_connected(ctx, { ec.value(), ec.message() });
        return;
    }

    if (endpoints.empty()) {
        this->on_connected(ctx, { EINVAL, "Could not find host " + ctx.to_string() });
        return;
    }

    auto ep = endpoints.begin();
    auto host = ep->host_name();
    auto port = std::to_string(ep->endpoint().port());
    ctx[types::Properties_::key_t("port")] = port;

    boost::asio::post(this->get_context(ctx), [this, ctxRef = std::ref(ctx), gps, host, port]() -> void {
        TLOG(DEBUG) << "Opening GPSD connection to " << host << ":" << port;

        int ret = gps_open(host.c_str(), port.c_str(), gps.get());

        boost::system::error_code ec;
        if (ret)
            ec = boost::system::errc::make_error_code(static_cast<boost::system::errc::errc_t>(errno));
        else
            ec = boost::system::errc::make_error_code((boost::system::errc::errc_t::success));

        this->on_gps_open(ec, ctxRef);
    });
}

void TmxGpsdBrokerClient::disconnect(TmxBrokerContext &ctx) noexcept {
    if (ctx.count(_sock)) {
        auto sock = types::as<socket>(ctx.at(_sock));
        if (sock) {
            boost::asio::post(sock->get_executor(), [sock, &ctx]() {
                error_code ec;
                sock->cancel(ec);
                sock->close(ec);

                if (ctx.count(_gps)) {
                    auto gps = types::as<struct gps_data_t>(ctx.at(_gps));
                    if (gps)
                        gps_close(gps.get());
                }
            });
        }
    }

    super::disconnect(ctx);
}

void on_read_config_options(const boost::system::error_code &ec, std::size_t bytes,
                            FILE *pipe, std::shared_ptr<boost::asio::streambuf> buffer,
                            std::reference_wrapper<TmxBrokerContext> ctx, TmxMessage const &msg) noexcept {
    auto broker = TmxBrokerClient::get_broker(ctx.get());
    if (!broker)
        return;

    if (!pipe) {
        broker->on_published(ctx.get(), { errno, std::strerror(errno) }, msg);
        return;
    }

}

void TmxGpsdBrokerClient::publish(TmxBrokerContext &ctx, TmxMessage const &msg) noexcept {
#ifdef TMX_UX_POSIX
    static std::string baseCmd = "PATH=$PATH:/usr/bin:/usr/local/bin ubxtool ";

    // Currently only supports writing a u-Blox configuration using the provided tool
    if (std::strcmp("gpsd", ctx.get_scheme().c_str()) == 0) {
        // Get all the configuration options

        // Open the pipe
        FILE *pipe = ::popen((baseCmd + "-h -v5 | awk '$1 ~ /CFG-.*-/ {print $1}'").c_str(), "r");

        // Wrap with ASIO
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::posix::stream_descriptor stream { this->get_context(ctx), pipe };
        stream.async_read_some(buffer->prepare(1024), std::bind(&on_read_config_options,
                                                                std::placeholders::_1, std::placeholders::_2,
                                                                pipe, buffer, std::ref(ctx), msg));
    }
#else
    super::publish(ctx, msg);
#endif
}

common::TmxError TmxGpsdBrokerClient::to_message(boost::asio::const_buffer const &buffer, TmxBrokerContext &ctx,
                                                 message::TmxMessage &msg) const noexcept {
    // Parse the incoming message
    auto err = super::to_message(buffer, ctx, msg);
    if (err) return err;

    std::shared_ptr<struct gps_data_t> gps;
    if (ctx.count(_gps))
        gps = types::as<struct gps_data_t>(ctx.at(_gps));

    if (gps && std::strlen(gps->dev.path))
        msg.set_source(std::string(gps->dev.path));

    auto const &payload = msg.get_payload_string();

    // Check for the specific topic
    if (payload.length() >= 7 && payload[0] == '$' && payload[6] == ',') {
        // This is a NMEA specific message
        msg.set_topic(std::string("nmea") + std::filesystem::path::preferred_separator);
        msg.get_topic().append(payload.substr(3, 3).c_str());
    } else if (msg.get_encoding() == "json") {
        // This is a GPSD specific message
        msg.set_topic(std::string("gpsd") + std::filesystem::path::preferred_separator);
        auto idx = payload.find_first_of(',');
        if (idx != payload.npos && idx > 11 && payload[0] == '{' && payload[idx - 1] == '"' &&
            payload.substr(2, 5) == "class")
            msg.get_topic().append(payload.substr(10, idx - 11).c_str());
        else
            msg.get_topic().append("UNKNOWN");
    } else {
        // This must be a raw byte message
        msg.set_topic(std::string("gnss") + std::filesystem::path::preferred_separator);

        // See if it is RTCM data or a specific UBX message
        if (gps->set & RTCM2_SET)
            msg.get_topic().append("RTCM2");
        else if (gps->set & RTCM3_SET)
            msg.get_topic().append("RTCM3");
        else if (payload.length() >= 2 && payload[0] == '5' && std::toupper(payload[1]) == 'B')
            msg.get_topic().append("UBX");
        else
            msg.get_topic().append("UNKNOWN");
    }

        return { };
}

static TmxGpsdBrokerClient _gpsd_broker;

} /* End namespace gpsd */
} /* End namespace broker */
} /* End namespace tmx */