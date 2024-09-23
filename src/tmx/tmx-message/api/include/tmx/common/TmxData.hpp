/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxData.hpp
 *
 *  Created on: Jul 20, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TMXDATA_HPP_
#define API_INCLUDE_TMX_COMMON_TMXDATA_HPP_

#include <tmx/common/types/array_type.hpp>
#include <tmx/common/types/boolean_type.hpp>
#include <tmx/common/types/bytes_type.hpp>
#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/float_type.hpp>
#include <tmx/common/types/int_type.hpp>
#include <tmx/common/types/map_type.hpp>
#include <tmx/common/types/null_type.hpp>
#include <tmx/common/types/record_type.hpp>
#include <tmx/common/types/string_type.hpp>

#include <tmx/common/types/support/introspect.hpp>
#include <tmx/common/types/support/sizeof.hpp>

#include <any>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <typeindex>
#include <typeinfo>

namespace tmx {
namespace common {

class TmxTypeVariant;

/*!
 * @brief A container class that holds any of the standard TMX data types
 */
class TmxData {
	friend class TmxTypeVisitor;

public:
	/*!
	 * This is an any container
	 */
	typedef std::any type;

	/*!
	 * This represents the "any" type
	 */
	static constexpr auto name = types::support::type_short_name<type>().data();

	// Constructors

	/*!
	 * @brief Construct an empty container
	 */
	explicit TmxData();

	/*!
	 * @brief Construct a copy of the supplied container
	 */
	TmxData(TmxData &&);

	/*!
	 * @brief Construct a copy of the supplied container
	 */
	TmxData(const TmxData &);

	// Destructors
	virtual ~TmxData();

	// Assignment operators

	/*!
	 * @brief Assign from a copy of the supplied container
	 */
	TmxData &operator =(TmxData &&);

	/*!
	 * @brief Assign from a copy of the supplied container
	 */
	TmxData &operator =(const TmxData &);

	/*!
	 * @brief Assign null data
	 *
	 * @param[in] The null pointer
	 */
	TmxData &operator =(typename types::null_type::type &&);

	/*!
	 * @brief Assign null data
	 *
	 * @param[in] The null type
	 */
	TmxData &operator =(typename types::null_type &&);

	/*!
	 * @brief Assign null data
	 *
	 * @param[in] The null type
	 */
	TmxData &operator =(const types::null_type &);

	/*!
	 * @brief Assign boolean data
	 *
	 * @param[in] The boolean value
	 */
	TmxData &operator =(typename types::boolean_type::type &&);

	/*!
	 * @brief Assign boolean data
	 *
	 * @param[in] The boolean type
	 */
	TmxData &operator =(types::boolean_type &&);

	/*!
	 * @brief Assign boolean data
	 *
	 * @param[in] The boolean type
	 */
	TmxData &operator =(const types::boolean_type &);

	/*!
	 * @brief Assign 8-bit signed integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::int8::type);

	/*!
	 * @brief Assign 16-bit signed integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::int16::type);
	/*!
	 * @brief Assign 32-bit signed integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::int32::type);

	/*!
	 * @brief Assign 64-bit signed integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::int64::type);

	/*!
	 * @brief Assign 8-bit unsigned integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::uint8::type);

	/*!
	 * @brief Assign 16-bit unsigned integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::uint16::type);

	/*!
	 * @brief Assign 32-bit unsigned integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::uint32::type);

	/*!
	 * @brief Assign 64-bit unsigned integer data
	 *
	 * @param[in] value The integer data
	 */
	TmxData &operator =(typename types::uint64::type);

	/*!
	 * @brief Assign any sized signed integer data
	 *
	 * @param[in] value The integer type
	 */
	template <uint8_t _Sz>
	TmxData &operator =(types::Int<_Sz> &&value) {
		this->operator =(*value);
		this->_contents = typeid(types::Int<_Sz>);
		return *this;
	}

	/*!
	 * @brief Assign any sized signed integer data
	 *
	 * @param[in] value The integer type
	 */
	template <uint8_t _Sz>
	TmxData &operator =(const types::Int<_Sz> &value) {
		return this->operator =(std::move(value));
	}

	/*!
	 * @brief Assign any sized unsigned integer data
	 *
	 * @param[in] value The integer type
	 */
	template <size_t _Sz>
	TmxData &operator =(types::UInt<_Sz> &&value) {
		this->operator =(*value);
		this->_contents = typeid(types::UInt<_Sz>);
		return *this;
	}

	/*!
	 * @brief Assign any sized unsigned integer data
	 *
	 * @param[in] value The integer type
	 */
	template <size_t _Sz>
	TmxData &operator =(const types::UInt<_Sz> &value) {
		return this->operator =(std::move(value));
	}

