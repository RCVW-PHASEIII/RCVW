/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxData.hpp
 *
 *  Created on: May 10, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef MESSAGE_INCLUDE_TMX_MESSAGE_TMXDATA_HPP_
#define MESSAGE_INCLUDE_TMX_MESSAGE_TMXDATA_HPP_

#include <tmx/common/TmxError.hpp>
#include <tmx/common/types/Any.hpp>

#include <typeinfo>
#include <type_traits>

namespace tmx {
namespace message {

/*!
 * @brief A class to manage the data contained in the Any type
 *
 * This is intended to provide a decent API for accessing the
 * contained data, converting types if necessary. This class
 * represents a more generic API for manipulating the data types
 * than the previous "message" class in earlier TMX versions.
 *
 * The data could be built programmatically using an empty TmxData
 * to begin with, or it could be built up from an existing Any type
 * object. When a reference to that Any object is used in construction
 * or assignment, then the TmxData class will manipulate the data
 * contained in the reference. Thus, care should be taken so that
 * such usage should NOT be done with temporaries, but with copies.
 *
 * No exceptions should ever occur when using this class, although
 * the data returned cannot be fully guaranteed because type
 * safety is generally disregarded.
 *
 * The TmxData non-const accessors will manipulate the data
 * in-place, meaning that changes made through this class will
 * be reflected in the underlying Any type container. This is
 * particularly useful for creating or changing Maps and Arrays.
 * Any values newly added to Maps and Arrays are Null by default.
 *
 * The TmxData const accessors provide an up-to-date view
 * of the underlying data, without modifying it. This is useful
 * for read-only contexts such as checking a flag that may
 * or may not be set. Trying to access a value that does not
 * exist in a Map or an Array will result in a Null return.
 *
 * Because of the duplicate accessors, care must be given to use
 * correct "const"-ness for a TmxData object, or else ambiguity may
 * arise.
 */
class TmxData {
    typedef common::types::Array<common::types::Any> array_type;
    typedef common::types::Properties<common::types::Any> properties_type;

public:
    /*!
     * @brief Default constructor stores a Null
     */
    TmxData() noexcept;

    /*!
     * @brief Copy constructor
     *
     * This will carry forward any references to
     * external containers, if possible
     *
     * @param[in] The TmxData object to copy
     */
    TmxData(TmxData const &) noexcept;

    /*!
     * @brief Construct using a copy of the given container
     *
     * @param[in] The Any type object to copy
     */
    TmxData(common::types::Any const &) noexcept;

    /*!
     * @brief Construct using Any data
     *
     * @param[in] The Any type object to construct with
     */
    TmxData(common::types::Any &&) noexcept;

    /*!
     * @brief Construct using the given container
     *
     * @param[in] The Any type object to manipulate
     */
    TmxData(common::types::Any &) noexcept;

    /*!
     * @brief Copy assignment
     *
     * This will carry forward any references to
     * external containers, if possible.
     *
     * @param[in] The TmxData object to copy
     * @return This object
     */
    TmxData &operator=(TmxData const &) noexcept;

    /*!
     * @brief Assign using a copy of the given container
     *
     * @param[in] The Any type object to copy
     * @return This object
     */
    TmxData &operator=(common::types::Any const &) noexcept;

    /*!
     * @brief Assign using Any data
     *
     * @param[in] The Any type object to assign
     * @return This object
     */
    TmxData &operator=(common::types::Any &&) noexcept;

    /*!
     * @brief Assign using a new container
     *
     * @param[in] The Any type object to manipulate
     * @return This object
     */
    TmxData &operator=(common::types::Any &) noexcept;

    /*!
     * @return A read/write reference to the underlying container
     */
    common::types::Any &get_container() noexcept;

    /*!
     * @return A read-only reference to the underlying container
     */
    common::types::Any const &get_container() const noexcept;

    /*!
     * @return True if the container has no or null value. False otherwise
     */
    bool is_empty() const noexcept;

    /*!
     * @return True if the container has a scalar value. False otherwise
     */
    bool is_simple() const noexcept;

    /*!
     * @return True if the container has an array value. False otherwise
     */
    bool is_array() const noexcept;

    /*!
     * @return True if the container has a properties value. False otherwise
     */
    bool is_map() const noexcept;

