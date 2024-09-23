/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Enum.hpp
 *
 *  Created on: Jun 7, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TYPES_ENUM_HPP_
#define INCLUDE_TMX_COMMON_TYPES_ENUM_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/types/TmxDataType.hpp>
#include <tmx/common/types/Array.hpp>
#include <tmx/common/types/Int.hpp>
#include <tmx/common/types/String.hpp>

#include <type_traits>
#include <utility>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief Default helper structure for integer Enum types
 */
template <typename _Tp, bool>
struct TmxEnumType_t {
	typedef TmxValueTypeOf<_Tp> type;
};

/*!
 * @brief Helper structure for real C++ enumeration types
 */
template <typename _E>
struct TmxEnumType_t<_E, true> {
	typedef _E type;
};

/*!
 * @brief An alias to determine how to store the enumeration
 *
 * If _E is really a C++ enumeration type, then we store the
 * underlying value type. Otherwise, we store as the appropriate
 * integer TMX data type.
 */
template <typename _E>
using TmxEnumType = typename TmxEnumType_t<_E, tmx::common::is_enum<_E>::value>::type;

/*!
 * @brief The default enumeration underlying type boxer
 */
template <typename _E, bool>
struct TmxEnumIntegerType_t {
	typedef TmxEnumType<_E> type;
};

/*!
 * @brief A C++ enumeration underlying type boxer
 */
template <typename _E>
struct TmxEnumIntegerType_t<_E, true> {
	typedef typename std::underlying_type< TmxEnumType<_E> >::type type;
};

/*!
 * @brief An alias for the underlying integer type of the enumeration
 */
template <typename _E>
using TmxEnumIntegerType =
		typename TmxEnumIntegerType_t<_E, tmx::common::is_enum<_E>::value>::type;

/*!
 * @brief An alias to check if the type is a C++ enumeration
 */
template <typename _E>
using IsTmxEnumType = tmx::common::is_enum< TmxEnumType<_E> >;

/*!
 * @brief An alias to check if the type is a C++ enumeration with names
 */
template <typename _E>
using IsValidTmxEnumType = tmx::common::enums::is_enum_valid< TmxEnumType<_E> >;

/*!
 * @brief An alias to a std::enable exclusively if _E is a C++ enumeration
 */
template <typename _E, typename _Tp = void>
using TmxEnumEnable = typename std::enable_if<IsTmxEnumType<_E>::value, _Tp>::type;

/*!
 * @brief An alias to a std::enable exclusively if _E is NOT a C++ enumeration
 */
template <typename _E, typename _Tp = void>
using TmxEnumDisable = typename std::enable_if<!IsTmxEnumType<_E>::value, _Tp>::type;

/*!
 * @brief An alias to a std::enable exclusively if _E is a C++ enumeration with names
 */
template <typename _E, typename _Tp = void>
using ValidTmxEnumEnable = typename std::enable_if<IsValidTmxEnumType<_E>::value, _Tp>::type;

/*!
 * @brief An alias to a std::enable exclusively if _E is NOT a C++ enumeration with names
 */
template <typename _E, typename _Tp = void>
using ValidTmxEnumDisable = typename std::enable_if<!IsValidTmxEnumType<_E>::value, _Tp>::type;

