/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataType.hpp
 *
 *  Created on: Oct 4, 2022
 *      @author: gmb
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_TMXDATATYPE_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_TMXDATATYPE_HPP_

#include <tmx/platform.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <istream>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief A base class to use for scalar types
 *
 * Unlike regular classes, native C++ scalar types do not have
 * a class construct, and thus no "this" pointer. Therefore,
 * this class serves as a way to create an internal value that
 * can be accessed like a part of the TMX data type object
 */
template <typename _Tp>
class TmxScalarType {
public:
	virtual ~TmxScalarType() { }

	/*!
	 * @return True if lhs == rhs. False otherwise.
	 */
	template <typename _Other>
	bool equals(_Tp const &lhs, _Other const &rhs) const noexcept {
		return lhs == rhs;
	}

protected:
	/*!
	 * @brief Construct a new TMX scalar type with the default value
	 *
	 * The default value is the type-equivalent of "0".
	 */
	TmxScalarType(): _value(static_cast<_Tp>(0)) { }

	/*!
	 * @brief Construct a new TMX scalar type with the given value
	 */
	TmxScalarType(_Tp const &value): _value(value) { }

	/*!
	 * @brief Storage for a data value copy
	 */
	_Tp _value;
};

/*!
 * @brief A base class to use for class types
 *
 * This is primarily used to provide a consistent interface
 * to the super class. The value instance is just a reference
 * to the object.
 */
template <typename _Tp>
class TmxObjectType: public _Tp {
public:
	virtual ~TmxObjectType() { }

	/*!
	 * @return True if lhs == rhs. False otherwise.
	 */
	template <typename _T, typename _Other>
	enable_equal<_T, _Other, bool> equals(_T const &lhs, _Other const &rhs) const noexcept {
		return lhs == rhs;
	}

	/*!
	 * @return True if lhs and rhs point to the same location. False otherwise.
	 */
	template <typename _T, typename _Other>
	disable_equal<_T, _Other, bool> equals(_T const &lhs, _Other const &rhs) const noexcept {
		return &lhs == &rhs;
	}
protected:
	/*!
	 * @brief Construct a new TMX object type with the default value
	 *
	 * The default value is the no-argument construction
	 */
	TmxObjectType(): _value(*this) { }

	/*!
	 * @brief Construct a new TMX object type with the given value
	 *
	 * This uses copy semantics for construction
	 */
	TmxObjectType(_Tp const &value):
			_Tp(value), _value(*this) { }

	/*!
	 * @brief A reference to the data value
	 */
	_Tp &_value;
};

/*!
 * @brief The base class to use for the type _Tp
 *
 * This checks to see if _Tp is a class type, which will then become the
 * base class for the TMX data type. If _Tp is not a class, then the
 * base class is empty.
 *
 * @param The C or C++ type to wrap
 */
template <typename _Tp>
using TmxDataTypeBase = typename std::conditional<tmx::common::is_scalar<_Tp>::value,
		TmxScalarType<_Tp>, TmxObjectType<_Tp> >::type;