    /*!
     * @brief Interpret the value as a Boolean
     *
     * If the container is empty, this returns false.
     * If the container has a Boolean type, then return that Boolean value
     * If the container has a numeric type, then return false for 0, else true
     * If the container has a string type, then convert to Boolean or Numeric type
     * If the container has an array type, then return false for empty, else true
     * If the container has a map type, then return false for empty, else true
     * @return True if the value is "true". False otherwise.
     */
    operator typename common::types::Boolean::value_type () const noexcept;

    /*!
     * @brief Interpret the value as an unsigned Integer
     *
     * If the container is empty, this returns 0.
     * If the container has a Boolean type, then return 0 for false, else 1
     * If the container has a numeric type, then return the number
     * If the container has a string type, then convert to Numeric type
     * If the container has an array type, then return size()
     * If the container has a map type, then return size()
     * @return The unsigned Integer equivalent of the value
     */
    operator typename common::types::UInt64::value_type () const noexcept;
    operator typename common::types::UInt32::value_type () const noexcept;
    operator typename common::types::UInt16::value_type () const noexcept;
    operator typename common::types::UInt8::value_type () const noexcept;

    /*!
     * @brief Interpret the value as an signed Integer
     *
     * If the container is empty, this returns 0.
     * If the container has a Boolean type, then return 0 for false, else 1
     * If the container has a numeric type, then return the number
     * If the container has a string type, then convert to Numeric type
     * If the container has an array type, then return size()
     * If the container has a map type, then return size()
     * @return The signed Integer equivalent of the value
     */
    operator typename common::types::Int64::value_type () const noexcept;
    operator typename common::types::Int32::value_type () const noexcept;
    operator typename common::types::Int16::value_type () const noexcept;
    operator typename common::types::Int8::value_type () const noexcept;

    /*!
     * @brief Interpret the value as a Double
     *
     * If the container is empty, this returns 0.
     * If the container has a Boolean type, then return 0 for false, else 1
     * If the container has a numeric type, then return the number
     * If the container has a string type, then convert to Numeric type
     * If the container has an array type, then return size()
     * If the container has a map type, then return size()
     * @return The Double equivalent of the value
     */
    operator typename common::types::Float128::value_type () const noexcept;
    operator typename common::types::Float64::value_type () const noexcept;
    operator typename common::types::Float32::value_type () const noexcept;

    /*!
     * @brief Interpret the value as a String
     *
     * If the container is empty, this returns "null".
     * If the container has a Boolean type, then return "false" or "true"
     * If the container has a numeric type, then convert the number
     * If the container has a string type, then return the string
     * If the container has an array type, then return the JSON equivalent
     * If the container has a map type, then return the JSON equivalent
     * @return The String equivalent of the value
     */
    operator typename common::types::String8::value_type () const noexcept;
    operator typename common::types::String16::value_type () const noexcept;
    operator typename common::types::String32::value_type () const noexcept;

    template <typename _E, class = common::enable_enum<_E> >
    operator _E () const noexcept {
        typedef common::types::Enum<_E> type;

        // First check to see if this value is encoded by string
        const auto str = this->to_string();
        if (str.length() > 0 && !std::isdigit(str[0])) {
            auto val = common::enums::enum_cast<_E>(str.data());
            if (val)
                return *val;
        }

        const auto val = common::enums::enum_cast<_E>((typename type::underlying_type)(*this));
        if (val)
            return *val;

        return common::enums::enum_entries<_E>()[0].first;
    }

    /*!
     * @brief Interpret the value as a read-only Array and return the value at the index
     *
     * If the container is not an Array, this returns an empty object
     * If the container is an Array, and index is out of range, this returns an empty object
     * Otherwise, this returns the value at the given index
     *
     * @param[in] The index
     * @return The value at the given index, if it exists
     */
    TmxData operator[](common::types::UIntmax const &) const noexcept;

    /*!
     * @brief Interpret the value as a read/write Array and return the value at the index
     *
     * If the container is not an Array, then the container is rebuilt as an Array,
     * removing any previous contained data.
     * If the container is an Array, and the index is out of range,
     * then the Array is resized to contain at least that number of elements
     * Therefore, this always returns the value at the given index
     *
     * @param[in] The index
     * @return The value at the given index
     */
    TmxData operator[](common::types::UIntmax const &) noexcept;

