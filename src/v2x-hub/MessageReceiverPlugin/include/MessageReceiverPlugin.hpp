/*
 * MessageReceiverPlugin.hpp
 *
 *  Created on: May 10, 2016
 *      Author: ivp
 *  Edited: May 16, 2024
 *      @author: Noah Limes
 */

#ifndef SRC_MESSAGERECEIVERPLUGIN_HPP_
#define SRC_MESSAGERECEIVERPLUGIN_HPP_

#include <atomic>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

#include <boost/asio.hpp>
#include <memory>
//#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace tmx {
namespace plugin {
namespace v2x {
namespace MessageReceiver {

class MessageReceiverPlugin : public tmx::plugin::TmxPlugin {
    friend class tmx::plugin::TmxPlugin;

public:
    MessageReceiverPlugin();
    ~MessageReceiverPlugin();

    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    common::TmxError main() noexcept override;

protected:
    void init() override;
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

private:
    // Handlers
    void handle_config_update(TmxPluginDataUpdate const &, tmx::message::TmxMessage const &);

    std::atomic<std::uint64_t> _recvMsgs { 0 }; //increment these in on_message_received
    std::atomic<std::uint64_t> _sentMsgs { 0 };

    std::mutex _syncLock;

    //Frequency Throttles
    utils::FrequencyThrottle<int> _statusThrottle;

    std::mutex _dataLock;

    //Status variables
    static std::atomic<uint64_t> _startTime;
    static std::atomic<uint64_t> _totalBytes; //increment in on_message_received
    static std::map<std::string, std::atomic<uint32_t> > _totalCount; //count per message type
    static std::mutex _mapMutex; // Mutex for protecting the map

    std::string readFileAsByteString(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return "";
        }

        std::ostringstream oss;
        oss << file.rdbuf();
        return oss.str();
    }

};

} /* End namespace MessageReceiver */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* SRC_MESSAGERECEIVERPLUGIN_HPP_ */
