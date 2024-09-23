/*
 * RtcmVersion.h
 *
 * Definition and operations on available RTCM message versions.
 *
 *  Created on: May 11, 2018
 *      Author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_RTCM_RTCMVERSION_H_
#define INCLUDE_RTCM_RTCMVERSION_H_

#include <tmx/common/types/Enum.hpp>

#include <cctype>
#include <cstdlib>

namespace tmx {
namespace message {
namespace v2x {
namespace rtcm {

/**
 * Enumerated value for possible RTCM message versions, as well
 * as the associated numeric value.
 */

enum class RTCM_VERSION : std::uint8_t {
    UNKNOWN = 0,
    SC10402_3 = 2,
    SC10403_3 = 3
};

TMX_CONSTEXPR_FN inline const char *RtcmVersionName(RTCM_VERSION version) {
    return common::enums::enum_name(version).data();
}

template <RTCM_VERSION _Ver>
TMX_CONSTEXPR_FN inline const char *RtcmVersionName() {
    return common::enums::enum_name<_Ver>().data();
}

inline RTCM_VERSION RtcmVersion(typename common::types::Enum<RTCM_VERSION>::underlying_type version) {
    auto ret = common::enums::enum_cast<RTCM_VERSION>(version);
    return ret ? *ret : RTCM_VERSION::UNKNOWN;
};

inline RTCM_VERSION RtcmVersion(const char *version) {
    if (version && std::isdigit(*version))
        return RtcmVersion(*version - '0');

    auto ret = common::enums::enum_cast<RTCM_VERSION>(version);
    return ret ? *ret : RTCM_VERSION::UNKNOWN;
}


} /* End namespace rtcm */
} /* End namespace v2x */
} /* End namespace message */
} /* End namespace tmx */

#endif /* INCLUDE_RTCM_RTCMVERSION_H_ */
