/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxData.cpp
 *
 *  Created on: May 10, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/message/TmxData.hpp>

#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <sstream>
#include <type_traits>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {

template <typename _Ret>
enable_arithmetic<_Ret> scalar_handle(Null const &val) {
    return static_cast<_Ret>(0);
}

template <typename _Ret>
enable_arithmetic<_Ret> scalar_handle(Boolean const &val) {
    return static_cast<_Ret>(val ? 0 : 1);
}

template <typename _Ret, std::uint8_t _Sz>
enable_arithmetic<_Ret> scalar_handle(UInt<_Sz> const &val) {
    return static_cast<_Ret>((typename UInt<_Sz>::value_type)val);
}

template <typename _Ret, std::uint8_t _Sz>
enable_arithmetic<_Ret> scalar_handle(Int<_Sz> const &val) {
    return static_cast<_Ret>((typename Int<_Sz>::value_type)val);
}

template <typename _Ret, std::uint8_t _Sz>
enable_arithmetic<_Ret> scalar_handle(Float<_Sz> const &val) {
    return static_cast<_Ret>((typename Float<_Sz>::value_type)val);
}

template <typename _Ret, typename _E>
enable_arithmetic<_Ret> scalar_handle(Enum<_E> const &val) {
    return static_cast<_Ret>(val.get_integer_value());
}

template <typename _Ret, typename _T>
enable_arithmetic<_Ret> scalar_handle(Array<_T> const &val) {
    return static_cast<_Ret>(val.size());
}

template <typename _Ret, typename _Key, typename _T>
enable_arithmetic<_Ret> scalar_handle(Map<_Key, _T> const &val) {
    return static_cast<_Ret>(val.size());
}

template <typename _Ret, typename _T>
enable_bool<_Ret> scalar_handle(_T const &val) {
    if TMX_CONSTEXPR_FN (std::is_same<_T, Null>::value) {
        return false;
    } else {
        auto v = scalar_handle<typename UIntmax::value_type>(val);
        return v != 0;
    }
}

template <typename _Ret>
enable_bool<_Ret> scalar_handle(String8 const &val) {
    if (boost::iequals(val.c_str(), "false") ||
        boost::iequals(val.c_str(), "no") ||
        boost::iequals(val.c_str(), "off"))
        return false;
    else
        return val.length() ? true : false;
}

template <typename _Ret>
enable_int< disable_bool<_Ret> > scalar_handle(String8 const &val) {
    if TMX_CONSTEXPR_FN (std::is_signed<TmxValueTypeOf<_Ret> >::value) {
        if TMX_CONSTEXPR_FN (TmxTypeOf<_Ret>::numBits < 64)
            return ::strtol(val.c_str(), NULL, 0);
        else
            return ::strtoll(val.c_str(), NULL, 0);
    } else {
        if TMX_CONSTEXPR_FN (TmxTypeOf<_Ret>::numBits < 64)
            return ::strtoul(val.c_str(), NULL, 0);
        else
            return ::strtoull(val.c_str(), NULL, 0);
    }
}

template <typename _Tp>
enable_float<_Tp> scalar_handle(String8 const &val) {
    if TMX_CONSTEXPR_FN (std::is_same<Floatmax, Float<32> >::value)
        return ::strtof(val.c_str(), NULL);
    else if TMX_CONSTEXPR_FN (std::is_same<Floatmax, Float<64> >::value)
        return ::strtod(val.c_str(), NULL);
    else
        return ::strtold(val.c_str(), NULL);
}

template <typename _Tp, typename _T>
enable_string<_Tp> scalar_handle(_T const &val) {
    return std::to_string((TmxValueTypeOf<_T>)val);
}

template <typename _Ret>
enable_string<_Ret> scalar_handle(Null const &) {
    return "null";
}

template <typename _Ret>
enable_string<_Ret> scalar_handle(String8 const &val) {
    return val.c_str();
}

template <typename _Ret, typename _E>
enable_string<_Ret> scalar_handle(Enum<_E> const &val) {
    return val.get_enum_name();
}

