/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Int.hpp
 *
 *  Created on: Jun 19, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_INT_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_INT_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <cinttypes>
#include <limits>
#include <string>
#include <type_traits>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief Template structure for determining the minimum size of the type to use based on the number of bits
 */
template <std::uint8_t _Sz>
struct _int_container
{ typedef typename _int_container<_Sz - 1>::type type; };

// Template specializations
template <> struct _int_container<0>
{ typedef int8_t  type; };
template <> struct _int_container<tmx::common::bit_size<int8_t>::value + 1>
{ typedef int16_t type; };
template <> struct _int_container<tmx::common::bit_size<int16_t>::value + 1>
{ typedef int32_t type; };
template <> struct _int_container<tmx::common::bit_size<int32_t>::value + 1>
{ typedef int64_t type; };
#ifdef TMX_INT128
template <> struct _int_container<tmx::common::bit_size<int64_t>::value + 1>
{ typedef TMX_INT128 type; };

#ifndef TMX_MAX_INT
#define TMX_MAX_INT TMX_INT128
#endif
#else
#ifndef TMX_MAX_INT
#define TMX_MAX_INT int64_t
#endif
#endif

typedef typename _int_container<0>::type _intmin_t;
typedef typename _int_container<0xFF>::type _intmax_t;
typedef tmx::common::smaller_bit_size<std::int32_t, _intmax_t> _int_t;

typedef typename std::make_unsigned<_intmin_t>::type _uintmin_t;
typedef typename std::make_unsigned<_intmax_t>::type _uintmax_t;
typedef typename std::make_unsigned<_int_t>::type _uint_t;


/*!
 * @brief Template structure for Determining the bit mask for an integer
 */
template <std::uint8_t _Sz>
struct _int_mask
{ static constexpr auto value = _int_mask<_Sz + 1>::value >> 1; };

// Template specializations
template <> struct _int_mask<0>
{ static constexpr std::size_t value = 0x0000000000000000; };
template <> struct _int_mask<tmx::common::bit_size<int8_t>::value>
{ static constexpr std::size_t value = 0x00000000000000FF; };
template <> struct _int_mask<tmx::common::bit_size<int16_t>::value>
{ static constexpr std::size_t value = 0x000000000000FFFF; };
template <> struct _int_mask<tmx::common::bit_size<int32_t>::value>
{ static constexpr std::size_t value = 0x00000000FFFFFFFF; };
template <> struct _int_mask<tmx::common::bit_size<int64_t>::value>
{ static constexpr std::size_t value = 0xFFFFFFFFFFFFFFFF; };
#ifdef TMX_INT128
template <> struct _int_mask<tmx::common::bit_size<TMX_INT128>::value> {
	static constexpr typename std::make_unsigned<TMX_INT128>::type value = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;
};
#endif

/*!
 * @brief Base class for both integral types so they both have the same interface
 *
 * The integer value is stored inside the next-biggest size C++ native
 * integer type, but in two's complement form for signed values. This is for
 * convenience in converting to byte form. However, this class always returns
 * the correct native number (as the native type) when used in C++ expressions.
 */
template <std::uint8_t _Sz, typename _Tp>
class IntegralBase: public TmxDataType<_Tp> {
	static_assert(_Sz > 0,
			"The specified number of bits cannot be 0");
	static_assert(_Sz <= tmx::common::bit_size<TMX_MAX_INT>::value,
			"The specified number of bits is beyond a valid integer size");
	static_assert(tmx::common::is_integer<_Tp>::value,
			"Container must contain an integral type");

	/*!
	 * @brief The base class
	 */
	typedef TmxDataType<_Tp> super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	IntegralBase() noexcept: super() { }
	IntegralBase(value_type &t) noexcept: super(t) { };
	IntegralBase(value_type &&t) noexcept: super(std::forward<value_type>(t)) { }

	/*!
	 * @brief A reflective type
	 */
	typedef IntegralBase<_Sz, value_type> self_type;

	/*!
	 * @brief Whether the type is a signed integer or not
	 */
	static constexpr bool is_signed = std::is_signed<value_type>::value;

	/*!
	 * @brief The bit mask needed for an integer of this size
	 */
	static constexpr auto mask = _int_mask<_Sz>::value;

