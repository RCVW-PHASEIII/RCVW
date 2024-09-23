/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsnDot1Schema.cpp
 *
 *  Created on: Aug 08, 2023
 *      @author: Gregory M. Baumgardner
 */


#ifndef ASNTYPE_INCLUDE_TMX_MESSAGE_CODEC_ASN_TMXASNDOT1SCHEMA_HPP_
#define ASNTYPE_INCLUDE_TMX_MESSAGE_CODEC_ASN_TMXASNDOT1SCHEMA_HPP_

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/codec/asn/asn_application.h>

#include <memory>

namespace tmx {
namespace message {
namespace codec {
namespace asn {
namespace schema {

asn_TYPE_descriptor_t const *get_schema(common::TmxTypeDescriptor const &) noexcept;
common::TmxError dump(common::TmxTypeDescriptor const &, const void *, common::types::Any &) noexcept;
common::TmxError load(common::TmxTypeDescriptor const &, common::types::Any const &, void **) noexcept;

template <typename _T>
common::TmxError dump(const _T *obj, common::types::Any &out) {
    return dump(common::TmxTypeRegistry().get(typeid(_T), true), (const void *)obj, out);
}

template <typename _T>
common::TmxError load(common::types::Any const &obj, _T **out) {
    return load(common::TmxTypeRegistry().get(typeid(_T), true), obj, out);
}


} /* End namespace schema */
} /* End namespace asn */
} /* End namespace codec */
} /* End namespace message */
} /* End namespace tmx */

#endif /* ASNTYPE_INCLUDE_TMX_MESSAGE_CODEC_ASN_TMXASNDOT1SCHEMA_HPP_ */
