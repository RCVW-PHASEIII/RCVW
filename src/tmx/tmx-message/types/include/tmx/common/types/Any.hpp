/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Any.hpp
 *
 *  Created on: Oct 19, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TYPES_ANY_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TYPES_ANY_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxTypeDescriptor.hpp>

// Source all the types, simplier types first
#include <tmx/common/types/TmxDataType.hpp>
#include <tmx/common/types/Null.hpp>
#include <tmx/common/types/Boolean.hpp>
#include <tmx/common/types/Int.hpp>
#include <tmx/common/types/Float.hpp>
#include <tmx/common/types/Enum.hpp>
#include <tmx/common/types/String.hpp>
#include <tmx/common/types/Array.hpp>
#include <tmx/common/types/Map.hpp>

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

namespace tmx {
namespace common {
namespace types {

/*!
 * @brief The Any type definition
 */
typedef tmx::common::any Any;

/*!
 * @brief Determine the contents of the Any value
 *
 * Note that the type must be registered in the given namespace
 * to have a valid name and instance.
 *
 * @param[in] The Any value
 * @param[in] The registry namespace to search in
 * @return The type descriptor for the contents
 */
TmxTypeDescriptor contents(Any const &, const_string = empty_string()) noexcept;

/*!
 * @brief Check if the contents of the Any value matches the specified type
 *
 * Note that this also checks registered aliases in the given namespace,
 * including those matching the same type identifier under a different name
 * and those matching the same type name under a different identifier.
 *
 * @param[in] The Any value
 * @param[in] The type identifier to check for
 * @param[in] The registry namespace to search in
 * @return True if the contents of the Any value are of the given type
 */
bool contains(Any const &, std::type_info const &, const_string = empty_string()) noexcept;

/*!
 * @return True if the contents hold a supported TMX data type
 */
bool contains_tmx(Any const &) noexcept;

/*!
 * @brief Check if the contents of the Any value matches the specified type
 *
 * Note that this also checks registered aliases in the given namespace,
 * including those matching the same type identifier under a different name
 * and those matching the same type name under a different identifier.
 *
 * @tparam _Tp The type to check for
 * @param[in] The Any value
 * @return True if the contents of the Any value are of the given type
 */
template <typename _Tp>
bool contains(Any const &data) noexcept {
    if (IsTmxType<_Tp>::value)
        return contains(data, typeid(TmxTypeOf<_Tp>)) || contains(data, typeid(TmxValueTypeOf<_Tp>));
    else
        return contains(data, typeid(_Tp), type_namespace<_Tp>());
}

template <typename _T>
std::shared_ptr<_T> as(Any &data) noexcept {
    auto _shared = tmx::common::any_cast< std::shared_ptr<_T> >(&data);
    if (_shared) return { *_shared };

    auto _val = std::any_cast<_T>(&data);
    if (_val) return { _val, [](auto *) { } };
    return { };
}

template <typename _T>
std::shared_ptr<const _T> as(Any const &data) noexcept {
    auto _shared = tmx::common::any_cast< std::shared_ptr<_T> >(&data);
    if (_shared) return { *_shared };

    auto _val = std::any_cast<_T>(&data);
    if (_val) return { _val, [](auto *) { } };
    return { };
}

template <typename _Tp>
constexpr auto (*as_scalar)(Any &) = &as<_Tp>;

constexpr auto (*as_bool) = as_scalar<bool>;
constexpr auto (*as_char) = as_scalar<char>;
constexpr auto (*as_uchar) = as_scalar<unsigned char>;
constexpr auto (*as_short) = as_scalar<short>;
constexpr auto (*as_ushort) = as_scalar<unsigned short>;
constexpr auto (*as_int) = as_scalar<int>;
constexpr auto (*as_uint) = as_scalar<unsigned int>;
constexpr auto (*as_long) = as_scalar<long>;
constexpr auto (*as_ulong) = as_scalar<unsigned long>;
constexpr auto (*as_float) = as_scalar<float>;
constexpr auto (*as_double) = as_scalar<double>;
constexpr auto (*as_ldouble) = as_scalar<long double>;

constexpr auto (*as_string8)(Any &) = &as< String<8> >;
constexpr auto (*as_string16)(Any &) = &as< String<16> >;
constexpr auto (*as_string32)(Any &) = &as< String<32> >;

template <typename _Tp = Any>
constexpr auto (*as_array)(Any &) = &as< Array<_Tp> >;
template <typename _Tp = Any>
constexpr auto (*as_list)(Any &) = &as< Array<_Tp> >;
template <typename _Tp = Any>
constexpr auto (*as_queue)(Any &) = &as< Array<_Tp> >;
template <typename _Tp = Any>
constexpr auto (*as_properties)(Any &) = &as< Properties<_Tp> >;
template <typename _Tp = Any>
constexpr auto (*as_dictionary)(Any &) = &as< Properties<_Tp> >;

//template <typename _Tp = Any, typename _Str>
//enable_string<_Str, _Tp const &, typename _Str::value_type> at(Any const &data, _Str name) {
//    auto _props = tmx::common::any_cast< Properties<_Tp> >(&data);
//    if (_props)
//        return _props->at(name);
//
//    auto _vprops = tmx::common::any_cast< TmxValueTypeOf< Properties<_Tp> > >(&data);
//    if (_vprops)
//        return _vprops->at(name);
//
//    static _Tp _defaultValue;
//    return _defaultValue;
//}
//
//template <typename _Tp = Any, typename _CharT>
//_Tp const & at(Any const data, const _CharT *name) {
//    return at(data, TmxValueTypeOf< string_type<_CharT> >(name));
//}
//
//template <typename _Tp = Any, typename _Int>
//enable_int<_Int, _Tp const &> at(Any const &data, _Int index) {
//    auto _array = tmx::common::any_cast< Array<_Tp> >(data);
//    if (_array)
//        return _array->at(index);
//
//    auto _varray = tmx::common::any_cast< TmxValueTypeOf< Array<_Tp> > >(&data);
//    if (_varray)
//        return _varray->at(index);
//
//    static _Tp _defaultValue;
//    return _defaultValue;
//}


/*!
 * @brief Construct an Any type from some value
 *
 * This could be a known TMX data type, which includes all C and C++
 * native types, string types, STL array types and STL map types.
 * Each of those will see the isTmxType flag to true. This class
 * also supports other object types for consistency with the
 * std::any class. These will also be wrapped in a TMX data type,
 * but isTmxType will be false.
 *
 * @param[in] The value to use
 * @return The Any value
 */
template <class _Tp>
typename std::enable_if<!IsTmxType<_Tp>::value, Any>::type make_any(_Tp &&value) noexcept {
    return { std::forward<_Tp>(value) };
}

/*!
 * @brief Construct an Any type from some value
 *
 * This could be a known TMX data type, which includes all C and C++
 * native types, string types, STL array types and STL map types.
 * Each of those will see the isTmxType flag to true. This class
 * also supports other object types for consistency with the
 * std::any class. These will also be wrapped in a TMX data type,
 * but isTmxType will be false.
 *
 * @param[in] The value to use
 * @return The Any value
 */
template <class _Tp>
typename std::enable_if<IsTmxType<_Tp>::value, Any>::type make_any(_Tp &&value) noexcept {
    return { TmxTypeOf<_Tp>(std::forward<_Tp>(value)) };
}

/*!
 * @brief Construct an Any type from some reference
 *
 * This could be a known TMX data type, which includes all C and C++
 * native types, string types, STL array types and STL map types.
 * Each of those will see the isTmxType flag to true. This class
 * also supports other object types for consistency with the
 * std::any class. These will also be wrapped in a TMX data type,
 * but isTmxType will be false.
 *
 * Note that this value will refer to the other. Therefore, the
 * referred value must remain at least as long as this object
 * holds a reference to it.
 *
 * @param[in] The value to use
 * @return The Any value
 */
template <class _Tp>
typename std::enable_if<IsTmxType<_Tp>::value, Any>::type make_any(_Tp &value) noexcept {
    return { TmxTypeOf<_Tp>(value) };
}

/*!
 * @brief Construct an Any type from some constant reference
 *
 * This could be a known TMX data type, which includes all C and C++
 * native types, string types, STL array types and STL map types.
 * Each of those will see the isTmxType flag to true. This class
 * also supports other object types for consistency with the
 * std::any class. These will also be wrapped in a TMX data type,
 * but isTmxType will be false.
 *
 * Note that this value will be a copy of the other since the
 * parameter is a constant and thus can not be changed.
 *
 * @param[in] The value to use
 * @return The Any value
 */
template <class _Tp>
typename std::enable_if<!IsTmxType<_Tp>::value, Any>::type make_any(_Tp const &value) noexcept {
    return { value };
}

/*!
 * @brief Construct an Any type from some constant reference
 *
 * This could be a known TMX data type, which includes all C and C++
 * native types, string types, STL array types and STL map types.
 * Each of those will see the isTmxType flag to true. This class
 * also supports other object types for consistency with the
 * std::any class. These will also be wrapped in a TMX data type,
 * but isTmxType will be false.
 *
 * Note that this value will be a copy of the other since the
 * parameter is a constant and thus can not be changed.
 *
 * @param[in] The value to use
 * @return The Any value
 */
template <class _Tp>
typename std::enable_if<IsTmxType<_Tp>::value, Any>::type make_any(_Tp const &value) noexcept {
    return { TmxTypeOf<_Tp>(value) };
}

// Some factories to handle special use cases

/*!
 * @brief Construct an Any type from a C-style string constant
 *
 * @param[in] The string value to use
 * @return The Any value
 */
template <typename _CharT>
Any make_any(const _CharT *value) noexcept {
    return { string_type<_CharT>(value) };
}

template <typename _Tp>
struct _stl_selector {
    typedef typename std::decay<_Tp>::type value_t;
    typedef Array<value_t> type;
};

template <typename _Key, typename _Tp>
struct _stl_selector< std::pair<_Key, _Tp> > {
    typedef typename std::decay<_Key>::type key_t;
    typedef typename std::decay<_Tp>::type value_t;
    typedef Map<key_t, value_t> type;
};

template <typename _Tp>
using stl_select = typename _stl_selector<_Tp>::type;

/*!
 * @brief Construct an Any type from single-value STL container
 *
 * If the value type is a pair, it is assumed to be a name-value
 * for a Map type. Otherwise, an array of that type is created.
 *
 * @param[in] first The beginning of the container
 * @param[in] last The end of the container
 * @return The Any value
 */
template <class InputIterator, typename _Tp = typename InputIterator::value_type>
Any make_any(InputIterator first, InputIterator last) noexcept {
    return { stl_select<_Tp>(first, last) };
}

/*!
 * @brief Construct an Any type from the given array contents
 *
 * @param[in] ilist The array initializer list
 * @return The Any value
 */
template <typename _Tp>
Any make_any(std::initializer_list<_Tp> ilist) noexcept {
    return { Array<_Tp>(ilist) };
}

/*!
 * @brief Construct an Any type from the given map contents
 *
 * @param[in] ilist The map initializer list
 * @return The Any value
 */
template <class _Key, class _Tp>
Any make_any(std::initializer_list<std::pair<_Key, _Tp> > ilist) noexcept {
    return { Map<typename std::decay<_Key>::type, _Tp>(ilist) };
}

/*!
 * @return An empty Any type to represent no data
 */
inline Any &no_data() noexcept {
    static Any _data;
    return _data;
}

template <>
struct TmxType<Any, void> {
    typedef std::true_type TMX;
    typedef Any type;
    static constexpr auto name = type_short_name<type>();
};

template <typename>
struct _map_types;

template <typename... _Tp>
struct _map_types< std::tuple<_Tp...> > {
    typedef std::tuple< Map<_Tp, Any>... > types;
};

template <typename _Tp>
using TmxVariantMapTypes = typename _map_types<_Tp>::types;

/*
 * Combine TMX types to build other use cases
 */
using TmxIntegralTypes = tmx::common::tuple_cat< TmxBooleanTypes, TmxSupportedUIntTypes, TmxSupportedIntTypes >;
using TmxAllIntegralTypes = tmx::common::tuple_cat< TmxBooleanTypes, TmxUIntTypes, TmxIntTypes >;
using TmxArithmeticTypes = tmx::common::tuple_cat< TmxIntegralTypes, TmxFloatTypes >;
using TmxAllArithmeticTypes = tmx::common::tuple_cat< TmxAllIntegralTypes, TmxFloatTypes >;
using TmxScalarTypes = tmx::common::tuple_cat< TmxNullTypes, TmxArithmeticTypes, TmxEnumTypes >;
using TmxAllScalarTypes = tmx::common::tuple_cat< TmxNullTypes, TmxAllArithmeticTypes, TmxEnumTypes >;
using TmxSimpleTypes = tmx::common::tuple_cat< TmxScalarTypes, TmxStringTypes >;
using TmxAllSimpleTypes = tmx::common::tuple_cat< TmxAllScalarTypes, TmxStringTypes >;

using TmxBasicArrayTypes = std::tuple< Array<Any> >;
using TmxAllArrayTypes = tmx::common::tuple_cat< TmxBasicArrayTypes, TmxArrayTypes<TmxSimpleTypes> >;
using TmxBasicPropertiesTypes = TmxVariantMapTypes< TmxStringTypes >;
using TmxEnumeratedPropertiesTypes = TmxVariantMapTypes< TmxEnumTypes >;
using TmxAllPropertiesTypes = tmx::common::tuple_cat< TmxBasicPropertiesTypes, TmxEnumeratedPropertiesTypes >;
using TmxBasicComplexTypes = tmx::common::tuple_cat< TmxBasicArrayTypes, TmxBasicPropertiesTypes >;
using TmxAllComplexTypes = tmx::common::tuple_cat< TmxAllArrayTypes, TmxAllPropertiesTypes >;

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TYPES_ANY_HPP_ */