/*!
 * @brief A container for holding enumerated values
 *
 * Contrary to intuition, this class is actually <b>not</b> a direct
 * wrapper of a C++ enum type. This is because an enum is effectively
 * an integer value with a more convenient name, yet because it is a
 * specific C++ type, that would mean both ends of the communication
 * would need to support that exact type, which can never be
 * guaranteed. Therefore, to be more flexible, this class can be used
 * just like an integer container in cases for which the name of each
 * enumeration value is unknown.
 *
 * The value contained in this Enum is dependent on the underlying value
 * type of the C++ enumeration, which must be some kind of integer. When
 * an enum is declared, one can give the specific integer storage class
 * of that type. For example:
 *
 * enum class OnOff: unsigned char {
 * 		Off = 0,
 * 		On = 1,
 * 		Unknown = 0xFF
 * };
 *
 * The OnOff type is effectively an unsigned 8-bit integer (char), which is
 * a TMX Int16. Therefore, the Enum<OnOff> class is just a specialized
 * version of Enum<unsigned char> or Enum<UInt8> that restricts the values
 * allowed to those defined in OnOff (0, 1, 255). Additionally, however,
 * the values of Enum<OnOff> may be assigned by name ("Off", "On", "Unknown").
 * However, this is only possible if the OnOff names can be determined
 * through some programming technique, including code generation. On
 * the other hand, an Enum<UInt8> value can be any unsigned 8-bit integer
 * with any name the programmer chooses. Whereas this seems like a logic
 * error waiting to happen, it is definitely useful because it allows
 * Enum types to be created and used at run-time based on some known
 * sequence of name-value pairs. In other words, the receiving end does
 * NOT have to implement a native C++ enum in order to be able to process
 * the type as long as it can determine what the sequence is. One example
 * would be to pass the sequence within the message or as a separate
 * metadata message.
 *
 * There are still convenience methods in this class to support using it
 * as a wrapper for existing C++ enum types. However, this requires the
 * ability to sequence the enum and convert individual values to their
 * equivalent name. This can automatically be done at compile-time by using
 * the supplied Magic Enum library. However, because of the tricks used in
 * that library, only certain compiler versions are supported. If the
 * compiler does not support Magic Enum, the sequence must be created by
 * hand, which can be done with the help of the TMX_ENUM() macro.
 *
 * Encoding an Enum type will depend on a few factors. First, if the name can
 * not automatically be determined, then the Enum must be encoded as its
 * integer value. However, if a name is actually known, then the Enum should
 * be encoded by name, if possible. This, of course, assumes that both ends
 * of the communication understand the naming of the values. So, it is
 * probably more useful in many cases to force a by-value encoding so that
 * the receiving end may know the Enum by whatever name is convenient.
 * Returning to the OnOff example, encoding the type as "Off" requires that
 * the receiver knows that "Off" means 0. However, if it is instead
 * encoded as numeric 0, then the receiving end can use the name "Off" or
 * "No" or "False", etc., depending on the situation. Additionally, the
 * value restriction of the OnOff type can also be removed at the receiving
 * end because an encoding 0 can be interpreted as "Off", 255 as "Unknown",
 * but everything else as "On", similar to the way integer to Boolean
 * conversion is done in C++.
 *
 * @see #TMX_ENUM()
 * @see https://github.com/Neargye/magic_enum/blob/master/doc/limitations.md
 */
template <typename _E>
class Enum: public TmxDataType< TmxEnumType<_E> > {
	/*!
	 * @brief The base class
	 */
	typedef TmxDataType< TmxEnumType<_E> > super;

public:
	/*!
	 * @brief The value type
	 */
	typedef typename super::value_type value_type;

	/*!
	 * @brief The underlying integer type of the value
	 */
	typedef TmxEnumIntegerType<_E> underlying_type;

	/*!
	 * @brief The underlying string type to use for names
	 */
	typedef typename String_::value_type string;

	/*!
	 * @brief A reflective type
	 */
	typedef Enum<value_type> self_type;

	static_assert(tmx::common::is_integer<underlying_type>::value,
			"Enums can only contain an integral type");
	static_assert(TmxTypeOf<underlying_type>::numBits > 0,
			"The specified number of bits cannot be 0");
	static_assert(TmxTypeOf<underlying_type>::numBits <= tmx::common::bit_size<TMX_MAX_INT>::value,
			"The specified number of bits is beyond a valid integer size");
	/*!
	 * @brief The default constructor produces an Enum with the equivalent of 0
	 */
	Enum(): super() {
		this->setEnumName(empty_string());
	}

	/*!
	 * @brief Construct an Enum from the given value and name
	 *
	 * If name is empty, the default name of the value will be used.
	 * This is obtained automatically from _E if the names can be
	 * automatically determined, or else it will just be a stringified
	 * version of the value, implying that the only encoding
	 * possible for this Enum is by value.
	 *
	 * @param[in] e The enumeration value
	 * @param[in] name The enumeration name, defaults to empty
	 */
	template <typename _Tp>
	Enum(_Tp &&value, const_string name = empty_string()):
			super(static_cast<value_type>(value)) {
		this->setEnumName(name);
	}

