/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file exception.hpp
 *
 *  Created on: Jul 22, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_EXCEPTION_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_EXCEPTION_HPP_

#include <tmx/common/TmxException.hpp>

#include <string>
#include <string_view>

namespace tmx {
namespace common {
namespace types {
namespace support {

typedef boost::error_info<struct tag_invalid_type, std::string_view> err_invalid_type;

class TmxInvalidTypeError: public tmx::common::TmxException {
public:
	TmxInvalidTypeError(): TmxException("Invalid type value") { }

	TmxInvalidTypeError(std::string_view reason): TmxInvalidTypeError() {
		*this << err_invalid_type(reason);
	}

	TmxInvalidTypeError(const char *reason): TmxInvalidTypeError() {
		*this << err_invalid_type(std::string_view(reason));
	}

	template <typename _T>
	TmxInvalidTypeError(_T value): TmxInvalidTypeError(std::to_string(value)) { }
};

} /* End namespace support */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_EXCEPTION_HPP_ */
