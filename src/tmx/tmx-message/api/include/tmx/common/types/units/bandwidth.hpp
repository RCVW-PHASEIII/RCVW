/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Bandwidth.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_SUPPORT_UNITS_BANDWIDTH_HPP_
#define INCLUDE_TMX_SUPPORT_UNITS_BANDWIDTH_HPP_

#include <tmx/support/units.hpp>

// Bandwidth Ratios.  Nominal unit is mega bits per second (Mbps)
namespace tmx {
namespace support {
namespace units {

enum class Bandwidth
{
		bps,
		kbps,
		Mbps,
		Gbps,
		Tbps,
		Pbps,
		Bytesps,
		kBytesps,
		MBytesps,
		GBytesps,
		TBytesps,
		PBytesps
};

} /* End namespace units */

template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::bps> { typedef std::micro type; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::kbps> { typedef std::milli type; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::Gbps> { typedef std::kilo type; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::Tbps> { typedef std::mega type; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::Pbps> { typedef std::giga type; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::Bytesps> { static constexpr double value = 8.0; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::Bytesps> { typedef std::micro type; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::kBytesps> { static constexpr double value = 8.0; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::kBytesps> { typedef std::milli type; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::MBytesps> { static constexpr double value = 8.0; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::GBytesps> { static constexpr double value = 8.0; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::GBytesps> { typedef std::kilo type; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::TBytesps> { static constexpr double value = 8.0; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::TBytesps> { typedef std::mega type; };
template <> struct unit_base_t<units::Bandwidth, units::Bandwidth::PBytesps> { static constexpr double value = 8.0; };
template <> struct unit_ratio_t<units::Bandwidth, units::Bandwidth::PBytesps> { typedef std::giga type; };


} /* End namespace support */
} /* End namespace tmx */




#endif /* INCLUDE_TMX_SUPPORT_UNITS_BANDWIDTH_HPP_ */