/*!
 * @brief The base data type wrapper for TMX
 *
 * This class is the base for all other TMX data types.

 * The assumption here is that all the TMX data types can be held within some
 * existing C/C++ data type, e.g., an double can be used for the Float<64>.
 * Therefore, instead of re-implementing native code, this wrapper class
 * can be used to provide a common TMX interface for any of its known types.
 *
 * Having a templated based class also serves a key purpose for polymorphic
 * behavior. Because the underlying type is boxed into the base class
 * template, it becomes easy to un-box within a template function containing
 * an deduced input parameter, e.g.:
 *
 * template <typename _Tp>
 * auto typeBasedOperation(tmx::common::types::TmxDataType<_Tp> const &);
 *
 * Any TMX data type can be used within this type-based operation, where the
 * underlying C-type and value can be unboxed.
 *
 * @see #getValueOf()
 *
 * The value stored within this wrapper type depends on the usage. By default,
 * an object contains its own data storage, to which values can be assigned
 * and reassigned. However, copying values from other data structures is be
 * too inefficient considering this base class will be used over and over again
 * during encoding and decoding the TMX data. Therefore, an object or type
 * containing the value is used by reference, ensuring that the cost of using a
 * TMX data type class is restricted to the copy of a pointer and altering
 * the data reference would also modify the value contained in this class.
 *
 * That said, it should be made clear that the wrapped variable must be a
 * valid pointer. If the data goes out of scope, for example, or the pointer
 * becomes otherwise invalid, then the behavior of this class is by
 * definition undefined. With usage internal to TMX, valid pointers are ensured.
 * However, if used externally, then extra care must be taken to ensure no
 * references are lost. The best solution is to pass in a shared_ptr with a
 * functioning deleter, since this class will only add to the reference count.
 *
 * Likewise, all copies and assignments of this data type to a second one will
 * result in a shallow copy. In other words, the two objects will point to the
 * same value. For example:
 *
 * TmxDataType<int> one(11);
 * TmxDataType<int> two;
 * two = one;	// Two references the value of one (11)
 * one = 70; 	// Both one and two contain the value 70
 *
 * To force a copy of the data value, you must un-box that value. For example:
 *
 * TmxDataType<int> one(11);
 * TmxDataType<int> two;
 * two = (int)one;	// This creates a new integer
 * one = 70; 		// One contains 70, but two still has 11
 *
 * @param The C or C++ type to wrap.
 */
template <typename _Tp>
class TmxDataType: public TmxDataTypeBase<_Tp> {
	static constexpr auto _nonDeleter = [](_Tp *) { };

protected:
	/*!
	 * @brief The base class
	 */
	typedef TmxDataTypeBase<_Tp> super;

	/*!
	 * @brief A pointer to the data value
	 *
	 * This is left null if using the internal value
	 */
	std::shared_ptr<_Tp> _ptr { };

public:
	/*
	 * @brief The value type
	 */
	typedef _Tp value_type;

	/*
	 * @brief All TMX data types should have a reflective type
	 */
	typedef TmxDataType<value_type> self_type;

	/*!
	 * @brief A type to use to hash this TMS data type
	 *
	 * This is necessary to store as keys in unsorted maps
	 */
	typedef std::hash<value_type> hasher_type;

	/*!
	 * @brief No real cleanup is needed
	 */
	virtual ~TmxDataType() { }

	/*!
	 * @brief Construct a new TMX data type with a default value
	 *
	 * The default value is the type-equivalent of "0",
	 * unless otherwise specified in the sub-class.
	 *
	 * This is a public constructor so it can be used implicitly
	 * by the sub-class.
	 */
	TmxDataType() noexcept: super() { }

	/*!
	 * @brief Construct a new TMX data type with the specified pointer
	 *
	 * The object will reference the same value pointed by p, if
	 * that pointer is not null. Otherwise, this constructor
	 * acts just like the default.
	 *
	 * It is assumed that the deleter for the shared pointer is
	 * already set.
	 *
	 * This is a public constructor so it can be used implicitly
	 * by the sub-class.
	 *
	 * @param p The value pointer to use
	 */
	TmxDataType(std::shared_ptr<value_type> p) noexcept:
			TmxDataType() { this->_ptr = p; }

	/*!
	 * @brief Construct a new TMX data type from a copy
	 *
	 * The object will reference the same value as the copy.
	 *
	 * Note that there is no direct was to copy-construct
	 * TMX data of a different type. This must be done through
	 * some cast of the copy's value reference.
	 *
	 * This is a public constructor so it can be used implicitly
	 * by the sub-class.
	 *
	 * @param copy The copy to use
	 */
	TmxDataType(self_type const &copy) noexcept:
			super((value_type)copy), _ptr(copy._ptr) { }

	/*!
	 * @brief Construct a new TMX data type with the specified value
	 *
	 * The object will create its own copy of the value. This is
	 * useful in constructing from literal values.
	 *
	 * This requires _Tp to have copy or move assignment semantics.
	 * Otherwise, this constructor acts just like the default.
	 *
	 * This is a public constructor so it can be used implicitly
	 * by the sub-class.
	 *
	 * @param value The value to use
	 */
	TmxDataType(value_type &&value) noexcept: super(value) { }

