/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file chararray_signature_introspect.hpp
 *
 * This file defines the version 1 introspection, but without the
 * benefit of a constexpr pretty printed signature. Instead, this
 * implementation parses the character array directly and converts
 * it piecemeal into a static array of characters.
 *
 *  Created on: Oct 10, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CHARARRAY_SIGNATURE_INTROSPECT_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CHARARRAY_SIGNATURE_INTROSPECT_HPP_

#include <tmx/common/platform/types/arrays.hpp>
#include <tmx/common/platform/types/bytes.hpp>
#include <tmx/common/platform/types/introspect.hpp>

#include <tuple>
#include <utility>

#ifndef TMX_INTROSPECTION_VERSION
#define TMX_INTROSPECTION_VERSION 2
#endif

namespace tmx {
namespace common {

template <typename _T>
class introspection_t<_T, 2> {
public:
	static constexpr const char *name_s = introspection_t<_T, 1>::name_s;
	static constexpr const_string name { introspection_t<_T, 1>::name };

private:
	/*!
	 * @return The character array at the given indexes as a tuple of characters
	 */
	template <std::size_t ... X>
	static constexpr auto as_tuple(std::index_sequence<X...> const &) {
		return std::make_tuple( name.data()[X] ... );
	}

	/*!
	 * @brief Holds the character tuple equivalent of the full name
	 */
	static constexpr auto name_t = as_tuple(std::make_index_sequence<name.length()> {});

	/*!
	 * @brief The block size to split the characters into
	 */
	static constexpr std::size_t block_size = 64;

	/*!
	 * @brief The number of blocks contained in the name
	 */
	static constexpr std::size_t num_blocks = name.length() / block_size +
			(name.length() % block_size ? 1 : 0);

	/*!
	 * @brief Get the character from the tuple at the specified index
	 *
	 * If the size of the tuple is smaller than the index, then
	 * a null character or string terminator is returned.
	 *
	 * @param _I The index to get
	 * @param The Boost::Hana constant equivalent of the character
	 */
	template <std::size_t _I>
	static constexpr auto _get() {
	    if constexpr (_I < name.length())
            return boost::hana::char_c< std::get<_I>(name_t) >;
	    else
            return boost::hana::char_c< '\0' >;
	}

	/*!
	 * @brief Get the next block of characters
	 *
	 * This must be hard-coded to call the _get() function over
	 * and over again.
	 *
	 * @return The Boost::Hana static string equivalent of the block
	 */
	template <std::size_t _I>
	static constexpr auto next_block() {
		return boost::hana::make_string(
				_get<_I+0> (),  _get<_I+1> (),  _get<_I+2> (),  _get<_I+3> (),
				_get<_I+4> (),  _get<_I+5> (),  _get<_I+6> (),  _get<_I+7> (),
				_get<_I+8> (),  _get<_I+9> (),  _get<_I+10>(),  _get<_I+11>(),
				_get<_I+12>(),  _get<_I+13>(),  _get<_I+14>(),  _get<_I+15>(),
				_get<_I+16>(),  _get<_I+17>(),  _get<_I+18>(),  _get<_I+19>(),
				_get<_I+20>(),  _get<_I+21>(),  _get<_I+22>(),  _get<_I+23>(),
				_get<_I+24>(),  _get<_I+25>(),  _get<_I+26>(),  _get<_I+27>(),
				_get<_I+28>(),  _get<_I+29>(),  _get<_I+30>(),  _get<_I+31>(),
				_get<_I+32>(),  _get<_I+33>(),  _get<_I+34>(),  _get<_I+35>(),
				_get<_I+36>(),  _get<_I+37>(),  _get<_I+38>(),  _get<_I+39>(),
				_get<_I+40>(),  _get<_I+41>(),  _get<_I+42>(),  _get<_I+43>(),
				_get<_I+44>(),  _get<_I+45>(),  _get<_I+46>(),  _get<_I+47>(),
				_get<_I+48>(),  _get<_I+49>(),  _get<_I+50>(),  _get<_I+51>(),
				_get<_I+52>(),  _get<_I+53>(),  _get<_I+54>(),  _get<_I+55>(),
				_get<_I+56>(),  _get<_I+57>(),  _get<_I+58>(),  _get<_I+59>(),
				_get<_I+60>(),  _get<_I+61>(),  _get<_I+62>(),  _get<_I+63>()
		);
	}

	/*!
	 * @brief Convert the name to a static array structure
	 *
	 * This is necessary when the name resulting from pretty printing of
	 * the function signature is not a constexpr type. Such a variable
	 * still has the backing of a compile-time character array, but
	 * because that variable is not a constexpr, it cannot directly be
	 * used to generate the TMX (more precisely the Boost::Hana)
	 * static array type. Instead, we have to build up that type
	 * piecemeal at compile time.
	 *
	 * Of course, this function is going to take longer to compile
	 * than if the pretty printed signature was constexpr, but it
	 * does at least create an equivalent structure.
	 *
	 * @return The Boost::Hana static string equivalent of the name
	 */
	template <std::size_t _I = 0, std::size_t _Max = num_blocks>
	static constexpr auto get() {
		constexpr auto block = next_block<_I * block_size>();
		if constexpr (_I == _Max)
			return block;
		else
			return block + get<_I+1, _Max>();
	}

public:
	/*!
	 * @brief Holds the Boost::Hana static string equivalent of the name
	 */
	static constexpr auto str = get();
};

/*!
 * @brief The static character array type equivalent for the name of _T
 *
 * This converts the constant character sequence name into a static array
 * structure of the characters in the string. This works generally for
 * any compile-time generated character arrays, but at the cost of
 * compile speed. Therefore, it will only be utilized if the macro
 * TMX_PRETTY_FUNCTION_NOT_CONSTEXPR is defined.
 *
 * The pretty printed signature function signature will always produce a
 * compile-time character array, but in some compilers the resultant array
 * can NOT be used in a constexpr. Therefore, even though name is backed by
 * a compile-time character array, it can NOT be transformed directly into the
 * static array structures of TMX or Boost::Hana. Instead, this class must convert
 * the entire array to a std::tuple of characters, which are then extracted
 * individually and built piecemeal into the static string. Extra padding
 * characters are added until the correct length is surpassed, and then
 * the string structure is shrunk to its correct size.
 *
 * Since the type of this value is the resulting static character array
 * structure, with the characters of the name encoded in the variadic
 * template, it can be used to concatenate strings at compile time, if
 * necessary.
 *
 * @see #name
 * @see #concat
 */
template <typename _T>
constexpr auto _fqname(introspection_t<_T, 2> const &) noexcept {
	typedef introspection_t<_T, 2> type;
	return make_array(type::str, std::make_index_sequence<type::name.length()> {});
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_IMPL_CHARARRAY_SIGNATURE_INTROSPECT_HPP_ */
