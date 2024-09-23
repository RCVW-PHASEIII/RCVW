/*
 * RtcmMessage.h
 *
 * This file contains the base RTCM message for use in TMX.
 * There are significant differences in the content by version and
 * by message type.  Therefore, a template specialization is declared
 * elsewhere that details the specific contents for that message version
 * and type.  The classes here only provide a generic implementations
 * across each RTCM version and message type.
 *
 *  Created on: May 11, 2018
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMMESSAGE_H_
#define INCLUDE_RTCM_RTCMMESSAGE_H_

#include <tmx/message/v2x/rtcm/RtcmVersion.hpp>

#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <atomic>
#include <bitset>
#include <string>
#include <tuple>

// The default TMX message namespace is tmx::messages
namespace tmx {
namespace message {
namespace v2x {
namespace rtcm {

typedef uint16_t msgtype_type;

/**
 * The virtual base class for all RTCM messages
 */
class TmxRtcmMessage {
public:

    TmxRtcmMessage() = default;

    typedef std::basic_string<common::byte_t> byte_string;

    TmxRtcmMessage(const TmxRtcmMessage &other) = default;

    virtual ~TmxRtcmMessage() = default;

    static constexpr const char *MessageType = "RTCM";
    static constexpr const char *MessageSubType = "Unknown";

    // Virtual functions.
    virtual inline rtcm::RTCM_VERSION get_version() const noexcept { return rtcm::RTCM_VERSION::UNKNOWN; };

    std::string get_version_name() const noexcept { return rtcm::RtcmVersionName(this->get_version()); }

    virtual inline rtcm::msgtype_type get_message_type() const noexcept { return 0; };

    virtual bool is_valid() const noexcept { return false; }

    virtual inline size_t size() const noexcept { return 0; }

protected:

};

/**
 *  The base template instance for all RTCM message implementation by version.
 */
template <rtcm::RTCM_VERSION _Ver = rtcm::RTCM_VERSION::UNKNOWN>
class RTCMMessage : public TmxRtcmMessage {
public:
    RTCMMessage() : TmxRtcmMessage() { }

    RTCMMessage(const TmxRtcmMessage &other) : TmxRtcmMessage(other) { }

    virtual ~RTCMMessage() { }

    virtual inline rtcm::RTCM_VERSION get_version() const noexcept override { return _Ver; }
};

/**
 * The base template instance for all RTCM message type implementation by version and type
 */
template <rtcm::RTCM_VERSION _Ver = rtcm::RTCM_VERSION::UNKNOWN, rtcm::msgtype_type _Id = 0>
class RTCMMessageType : public RTCMMessage<_Ver> {
public:
    RTCMMessageType() : RTCMMessage<_Ver>() { }

    RTCMMessageType(const TmxRtcmMessage &other) : RTCMMessage<_Ver>(other) { }

    virtual ~RTCMMessageType() { }

    virtual inline rtcm::RTCM_VERSION get_version() const noexcept override { return _Ver; }

    virtual inline rtcm::msgtype_type get_message_type() const noexcept override { return _Id; };
};

template <RTCM_VERSION _Ver>
void register_rtcm();

} /* End namespace rtcm */
} /* End namesoace v2x */

namespace codec {

template <v2x::rtcm::RTCM_VERSION _Ver>
class RtcmEncoder: public TmxEncoder {
    typedef RtcmEncoder<_Ver> self_type;

public:
    RtcmEncoder() {
        this->register_encoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        auto _descr = TmxEncoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), v2x::rtcm::RtcmVersionName<_Ver>() };
    }

    // Requires specialization for the version-specific implementation
    common::TmxError encode(v2x::rtcm::RTCMMessage<_Ver> const &, byte_stream &os) const {
        return this->unsupported(TMX_PRETTY_FUNCTION);
    }

    common::TmxError execute(common::TmxTypeDescriptor const &type,
                             std::reference_wrapper<common::TmxArgList> args) const override {
        if (args.get().size() < 0)
            return { EINVAL, "Invalid argument: Missing data to encode" };

        std::shared_ptr<byte_stream> os;
        if (args.get().size() > 1)
            os = common::types::as<byte_stream>(args.get()[1]);

        if (!os || !os.get())
            return { EINVAL, "Invalid argument: Missing output stream to encode to." };

        auto msg = common::types::as< v2x::rtcm::RTCMMessage<_Ver> >(args.get()[0]);
        if (!msg) {
            std::string err { "Encoder " };
            err.append(this->get_descriptor().get_type_name());
            err.append(" only works for RTCM version ");
            err.append(std::to_string((int)_Ver));
            return { ENOTSUP, err };
        }

        return this->encode(*msg, *os);
    }
};

template <v2x::rtcm::RTCM_VERSION _Ver>
class RtcmDecoder: public TmxDecoder {
    typedef RtcmDecoder<_Ver> self_type;

public:
    RtcmDecoder() {
        this->register_decoder();
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept override {
        auto _descr = TmxDecoder::get_descriptor();
        return { _descr.get_instance(), typeid(self_type), v2x::rtcm::RtcmVersionName<_Ver>() };
    }

    // Requires specialization for the version-specific implementation
    common::TmxError decode(common::byte_sequence const &, v2x::rtcm::RTCMMessage<_Ver> &msg) const {
        return this->unsupported(TMX_PRETTY_FUNCTION);
    }

    /*
     * TODO: At some point support building specific message types in the version
     *
     * This requires the use of the type parameter, but it varies from by the version
     * and also probably needs to use a pointer
     */
    common::TmxError execute(common::TmxTypeDescriptor const &type,
                             std::reference_wrapper<common::TmxArgList> args) const override {
        if (args.get().size() > 0) {
            auto bytes = common::types::as<common::byte_sequence>(args.get()[0]);
            if (bytes) {
                v2x::rtcm::RTCMMessage<_Ver> msg;
                auto err = this->decode(*bytes, msg);
                if (!err)
                    args.get().emplace_back(std::move(msg));

                return err;
            }
        }

        return { EINVAL, "No bytes to decode" };
    }
};

} /* End namesoace codec */
} /* End namespace message */
} /* End namespace tmx */


#ifndef IGNORE_RTCM2
#include "RTCM2.hpp"
#endif
#ifndef IGNORE_RTCM3
#include "RTCM3.hpp"
#endif

namespace tmx {
namespace message {
namespace v2x {
namespace rtcm {

inline void register_rtcm() {
#ifndef IGNORE_RTCM2
    register_rtcm<RTCM_VERSION::SC10402_3>();
#endif
#ifndef IGNORE_RTCM3
    register_rtcm<RTCM_VERSION::SC10403_3>();
#endif
}

} /* End namespace rtcm */
} /* End namesoace v2x */
} /* End namespace message */
} /* End namespace tmx */

#endif /* INCLUDE_RTCM_RTCMMESSAGE_H_ */