	/*!
	 * @brief Construct a new TMX data type with the specified reference
	 *
	 * The object will reference the same value
	 *
	 * This is a public constructor so it can be used implicitly
	 * by the sub-class.
	 *
	 * @param value The value reference to use
	 */
	TmxDataType(value_type &value) noexcept:
			TmxDataType(std::shared_ptr<value_type>(&value, _nonDeleter)) { }

	/*!
	 * @return The number of bytes needed to store this TMX type
	 */
	virtual std::size_t get_byte_count() const noexcept {
		return tmx::common::byte_size<value_type>::value;
	}

	/*!
	 * @return The number of bits needed to store this TMX type
	 */
	virtual std::size_t get_bit_count() const noexcept {
		return tmx::common::bit_size<value_type>::value;
	}

	/*!
	 * @brief De-reference the held pointer
	 *
	 * This helps access the contents of the held type
	 * reference, particularly useful for simple scalar
	 * types. The contained value is mutable.
	 *
	 * @return A reference to the contained value
	 */
	virtual value_type &operator *() noexcept {
		if (this->_ptr) return *(this->_ptr);
		return this->_value;
	}

	/*!
	 * @brief Get the held pointer
	 *
	 * This helps call an operation on the held type,
	 * particularly useful for objects. The contained
	 * value is mutable.
	 *
	 * @return A pointer to the contained value
	 */
	virtual value_type *operator ->() noexcept {
		if (this->_ptr) return this->_ptr.get();
		return &(this->_value);
	}

	/*!
	 * @return A copy of the contained value
	 */
	virtual operator value_type () const noexcept {
		if (this->_ptr) return *(this->_ptr);
		return this->_value;
	}

	/*!
	 * @brief Assign a new TMX data type with the specified reference
	 *
	 * The object will reference the same value.
	 *
	 * @param value The value reference to use
	 */
	self_type &operator=(value_type const &value) noexcept {
		if (this->_ptr) *(this->_ptr) = value;
        else this->_value = value;
		return *this;
	}

	/*!
	 * @brief Assign a new TMX data type with the specified value
	 *
	 * The object will create its own copy of the value. This is
	 * useful in assigning from literal values.
	 *
	 * @param value The value to use
	 */
	self_type &operator=(value_type &&value) noexcept {
		return this->operator=(value);
	}

	/*!
	 * @brief Assign a new TMX data type from a copy
	 *
	 * The object will reference the same value as the copy,
	 * but will also lose any reference to the previous
	 * value it had.
	 *
	 * @param copy The copy to use
	 */
	self_type &operator=(self_type const &copy) noexcept {
		this->_value = copy._value;
		this->_ptr = copy._ptr;
		return *this;
	}

	/*!
	 * @return True if this type has the same contents as the other
	 */
	bool operator==(value_type const &other) noexcept {
		return this->equals((value_type)(*this), other);
	}

	/*!
	 * @return True if this type has the same contents as the other
	 */
	bool operator==(value_type &&other) noexcept {
		return this->operator==(other);
	}

	/*!
	 * @return True if this type does not have the same contents as the other
	 */
	bool operator!=(value_type const &other) noexcept {
		return !this->operator==(other);
	}

	/*!
	 * @return True if this type does not have the same contents as the other
	 */
	bool operator!=(value_type &&other) noexcept {
		return this->operator!=(other);
	}

	/*!
	 * @brief Get the type information for the TMX data type
	 *
	 * @see #std::type_info
	 * @return The typeid of this TMX data type
	 */
	virtual const std::type_info &get_typeid() const noexcept {
		return typeid(value_type);
	}

	/*!
	 * @brief Construct a new TMX data type with the specified value
	 *
	 * hash code.
	 *
	 * @see getTypeIndex()
	 * @return The hash code for this TMX data type
	 */
	virtual std::size_t get_type_hash_code() const noexcept {
		return get_typeid().hash_code();
	}
};

/*!
 * @param Any TMX data type
 * @return A copy the contained type in t
 */