    template <std::uint8_t _Sz>
    typename std::enable_if<(_Sz < common::types::UIntmax::numBits), TmxData>::type
    operator[](common::types::UInt<_Sz> const &index) const noexcept {
        return this->operator[](common::types::UIntmax((typename decltype(index)::value_type)index));
    }

    template <std::uint8_t _Sz>
    typename std::enable_if<(_Sz < common::types::UIntmax::numBits), TmxData>::type
    operator[](common::types::UInt<_Sz> const &index) noexcept {
        return this->operator[](common::types::UIntmax((typename decltype(index)::value_type)index));
    }

    template <std::uint8_t _Sz>
    TmxData operator[](common::types::Int<_Sz> const &index) const noexcept {
        auto _i = common::types::make_unsigned(index);
        return this->operator[](common::types::UIntmax((typename decltype(_i)::value_type)_i));
    }

    template <std::uint8_t _Sz>
    TmxData operator[](common::types::Int<_Sz> const &index) noexcept {
        auto _i = common::types::make_unsigned(index);
        return this->operator[](common::types::UIntmax((typename decltype(_i)::value_type)_i));
    }

    template <typename _T>
    common::enable_int<_T, TmxData> operator[](_T index) {
        return this->operator[](common::types::UIntmax(index));
    }

    template <typename _T>
    common::enable_int<_T, TmxData> operator[](_T index) const {
        return this->operator[](common::types::UIntmax(index));
    }

    /*!
     * @brief Interpret the value as a read-only Properties and return the value at the key
     *
     * If the container is not an Properties, this returns an empty object
     * If the cotnainer is an Properties, and the key is out of range, this returns an empty object
     * Otherwise, this returns the value at the given key
     *
     * @param[in] The key
     * @return The value at the given key, if it exists
     */
    TmxData operator[](common::types::String8 const &) const noexcept;
    TmxData operator[](typename common::types::String8::char_t const *) const noexcept;

    /*!
     * @brief Interpret the value as a read/write Array and return the value at the index
     *
     * If the container is a scalar, then the container is rebuilt as a Properties,
     * removing any previous contained data.
     * If the container is an Map, and the key is out of range, this constructs a new object for the key
     * Therefore, this always returns the value at the given key
     *
     * @param[in] The key
     * @return The value at the given key
     */
    TmxData operator[](common::types::String8 const &) noexcept;
    TmxData operator[](typename common::types::String8::char_t const *) noexcept;

    /*!
     * @brief Interpret the value as a read-only Array or Map and return the value at the Enum key
     *
     * If the contained value is already an Array, then this functions
     * uses the Enum integer value as the index to the Array.
     * Otherwise, this interprets the value as a Properties,
     * and uses the Enum name as the key to the Map.
     *
     * @param[in] val The Enum
     * @return The data at the key/index
     */
    template <typename _E>
    common::enable_enum<_E, TmxData> operator[](const _E val) const noexcept {
        common::types::Enum<_E> _value { val };

        if (this->is_array()) {
            return this->operator[](_value.get_integer_value());
        } else {
            return this->operator[](_value.get_enum_name().c_str());
        }
    }

    /*!
     * @brief Interpret the value as a read/write Array or Map and return the value at the Enum key
     *
     * If the contained value is already an Array, then this functions
     * uses the Enum integer value as the index to the Array.
     * Otherwise, this interprets the value as a Properties,
     * and uses the Enum name as the key to the Map.
     *
     * @param[in] val The Enum
     * @return the data at the key/index
     */
    template <typename _E>
    common::enable_enum<_E, TmxData> operator[](const _E val) noexcept {
        common::types::Enum<_E> _value { val };

        if (this->is_array()) {
            return this->operator[](_value.get_integer_value());
        } else {
            return this->operator[](_value.get_enum_name().c_str());
        }
    }

    /*!
     * @see #operator bool()
     * @return The Boolean equivalent of the value
     */
    inline common::types::Boolean to_bool() const {
        return { (typename common::types::Boolean::value_type)(*this) };
    }

