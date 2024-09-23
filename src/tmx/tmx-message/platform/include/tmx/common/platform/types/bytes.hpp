/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file byte.hpp
 *
 * This header includes definition and helper structures for
 * creating generic byte sequences from literals. This utilizes
 * a constant string type, which should be equivalent to the
 * C++ 17 string_view class.
 *
 *  Created on: Oct 5, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_BYTES_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_BYTES_HPP_

#include <tmx/common/platform/types/size_of.hpp>

#include <cstdint>
#include <cstddef>
#include <istream>
#include <ostream>
#include <type_traits>

#ifndef TMX_CONST_STRING
#include <string_view>
#define TMX_CONST_STRING std::basic_string_view
#endif

/*!
 * Some general pre-processor macros
 */
#define TMX_CPP_Q(X) #X
#define TMX_CPP_QUOTE(X) TMX_CPP_Q(X)
#define TMX_CPP_CAT(X, Y) X ## Y
#define TMX_CPP_CONCAT(X, Y) TMX_CPP_CAT(X, Y)
#define TMX_CPP_PREFIX_(X) _ ## X
#define TMX_CPP_POSTFIX_(X) X ## _
#define TMX_CPP_PREFIX_UNDERSCORE(X) TMX_CPP_PREFIX_(X)
#define TMX_CPP_POSTFIX_UNDERSCORE(X) TMX_CPP_POSTFIX_(X)

#ifndef TMX_BYTE_TYPE
#define TMX_BYTE_TYPE std::byte
#endif

#ifndef TMX_CHAR_TYPE
#define TMX_CHAR_TYPE char
#endif