	/*!
	 * @brief Construct an Enum from the given reference and name
	 *
	 * Note that this could use any integer reference.
	 *
	 * If name is empty, the default name of the value will be used.
	 * This is obtained automatically from _E if the names can be
	 * automatically determined, or else it will just be a stringified
	 * version of the value, implying that the only encoding
	 * possible for this Enum is by value.
	 *
	 * @param[in] e The enumeration value
	 * @param[in] name The enumeration name, defaults to empty
	 */
	Enum(value_type &e, const_string name = empty_string()): super(e) {
		this->setEnumName(name);
	}

	/*!
	 * @brief Construct an Enum from the given reference and name
	 *
	 * Note that this copies the enumeration value
	 *
	 * If name is empty, the default name of the value will be used.
	 * This is obtained automatically from _E if the names can be
	 * automatically determined, or else it will just be a stringified
	 * version of the value, implying that the only encoding
	 * possible for this Enum is by value.
	 *
	 * @param[in] e The enumeration value
	 * @param[in] name The enumeration name, defaults to empty
	 */
	template <typename _Tp>
	Enum(_Tp const &e, const_string name = empty_string()):
			super(static_cast<value_type>(e)) {
		this->setEnumName(name);
	}

	template <typename _Tp>
	Enum(std::pair<_Tp, const_string> const &pair):
			Enum(static_cast<value_type>(pair.first), pair.second) {
	}

	/*!
	 * @brief Copy constructor from another Enum type
	 *
	 * Note that this will copy the value and the name
	 *
	 * @param[in] other The other Enum to copy from
	 */
	Enum(self_type const &other): Enum(other.get_enum_value(), other._name) { }

	/*!
	 * @brief Assign this Enum contents from a copy
	 *
	 * The object will reference the same value as the copy,
	 * but will also lose any reference to the previous
	 * value it had.
	 *
	 * @param copy The copy to use
	 */
	template <typename _Tp>
	typename std::enable_if<std::is_same<_Tp, _E>::value, self_type &>::type
	operator=(Enum<_Tp> const &copy) {
		super::operator=(copy);
		this->_name = copy._name;
		return *this;
	}

	/*!
	 * @brief Assign this Enum contents from a copy of a different Enum
	 *
	 * Note that the contents in this case are copied.
	 *
	 * @param copy The copy to use
	 */
	template <typename _Tp>
	typename std::enable_if<!std::is_same<_Tp, _E>::value, self_type &>::type
	operator=(Enum<_Tp> const &copy) {
		super::operator=(value_type(static_cast<value_type>(*copy)));
		this->setEnumName(string(*(copy._name)));
		return *this;
	}

	/*!
	 * @brief Assign this Enum contents from the given C++ enumeration
	 *
	 * Note that this will copy the enumeration value and set the
	 * default name.
	 *
	 * @see #operator=(Enum<_E> const &)
	 * @param[in] value The enumeration value
	 */
	template <typename _Tp>
	self_type & operator=(_Tp const &value) {
		return this->operator=(Enum<_Tp>(value));
	}

	/*!
	 * @brief Assign this Enum contents from the given value
	 *
	 * Note that this only copies the enumeration value and
	 * sets the default name. If you want to copy the name
	 * as well, then you need to assign a full Enum type.
	 *
	 * @see #operator=(Enum<_E> const &)
	 * @param[in] value The enumeration value
	 */
	template <typename _Tp>
	self_type &operator=(_Tp &&value) {
		_Tp const &_tmp = value;
		return this->operator=(_tmp);
	}

	/*!
	 * @return The enumeration name assigned to this value
	 */
	string get_enum_name() const noexcept {
		return { this->_name };
	}

	/*!
	 * @return The enumeration value
	 */
	value_type get_enum_value() const noexcept {
		return (value_type)(*this);
	}

	/*!
	 * @return The integer value
	 */
	underlying_type get_integer_value() const noexcept {
		return static_cast<underlying_type>(this->get_enum_value());
	}

private:
	string _name;

	/*!
	 * @brief Get the default name of the enumeration value
	 *
	 * This version obtains the name automatically from a C++
	 * enumeration type with names
	 *
	 * @return The default name of the enumeration value
	 */
	template <typename _Tp>
	static ValidTmxEnumEnable<_Tp, string> getDefaultEnumName(_Tp value) {
		return string(tmx::common::enums::enum_name<_Tp>(value));
	}

