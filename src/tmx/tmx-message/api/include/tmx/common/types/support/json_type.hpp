/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file json_type.hpp
 *
 *  Created on: Jul 20, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_JSON_TYPE_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_JSON_TYPE_HPP_

#define BOOST_JSON_STANDALONE
#if __has_include(<boost/json.hpp>)
#include <boost/json.hpp>
#else
#include <tmx/common/types/support/thirdparty/boost/json.hpp>
#endif

namespace tmx {
namespace common {
namespace types {
namespace support {

namespace json = boost::json;

}
}
}
}

#endif /* API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_JSON_TYPE_HPP_ */
