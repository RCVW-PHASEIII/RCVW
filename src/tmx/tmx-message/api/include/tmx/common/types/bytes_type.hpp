/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file bytes_type.hpp
 *
 *  Created on: Jun 17, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_TMX_COMMON_TYPES_BYTES_TYPE_HPP_
#define SRC_TMX_COMMON_TYPES_BYTES_TYPE_HPP_

#include <tmx/common/types/support/byte_order.hpp>
#include <tmx/common/types/support/introspect.hpp>
#include <tmx/common/types/enum_type.hpp>
#include <tmx/common/types/int_type.hpp>
#include <tmx/common/types/float_type.hpp>
#include <tmx/common/types/string_type.hpp>

#include <array>
#include <algorithm>
#include <memory>
#include <iterator>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace tmx {
namespace common {
namespace types {

// Some type aliases that may be used

/*!
 * @brief A sequence of bytes of the given type
 */
template <typename _Byte>
using byte_sequence = std::basic_string_view<_Byte, std::char_traits<_Byte> >;

/*!
 * @brief A string of characters of the given type
 */
template <typename _Byte>
using byte_string = std::basic_string<_Byte, std::char_traits<_Byte> >;

/*!
 * @brief A stream of characters of the given type
 */
template <typename _Byte>
using byte_stream = std::vector<_Byte>;

/*!
 * @brief A fixed array of characters of the given type
 */
template <typename _Byte, std::size_t _Sz>
using byte_array = std::array<_Byte, _Sz>;

/*!
 * @brief A class for holding a sequence of bytes.
 *
 * By default, a std::byte is used for the byte type, but this class is capable
 * of supporting a byte size of any length or type.
 *
 * The byte order of the individual character types must be specified in order
 * to maintain consistency. There is no implicit byte order conversion in this
 * class, because no implicit copy of the data is permitted in this class. However,
 * there are some converter functions that must be explicitly called to handle this.
 * The default is always to use network byte order since the bytes may be
 * transmitted.
 */
template <typename _Byte = std::byte, support::byte_order _E = TMX_NETWORK_BYTE_ORDER>
class bytes_type: public byte_sequence<_Byte> {
	static constexpr auto _base = TMX_CHAR_TUPLE_STRING("bytes");
	static constexpr auto _lt = TMX_CHAR_TUPLE_STRING("<");
	static constexpr auto _gt = TMX_CHAR_TUPLE_STRING(">");
	static constexpr auto _concat = support::concat(TMX_STATIC_STRING("Bytes"),
			TMX_STATIC_STRING_WRAP('<', support::make_string<support::bit_sizeof<_Byte>()>(), '>'));

public:
	typedef bytes_type<_Byte, _E> self_type;
	typedef _Byte byte_type;
	typedef UInt<support::bit_sizeof<_Byte>()> byte_int_type;

	/*!
	 * @brief This is a sequence of bytes
	 */
	typedef byte_sequence<_Byte> type;

	/*!
	 * @brief The name of this type depends on the byte size
	 */
	static constexpr auto name = _concat.c_str();

	/*!
	 * @brief The byte order for this type
	 */
	static constexpr auto byte_order = _E;

	// Unlike the default string_view, this class has a number
	// of constructors and assignment operators to facilitate different
	// use cases

	/*!
	 * @brief Construct an empty byte sequence
	 */
	constexpr bytes_type() noexcept { }

	/*!
	 * @brief Create a byte sequence from an existing byte_type
	 *
	 * Note that this does not copy the data
	 *
	 * @param[in] other The existing byte sequence
	 */
	constexpr bytes_type(const self_type &other) noexcept:
		byte_sequence<_Byte>(other) { }

	/*!
	 * @brief Create a byte sequence from an existing array of bytes
	 *
	 * Note that this does not copy any data
	 *
	 * @param[in] bytes A pointer to the existing byte sequence array
	 * @param[in] length The number of bytes in the array
	 */
	constexpr bytes_type(const byte_type *bytes, std::size_t length):
		byte_sequence<_Byte>(bytes, length) { }

	/*!
	 * @brief Create a byte sequence from an existing array of unsigned integers
	 *
	 * Note that this does not copy any data
	 *
	 * @param[in] bytes A pointer to the existing unsigned integer array
	 * @param[in] length The number of bytes in the array
	 */
	constexpr bytes_type(const byte_int_type *bytes, std::size_t length):
		self_type((const byte_type *)bytes, length) { }

	/*!
	 * @brief Create a byte sequence from an existing array of integers
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes A pointer to the existing integer array
	 * @param[in] length The number of integers in the array
	 */
	template <typename _OtherByte>
	constexpr bytes_type(const _OtherByte *bytes, std::size_t length):
		self_type((const byte_int_type *)bytes, length * support::byte_sizeof<_OtherByte>()) { }