	/*!
	 * @brief The number of bytes needed to store this integer type
	 */
	static constexpr auto numBytes =
			tmx::common::byte_size<value_type>::value;

	/*!
	 * @brief Only need _Sz number of bits to store the integer
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
	 * @return A copy of the contained value
	 */
	operator value_type () const noexcept override {
		value_type _value = super::operator value_type () & mask;
		if (is_signed && (_value >> (numBits - 1)))
			return -1 * ((_value ^ mask) + 1);
		else
			return _value;
	}

	/*!
	 * @return A static instance of the numeric limits for this integer type
	 */
	static auto &limits() noexcept {
		static std::numeric_limits<_Tp> _singleton;
		return _singleton;
	}
};

/*!
 * @brief A class that holds an integer type up to the specified number of bits
 *
 * @param _Sz The number of bits that the integer storage contains
 */
template <std::uint8_t _Sz>
class Int: public IntegralBase<_Sz, typename _int_container<_Sz>::type> {
	/*!
	 * @brief The base class
	 */
	typedef IntegralBase<_Sz, typename _int_container<_Sz>::type> super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A reflective type
	 */
	typedef Int<_Sz> self_type;

	Int() noexcept: super() { }
	Int(value_type &&t) noexcept: super(std::forward<value_type>(t)) { }
	Int(value_type &t) noexcept: super(t) { };
	Int(value_type const &t) noexcept: Int(value_type(t)) { }

	using TmxDataType<value_type>::operator =;

	using super::limits;
};

template <typename _Tp>
using _unsigned = typename std::make_unsigned<_Tp>::type;

/*!
 * @brief A class that holds an unsigned integer type up to the specified number of bits
 *
 * @param _Sz The number of bits that the integer storage contains
 */
template <std::uint8_t _Sz>
class UInt: public IntegralBase<_Sz, _unsigned<typename _int_container<_Sz>::type> > {
	/*!
	 * @brief The base class
	 */
	typedef IntegralBase<_Sz, _unsigned<typename _int_container<_Sz>::type> > super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief A reflective type
	 */
	typedef UInt<_Sz> self_type;

	UInt() noexcept: super() { }
	UInt(value_type &t) noexcept: super(t) { };
	UInt(value_type &&t) noexcept: super(std::forward<value_type>(t)) { }
	UInt(value_type const &t) noexcept: UInt(value_type(t)) { }

	using TmxDataType<value_type>::operator =;

	using super::limits;
};

/*!
 * @brief Convert the integer type to signed
 */
template <class _C>
using make_signed_int = Int< _C::numBits >;

/*!
 * @param[in] v The unsigned integer to convert
 * @return The signed integer equivalent of v
 */
template <std::uint8_t _Sz>
make_signed_int< UInt<_Sz> > make_signed(UInt<_Sz> const &v) {
    typedef UInt<_Sz> type;
    typedef make_signed_int<type> stype;

    return { static_cast<typename stype::value_type>((typename type::value_type) v) };
}

/*!
 * @brief Convert the integer type to unsigned
 */
template <class _C>
using make_unsigned_int = UInt< _C::numBits >;

/*!
 * @param[in] v The signed integer to convert
 * @return The unsigned integer equivalent of v
 */
template <std::uint8_t _Sz>
make_unsigned_int< Int<_Sz> > make_unsigned(Int<_Sz> const &v) {
    typedef Int<_Sz> type;
    typedef make_unsigned_int<type> utype;

    return { static_cast<typename utype::value_type>((typename type::value_type) v) };
}

template <typename _Tp = _int_t>
using int_type = Int< tmx::common::bit_size<_Tp>::value >;

/*!
 * @brief The smallest allowed TMX signed integer type
 */
typedef int_type<_intmin_t> Intmin;

/*!
 * @brief The largest allowed TMX signed integer type
 */
typedef int_type<TMX_MAX_INT> Intmax;

/*!
 * @brief The default TMX signed integer type
 *
 * By convention, this is set to 32-bits, unless the maximum
 * integer allowed is actually less than that, in which case
 * the maximum integer type is used.
 */
typedef int_type<> Int_;

template <typename _Tp = _uint_t>
using uint_type = UInt< tmx::common::bit_size<_Tp>::value >;

/*!
 * @brief The smallest allowed TMX unsigned integer type
 */
typedef uint_type<_uintmin_t> UIntmin;

