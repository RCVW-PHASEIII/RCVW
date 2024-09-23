/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBaseNEncoder.cpp
 *
 *  Created on: Jul 19, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_BYTE_STRING_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_BYTE_STRING_HPP_

#include <tmx/common/platform/iteration/cursor.hpp>
#include <tmx/common/platform/iteration/sequence.hpp>
#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/arrays.hpp>

#ifndef TMX_DEFAULT_BYTE_ENCODING
#define TMX_DEFAULT_BYTE_ENCODING tmx::common::hexadecimal::value
#endif

namespace tmx {
namespace common {

template <std::uint8_t _Sz>
struct _log2_t: public std::integral_constant<std::uint8_t, 1 + _log2_t<_Sz / 2>::value> { };

template <>
struct _log2_t<1>: public std::integral_constant<std::uint8_t, 0> { };

template <class _C>
struct byte_encoder_traits;

template <typename _Tp, _Tp ... _Vals>
struct byte_encoder_traits< static_array<_Tp, _Vals...> > {
    typedef static_array<_Tp, _Vals...> type;
    typedef typename type::value_type char_type;

    static constexpr std::uint16_t base = type::size;
    static_assert(base <= 256, "byte_encoder_traits works only up to an 8-bit type");

    static constexpr std::uint16_t mask = base - 1;
    static constexpr std::uint16_t bits = _log2_t<base>::value;

    typedef std::ratio<1, 8> eighth;
    typedef typename std::ratio_multiply<eighth, std::ratio<bits, 1> >::type bytes_per_char;

    static constexpr std::uint16_t bytes = bytes_per_char::num;
    static constexpr std::uint16_t chars = bytes_per_char::den;

    static constexpr char_type padding = (bytes == 1 ? '\0' : '=');