template <typename _Ret, typename _T>
enable_string<_Ret> scalar_handle(Array<_T> const &val) {
    std::ostringstream os;
    auto enc = codec::TmxEncoder::get_encoder("json");
    if (enc)
        enc->encode(val, os);

    return os.str();
}

template <typename _Ret, typename _Key, typename _T>
enable_string<_Ret> scalar_handle(Map<_Key, _T> const &val) {
    std::ostringstream os;
    auto enc = codec::TmxEncoder::get_encoder("json");
    if (enc)
        enc->encode(val, os);

    return os.str();
}

template <typename _Ret, typename _T>
_Ret scalar_handle(std::reference_wrapper<_T> const &val) {
    return scalar_handle<_Ret>(val.get());
}

template <typename _Handler, typename _Variant, typename _Tp>
TmxError _do_handle_scalar(_Handler const &handler, _Variant &var, TmxTypeDescriptor const &descr, Any const &val, _Tp &out) {
    auto _ret = set_variant(var, descr);
    if (_ret) return _ret;

    TmxArgList args { val };
    out = do_handle(handler, var, args);
    return { };
}

template <class _T>
_T _to_scalar(Any const &container) {
    // Check for direct access
    if (!container.has_value() || container.type() == typeid(Null))
        return scalar_handle<typename _T::value_type>(Null());
    else if (container.type() == typeid(_T))
        return tmx::common::any_cast<_T>(container);
    else if (container.type() == typeid(typename _T::value_type))
        return tmx::common::any_cast<typename _T::value_type>(container);
    else if (container.type() == typeid(std::reference_wrapper<typename _T::value_type>))
        return tmx::common::any_cast<std::reference_wrapper<typename _T::value_type> >(container).get();

    TmxVariant<TmxScalarTypes, String8, TmxBasicComplexTypes> _var1;

    static auto _handler = TmxTypeHandlerOverload {
        [](auto const &val, TmxArgList &args) {
            return scalar_handle<typename _T::value_type>(TmxTypeOf<decltype(val)>(val));
        }
    };

    // TODO: Log error messages
    _T _tmp;
    auto _descr = TmxTypeRegistry().get(container.type(), true);
    if (_descr)
        _do_handle_scalar(_handler, _var1, _descr, container, _tmp);

    // Make sure to make a copy
    return { static_cast<const _T &>(_tmp) };
}

TmxData::operator typename Boolean::value_type() const noexcept {
    return _to_scalar<Boolean>(this->get_container());
}

TmxData::operator typename UInt64::value_type () const noexcept {
    return _to_scalar<UInt64>(this->get_container());
}

TmxData::operator typename UInt32::value_type () const noexcept {
    return _to_scalar<UInt32>(this->get_container());
}

TmxData::operator typename UInt16::value_type () const noexcept {
    return _to_scalar<UInt16>(this->get_container());
}

TmxData::operator typename UInt8::value_type () const noexcept {
    return _to_scalar<UInt8>(this->get_container());
}

TmxData::operator typename Int64::value_type () const noexcept {
    return _to_scalar<Int64>(this->get_container());
}

TmxData::operator typename Int32::value_type () const noexcept {
    return _to_scalar<Int32>(this->get_container());
}

TmxData::operator typename Int16::value_type () const noexcept {
    return _to_scalar<Int16>(this->get_container());
}

TmxData::operator typename Int8::value_type () const noexcept {
    return _to_scalar<Int8>(this->get_container());
}

TmxData::operator typename Float128::value_type () const noexcept {
    return _to_scalar<Float128>(this->get_container());
}

TmxData::operator typename Float64::value_type () const noexcept {
    return _to_scalar<Float64>(this->get_container());
}

TmxData::operator typename Float32::value_type () const noexcept {
    return _to_scalar<Float32>(this->get_container());
}

TmxData::operator typename String8::value_type () const noexcept {
    return _to_scalar<String8>(this->get_container());
}

} /* End namespace message */
} /* End namespace tmx */