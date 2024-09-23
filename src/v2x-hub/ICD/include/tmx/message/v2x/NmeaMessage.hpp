/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file NmeaMessage.cpp
 *
 *  Created on: Apr 29, 2024
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_MESSAGE_V2X_NMEAMESSAGE_HPP_
#define INCLUDE_TMX_MESSAGE_V2X_NMEAMESSAGE_HPP_

#include <tmx/common/types/Any.hpp>
#include <tmx/plugin/dao/TmxDaoAttributes.hpp>

namespace tmx {
namespace message {
namespace v2x {

class NmeaMessage {
    typedef NmeaMessage self_type;

public:
    tmx_dao_attribute(std::string, sentence, "");

    inline std::string get_sentence_type() {
        if (this->get_sentence().length() >= 6)
            return this->get_sentence().substr(3, 3);

        return { };
    }

    inline NmeaMessage() = default;
    inline NmeaMessage(std::string sentence): _sentence(sentence) { }
    inline NmeaMessage(NmeaMessage const &copy): _sentence(copy.get_sentence()) { }
    inline NmeaMessage(common::types::Any const &data) {
        const TmxData _tmp { data };
        if (_tmp[sentence_attr::name]) this->set_sentence(_tmp[sentence_attr::name].to_string());
    }
};

}
}
}

#endif // INCLUDE_TMX_MESSAGE_V2X_NMEAMESSAGE_HPP_
