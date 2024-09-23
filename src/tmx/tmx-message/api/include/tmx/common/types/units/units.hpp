/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file units.hpp
 *
 *  Created on: Apr 17, 2017
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_UNITS_UNITS_HPP_
#define INCLUDE_TMX_COMMON_TYPES_UNITS_UNITS_HPP_

#if __cplusplus < 201103L
#error C++11 is minimally required for unit conversions with std::ratio.  Please use a different compiler!
#else
#include <ratio>
#endif

#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/support/function.hpp>

namespace support = tmx::common::types::support;

namespace tmx {
namespace common {
namespace types {
namespace units {

// Unit helper classes

template <class Ratio>
using _unit_linear_fn = support::linear_fn<Ratio>;

/*!
 * A class to hold a conversion ratio.
 *
 * This ratio must specifically be the number of nominal units exists per each of
 * the units.  The nominal unit is arbitrary, but should generally be the most
 * commonly used unit.
 *
 * So, for example, for Time units, the nominal unit may be seconds, so the
 * millisecond ratio would be the number of seconds per millisecond,
 * which is 1/1000, or std::milli.
 *
 * This template must be specialized for all units, unless the conversion is 1:1
 */
template <typename _U, _U Val>
struct _unit_ratio_t {
	typedef support::ratio_identity type;
};

/*!
 * An alias for un-boxing the ratio
 */
template <typename _U, _U Val>
using ratio = typename _unit_ratio_t<_U, Val>::type;

template <typename _U, _U Val>
struct _unit_conversion_fn_t {
	typedef _unit_linear_fn< ratio<_U, Val> > type;
};

template <typename _U, _U Val>
using unit_conversion_fn = typename _unit_conversion_fn_t<_U, Val>::type;

template <typename _U, _U Val>
using unit_inverse_conversion_fn = typename unit_conversion_fn<_U, Val>::inverter_fn;

/*!
 * The base factor to use for a unit conversion.
 *
 * If the ratio is based on the nominal unit, then the default value
 * of one is almost always sufficient. If the unit is based off of some
 * other unit factor that is not the nominal unit, or if the factor is
 * shifted by some additional factor, such as pi for radians, then
 * this structure must be specialized.
 */
template <typename T, T Val>
struct unit_base_t {
	static constexpr double value = 1.0;
};

/*!
 * A class which calculates the conversion factor, which is the base factor
 * times the ratio factor.
 */
template <typename T, T Val>
struct unit_factor_t {
	static constexpr const double value =
			unit_base_t<T, Val>::value * ratio<T, Val>::num / ratio<T, Val>::den;
};

/*!
 * An alias for un-boxing the conversion factor
 */
template <typename T, T Val>
using factor = unit_factor_t<T, Val>;


/**
 * Function for converting between units of the same type using the unit ratios
 */
template <typename _U, _U From, _U To, typename _T>
static inline auto Convert(_T in) {
	static unit_inverse_conversion_fn<_U, From> _from;
	static unit_conversion_fn<_U, To> _to;

	// TODO: Can we do a compile type optimization to make this even faster?
	return _to(_from(in));
}

template <typename _U>
struct _unit_t: public tmx::common::types::enum_type<_U> {

};

template <typename _U>
using unit_type = _unit_t<_U>;

/**
 * UnitRatio template specializations
 *
 * These must be updated if new units are added
 */

// Some helper macros
#define UNIT_DECLR(A, B) template <> struct UnitRatio<A, A::B>
#define UNIT_SPEC1(A, B, C) UNIT_DECLR(A, B) { typedef C type; }
#define UNIT_SPEC2(A, B, C, D) UNIT_DECLR(A, B) { typedef std::ratio<C, D> type; }
#define UNIT_COMP1(A, B, C, D, E) UNIT_DECLR(A, B) { typedef typename UnitCompoundRatio<A, A::C, std::ratio<D, E> >::type type; }
#define UNIT_COPY1(A, B, C, D) UNIT_DECLR(A, B) { typedef typename UnitRatio<C, C::D>::type type; }
#define UNIT_CNVRT(A, B, C, D) template <> inline double Convert<A, A::B, A::C>(double in) { return Convert<A, A::B, A::C, A::D>(in); }
#define UNIT_CVRT1(A, B, C, D) UNIT_CNVRT(A, B, C, D); UNIT_CNVRT(A, C, B, D);

// Time Ratios.  Nominal unit is second (s)
UNIT_SPEC1(Time, ns, std::nano);
UNIT_SPEC1(Time, us, std::micro);
UNIT_SPEC1(Time, ms, std::milli);
UNIT_SPEC2(Time, min, 60, 1);
UNIT_COMP1(Time, hrs, min, 60, 1);
UNIT_COMP1(Time, days, hrs, 24, 1);


// Volume Ratios.  Nominal unit is liter (l)
UNIT_SPEC1(Volume, ml, std::milli);
UNIT_SPEC1(Volume, cubicm, std::kilo);
UNIT_DECLR(Volume, cubicin) { typedef typename std::ratio_multiply<std::kilo, std::ratio_multiply<typename UnitRatio<Area, Area::sqin>::type, typename UnitRatio<Distance, Distance::in>::type>::type>::type type; };
UNIT_COMP1(Volume, cubicft, cubicin, 1728, 1);
UNIT_COMP1(Volume, cubicyd, cubicft, 27, 1);
UNIT_COMP1(Volume, tbsp, ml, 1478676478125, 100000000000);
UNIT_COMP1(Volume, tsp, tbsp, 1, 3);
UNIT_COMP1(Volume, floz, tbsp, 2, 1);
UNIT_COMP1(Volume, cup, floz, 8, 1);
UNIT_COMP1(Volume, pt, cup, 2, 1);
UNIT_COMP1(Volume, qt, pt, 2, 1);
UNIT_COMP1(Volume, gal, qt, 4, 1);



} /* End namespace units */
} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TYPES_UNITS_UNITS_HPP_ */
