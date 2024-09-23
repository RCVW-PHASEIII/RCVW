/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsynchronousIOContext.hpp
 *
 *  Created on: May 01, 2024
 *      @author: Gregory M. Baumgardner
 */
#include <tmx/broker/async/TmxAsynchronousIOBroker.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <boost/asio.hpp>
#include <chrono>
#include <future>
#include <thread>

using namespace tmx::common;
using namespace tmx::message;
using namespace boost;

namespace tmx {
namespace broker {
namespace async {

typedef typename common::TmxFunctor<common::types::Any const &, message::TmxMessage const &>::type::type cb_type;

void TmxAsynchronousIOBrokerClient::on_read(boost::system::error_code const &ec, std::size_t bytes,
                                            std::shared_ptr<boost::asio::streambuf> buffer,
                                            std::reference_wrapper<TmxBrokerContext> ctx) noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with (" << ec.value() << ") " << ec.message();

    TLOG(DEBUG1) << ctx.get().get_id() << ": " << bytes << " data bytes read from the I/O device.";

    bool eof = (ec.value() == boost::asio::error::eof);

    if (ec && !eof) {
        this->on_error(ctx.get(), { ec.value(), ec.message() });
        return;
    }

    // Skip empty reads
    if (buffer && bytes) {
        buffer->commit(bytes);

        TmxMessage msg;
        msg.set_topic("UNKNOWN");
        msg.set_timepoint();

        boost::asio::const_buffer _tmp { buffer->data().data(), bytes };

        auto err = this->to_message(_tmp, ctx.get(), msg);
        if (err)
            this->on_error(ctx, err);

        // Clear the buffer
        buffer->consume(bytes);

        this->callback(ctx.get().get_id(), msg);
    }
}

void TmxAsynchronousIOBrokerClient::on_write(boost::system::error_code const &ec, std::size_t bytes,
                                             std::reference_wrapper<TmxBrokerContext> ctx, TmxMessage const &msg) noexcept {
    TLOG(DEBUG1) << ctx.get().get_id() << ": " << bytes << " data bytes written to the I/O device.";

    this->on_published(ctx.get(), { ec.value(), ec.message() }, msg);
}

void TmxAsynchronousIOBrokerClient::subscribe(TmxBrokerContext &ctx, common::const_string topic,
                                              TmxTypeDescriptor const &cb) noexcept {
    if (!cb) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not valid.");

        this->on_subscribed(ctx, { 50, err }, topic, cb);
        return;
    }

    auto callback = cb.as_instance<cb_type>();
    if (!callback) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not the correct signature. Expecting ");
        err.append(type_fqname<cb_type>());

        this->on_subscribed(ctx, { 60, err }, topic, cb);
        return;
    }

    // Register the handler
    callback_registry(ctx.get_id(), topic.data()).register_handler(*callback, cb.get_typeid(), cb.get_type_short_name());
    TmxBrokerClient::subscribe(ctx, topic, cb);
}

void TmxAsynchronousIOBrokerClient::unsubscribe(TmxBrokerContext &ctx, common::const_string topic,
                                                TmxTypeDescriptor const &cb) noexcept {
    callback_registry(ctx.get_id(), topic).unregister(cb.get_typeid());
    TmxBrokerClient::unsubscribe(ctx, topic, cb);
}

boost::asio::thread_pool &TmxAsynchronousIOBrokerClient::get_context(TmxBrokerContext &ctx) const noexcept {
    typedef boost::asio::thread_pool pool_t;
    static typename types::Properties_::key_t _pool { type_short_name<pool_t>().data() };

    if (ctx.count(_pool)) {
        auto ptr = types::as<pool_t>(ctx.at(_pool));
        if (ptr)
            return *ptr;
    }

    // Nothing stored yet, so add it
    // First check this current executor
    auto exec = ctx.get_executor();
    if (exec && exec->get_implementation()) {
        auto pool = static_cast<boost::asio::thread_pool *>(exec->get_implementation());
        if (pool)
            return *(ctx[_pool].emplace<std::shared_ptr<pool_t> >(pool, [](auto *) { }));
    }

    // Otherwise, add a new pool
    const TmxData params { ctx.get_parameters() };
    std::size_t threadSz = 1;
    if (params["asio-pool-size"])
        threadSz = params["asio-pool-size"];

    return *(ctx[_pool].emplace<std::shared_ptr<pool_t> >(new pool_t(threadSz)));
}