/*!
 * @brief The largest allowed TMX unsigned integer type
 */
typedef uint_type<_uintmax_t> UIntmax;

/*!
 * @brief The default TMX unsigned integer type
 *
 * By convention, this is set to 32-bits, unless the maximum
 * integer allowed is actually less than that, in which case
 * the maximum integer type is used.
 */
typedef uint_type<> UInt_;

template <std::uint8_t _Sz, class _C = Int<_Sz> >
using _safe_int =
		typename std::conditional<tmx::common::bit_size_difference< _C, Intmax>::num <= 0, _C, void>::type;

// Signed integer types
typedef _safe_int<1>  Int1;
typedef _safe_int<2>  Int2;
typedef _safe_int<3>  Int3;
typedef _safe_int<4>  Int4;
typedef _safe_int<5>  Int5;
typedef _safe_int<6>  Int6;
typedef _safe_int<7>  Int7;
typedef _safe_int<8>  Int8;
typedef _safe_int<9>  Int9;
typedef _safe_int<10> Int10;
typedef _safe_int<11> Int11;
typedef _safe_int<12> Int12;
typedef _safe_int<13> Int13;
typedef _safe_int<14> Int14;
typedef _safe_int<15> Int15;
typedef _safe_int<16> Int16;
typedef _safe_int<17> Int17;
typedef _safe_int<18> Int18;
typedef _safe_int<19> Int19;
typedef _safe_int<20> Int20;
typedef _safe_int<21> Int21;
typedef _safe_int<22> Int22;
typedef _safe_int<23> Int23;
typedef _safe_int<24> Int24;
typedef _safe_int<25> Int25;
typedef _safe_int<26> Int26;
typedef _safe_int<27> Int27;
typedef _safe_int<28> Int28;
typedef _safe_int<29> Int29;
typedef _safe_int<30> Int30;
typedef _safe_int<31> Int31;
typedef _safe_int<32> Int32;
typedef _safe_int<33> Int33;
typedef _safe_int<34> Int34;
typedef _safe_int<35> Int35;
typedef _safe_int<36> Int36;
typedef _safe_int<37> Int37;
typedef _safe_int<38> Int38;
typedef _safe_int<39> Int39;
typedef _safe_int<40> Int40;
typedef _safe_int<41> Int41;
typedef _safe_int<42> Int42;
typedef _safe_int<43> Int43;
typedef _safe_int<44> Int44;
typedef _safe_int<45> Int45;
typedef _safe_int<46> Int46;
typedef _safe_int<47> Int47;
typedef _safe_int<48> Int48;
typedef _safe_int<49> Int49;
typedef _safe_int<50> Int50;
typedef _safe_int<51> Int51;
typedef _safe_int<52> Int52;
typedef _safe_int<53> Int53;
typedef _safe_int<54> Int54;
typedef _safe_int<55> Int55;
typedef _safe_int<56> Int56;
typedef _safe_int<57> Int57;
typedef _safe_int<58> Int58;
typedef _safe_int<59> Int59;
typedef _safe_int<60> Int60;
typedef _safe_int<61> Int61;
typedef _safe_int<62> Int62;
typedef _safe_int<63> Int63;
typedef _safe_int<64> Int64;

template <std::uint8_t _Sz>
using _safe_uint = _safe_int<_Sz, UInt<_Sz> >;

