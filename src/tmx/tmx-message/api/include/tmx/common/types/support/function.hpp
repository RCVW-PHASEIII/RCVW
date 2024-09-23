/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file function.hpp
 *
 *  Created on: Jun 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_SUPPORT_FUNCTION_HPP_
#define INCLUDE_TMX_COMMON_TYPES_SUPPORT_FUNCTION_HPP_

#include <tmx/common/types/support/type_traits.hpp>

#include <cinttypes>
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#include <cmath>
#define __STRICT_ANSI__
#else
#include <cmath>
#endif
#include <ratio>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {
namespace support {

template <intmax_t _N, intmax_t _D = 1>
using rational = std::ratio<_N, _D>;

using ratio_identity = rational<1>;
using ratio_zero = rational<0>;

template <class Ratio>
using ratio_negate = std::ratio_multiply<std::ratio<-1, 1>, Ratio>;

template <class Ratio>
using ratio_reciprocate = std::ratio<Ratio::den, Ratio::num>;

template <typename _Fn>
static inline constexpr bool has_inverse() {
	return TMX_CHECK_FOR_TYPENAME(_Fn, inverse_type);
}

template <typename _Fn, bool HasType>
struct _inverter_type_t { typedef void type; };

template <typename _Fn>
struct _inverter_type_t<_Fn, true> { typedef typename _Fn::inverse_type type; };

template <typename _Fn>
struct _function_t {
	typedef _Fn fn_type;
	typedef _function_t<_Fn> self_type;
	typedef typename _inverter_type_t<_Fn, has_inverse<_Fn>()>::type inverter_type;
	typedef _function_t<inverter_type> inverter_fn;

	template <typename _T>
	inline constexpr auto operator()(_T input) {
		return _Fn::evaluate(input);
	}
};

template <>
struct _function_t<void> {
	typedef _function_t<void> self_type;
	typedef self_type inverter_fn;

	template <typename _T>
	inline constexpr void operator()(_T input) { }
};

template <typename _Fn>
using fn = _function_t<_Fn>;

template <class Slope, class YIntercept>
struct _linear_t {
	typedef _linear_t<Slope, YIntercept> self_type;
	typedef _linear_t<ratio_reciprocate<Slope>,
			std::ratio_divide<ratio_negate<YIntercept>, Slope>
	> inverse_type;

	static constexpr auto slope = 1.0 * Slope::num / Slope::den;
	static constexpr auto yintercept = 1.0 * YIntercept::num / YIntercept::den;

	template <typename _T>
	static inline constexpr auto evaluate(_T input) {
		return input * slope + yintercept;
	}
};

template <class Slope = ratio_identity, class YIntercept = ratio_zero>
using linear_fn = fn< _linear_t<Slope, YIntercept> >;

template <intmax_t Value>
using addition_fn = linear_fn< ratio_identity, rational<Value> >;

template <intmax_t Value>
using subtraction_fn = linear_fn< ratio_identity, ratio_negate< rational<Value> > >;

template <intmax_t Value>
using multiplication_fn = linear_fn< rational<Value> >;

template <intmax_t Value>
using division_fn = linear_fn< ratio_reciprocate< rational<Value> > >;

template <class> struct _logarithm_t;
struct ln { };

template <class Base>
struct _power_t {
	typedef _logarithm_t<Base> inverse_type;

	static constexpr auto base = 1.0 * Base::num / Base::den;

	template <typename _T>
	static inline auto evaluate(_T input) {
		return ::pow(base, input);
	}
};

template <>
struct _power_t< std::ratio<2, 1> > {
	typedef _logarithm_t< std::ratio<2, 1> > inverse_type;

	static constexpr auto base = 2;

	template <typename _T>
	static inline auto evaluate(_T input) {
		return ::exp2(input);
	}
};

template <>
struct _power_t<ln> {
	typedef _logarithm_t<ln> inverse_type;

	static constexpr auto base = ::exp(1);

	template <typename _T>
	static inline auto evaluate(_T input) {
		return ::exp(input);
	}
};

template <class Base>
struct _logarithm_t {
	typedef _power_t<Base> inverse_type;

	static constexpr auto base = inverse_type::base;
	static constexpr auto base_log = ::log10(base);

	template <typename T>
	static inline auto evaluate(T input) {
		return ::log10(input) / (base == 10 ? 1.0 : base_log);
	}
};

template <>
struct _logarithm_t< std::ratio<2, 1> > {
	typedef _power_t< std::ratio<2, 1> > inverse_type;

	template <typename _T>
	static inline auto evaluate(_T input) {
		return ::log2(input);
	}
};

template <>
struct _logarithm_t<ln> {
	typedef _power_t<ln> inverse_type;

	static constexpr auto base = inverse_type::base;

	template <typename _T>
	static inline auto evaluate(_T input) {
		return ::log(input);
	}
};

template <uint8_t Base = 10>
using power_fn = fn< _power_t< rational<Base> > >;

typedef fn< _power_t<ln> > e_fn;

template <uint8_t Base = 10>
using logarithm_fn = fn< _logarithm_t< rational<Base> > >;

typedef fn< _logarithm_t<ln> > ln_fn;

template <typename _Fn>
struct _recursive_function_t {

};

// Some useful fn class trait functions
template <typename _Fn>
static inline constexpr bool has_slope() {
	return TMX_CHECK_FOR_STATIC_CONSTEXPR(_Fn, slope);
}

template <typename _Fn, bool HasValue>
struct _slope_type_t { static constexpr auto value = 0.0; };

template <typename _Fn>
struct _slope_type_t<_Fn, true> { static constexpr auto value = _Fn::slope; };

template <class _C, typename _Fn = typename _C::fn_type>
inline constexpr bool is_linear_multiplicative() {
	return _slope_type_t<_Fn, has_slope<_Fn>()>::value != 0;
}

template <typename _Fn>
static inline constexpr bool has_yintercept() {
	return TMX_CHECK_FOR_STATIC_CONSTEXPR(_Fn, yintercept);
}

template <typename _Fn, bool HasValue>
struct _yintercept_type_t { static constexpr auto value = 0.0; };

template <typename _Fn>
struct _yintercept_type_t<_Fn, true> { static constexpr auto value = _Fn::yintercept; };

template <class _C, typename _Fn = typename _C::fn_type>
inline constexpr bool is_linear_additive() {
	return _yintercept_type_t<_Fn, has_yintercept<_Fn>()>::value != 0;
}

template <class _C, typename _Inv = typename _C::inverter_type>
inline constexpr bool is_invertible() {
	return !std::is_same_v<_Inv, void>();
}

} /* End namespace support */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* INCLUDE_TMX_COMMON_TYPES_SUPPORT_FUNCTION_HPP_ */