template <typename _Tp>
typename TmxDataType<_Tp>::value_type getValueOf(TmxDataType<_Tp> const &t) {
	return (typename TmxDataType<_Tp>::value_type)t;
}

/*!
 * @brief An structure for determining the TMX data type from native type
 *
 * The default is a complete type, but returns a non-TMX
 * type. Therefore, any C++ type may be used here, but only
 * those with the TMX flag set to true is a standard TMX
 * data type.
 *
 * This structure must be specialized by each data type
 * implementation.
 */
template <typename _Tp, typename = void>
struct TmxType {
	/*!
	 * @brief A marker to denote this is a TMX data type
	 *
	 * If _Tp is already a TmxDataType class, and its value type is
	 * a supported TMX type already, then this will be true. Otherwise,
	 * this is false.
	 *
	 * This depends on TmxType specialization for each of the supported
	 * value types.
	 */
	typedef std::false_type TMX;

	/*!
	 * @brief The TMX data type to use for _Tp
	 *
	 * If _Tp is already a TmxDataType class, then that type will
	 * be used. Otherwise, the type will be wrapped to provide
	 * a consistent interface
	 */
	typedef TmxDataType<typename std::decay<_Tp>::type> type;

    /*!
     * @brief A name to use for the TMX data type
     *
     * This defaults to the fully qualified C++ type name
     */
    static constexpr auto name = type_fqname<typename std::decay<_Tp>::type>();
};

/*!
 * @brief A specialization for void types
 */
template <>
struct TmxType<void> {
	typedef std::false_type TMX;
	typedef void type;
    static constexpr auto name = empty_string();
};

/*!
 * @brief A type alias to construct the TMX type traits
 *
 * This is the preferred manner to create a TmxType structure
 * since it decays the template parameter.
 */
template <typename _Tp>
using TmxTypeTraits = TmxType<typename std::decay<_Tp>::type>;

/*!
 * @brief A type alias to extract the TMX data type to use
 */
template <typename _Tp>
using TmxTypeOf = typename TmxTypeTraits<_Tp>::type;

/*!
 * @brief A helper class for simplicity
 *
 * This only un-boxes the TMX field from the TmxType structure,
 * which (if not specialized) will check to see if the class has
 * a value type and is a base class of TmxDataType. However,
 * the value type itself must be a support TMX type for _Tp to
 * be useful. Use IsTmxType to determine both conditions are
 * true.
 *
 * @see #IsTmxType
 */
template <typename _Tp>
using IsTmxType = typename TmxTypeTraits<_Tp>::TMX;

/*!
 * @brief A type alias to unbox the TmxDataType value_type
 */
template <typename _Tp>
using TmxValueTypeOf = typename std::decay<typename TmxTypeOf<_Tp>::value_type>::type;

/*!
 * @brief A alias for obtaining a tuple of TMX data types
 */
template <typename... _Tp>
using TmxTypes = std::tuple< TmxTypeOf<_Tp>... >;

template <typename _Tp>
TMX_CONSTEXPR_FN auto tmx_fqname(_Tp const &) {
    return type_fqname< TmxTypeOf<_Tp> >();
}

template <typename _Tp>
TMX_CONSTEXPR_FN auto tmx_short_name(_Tp const &) {
    return type_short_name< TmxTypeOf<_Tp> >();
}

template <typename _Tp>
typename std::enable_if<!IsTmxType<_Tp>::value, _Tp>::type make_type(_Tp const &value) {
    return _Tp(value);
}

template <typename _Tp>
typename std::enable_if<!IsTmxType<_Tp>::value, _Tp>::type make_type(std::shared_ptr<_Tp> const &value) {
    return _Tp(*value);
}

template <typename _Tp>
typename std::enable_if<IsTmxType<_Tp>::value, TmxTypeOf<_Tp> >::type make_type(_Tp const &value) {
    return TmxTypeOf<_Tp>(value);
}

template <typename _Tp>
typename std::enable_if<IsTmxType<_Tp>::value, TmxTypeOf<_Tp> >::type make_type(std::shared_ptr<_Tp> const &value) {
    return TmxTypeOf<_Tp>(*value);
}

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_TMXDATATYPE_HPP_ */