	/*!
	 * @brief Assign 32-bit floating point
	 *
	 * @param[in] value The float data
	 */
	TmxData &operator =(typename types::float32::type);

	/*!
	 * @brief Assign 32-bit floating point
	 *
	 * @param[in] value The float data
	 */
	TmxData &operator =(typename types::float64::type);

#ifdef TMX_FLOAT128
	/*!
	 * @brief Assign 32-bit floating point
	 *
	 * @param[in] value The float data
	 */
	TmxData &operator =(typename types::float128::type);
#endif

	/*!
	 * @brief Assign any size floating point
	 *
	 * @param[in] value The float type
	 */
	template <size_t _Sz>
	TmxData &operator =(types::Float<_Sz> &&value) {
		this->operator =(*value);
		this->_contents = typeid(types::Float<_Sz>);
		return *this;
	}

	/*!
	 * @brief Assign any size floating point
	 *
	 * @param[in] value The float type
	 */
	template <size_t _Sz>
	TmxData &operator =(const types::Float<_Sz> &value) {
		return this->operator =(std::move(value));
	}

	// Note that because the character strings may accidentally be
	// converted to a boolean if there is no direct match for the
	// function, these are explicitly defined

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string8::type);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string8::view_type);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(const typename types::string8::char_t *);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(typename types::string8 &&);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(const typename types::string8 &);

	/*!
	 * @brief Assign an 16-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string16::type);

	/*!
	 * @brief Assign an 16-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string16::view_type);

	/*!
	 * @brief Assign an 16-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(const typename types::string16::char_t *);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(typename types::string16 &&);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(const typename types::string16 &);

	/*!
	 * @brief Assign an 32-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string32::type);

	/*!
	 * @brief Assign an 32-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::string32::view_type);

	/*!
	 * @brief Assign an 32-bit character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(const typename types::string32::char_t *);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(typename types::string32 &&);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(const typename types::string32 &);

	/*!
	 * @brief Assign a wide character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::wstring::type);

	/*!
	 * @brief Assign a wide character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(typename types::wstring::view_type);

	/*!
	 * @brief Assign a wide character string
	 *
	 * @param[in] value The string data
	 */
	TmxData &operator =(const typename types::wstring::char_t *);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(typename types::wstring &&);

	/*!
	 * @brief Assign an 8-bit character string
	 *
	 * @param[in] value The string type
	 */
	TmxData &operator =(const typename types::wstring &);

	/*!
	 * @brief Assign an array of values
	 *
	 * @param[in] The array type
	 */
	TmxData &operator =(typename types::array_type<TmxData> &&);

	/*!
	 * @brief Assign an array of values
	 *
	 * @param[in] The array type
	 */
	TmxData &operator =(const types::array_type<TmxData> &);

	/*!
	 * @brief Assign an array of values
	 *
	 * @param[in] value The array type
	 */
	template <typename _T>
	TmxData &operator =(types::array_type<_T> &&value) {
		this->clear();

		for (size_t i = 0; i < value.size(); i++)
			this->operator[](i) = value[i];

		return *this;
	}

	/*!
	 * @brief Assign an array of values
	 *
	 * @param[in] value The array type
	 */
	template <typename _T>
	TmxData &operator =(const types::array_type<_T> &value) {
		return this->operator =(types::array_type<_T>(value));
	}

	/*!
	 * @brief Assign a array of values
	 *
	 * @param[in] value The array data
	 */
	template <typename _T>
	TmxData &operator =(types::array_type_t<_T> &&value) {
		return this->operator=(types::array_type<_T>(std::forward< types::array_type_t<_T> >(value)));
	}

	/*!
	 * @brief Assign a array of values
	 *
	 * @param[in] value The array data
	 */
	template <typename _T>
	TmxData &operator =(const types::array_type_t<_T> &value) {
		return this->operator=(types::array_type<_T>(value));
	}

	/*!
	 * @brief Assign an map of values
	 *
	 * @param[in] The map type
	 */
	TmxData &operator =(types::map_type<types::string8, TmxData> &&);

	/*!
	 * @brief Assign an map of values
	 *
	 * @param[in] The map type
	 */
	TmxData &operator =(const types::map_type<types::string8, TmxData> &);

	/*!
	 * @brief Assign a map of values
	 *
	 * @param[in] value The map type
	 */
	template <typename _Key, typename _T>
	TmxData &operator =(types::map_type<_Key, _T> &&value) {
		this->clear();
		TmxData key;

		for (auto keyVal: value) {
			key = keyVal.first;
			this->operator[]((const char *)key) = keyVal.second;
		}

		return *this;
	}

	/*!
	 * @brief Assign a map of values
	 *
	 * @param[in] value The map type
	 */
	template <typename _Key, typename _T>
	TmxData &operator =(const types::map_type<_Key, _T> &value) {
		return this->operator=(types::map_type<_Key, _T>(value));
	}

