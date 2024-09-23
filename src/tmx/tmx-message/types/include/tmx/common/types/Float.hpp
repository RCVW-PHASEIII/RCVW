/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Float.hpp
 *
 *  Created on: Jun 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_FLOAT_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_FLOAT_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <cstdint>
#include <float.h>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief A helper enumeration to track the floating point precision
 *
 * Values are based on the IEEE 754 definitions. There is no support
 * for half-precision.
 */
enum class float_precision: std::uint8_t {
	unknown   = 0x00,
	single    = 0x01,
	double_   = 0x02,
	quadruple = 0x04,
	octuple   = 0x08,
	extended  = 0x10
};

/*!
 * @brief Template structure for determining the minimum size of the type to use based on the number of bits
 */
template <std::uint8_t _Sz>
struct _float_container
{ typedef typename _float_container<_Sz - 1>::type type; };

// Template specializations
template <> struct _float_container<0>
{ typedef float type; };
template <> struct _float_container<tmx::common::bit_size<float>::value + 1>
{ typedef double type; };
template <> struct _float_container<tmx::common::bit_size<double>::value + 1>
{ typedef long double type; };
#ifdef TMX_FLOAT128
template <> struct _float_container<tmx::common::bit_size<long double>::value + 1>
{ typedef TMX_FLOAT128 type; };

#ifndef TMX_MAX_FLOAT
#define TMX_MAX_FLOAT TMX_FLOAT128
#endif
#else
#ifndef TMX_MAX_FLOAT
#define TMX_MAX_FLOAT long double
#endif
#endif

typedef typename _float_container<0>::type _floatmin_t;
typedef typename _float_container<0xFF>::type _floatmax_t;
typedef tmx::common::smaller_bit_size<double, _floatmax_t> _float_t;


/*!
 * @brief A class to contain a floating point value up to the specified number of bits
 *
 * All floating points are signed values stored in IEEE 754 notation. TMX should support
 * at minimum single-precision (float or float32), double-precision (double or float64),
 * and quadruple-precision (float128) formats, although it may vary by compiler.
 *
 * @see https://en.wikipedia.org/wiki/IEEE_754
 */
template <std::uint8_t _Sz>
class Float: public TmxDataType< typename _float_container<_Sz>::type > {
	/*!
	 * @brief The base class
	 */
	typedef TmxDataType< typename _float_container<_Sz>::type > super;

	static_assert(_Sz > 0,
			"The specified number of bits cannot be 0");
	static_assert(_Sz <= tmx::common::bit_size<_floatmax_t>::value,
			"The specified number of bits is beyond a valid floating point size");
	static_assert(tmx::common::is_float<typename super::value_type>::value,
			"Container must contain a floating point type");
public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A reflective type
	 */
	typedef Float<_Sz> self_type;

	/*!
	 * @brief Whether the type is a signed integer or not
	 */
	static constexpr bool is_signed = std::is_signed<value_type>::value;

	Float() noexcept: super() { }
	Float(value_type &&t) noexcept: super(std::forward<value_type>(t)) { }
	Float(value_type &t) noexcept: super(t) { };
	Float(value_type const &t) noexcept: Float(value_type(t)) { }

	using super::operator =;

	/*!
	 * @brief The number of bytes needed to store this floating point type
	 */
	static constexpr auto numBytes =
			tmx::common::byte_size<value_type>::value;

	/*!
	 * @brief Only need _Sz number of bits to store the floating point
	 */
	static constexpr auto numBits = _Sz;

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	std::size_t get_byte_count() const noexcept override {
		return numBytes;
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	std::size_t get_bit_count() const noexcept override {
		return numBits;
	}

	/*!
	 * @return A static instance of the numeric limits for this floating point type
	 */
	static auto &limits() noexcept {
		static constexpr std::numeric_limits<value_type> _singleton;
		return _singleton;
	}

private:
	static constexpr std::uint8_t base =
			static_cast<uint8_t>(tmx::common::bit_size<_floatmin_t>::value & 0xFF);
	static constexpr std::uint8_t expected =
			static_cast<uint8_t>(tmx::common::bit_size<value_type>::value & 0xFF);

public:
	/*!
	 * @brief The precision for this floating point
	 */
	static constexpr std::uint8_t precision = (expected / base) |
			(numBits == expected ? 0x00 : 0x10);

	/*!
	 * @brief Check if this floating point value has the specified precision
	 *
	 * Note that there may be multiple precisions, such as the quadruple
	 * exended precision supported by float80
	 *
	 * @return True if this value is at precision _P
	 */
	template <float_precision _P>
	static TMX_CONSTEXPR_FN bool is_precision() {
		return precision & static_cast<uint8_t>(_P);
	}
};

template <typename _Tp = _float_t>
using float_type = Float< tmx::common::bit_size<_Tp>::value >;


/*!
 * @brief The smallest allowed TMX floating point type
 */
typedef float_type<_floatmin_t> Floatmin;

/*!
 * @brief The largest allowed TMX floating point type
 */
typedef float_type<_floatmax_t> Floatmax;

/*!
 * @brief The default TMX signed integer type
 *
 * By convention, this is set to 32-bits, unless the maximum
 * integer allowed is actually less than that, in which case
 * the maximum integer type is used.
 */
typedef float_type<> Float_;

template <std::uint8_t _Sz, class _C = Float<_Sz> >
using _safe_float =
		typename std::conditional<tmx::common::bit_size_difference< _C, Floatmax>::num <= 0, _C, void>::type;

typedef _safe_float<32> Float32;
typedef _safe_float<64> Float64;
typedef _safe_float<128> Float128;

typedef Float64 Double;

template <std::uint8_t _Sz>
struct TmxType<Float<_Sz>, void> {
	typedef std::true_type TMX;
	typedef Float<_Sz> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _Tp>
struct TmxType<_Tp, tmx::common::enable_float<_Tp, void> > {
	typedef std::true_type TMX;
	typedef float_type<_Tp> type;
    static constexpr auto name = type_short_name<type>();
};

template <std::size_t>
struct _float_types;

template <>
struct _float_types<128> {
    typedef std::tuple< Float<32>, Float<64>, Float<128> > type;
};

template <>
struct _float_types<80> {
    typedef std::tuple< Float<32>, Float<64>, Float<80> > type;
};

template <>
struct _float_types<64> {
    typedef std::tuple< Float<32>, Float<64> > type;
};

template <>
struct _float_types<32> {
    typedef std::tuple< Float<32> > type;
};

using TmxFloatTypes = typename _float_types<Floatmax::numBits>::type;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_FLOAT_HPP_ */