	/*!
	 * @brief Create a byte sequence from an existing sequence of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte sequence
	 * @see byte_sequence
	 */
	template <typename _OtherByte>
	bytes_type(const byte_sequence<_OtherByte> &bytes):
		self_type(bytes.data(), bytes.length()) { }

	/*!
	 * @brief Create a byte sequence from an existing string of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte string
	 * @see byte_string
	 */
	bytes_type(const byte_string<byte_type> &bytes):
		self_type((byte_sequence<byte_type>)bytes) { }

	/*!
	 * @brief Create a byte sequence from an existing stream of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte stream
	 * @see byte_stream
	 */
	bytes_type(const byte_stream<byte_type> &bytes):
		self_type(bytes.data(), bytes.size()) { }

	/*!
	 * @brief Create a byte sequence from an existing array of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing array of bytes
	 */
	template <std::size_t _Sz>
	bytes_type(const byte_array<byte_type, _Sz> &bytes):
		self_type(bytes.data(), bytes.size()) { }


	using type::operator =;
	using type::operator [];

	/*!
	 * @brief Assign a new byte sequence from an existing sequence of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte sequence
	 * @see byte_sequence
	 */
	template <typename _OtherByte>
	constexpr self_type &operator=(byte_sequence<_OtherByte> &&bytes) {
		type other { (const byte_int_type *)bytes.data, bytes.length * support::byte_sizeof<_OtherByte>() };
		this->operator=(other);
		return *this;
	}

	/*!
	 * @brief Assign a new byte sequence from an existing sequence of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte sequence
	 * @see byte_sequence
	 */
	template <typename _OtherByte>
	constexpr self_type &operator=(const byte_sequence<_OtherByte> &bytes) {
		return this->operator=(std::move(bytes));
	}

	/*!
	 * @brief Assign a new byte sequence from an existing string of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte string
	 * @see byte_string
	 */
	template <typename _OtherByte>
	self_type &operator=(const byte_string<_OtherByte> &bytes) {
		self_type other { bytes.c_str(), bytes.length() };
		this->operator=(other);
		return *this;
	}

	/*!
	 * @brief Assign a new byte sequence from an existing stream of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing byte stream
	 * @see byte_stream
	 */
	template <typename _OtherByte>
	self_type &operator=(const byte_stream<_OtherByte> &bytes) {
		self_type other { bytes.data(), bytes.size() };
		this->operator=(other);
		return *this;
	}

	/*!
	 * @brief Assign a new byte sequence from an existing array of bytes
	 *
	 * Note that this does not copy any data, and assumes the incoming bytes
	 * are already in the correct byte order.
	 *
	 * @param[in] bytes The existing array of bytes
	 */
	template <typename _OtherByte, std::size_t _Sz>
	self_type &operator=(const byte_array<_OtherByte, _Sz> &bytes) {
		self_type other { bytes.data(), bytes.size() };
		operator=(other);
		return *this;
	}

	/*!
	 * @return The byte sequence
	 */
	constexpr operator type () const {
		return static_cast<type &>(*this);
	}

	/*!
	 * @brief Return a copy of the byte sequence as a string, so it can be printed.
	 *
	 * @return A copy of the byte sequence as a printable string
	 * @see byte_string
	 */
	auto to_string() const {
		return byte_string<byte_type>(this->data(), this->size());
	}

	/*!
	 * @brief Return a copy of the byte sequence as a string, so it can be printed.
	 *
	 * @return A copy of the byte sequence as a printable string
	 * @see byte_string
	 */
	auto to_byte_string() const {
		return this->to_string();
	}

	/*!
	 * @brief Return a copy of the byte sequence as a stream, so it may be manipulated
	 *
	 * Note that this assumes that the byte ordering is already set as desired. In other
	 * words, if the bytes are to be used internally to this program, then no consideration
	 * for the ordering is required.  However, if the bytes are to be transmitted, then it
	 * is required that the data be in network byte order.
	 *
	 * @return A copy of the byte sequence as a stream
	 * @see byte_stream
	 */
	auto to_bytes() const {
		return byte_stream<byte_type>(this->begin(), this->end());
	}

	/*!
	 * @brief Return a copy of the byte sequence as a stream, so it may be manipulated
	 *
	 * Note that this assumes that the byte ordering is already set as desired. In other
	 * words, if the bytes are to be used internally to this program, then no consideration
	 * for the ordering is required.  However, if the bytes are to be transmitted, then it
	 * is required that the data be in network byte order.
	 *
	 * @return A copy of the byte sequence as a stream
	 * @see byte_stream
	 */
	auto to_byte_stream() const {
		return this->to_bytes();
	}