namespace tmx {
namespace common {

/*!
 * @brief The default byte type for C++
 */
typedef TMX_BYTE_TYPE byte_t;

/*!
 * @brief A compiler check that we are using an actual byte
 */
static_assert(tmx::common::byte_size_of<byte_t>() == 1 &&
			  tmx::common::bit_size_of <byte_t>() == TMX_BITS_PER_BYTE,
		"The TMX_BYTE_TYPE must be precisely one byte");

/*!
 * @brief A byte sequence with the default byte type
 *
 * A byte sequence is just a structure wrapped around a constant
 * array of bytes. Therefore, unlike a typical string, the
 * size cannot grow and nor the bytes replaced. But, a key use
 * case for compile-time operations and constexpr evaluation.
 * Therefore, this structure must implement some key features
 * assumed for TMX.
 *
 * First, because the constant array of bytes may be pre-built
 * at compile time, this structure must have constexpr
 * constructors that accept a constant byte pointer.

 * Additionally, it must be able to determine the length of the
 * array at compile-time.
 *
 * Finally, it must implement some compile-time search and
 * substring operations.
 *
 * The basic traits for this structure is based on the
 * std::basic_string_view class added in C++ 17, so that API is
 * a safe one to use if building your own viiew. None of these
 * features are actually verified in this header, however.
 *
 * @see https://en.cppreference.com/w/cpp/string/basic_string_view
 */
template <typename _ByteT = byte_t>
using byte_sequence_t = TMX_CONST_STRING<_ByteT>;

/*!
 * @brief A byte sequence with a default byte type
 */
typedef byte_sequence_t<> byte_sequence;

/*!
 * @brief A character sequence is another byte sequence using characters
 */
template <typename _CharT = TMX_CHAR_TYPE>
using char_sequence = byte_sequence_t<_CharT>;

/*!
 * @brief A byte sequence with the default character type
 *
 * @see https://en.cppreference.com/w/cpp/string/basic_string_view
 */
typedef char_sequence<> const_string;

/*!
 * @brief The byte sequence with the C unsigned character type
 */
typedef char_sequence< std::make_unsigned_t<TMX_CHAR_TYPE> > uconst_string;

/*!
 * @brief A shortcut to the default character type
 */
typedef typename const_string::value_type char_t;

/*!
 * @brief A shortcut to the default unsigned character type
 */
typedef typename uconst_string::value_type uchar_t;

constexpr const_string empty_string() noexcept {
	constexpr const_string _empty { "" };
	return _empty;
}

/*!
 * @brief Convert a character sequence to a byte sequence
 *
 * This does not copy the characters, and assumes each byte is in the
 * correct byte order.
 *
 * @return A byte sequence view of the character sequence
 */
template <typename _CharT>
constexpr byte_sequence to_byte_sequence(byte_sequence_t<_CharT> const &seq) noexcept {
	constexpr auto charSz = byte_size_of<_CharT>();
	return byte_sequence((const byte_t *)seq.data(), seq.length() * charSz);
}

/*!
 * @brief Convert a string literal to a byte sequence
 *
 * This does not copy the characters, and assumes each byte is in the
 * correct byte order.
 *
 * @return A byte sequence view of the string literal
 */
template <typename _CharT>
constexpr byte_sequence to_byte_sequence(const _CharT *seq) noexcept {
	return to_byte_sequence(byte_sequence_t<_CharT>(seq));
}

/*!
 * @brief Convert a string literal to a byte sequence
 *
 * This does not copy the characters, and assumes each byte is in the
 * correct byte order.
 *
 * @return A byte sequence view of the string literal
 */
template <typename _CharT, typename _T>
constexpr byte_sequence to_byte_sequence(const _CharT *seq, _T len) noexcept {
    return to_byte_sequence(byte_sequence_t<_CharT>(seq, len));
}

/*!
 * @brief Convert one character sequence to another
 *
 * This does not copy the characters, and assumes each byte is in the
 * correct byte order.
 *
 * @return The character sequence view of the other sequence
 */
template <typename _CharT = char, typename _ByteT>
constexpr byte_sequence_t<_CharT> to_char_sequence(byte_sequence_t<_ByteT> const &seq) noexcept {
	constexpr auto charSz = byte_size_of<_CharT>();
	constexpr auto seqSz = byte_size_of<_ByteT>();
	return byte_sequence_t<_CharT>((const _CharT *)seq.data(), seq.length() * seqSz / charSz);
}

template <typename _CharT = char, typename _ByteT>
constexpr byte_sequence_t<_CharT> to_char_sequence(const _ByteT *_seq) noexcept {
    return to_char_sequence<_CharT>(byte_sequence_t<_ByteT> { _seq });
}

template <typename _CharT = char, typename _ByteT>
constexpr byte_sequence_t<_CharT> to_char_sequence(const _ByteT *_seq, std::size_t len) noexcept {
    return to_char_sequence(byte_sequence_t<_ByteT> { _seq, len });
}

/*!
 * @brief Returns the number of words in the character sequence
 *
 * A word can be defined as any number of characters. The default is 1.
 *
 * @tparam _Sz Thw word size to use
 * @tparam _CharT The size of the character used in the sequence
 * @param seq The character sequence
 * @return The number of words in the sequence
 */
template <std::uint8_t _Sz = byte_size_of<byte_t>(), typename _CharT>
constexpr std::size_t num_words(byte_sequence_t<_CharT> const &seq) noexcept {
    return seq.size() / _Sz + (seq.size() % _Sz ? 1 : 0);
}

/*!
 * @brief Returns the Nth word in the character sequence
 *
 * A word can be defined as any number of characters. The default is 1.
 *
 * @tparam _Sz The word size to use
 * @tparam _CharT The size of the character used in the sequence
 * @param N The 0-based index of the word to get
 * @param seq The character sequence
 * @return The first word of the sequence
 */
template <std::uint8_t _Sz = byte_size_of<byte_t>(), typename _CharT>
constexpr byte_sequence_t<_CharT> nth_word(std::size_t N, byte_sequence_t<_CharT> const &seq) noexcept {
    auto max = num_words<_Sz>(seq);
    if (N == max)
        return seq.substr(N * _Sz);
    else if (N < max)
        return seq.substr(N * _Sz, _Sz);
    else
        return { };
}

/*!
 * @brief Returns the first word in the character sequence
 *
 * A word can be defined as any number of characters. The default is 1.
 *
 * @tparam _Sz The word size to use
 * @tparam _CharT The size of the character used in the sequence
 * @param seq The character sequence
 * @return The first word of the sequence
 */
template <std::uint8_t _Sz = byte_size_of<byte_t>(), typename _CharT>
constexpr byte_sequence_t<_CharT> first_word(byte_sequence_t<_CharT> const &seq) noexcept {
    return nth_word<_Sz>(0, seq);
}

/*!
 * @brief Returns the last word in the character sequence
 *
 * A word can be defined as any number of characters. The default is 1.
 *
 * @tparam _Sz The word size to use
 * @tparam _CharT The size of the character used in the sequence
 * @param seq The character sequence
 * @return The last word of the sequence
 */
template <std::uint8_t _Sz = byte_size_of<byte_t>(), typename _CharT>
constexpr byte_sequence_t<_CharT> last_word(byte_sequence_t<_CharT> const &seq) noexcept {
    return nth_word<_Sz>(num_words<_Sz>(seq) - 1, seq);
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_BYTES_HPP_ */
