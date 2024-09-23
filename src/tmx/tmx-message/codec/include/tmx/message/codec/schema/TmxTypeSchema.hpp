/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeSchema.cpp
 *
 * This file contains different schema options for TMX types
 *
 *  Created on: Aug 08, 2023
 *      @author: Gregory M. Baumgardner
 */


#ifndef CODEC_INCLUDE_TMX_MESSAGE_CODEC_SCHEMA_TMXTYPESCHEMA_HPP_
#define CODEC_INCLUDE_TMX_MESSAGE_CODEC_SCHEMA_TMXTYPESCHEMA_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <cerrno>
#include <memory>

namespace tmx {
namespace message {
namespace codec {
namespace schema {

template <typename _Td>
class TmxTypeSchema: public common::TmxTypeHandler {
public:
    TmxTypeSchema(_Td &schema) noexcept: _schema(schema) { }
    virtual ~TmxTypeSchema() = default;

    /*!
     * @return The schema instance for this type
     */
    auto const &get_schema() const noexcept {
        return this->_schema;
    }
private:
    _Td &_schema;
};

}
}
}
}

#endif /* CODEC_INCLUDE_TMX_MESSAGE_CODEC_SCHEMA_TMXTYPESCHEMA_HPP_ */
