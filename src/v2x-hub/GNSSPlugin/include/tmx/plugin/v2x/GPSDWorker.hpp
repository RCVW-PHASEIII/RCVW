/*
 * GPSDWorker.h
 *
 *  Created on: May 14, 2020
 *      @author: Gregory M. Baumgardner
 */

#ifndef GPSDWORKER_H_
#define GPSDWORKER_H_

#include <tmx/plugin/v2x/GNSSPlugin.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/dao/TmxDaoAttributes.hpp>
//#include <tmx/plugin/utils/ThreadWorker.hpp>

namespace tmx {
namespace messages {

/**
 * GPSD Message types
 */

class GPSDMessage {
public:
	static constexpr auto main_topic = "GPSD";

	GPSDMessage() = default;
};

class GPSDSampleMessage: public GPSDMessage {
    typedef GPSDSampleMessage self_type;
public:
	static constexpr auto topic = "Sample";

	GPSDSampleMessage(): GPSDMessage() { }
	GPSDSampleMessage(uint32_t connId): GPSDMessage() {
		this->set_connectionId(connId);
	}

	tmx_dao_attribute(uint32_t, connectionId, 0)
};

class GPSDVersionMessage: public GPSDMessage {
    typedef GPSDVersionMessage self_type;
public:
    static constexpr auto topic = "Version";

	GPSDVersionMessage(): GPSDMessage() { }
	GPSDVersionMessage(std::string ver): GPSDMessage() {
		this->set_version(ver);
	}

    tmx_dao_attribute(std::string, version, "");
};

class GPSDDeviceMessage: public GPSDMessage {
    typedef GPSDDeviceMessage self_type;
public:
    static constexpr auto topic = "Device";

	GPSDDeviceMessage(): GPSDMessage() { }

    tmx_dao_attribute(std::string, path, "");
    tmx_dao_attribute(std::string, driver, "");
    tmx_dao_attribute(int, flags, 0);
    tmx_dao_attribute(uint64_t, activated, 0);
    tmx_dao_attribute(std::string, subtype, "");
    tmx_dao_attribute(std::string, subtype1, "");
};

class GPSDUBXConfigMessage: public GPSDMessage {
    typedef GPSDUBXConfigMessage self_type;
public:
    static constexpr auto topic = "Config";
	static constexpr const char *ArrayElement = "configs";

	GPSDUBXConfigMessage(): GPSDMessage() { }

    tmx_dao_attribute(std::string, parameter, "");
    tmx_dao_attribute(std::string, value, "");
};

class GPSDNAVPVTMessage: public GPSDMessage {
public:
	static constexpr auto topic = "NAVPVT";
};

class GPSDDecodeRawMessage: public GPSDMessage {
    typedef GPSDDecodeRawMessage self_type;
public:
    static constexpr auto topic = "DecodeRaw";

    tmx_dao_attribute(std::string, bytes, "");
};

} /* namespace messages */

namespace plugin {
namespace v2x {
namespace GNSS {

class GPSDWorker: public common::TmxFunctor<broker::TmxBrokerContext, common::types::Any const> {
public:
    GPSDWorker() = default;

    common::TmxError execute(broker::TmxBrokerContext &, common::types::Any const &) const noexcept override;

    common::TmxError handle_sample_message(broker::TmxBrokerContext &, tmx::messages::GPSDSampleMessage const &) const noexcept;

    //void handle_version_message(tmx::message::LocationMessage const &data, tmx::message::TmxMessage const &msg);

    common::TmxError handle_version_message(broker::TmxBrokerContext &, tmx::messages::GPSDVersionMessage const &) const noexcept;
    common::TmxError handle_device_message(broker::TmxBrokerContext &, tmx::messages::GPSDDeviceMessage const &) const noexcept;
    //common::TmxError handle_ubxconfig_message(broker::TmxBrokerContext &, tmx::messages::GPSDUBXConfigMessage const &) noexcept;
    common::TmxError handle_navpvt_message(broker::TmxBrokerContext &, tmx::messages::GPSDNAVPVTMessage const &) const noexcept;
    common::TmxError handle_decoderaw_message(broker::TmxBrokerContext &, tmx::messages::GPSDDecodeRawMessage const &) const noexcept;
};

} /* namespace GNSS */
} /* namespace v2x */
} /* namespace plugin */
} /* namespace tmx */

#endif /* GPSDWORKER_H_ */