	/*!
	 * @brief Return a copy of the byte sequence as an array, so it may be manipulated.
	 *
	 * The size of the array buffer must be known ahead of time.
	 *
	 * Note that this assumes that the byte ordering is already set as desired. In other
	 * words, if the bytes are to be used internally to this program, then no consideration
	 * for the ordering is required.  However, if the bytes are to be transmitted, then it
	 * is required that the data be in network byte order.
	 *
	 * @return A copy of the byte sequence as an array
	 * @see byte_array
	 */
	template <std::size_t _Sz>
	auto to_array() const {
		byte_array<byte_type, _Sz> out;
		std::copy_n(this->begin(), _Sz, std::back_inserter(out));
		return out;
	}
};

using byte_type = typename bytes_type<>::byte_type;

template <typename _Byte = byte_type>
using little_endian_bytes_type = bytes_type<_Byte, TMX_LITTLE_ENDIAN>;

template <typename _Byte = byte_type>
using big_endian_bytes_type    = bytes_type<_Byte, TMX_BIG_ENDIAN>;

template <typename _Byte = byte_type>
using host_bytes_type = bytes_type<_Byte, support::get_byte_order()>;

template <typename _Byte = byte_type>
using net_bytes_type  = bytes_type<_Byte, TMX_NETWORK_BYTE_ORDER>;

// Transform operations

/*!
 * @brief Convert the supplied byte sequence in the host byte order to network byte order
 *
 * Note that because the byte sequence type never performs a copy of data, this
 * function must return its own copied data, which is captured in a byte stream type.
 * The returned string, presuming it is not destroyed, is implicitly convertible
 * into a bytes_type.
 *
 * @return A copy of the byte sequence in network byte order
 * @see byte_string
 */
template <typename _Byte = byte_type, typename _OtherByte>
static inline byte_stream<_Byte> hton_bytes(const host_bytes_type<_OtherByte> &other) {
	// If the host byte order is already network byte order,
	if (other.byte_order == net_bytes_type<_Byte>::byte_order)
		return other.to_byte_stream();

	// This creates a copy
	byte_stream<_OtherByte> _stream = other.to_byte_stream();
	for (std::size_t i = 0; sizeof(_OtherByte) > 1 && i < _stream.size(); i++) {
	//	if (support::bit_sizeof<_OtherByte>() > 16)
	//		_stream[i] = ::htonl((uint32::type)_stream[i]);
	//	else
	//		_stream[i] = ::htons((uint16::type)_stream[i]);
	}

	// Have to copy again to a byte string type
	return net_bytes_type<_Byte>(_stream.data(), _stream.size()).to_byte_stream();
}

/*!
 * @brief Convert the supplied byte sequence in network byte order to the host byte order
 *
 * Note that because the byte sequence type never performs a copy of data, this
 * function must return its own copied data, which is captured in a byte stream type.
 * The returned string, presuming it is not destroyed, is implicitly convertible
 * into a bytes_type.
 *
 * @return A copy of the byte sequence in host byte order
 * @see byte_string
 */
template <typename _Byte = byte_type, typename _OtherByte>
static inline byte_stream<_Byte> ntoh_bytes(const net_bytes_type<_OtherByte> &other) {
	// If the host byte order is already network byte order,
	if (other.byte_order == host_bytes_type<_Byte>::byte_order)
		return other.to_byte_stream();

	// This creates a copy
	byte_stream<_OtherByte> _stream = other.to_byte_stream();
	for (std::size_t i = 0; i < _stream.size(); i++) {
	//	if (support::bit_sizeof<_OtherByte>() > 16)
	//		_stream[i] = (_OtherByte)::ntohl((uint32::type)_stream[i]);
	//	else
	//		_stream[i] = (_OtherByte)::ntohs((uint16::type)_stream[i]);
	}

	// Have to copy again to a byte string type
	return host_bytes_type<_Byte>(_stream.data(), _stream.size()).to_byte_stream();
}

// Serialization operators

template <typename _Byte, support::byte_order _E>
std::ostream &operator<<(std::ostream &os, const bytes_type<_Byte, _E> &bytes) {
	os << bytes.to_string();
	return os;
}

template <typename _Byte, support::byte_order _E>
std::istream &operator>>(std::istream &is, bytes_type<_Byte, _E> &bytes) {
	std::string input;
	is >> input;
	bytes = input;
	return is;
}

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */


#endif /* SRC_TMX_COMMON_TYPES_BYTES_TYPE_HPP_ */