    static constexpr auto decode(_Tp val) {
        return val - type::array.at(0);
    }
};

template <std::uint8_t _N>
struct _base_N;

template <>
struct _base_N<2> {
    static constexpr auto name = "base2";
    static constexpr auto value = tmx::common::make_array(tmx::common::make_integer_sequence<char, 2, '0'>());
};

typedef _base_N<2> binary;
typedef _base_N<2> base2;

template <>
struct _base_N<8> {
    static constexpr auto name = "base8";
    static constexpr auto value = tmx::common::make_array(tmx::common::make_integer_sequence<char, 8, '0'>());
};

typedef _base_N<8> octal;
typedef _base_N<8> base8;

template <>
struct _base_N<16> {
    static constexpr auto name = "base16";
    static constexpr auto value = tmx::common::concat(tmx::common::numeric_characters { },
                                                      tmx::common::make_array(tmx::common::make_integer_sequence<char, 6, 'A'>()));
};

typedef _base_N<16> hexadecimal;
typedef _base_N<16> base16;

template <>
constexpr auto byte_encoder_traits<typename std::decay<decltype(base16::value)>::type>::decode(char val) {
    return val - type::array.at(0) + (val > '9' ? '9' + 1 - 'A': 0);
}

template <>
struct _base_N<32> {
    static constexpr auto name = "base32";
    static constexpr auto value = tmx::common::concat(tmx::common::uppercase_characters { },
                                                      tmx::common::make_array(tmx::common::make_integer_sequence<char, 6, '2'>()));
};

typedef _base_N<32> base32;

template <>
constexpr auto byte_encoder_traits<typename std::decay<decltype(base32::value)>::type>::decode(char val) {
    return val - type::array.at(0) + (val < 'A' ? 'Z' + 1 - '2': 0);
}

template <>
struct _base_N<64> {
    static constexpr auto name = "base64";
    static constexpr auto value = tmx::common::concat(tmx::common::concat(tmx::common::uppercase_characters { },
                                                                          tmx::common::lowercase_characters { }),
                                                      tmx::common::concat(tmx::common::numeric_characters { },
                                                                          static_array<char, '+', '/'> { }));
};

typedef _base_N<64> base64;

template <>
constexpr auto byte_encoder_traits<typename std::decay<decltype(base64::value)>::type>::decode(char val) {
    if (val == type::array.at(63))
        return 63;
    else if (val == type::array.at(62))
        return 62;

    return val - type::array.at(0) + (val > 'Z' ? 'Z' + 1 - 'a': 0) + (val < 'A' ? 'A' + 52 - '0': 0);
}

template <typename _CharT, typename _Iter, typename _IterTraits = std::iterator_traits<_Iter>, _CharT ... _Map>
std::enable_if_t<std::is_same_v<typename _IterTraits::value_type, byte_t>, std::basic_ostream<_CharT> &>
byte_string_encode(std::basic_ostream<_CharT> &os, _Iter iter, static_array<_CharT, _Map...> const &) noexcept {
    typedef byte_encoder_traits< static_array<_CharT, _Map...> > traits;

    std::uint64_t value = 0;
    std::size_t byteCnt = 0;
    for (const byte_t &byte: iter) {
        value <<= TMX_BITS_PER_BYTE;
        value |= (unsigned char)(byte);

        byteCnt++;

        if (byteCnt % traits::bytes == 0) {
            for (std::size_t i = traits::chars; i > 0; i--)
                os << traits::type::array.at((value >> (traits::bits * (i - 1))) & traits::mask);

            value = 0;
        }
    }

    auto remainder = (byteCnt % traits::bytes == 0 ? 0 : traits::bytes - (byteCnt % traits::bytes));
    if (remainder) {
        value <<= (8 * remainder);
        for (std::size_t i = traits::chars; i > remainder; i--)
            os << traits::type::array.at((value >> (traits::bits * (i - 1))) & traits::mask);

        // Add the padding
        for (std::size_t i = 0; i < remainder; i++)
            os << traits::padding;
    }

    return os;
}

template <typename _CharT, _CharT ... _Map>
std::basic_ostream<_CharT> &byte_string_encode(std::basic_ostream<_CharT> &os, byte_sequence const &bytes,
                                               static_array<_CharT, _Map...> const &m) noexcept {
    return byte_string_encode(os, make_byte_cursor(bytes), m);
}

template <typename _CharT = char_t, _CharT ... _Map>
std::basic_string<_CharT> byte_string_encode(byte_sequence const &bytes,
                                             static_array<_CharT, _Map...> const &m) noexcept {
    std::basic_ostringstream<_CharT> ss;
    byte_string_encode(ss, bytes, m);
    return ss.str();
}

template <typename _CharT = char_t, _CharT ... _Map>
std::basic_string<_CharT> byte_string_encode(byte_sequence const &bytes) noexcept {
    return byte_string_encode(bytes, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT, typename _Iter, typename _IterTraits = std::iterator_traits<_Iter> >
std::enable_if_t<std::is_same_v<typename _IterTraits::value_type, byte_t>, std::basic_ostream<_CharT> &>
operator<<(std::basic_ostream<_CharT> &os, _Iter iter) noexcept {
    return byte_string_encode(os, iter, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT>
std::basic_ostream<_CharT> &operator<<(std::basic_ostream<_CharT> &os, byte_sequence const &bytes) noexcept {
    return byte_string_encode(os, make_byte_cursor(bytes), TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT = char_t, typename _Tp, _CharT ... _Map>
enable_arithmetic<_Tp, std::basic_string<_CharT> >
byte_string_encode_value(_Tp const &value, static_array<_CharT, _Map...> const &m) noexcept {
    std::basic_ostringstream<_CharT> ss;
    byte_string_encode(ss, make_byte_cursor(value), m);
    return ss.str();
}

template <typename _CharT = char_t, typename _Tp>
enable_arithmetic<_Tp, std::basic_string<_CharT> > byte_string_encode_value(_Tp const &value) noexcept {
    return byte_string_encode_value(value, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT, _CharT ... _Map>
std::basic_istream<_CharT> &byte_string_decode(std::basic_istream<_CharT> &is, std::basic_string<byte_t> &bytes,
                                               static_array<_CharT, _Map...> const &) noexcept {
    typedef byte_encoder_traits< static_array<_CharT, _Map...> > traits;

    std::basic_string<_CharT> str(std::istreambuf_iterator<_CharT>(is), { });

    std::uint64_t value = 0;
    std::size_t charCnt = 0;
    for (auto iter = str.begin(); iter != str.end(); iter++) {
        value <<= traits::bits;
        if (*iter != traits::padding) {
            // Special case for HEX, in that the characters can be upper or lowercase
            if (traits::base == 16)
                value |= (unsigned char)(traits::decode(std::toupper(*iter)));
            else
                value |= (unsigned char)(traits::decode(*iter));
        }


        charCnt++;

        if (charCnt % traits::chars == 0) {
            std::size_t i = 0;
            for (const auto &byte: make_byte_cursor(value)) {
                if (i++ < sizeof(value) - traits::bytes) continue;
                bytes.push_back((byte_t) byte);
            }

            value = 0;
        }
    }

    return is;
}

template <typename _CharT, _CharT ... _Map>
std::basic_istream<_CharT> &byte_string_decode(std::basic_istream<_CharT> &is, std::basic_string<byte_t> &bytes) noexcept {
    return byte_string_decode(is, bytes, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT, _CharT ... _Map>
auto byte_string_decode(char_sequence<_CharT> const &str, static_array<_CharT, _Map...> const &m) noexcept {
    std::basic_string<byte_t> bytes;
    std::basic_istringstream<_CharT> ss(str.data());
    byte_string_decode(ss, bytes, m);
    return bytes;
}

template <typename _CharT>
auto byte_string_decode(char_sequence<_CharT> const &str) noexcept {
return byte_string_decode(str, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT, _CharT ... _Map>
auto byte_string_decode(const _CharT *str, static_array<_CharT, _Map...> const &m) noexcept {
    return byte_string_decode(char_sequence<_CharT> { str }, m);
}

template <typename _CharT>
auto byte_string_decode(const _CharT *str) noexcept {
    return byte_string_decode(str, TMX_DEFAULT_BYTE_ENCODING);
}

template <typename _CharT, _CharT ... _Map>
auto byte_string_decode(std::basic_string<_CharT> const &str, static_array<_CharT, _Map...> const &m) noexcept {
    return byte_string_decode(str.c_str(), m);
}

template <typename _CharT>
auto byte_string_decode(std::basic_string<_CharT> const &str) noexcept {
    return byte_string_decode(str.c_str());
}

template <typename _CharT>
std::basic_istream<_CharT> &operator>>(std::basic_istream<_CharT> &is, std::basic_string<byte_t> &bytes) noexcept {
    return byte_string_decode(is, bytes, TMX_DEFAULT_BYTE_ENCODING);
}

}
}

#endif /* API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_BYTE_STRING_HPP_ */