	/*!
	 * @brief Assign a map of values
	 *
	 * @param[in] value The map data
	 */
	template <typename _Key, typename _T>
	TmxData &operator =(types::map_type_t<_Key, _T> &&value) {
		return this->operator=(types::map_type<_Key, _T>(std::forward< types::map_type_t<_Key, _T> >(value)));
	}

	/*!
	 * @brief Assign a map of values
	 *
	 * @param[in] value The map data
	 */
	template <typename _Key, typename _T>
	TmxData &operator =(const types::map_type_t<_Key, _T> &value) {
		return this->operator=(types::map_type<_Key, _T>(value));
	}

	// Primitive type converters

	/*!
	 * @return The data as a null type
	 */
	operator typename types::null_type::type () const;

	/*!
	 * @return The data as a boolean
	 */
	operator typename types::boolean_type::type () const;

	/*!
	 * @return The data as an 8-bit signed integer
	 */
	operator typename types::int8::type () const;

	/*!
	 * @return The data as an 16-bit signed integer
	 */
	operator typename types::int16::type () const;

	/*!
	 * @return The data as an 32-bit signed integer
	 */
	operator typename types::int32::type () const;

	/*!
	 * @return The data as an 64-bit signed integer
	 */
	operator typename types::int64::type () const;

	/*!
	 * @return The data as an 8-bit unsigned integer
	 */
	operator typename types::uint8::type () const;

	/*!
	 * @return The data as an 16-bit unsigned integer
	 */
	operator typename types::uint16::type () const;

	/*!
	 * @return The data as an 32-bit unsigned integer
	 */
	operator typename types::uint32::type () const;

	/*!
	 * @return The data as a 64-bit unsigned integer
	 */
	operator typename types::uint64::type () const;

	/*!
	 * @return The data as a 32-bit floating point
	 */
	operator typename types::float32::type () const;

	/*!
	 * @return The data as a 64-bit floating point
	 */
	operator typename types::float64::type () const;

#ifdef TMX_FLOAT128
	/*!
	 * @return The data as a 128-bit floating point
	 */
	operator typename types::float128::type () const;
#endif

	// Other converters

	template <typename _T>
	std::optional<_T> as() const {
		if (this->contains<_T>())
			return std::optional<_T>(std::any_cast<_T>(*this));

		return std::optional<_T>();
	}


	/*!
	 * @return The data as the specified sized integer type
	 */
	template <size_t _Sz = types::int_type<>::bits>
	auto to_int() const {
		typedef types::Int<_Sz> int_t;
		return int_t((typename int_t::type) *this);
	}

	template <size_t _Sz = types::int_type<>::bits>
	auto to_uint() const {
		typedef types::UInt<_Sz> uint_t;
		return uint_t((typename uint_t::type) *this);
	}

	/*!
	 * @return The data as the specified sized floating point type
	 */
	template <size_t _Sz = types::support::bit_sizeof<typename types::float_type<>::type>()>
	auto to_float() const {
		typedef types::Float<_Sz> float_t;
		return float_t((typename float_t::type) *this);
	}

	/*!
	 * @return The data as a C-style character string
	 */
	operator const typename types::string::char_t *() const;

	/*!
	 * @return The data as a view to the character string
	 */
	operator typename types::string::view_type () const;

	/*!
	 * @brief Return the data as a string
	 *
	 * This function will print out the TMX data using the specified encoding.
	 * By default, this will print array types as CSV, and map types as name-value properties.
	 * This may not be desirable, however, for simple serialization for output.
	 *
	 * @param[in] The encoder to use. Empty string is the default.
	 * @return The data as the minimally sized character string
	 */
	types::string to_string(typename types::string::view_type encoding = "") const;

	/*!
	 * @return The data as an array of values
	 */
	types::array_type<TmxData> to_array() const;

	/*!
	 * @return The data as a map of values
	 */
	types::map_type<types::string8, TmxData> to_map() const;

	// Data accessors
	TmxData &operator[](const types::string8 &);
	TmxData &operator[](const types::string8 &) const;
	TmxData &operator[](const typename types::string8::type &);
	TmxData &operator[](const typename types::string8::type &) const;
	TmxData &operator[](typename types::string8::view_type);
	TmxData &operator[](typename types::string8::view_type) const;
	TmxData &operator[](typename types::string8::const_pointer);
	TmxData &operator[](typename types::string8::const_pointer) const;
	TmxData &operator[](typename types::int16::type);
	TmxData &operator[](typename types::int16::type) const;
	TmxData &operator[](typename types::int32::type);
	TmxData &operator[](typename types::int32::type) const;
	TmxData &operator[](typename types::int64::type);
	TmxData &operator[](typename types::int64::type) const;
	TmxData &operator[](typename types::uint16::type);
	TmxData &operator[](typename types::uint16::type) const;
	TmxData &operator[](typename types::uint32::type);
	TmxData &operator[](typename types::uint32::type) const;
	TmxData &operator[](typename types::uint64::type);
	TmxData &operator[](typename types::uint64::type) const;