// Unsigned Integer types
typedef _safe_uint<1>  UInt1;
typedef _safe_uint<2>  UInt2;
typedef _safe_uint<3>  UInt3;
typedef _safe_uint<4>  UInt4;
typedef _safe_uint<5>  UInt5;
typedef _safe_uint<6>  UInt6;
typedef _safe_uint<7>  UInt7;
typedef _safe_uint<8>  UInt8;
typedef _safe_uint<9>  UInt9;
typedef _safe_uint<10> UInt10;
typedef _safe_uint<11> UInt11;
typedef _safe_uint<12> UInt12;
typedef _safe_uint<13> UInt13;
typedef _safe_uint<14> UInt14;
typedef _safe_uint<15> UInt15;
typedef _safe_uint<16> UInt16;
typedef _safe_uint<17> UInt17;
typedef _safe_uint<18> UInt18;
typedef _safe_uint<19> UInt19;
typedef _safe_uint<20> UInt20;
typedef _safe_uint<21> UInt21;
typedef _safe_uint<22> UInt22;
typedef _safe_uint<23> UInt23;
typedef _safe_uint<24> UInt24;
typedef _safe_uint<25> UInt25;
typedef _safe_uint<26> UInt26;
typedef _safe_uint<27> UInt27;
typedef _safe_uint<28> UInt28;
typedef _safe_uint<29> UInt29;
typedef _safe_uint<30> UInt30;
typedef _safe_uint<31> UInt31;
typedef _safe_uint<32> UInt32;
typedef _safe_uint<33> UInt33;
typedef _safe_uint<34> UInt34;
typedef _safe_uint<35> UInt35;
typedef _safe_uint<36> UInt36;
typedef _safe_uint<37> UInt37;
typedef _safe_uint<38> UInt38;
typedef _safe_uint<39> UInt39;
typedef _safe_uint<40> UInt40;
typedef _safe_uint<41> UInt41;
typedef _safe_uint<42> UInt42;
typedef _safe_uint<43> UInt43;
typedef _safe_uint<44> UInt44;
typedef _safe_uint<45> UInt45;
typedef _safe_uint<46> UInt46;
typedef _safe_uint<47> UInt47;
typedef _safe_uint<48> UInt48;
typedef _safe_uint<49> UInt49;
typedef _safe_uint<50> UInt50;
typedef _safe_uint<51> UInt51;
typedef _safe_uint<52> UInt52;
typedef _safe_uint<53> UInt53;
typedef _safe_uint<54> UInt54;
typedef _safe_uint<55> UInt55;
typedef _safe_uint<56> UInt56;
typedef _safe_uint<57> UInt57;
typedef _safe_uint<58> UInt58;
typedef _safe_uint<59> UInt59;
typedef _safe_uint<60> UInt60;
typedef _safe_uint<61> UInt61;
typedef _safe_uint<62> UInt62;
typedef _safe_uint<63> UInt63;
typedef _safe_uint<64> UInt64;

// Some helper tools for packing multiple ints together
/*!
 * @brief Determines if the type is a TMX scalar type
 *
 * This will guarantee that the number of bits is specified in the class
 */
template <typename _Tp>
using _scalar_check = std::integral_constant<bool,
        common::types::IsTmxType<_Tp>::value && common::is_scalar<common::types::TmxValueTypeOf<_Tp> >::value>;

/*!
 * @brief Retrieve the number of bits used for the type
 *
 * All TMX scalar types, including the integers, will return the appropriate
 * size. Other types should return the 8 * size(_Tp)
 */
template <typename _Tp>
using _bit_count = typename std::conditional<_scalar_check<_Tp>::value,
        std::integral_constant<std::size_t, common::types::TmxTypeOf<_Tp>::numBits>,
        std::integral_constant<std::size_t, common::bit_size<_Tp>::value> >::type;

/*!
 * @brief A structure for calculating the number of bits needed
 *
 * @tparam _Empty The variadic template of types
 */
template <typename ... _Empty>
struct bit_count: public std::integral_constant<std::size_t, 0> { };

/*!
 * @brief A structure for calculating the number of bits needed
 *
 * @tparam _Tp The first type in the sequence
 * @tparam _Others The other types
 */
template <typename _Tp, typename ... _Others>
struct bit_count<_Tp, _Others...>: public std::integral_constant<std::size_t,
        _bit_count<_Tp>::value + bit_count<_Others...>::value> { };

/*!
 * @brief Pack the given TMX integer type into the integer value
 *
 * @param value The value to pack
 * @param word The value to add
 * @return The newly packed value
 */
template <typename _T, std::uint8_t _Sz, typename _Tp>
enable_int<_T> do_pack(_T value, common::types::IntegralBase<_Sz, _Tp> const &word) {
    if (value > 0) value <<= _Sz;
    return value | (_Tp)word;
}

/*!
 * @brief Pack the given TMX integer types into the integer value
 *
 * @param value The value to pack
 * @param word The next value to add
 * @param others The remaining values to add
 * @return The newly packed value
 */
template <typename _T, std::uint8_t _Sz, typename _Tp, typename ... _Others>
enable_int<_T> do_pack(_T value, common::types::IntegralBase<_Sz, _Tp> const &word, _Others const &... others) {
    value = do_pack<_T>(value, word);
    return do_pack<_T>(value, others...);
}

