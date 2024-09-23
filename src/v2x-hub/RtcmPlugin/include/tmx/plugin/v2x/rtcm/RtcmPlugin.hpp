/*
 * RtcmPlugin.h
 *
 *  Created on: Mar 22, 2018
 *      Author: gmb
 */

#ifndef SRC_RTCMPLUGIN_HPP_
#define SRC_RTCMPLUGIN_HPP_

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>

#include <tmx/message/v2x/rtcm/RtcmMessage.hpp>

#include <atomic>
#include <mutex>

namespace tmx {
namespace plugin {
namespace v2x {
namespace rtcm {

class RtcmPlugin: public TmxPlugin {
    friend class TmxPlugin;

public:
    RtcmPlugin();
    virtual ~RtcmPlugin() = default;

    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    common::TmxError main() noexcept override;

protected:
    void init() noexcept override;
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

    void on_gga_received(common::types::Any const &, message::TmxMessage const &);

    void on_rtcm_received(message::TmxData const &, message::TmxMessage const &);
#ifndef IGNORE_RTCM2
    void on_rtcmmsg_received(message::v2x::rtcm::RTCM2Message const &, message::TmxMessage const &);
#endif
#ifndef IGNORE_RTCM3
    void on_rtcmmsg_received(message::v2x::rtcm::RTCM3Message const &, message::TmxMessage const &);
#endif

private:
    std::mutex _lock;
    std::string _gga;
    std::string _ntrip;

    std::atomic<typename common::types::UIntmax::value_type> _count;
};

} /* namespace rtcm */
} /* namespace v2x */
} /* namespace plugin */
} /* namespace tmx */

#endif /* SRC_RTCMPLUGIN_HPP_ */