    /*!
     * @return The unsigned Integer equivalent of the value
     */
    template <std::uint8_t _Sz = common::types::UInt_::numBits>
    common::types::UInt<_Sz> to_uint() const {
        typedef common::types::UInt<_Sz> type;
        return { static_cast<typename type::value_type>((typename common::types::UIntmax::value_type)(*this)) };
    }

    /*!
     * @return The signed Integer equivalent of the value
     */
    template <std::uint8_t _Sz = common::types::Int_::numBits>
    common::types::Int<_Sz> to_int() const {
        typedef common::types::Int<_Sz> type;
        return { static_cast<typename type::value_type>((typename common::types::Intmax::value_type)(*this)) };
    }

    /*!
     * @return The floating point equivalent of the value
     */
    template <std::uint8_t _Sz = common::types::Float_::numBits>
    common::types::Float<_Sz> to_float() const {
        typedef common::types::Float<_Sz> type;
        return { static_cast<typename type::value_type>((typename common::types::Floatmax::value_type)(*this)) };
    }

    /*!
     * @return The string equivalent of the value
     */
    template <std::uint8_t _Sz = common::types::String_::numBits>
    common::types::String<_Sz> to_string() const {
        return { (typename common::types::String<_Sz>::value_type)(*this) };
    }

    /*!
     * @return The enum equivalent of the value
     */
    template <typename _E>
    common::enable_enum<_E, common::types::Enum<_E> > to_enum() const {
        return { (_E)(*this) };
    }

    array_type to_array() const;
    properties_type to_map() const;

private:
    common::types::Any _container;
    common::types::Any &_reference;
};

template <typename _E, _E ... _V>
class TmxEnumMappedData: public TmxData {
    typedef TmxEnumMappedData<_E, _V...> self_type;

    static_assert(common::is_enum<_E>::value, "Must use an enumeration type in TmxEnumMappedData");
    static constexpr std::size_t _size = sizeof...(_V);
public:
    typedef common::static_array<_E, _V...> sequence;

    TmxEnumMappedData() noexcept = default;
    TmxEnumMappedData(const self_type &copy) noexcept : TmxData(copy.get_container()) { }
    TmxEnumMappedData(TmxData &&copy) noexcept : TmxData(std::forward<TmxData>(copy)) { }

    ~TmxEnumMappedData() = default;

    TMX_CONSTEXPR_FN std::size_t size() const noexcept {
        return _size;
    }

    template <std::size_t ... _I>
    TMX_CONSTEXPR_FN auto get_sequence(common::integer_sequence<std::size_t, _I...> const &slice =
                                        common::make_index_sequence<_size>()) const noexcept {
        return common::make_array(sequence {}, slice);
    }

    template <_E _Key>
    common::types::Any &get() noexcept {
        return this->operator[](_Key).get_container();
    }

    template <_E _Key>
    common::types::Any &get() const noexcept {
        return this->operator[](_Key).get_container();
    }

    template <_E _Key, typename _T>
    auto get() noexcept {
        return common::types::as<_T>(this->get_container());
    }

    template <_E _Key, typename _T>
    auto get() const noexcept {
        return common::types::as<_T>(this->get_container());
    }

    template <_E _Key, typename _T>
    _T &put(_T &&val) noexcept {
        return this->operator[](_Key).get_container().template emplace<_T>(val);
    }

    template <_E ... _Keys, typename ... _Tp>
    auto load(common::static_array<_E, _Keys...> const &, _Tp && ... val) noexcept {
        return std::tuple<_Tp...>(this->put<_Keys>(std::forward<_Tp>(val))...);
    }

    template <typename ... _Tp>
    auto load_all(_Tp && ... val) noexcept {
        return this->load(sequence { }, std::forward<_Tp>(val)...);
    }
};

template <typename _E, _E ... _V>
struct _enum_mapped_data_selector {
    typedef TmxEnumMappedData<_E, _V...> type;
};

template <typename _E, _E ... _V>
struct _enum_mapped_data_selector< common::static_array<_E, _V...> > {
    typedef TmxEnumMappedData<_E, _V...> type;
};

template <typename _E>
using TmxAutoData = typename _enum_mapped_data_selector< decltype(common::enums::enum_sequence<_E>()) >::type;

} /* End namespace message */
} /* End namespace tmx */

#endif /* MESSAGE_INCLUDE_TMX_MESSAGE_TMXDATA_HPP_ */