/*!
 * @brief Pack the given TMX integer types into a new integer value
 *
 * This automatically calculates the size of the integer to use by
 * using the total number of bits needed with each type.
 *
 * @param args The values to pack
 * @return The newly packed value
 */
template <typename ... _Args>
auto pack(_Args const &... args) {
    typedef common::types::UInt<bit_count<_Args...>::value> type;
    const auto value = do_pack<typename type::value_type>((typename type::value_type)0, args...);
    return type(value);
}

/*!
 * @brief Unpack the given value to the supplied TMX integer types
 *
 * @param value The value to unpack
 * @param word The word to write to
 * @return The remaining value after unpacking
 */
template <typename _T, std::uint8_t _Sz, typename _Tp>
enable_int<_T> do_unpack(_T value, common::types::IntegralBase<_Sz, _Tp> &word) {
    typedef common::types::IntegralBase<_Sz, _Tp> type;
    word = static_cast<typename type::value_type>(value & type::mask);
    return value >> type::numBits;
}

/*!
 * @brief Unpack the given value to the supplied TMX integer types
 *
 * @param value The value to unpack
 * @param others The remaining words to write to
 * @param word The word to write to
 * @return The remaining value after unpacking
 */
template <typename _T, std::uint8_t _Sz, typename _Tp, typename... _Others>
enable_int<_T> do_unpack(_T value, common::types::IntegralBase<_Sz, _Tp> &word, _Others &... others) {
    // Unpack in reverse order
    value = do_unpack(value, others...);
    return do_unpack(value, word);
}

/*!
 * @brief Unpack the given value to the supplied TMX integer types
 *
 * @param value The value to unpack
 * @param args The words to write to
 * @return The remaining value after unpacking
 */
template <typename ... _Args>
auto unpack(typename common::types::UInt<bit_count<_Args...>::value>::value_type value, _Args &... args) {
    return do_unpack(value, args...);
}

template <std::uint8_t _Sz>
struct TmxType<Int<_Sz>, void> {
	typedef std::true_type TMX;
	typedef Int<_Sz> type;
    static constexpr auto name = type_short_name<type>();
};

template <std::uint8_t _Sz>
struct TmxType<UInt<_Sz>, void> {
	typedef std::true_type TMX;
	typedef UInt<_Sz> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _Tp>
struct TmxType<_Tp, tmx::common::enable_signed_int<_Tp, void> > {
	typedef std::true_type TMX;
	typedef int_type<_Tp> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _Tp>
struct TmxType<_Tp, tmx::common::enable_unsigned_int<_Tp, void> > {
	typedef std::true_type TMX;
	typedef uint_type<_Tp> type;
    static constexpr auto name = type_short_name<type>();
};

template <typename _T, bool>
struct _int_types;

template <std::size_t ... _I>
struct _int_types< static_array<std::size_t, _I...>, false > {
    typedef std::tuple< UInt<_I>... > type;
};

template <std::size_t ... _I>
struct _int_types< static_array<std::size_t, _I...>, true > {
    typedef std::tuple< Int<_I>... > type;
};

template <std::size_t _I>
struct _supported_seq;

template <>
struct _supported_seq<128> {
    typedef static_array<std::size_t, 8, 16, 32, 64, 128> type;
};

template <>
struct _supported_seq<64> {
    typedef static_array<std::size_t, 8, 16, 32, 64> type;
};

template <>
struct _supported_seq<32> {
    typedef static_array<std::size_t, 8, 16, 32> type;
};

template <>
struct _supported_seq<16> {
    typedef static_array<std::size_t, 8, 16> type;
};

template <>
struct _supported_seq<8> {
    typedef static_array<std::size_t, 8> type;
};

using TmxSupportedUIntTypes = typename _int_types<typename _supported_seq<UIntmax::numBits>::type, false>::type;
using TmxSupportedIntTypes = typename _int_types<typename _supported_seq<Intmax::numBits>::type, true>::type;
using TmxUIntTypes = typename _int_types<decltype(make_array(make_index_sequence<UIntmax::numBits, 1>())), false>::type;
using TmxIntTypes = typename _int_types<decltype(make_array(make_index_sequence<Intmax::numBits, 1>())), true>::type;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_INT_HPP_ */