TmxAsynchronousIOBrokerClient::strand_t TmxAsynchronousIOBrokerClient::make_strand(TmxBrokerContext &ctx) const noexcept {
    return boost::asio::make_strand(this->get_context(ctx));
}

TmxError TmxAsynchronousIOBrokerClient::to_message(boost::asio::const_buffer const &buf, TmxBrokerContext &ctx,
                                                   message::TmxMessage &msg) const noexcept {
    auto incoming = to_char_sequence((const char_t *)buf.data(), buf.size());

    TLOG(DEBUG2) << "Incoming: " << byte_string_encode(to_byte_sequence(incoming));

    // Do a quick trim of white space characters
    auto data = types::String_(incoming).trim();

    codec::TmxCodec codec;
    message::TmxData decoded;

    static auto preamble = message::TmxMessage::get_preamble();

    codec.get_message().set_source(ctx.get_id());
    codec.get_message().set_payload(data);

    // See if this is JSON or XML encoded
    const_string enc;
    if (!data.empty() && data[0] == '{' && data[data.size() - 1] == '}')
        enc = "json";
    else if (!data.empty() && data[0] == '<' && data[data.size() - 1] == '>')
        enc = "xml";

    if (!enc.empty()) {
        // Assume this is a payload
        codec.get_message().set_encoding(enc.data());

        TLOG(DEBUG2) << "Received message: " << codec.get_message().to_string();

        // TODO: See if the message was complete or not
//        auto err = codec.decode(decoded.get_container(), enc);
//        if (!err) {
//            const message::TmxData checker { decoded };
//            if (preamble == checker["preamble"].to_uint()) {
//                msg = codec.get_message();
//                return { };
//            }
//        }
//
        msg = codec.get_message();
        return { };
    }

    if (preamble == get_value<decltype(preamble)>(data.substr(0, byte_size_of<decltype(preamble)>()).data())) {
        // TODO: Chunk off each string and set to the message
        return { ENOTSUP, "No support yet for binary TMX message" };
    } else {
        TLOG(DEBUG3) << "Scanning for non-printable characters";

        // Scan for non-printable characters
        std::size_t i;
        for (i = 0; i < data.size(); i++) {
           auto chk = std::find(non_printable_characters::array.begin(),
                                           non_printable_characters::array.end(), data[i]);
           if (chk != non_printable_characters::array.end()) {
               // Make sure it is not a white space character also
               chk = std::find(whitespace_characters::array.begin(), whitespace_characters::array.end(), data[i]);
               if (chk == whitespace_characters::array.end()) {
                   TLOG(DEBUG2) << "Found non-printable character " << (int) data[i] << " at position " << i;
                   break;
               }
           }
        }

        if (i >= data.size()) {
            // This is a string payload
            codec.get_message().set_encoding("string");
        } else {
            auto bytes = to_byte_sequence(incoming.begin(), incoming.length());

            // This is a binary payload so encode the bytes
            if TMX_CONSTEXPR_FN (decltype(TMX_DEFAULT_BYTE_ENCODING)::size >= 16) {
                codec.get_message().set_base(decltype(TMX_DEFAULT_BYTE_ENCODING)::size);
                codec.get_message().set_payload(byte_string_encode(bytes));
            } else {
                codec.get_message().set_base(16);
                codec.get_message().set_payload(byte_string_encode(bytes, hexadecimal::value));
            }
        }

        msg = codec.get_message();
        return { };
    }

    return { };
}

} /* End namespace async */
} /* End namespace broker */
} /* End namespace tmx */