	/*!
	 * @brief Get the default name of the enumeration value
	 *
	 * This version cannot automatically determine the name, thus
	 * directly converts the value to a string.
	 *
	 * @return The default name of the enumeration value
	 */
	template <typename _Tp>
	static ValidTmxEnumDisable<_Tp, string> getDefaultEnumName(_Tp value) {
		return std::to_string(static_cast<value_type>(value));
	}

	/*!
	 * @brief Sets the name of this value
	 *
	 * If the name is empty, then it uses the default version.
	 * Otherwise, it accepts a trimmed version of the name.
	 */
	void setEnumName(const_string const name) {
		this->_name.clear();
		this->_name.assign(TmxTypeOf<string>(std::string(name.data())).trim());
		if (this->_name.empty())
			this->_name = getDefaultEnumName((value_type)(*this));
	}
};

typedef Enum<Int<8> >  Enum8;
typedef Enum<Int<16> > Enum16;
typedef Enum<Int<32> > Enum32;
typedef Enum<Int<64> > Enum64;
typedef Enum<UInt<8> >  UEnum8;
typedef Enum<UInt<16> > UEnum16;
typedef Enum<UInt<32> > UEnum32;
typedef Enum<UInt<64> > UEnum64;
typedef Enum<Intmin> Enummin;
typedef Enum<Intmax> Enummax;
typedef Enum<Int_> Enum_;
typedef Enum<UIntmin> UEnummin;
typedef Enum<UIntmax> UEnummax;
typedef Enum<UInt_> UEnum_;

/*!
 * @brief An alias name for the Enum type
 */
template <typename _E>
using enum_type = Enum<_E>;

template <typename _Tp = std::uint16_t>
auto makeEnumSequence(std::initializer_list< std::pair<_Tp, const_string> > iList) {
	typename Array< Enum<_Tp> >::value_type _seq;

	for (const auto &val: iList) {
		_seq.emplace_back(val.first, val.second);
	}

	return _seq;
}

/*!
 * @brief Automatically construct an Array of Enum types
 *
 * This function only is available if _E is an actual
 * C++ enumeration. If no automatic sequence is available
 * for _E, however, there will be a compiler error.
 *
 * @see #TMX_ENUM_SEQUENCE()
 * @return An array of the Enum name-value types for _E
 */
template <typename _E, class = TmxEnumEnable<_E> >
auto const &makeEnumSequence() {
	// These never change, so make it a static reference
	static typename Array< Enum<_E> >::value_type _seq;

	if (_seq.empty()) {
		for (const auto &e: tmx::common::enums::enum_entries<_E>()) {
			_seq.emplace_back(e.first, e.second);
		}
	}

	return _seq;
}

template <typename _E, bool = false>
struct _enum_name {
    static constexpr auto name = type_short_name< Array<_E> >();
};

template <typename _E>
struct _enum_name<_E, true> {
private:
    static constexpr auto _name_subpart = introspect_short_name< TmxTypeOf<_E> >();
    static constexpr auto _name_front = common::concat(TMX_STATIC_STRING("Enum<"), _name_subpart);
    static constexpr auto _name = common::concat(_name_front,
                                                 typename std::conditional<_name_front.array.back() == '>',
                                                         static_array<char, ' ', '>'>, static_array<char, '>'> >::type {});

public:
    static constexpr const_string name { _name.c_str(), _name.size };
};

template <typename _E>
using enum_name = _enum_name<_E, IsTmxType<_E>::value>;

template <typename _E>
struct TmxType<Enum<_E>, void> {
	typedef std::true_type TMX;
	typedef Enum<_E> type;
    static constexpr auto name = enum_name<_E>::name;
};

template <typename _E>
struct TmxType<_E, tmx::common::enable_enum<_E, void> > {
	typedef std::true_type TMX;
	typedef Enum<_E> type;
    static constexpr auto name = enum_name<_E>::name;
};

template <typename>
struct _enum_types {
};

template <typename... _Tp>
struct _enum_types< std::tuple<_Tp...> > {
    typedef TmxTypes< Enum<_Tp>... > types;
};

using TmxEnumTypes = tmx::common::tuple_cat< typename _enum_types<TmxSupportedUIntTypes>::types,
        typename _enum_types<TmxSupportedIntTypes>::types >;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* INCLUDE_TMX_COMMON_TYPES_ENUM_HPP_ */