	// Other operations

	/*!
	 * @brief Clear the contents of this container
	 */
	void clear();

	/*!
	 * @brief Test if the container is empty
	 *
	 * @return True if the contents of this container is empty. False otherwise
	 */
	bool empty() const;

	/*!
	 * @brief Check the contents of this container for a specific type
	 *
	 * @param[in] The unique name within the namespace for the type
	 * @param[in] The type namespace, or null for the default
	 * @return True if the contents of this container have the specified type. False otherwise
	 */
	bool contains(typename types::string::view_type, typename types::string::view_type) const;

	/*!
	 * @brief Check the contents of this container for a specific type
	 *
	 * @param[in] The unique name within the namespace for the type
	 * @param[in] The type namespace, or null for the default
	 * @return True if the contents of this container have the specified type. False otherwise
	 */
	bool contains(const typename types::string::char_t *, const typename types::string::char_t *) const;

	/*!
	 * @brief Check the contents of this container for a specific type
	 *
	 * @param[in] _T The type to check for
	 * @return True if the contents of this container have the specified type. False otherwise
	 */
	template <typename _T>
	bool contains() const {
		static constexpr const auto _nm = types::support::type_short_name<_T>();
		static constexpr const auto _nmspace = types::support::type_namespace<_T>();
		return this->contains(_nm, _nmspace);
	}

	const std::type_info &get_type() const;

	typename types::string::view_type contents() const;

private:
	// To simplify things, make a choice of three structures
	std::unique_ptr< TmxTypeVariant > _scalarValue;
	std::unique_ptr< types::array_type<TmxData> > _arrayValue;
	std::unique_ptr< types::map_type<types::string8, TmxData> > _mapValue;

	std::type_index _contents;
};

// Some template specializations
// TODO: With automatic registration of the underlying type, are these necessary?
template <> inline bool TmxData::contains< typename types::null_type::type >() const {
	return this->contains< types::null_type >();
}

template <> inline bool TmxData::contains< typename types::boolean_type::type >() const {
	return this->contains< types::boolean_type >();
}

template <> inline bool TmxData::contains< typename types::int8::type >() const {
	return this->contains< types::int8 >();
}

template <> inline bool TmxData::contains< typename types::int16::type >() const {
	return this->contains< types::int16 >();
}

template <> inline bool TmxData::contains< typename types::int32::type >() const {
	return this->contains< types::int32 >();
}

template <> inline bool TmxData::contains< typename types::int64::type >() const {
	return this->contains< types::int64 >();
}

template <> inline bool TmxData::contains< typename types::uint8::type >() const {
	return this->contains< types::uint8 >();
}

template <> inline bool TmxData::contains< typename types::uint16::type >() const {
	return this->contains< types::uint16 >();
}

template <> inline bool TmxData::contains< typename types::uint32::type >() const {
	return this->contains< types::uint32 >();
}

template <> inline bool TmxData::contains< typename types::uint64::type >() const {
	return this->contains< types::uint64 >();
}

template <> inline bool TmxData::contains< typename types::float32::type >() const {
	return this->contains< types::float32 >();
}

template <> inline bool TmxData::contains< typename types::float64::type >() const {
	return this->contains< types::float64 >();
}

#ifdef TMX_FLOAT128
template <> inline bool TmxData::contains< typename types::float128::type >() const {
	return this->contains< types::float128 >();
}
#endif

template <> inline bool TmxData::contains< typename types::string8::type >() const {
	return this->contains< types::string8 >();
}

template <> inline bool TmxData::contains< typename types::string16::type >() const {
	return this->contains< types::string16 >();
}

template <> inline bool TmxData::contains< typename types::string32::type >() const {
	return this->contains< types::string32 >();
}

template <> inline bool TmxData::contains< typename types::wstring::type >() const {
	return this->contains< types::wstring >();
}

using any = tmx::common::TmxData;

// Serialization operators
/*!
 * @brief write the string version of the contents to an output stream
 * @param out The output stream to write to
 * @param message The message to write
 * @return The output stream
 */
std::ostream &operator<<(std::ostream &, const any &);

/*!
 * @brief Extract the contents from an input stream
 *
 * @param[in] The input stream to read from
 * @param[out] The data to load
 * @return The input stream
 */
std::istream &operator>>(std::istream &, any &);

} /* End namespace common */
} /* End namespace tmx */

#endif /* API_INCLUDE_TMX_COMMON_TMXDATA_HPP_ */
