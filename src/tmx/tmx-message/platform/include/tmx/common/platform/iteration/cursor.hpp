/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file cursor.hpp
 *
 *  Created on: Jun 7, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_CURSOR_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_CURSOR_HPP_

#include <tmx/common/platform/endianness/byte_order.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <initializer_list>
#include <iterator>
#include <istream>
#include <iostream>
#include <list>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <utility>

namespace tmx {
namespace common {

/*!
 * @brief A cursor is a circular iterator over some sequence
 *
 * A cursor is a pointer to some specific position within a sequence
 * of items. This class may effectively be implemented as three
 * separate pointers or iterators: one at the beginning of the
 * sequence; one at the end of the sequence; and the current position.
 *
 * A cursor starts at the beginning, but may be moved forward or
 * backward along the sequence, wrapping around in both directions so
 * the current position is always valid within the sequence. The
 * behavior is undefined if the underlying sequence is modified
 * while the cursor is in use.
 *
 * This class was created due to a need to have a common iteration over
 * different containers of sequences, such as the STL array, list and
 * vector. All of these can hold a decent sequence, but each iterator
 * is totally different, thus requiring templates to encapsulate the
 * differences, or using an additional copy from one container to
 * another. Neither option is helpful with the Enum type, for example,
 * because the main point of that class is to have a common representation
 * of enumeration sequences, many of which are already built as compile-
 * time arrays. Therefore, the cursor was conceived to generically
 * hold a position within the sequence.
 *
 */
template <typename _T, class _Alloc = std::allocator<_T> >
class cursor {
public:
	typedef cursor<_T, _Alloc> self_type;

	// A cursor has iterator traits
	typedef typename _Alloc::value_type value_type;
	typedef typename _Alloc::pointer pointer;
	typedef typename _Alloc::reference reference;
	typedef typename _Alloc::const_pointer const_pointer;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type difference_type;

	/*!
	 * @brief Virtual destructor
	 */
	virtual ~cursor() {}

	/*!
	 * @return The distance from the beginning of the sequence to the end
	 */
	virtual size_type size() const noexcept { return 0; }

	/*!
	 * @return The distance of the current position from the beginning of the sequence
	 */
	virtual size_type index() const noexcept { return 0; }

	/*!
	 * @brief The distance of the current position from the end of the sequence
	 */
	virtual size_type rindex() const noexcept { return this->size() - this->index(); }

	/*!
	 * @brief Dereference the current position
	 *
	 * This should always be a safe operation because the current position
	 * is always valid in the sequence.
	 *
	 * @return A constant reference to the value_type stored at the current cursor position
	 */
	virtual const_reference operator*() const noexcept { return front(); }

	/*!
	 * @brief Pre-fix increment the cursor
	 *
	 * If the cursor goes past the end of the sequence, it is
	 * automatically reset to the beginning.
	 *
	 * @return This cursor.
	 */
	virtual self_type &operator++() noexcept { return this->operator+=(1); }

	/*!
	 * @brief Pre-fix decrement the cursor
	 *
	 * If the cursor goes past the beginning of the sequence, it is
	 * automatically reset to the end.
	 *
	 * @return This cursor.
	 */
	virtual self_type &operator--() noexcept { return this->operator-=(1); }

	/*!
	 * @brief Post-fix increment the cursor
	 *
	 * If the cursor goes past the end of the sequence, it is
	 * automatically reset to the beginning.
	 *
	 * @param The amount of increment, which is assumed to be one unless a positive integer is given
	 * @return This cursor.
	 */
	virtual self_type &operator++(int n) noexcept { return this->operator+=(n > 0 ? n : 1); }

	/*!
	 * @brief Post-fix decrement the cursor
	 *
	 * If the cursor goes past the beginning of the sequence, it is
	 * automatically reset to the end.
	 *
	 * @param The amount of decrement, which is assumed to be one unless a positive integer is given
	 * @return This cursor.
	 */
	virtual self_type &operator--(int n) noexcept { return this->operator-=(n > 0 ? n : 1); }

	/*!
	 * @brief Increment the cursor by the specified amount
	 *
	 * If the cursor goes past the end of the sequence, it is
	 * automatically reset to the beginning.
	 *
	 * @param The amount of increment
	 * @return This cursor.
	 */
	virtual self_type &operator+=(int) noexcept { return *this; };

	/*!
	 * @brief Decrement the cursor by the specified amount
	 *
	 * If the cursor goes past the beginning of the sequence, it is
	 * automatically reset to the end.
	 *
	 * @param The amount of decrement
	 * @return This cursor.
	 */
	virtual self_type &operator-=(int) noexcept { return *this; }

	/*!
	 * @brief Set the current position to the specified index
	 *
	 * @param The index to set the current position to
	 */
	virtual void set(size_type) noexcept { }

	/*!
	 * @brief Rewind the current position to the beginning of the sequence
	 */
	virtual void rewind() noexcept { this->set(0); }

	/*!
	 * @brief Fast-forward the current position to the end of the sequence
	 */
	virtual void fastfwd() noexcept { this->set(this->size() - 1); }

	/*!
	 * @return A reference to the front of the sequence
	 */
	virtual const_reference front() const { return _dummy; }

	/*!
	 * @return A reference to the back of the sequence
	 */
	virtual const_reference back() const { return _dummy; }

private:
    value_type _dummy;
};

/*!
 * @brief A cursor class that uses a traditional STL iterator type.
 *
 * This class would implement a cursor for any STL container
 * that features a constant bidirectional or random access iterator.
 */
template <class _Iter, typename _T = typename std::iterator_traits<_Iter>::value_type>
class iterator_cursor: public cursor<_T> {
public:
	typedef typename std::iterator_traits<_Iter>::iterator_category iterator_category;
	typedef typename std::iterator_traits<_Iter>::value_type value_type;
	typedef typename std::iterator_traits<_Iter>::difference_type difference_type;
	typedef typename std::iterator_traits<_Iter>::pointer pointer;
	typedef typename std::iterator_traits<_Iter>::reference reference;

	typedef typename cursor<_T>::const_reference const_pointer;
	typedef typename cursor<_T>::const_reference const_reference;
	typedef typename cursor<_T>::size_type size_type;

	//static_assert(std::is_same_v<iterator_category, std::bidirectional_iterator_tag> ||
	//			  std::is_same_v<iterator_category, std::random_access_iterator_tag>,
	//			  "Iterator must be bidirectional or random access!");

	/*!
	 * @brief Construct a new cursor from given the sequence
	 *
	 * Note That the begin and the end iterators are fixed
	 * for the life of the object, which means that one could
	 * build a cursor to a sub-sequence, but will never be
	 * able to access beyond the specified beginning and
	 * end of that sub-sequence with that cursor. Also, in order
	 * to be consistent with STL containers, the end iterator is
	 * actually one tick beyond the last item in the sequence.
	 *
	 * The current position is automatically set to the beginning
	 * of the sequence.
	 *
	 * @param begin An iterator to the beginning of the sequence
	 * @param end An iterator to the end of the sequence
	 */
	iterator_cursor(_Iter const &begin, _Iter const &end):
		_begin(begin), _end(end), _pos(begin) {}

    /*!
	 * @return A reference to the beginning iterator
	 */
	const _Iter &begin() const noexcept { return _begin; }

	/*!
	 * @return A reference to the end iterator
	 */
	const _Iter &end() const noexcept { return _end; }

	/*!
	 * #return A reference to the current position iterator
	 */
	const _Iter &pos() const noexcept { return _pos; }

	/*!
	 * @return The distance from the beginning of the sequence to the end
	 */
	size_type size() const noexcept override { return std::distance(begin(), end()); }

	/*!
	 * @return The distance of the current position from the beginning of the sequence
	 */
	size_type index() const noexcept override { return std::distance(begin(), pos()); }

	/*!
	 * @brief The distance of the current position from the end of the sequence
	 */
	size_type rindex() const noexcept override { return std::distance(pos(), end()); }

	/*!
	 * @return A reference to the object at the current position
	 */
	const_reference operator*() const noexcept override { return *(pos()); }

	/*!
	 * @brief Increment the cursor by the specified amount
	 *
	 * If the cursor goes past the end of the sequence, it is
	 * automatically reset to the beginning.
	 *
	 * @param The amount of increment
	 * @return This cursor.
	 */
	cursor<_T> &operator+=(int n) noexcept override {
		if (n < 0)
			return this->operator-=(-1 * n);

		if ((size_type) n > size())
			return this->operator+=(n % size());

		auto i = rindex();
		if (i > (size_type) n)
			this->_pos += n;
		else if (i == (size_type) n)
			this->rewind();
		else
			this->_pos = begin() + (n - i - 1);

		return *this;
	}

	/*!
	 * @brief Decrement the cursor by the specified amount
	 *
	 * If the cursor goes past the beginning of the sequence, it is
	 * automatically reset to the end.
	 *
	 * @param The amount of decrement
	 * @return This cursor.
	 */
	cursor<_T> &operator-=(int n) noexcept override {
		if (n < 0)
			return this->operator+=(-1 * n);

		if ((size_type) n > size())
			return this->operator-=(n % size());

		auto i = index();

		if (i > (size_type) n)
			this->_pos -= n;
		else if (i == (size_type) n)
			this->fastfwd();
		else
			this->_pos = end() - (n - i);

		return *this;
	}

	/*!
	 * @return A reference to the object at the front of the sequence
	 */
	const_reference front() const override { return *(begin()); }

	/*!
	 * @return A reference to the object at the back of the sequence
	 */
	const_reference back() const override { return *(end() - 1); }

	/*!
	 * @brief Set the current position to the specified index
	 *
	 * @param The index to set the current position to
	 */
	void set(size_type n) noexcept override {
		if ((size_type) n > size())
			return this->set(n % size());

		this->_pos = begin() + n;
	}

private:
	_Iter _begin;
	_Iter _end;
	_Iter _pos;
};

/*!
 * @brief A helper function to build a cursor to existing iterators
 *
 * @param begin An iterator to the beginning of the sequence
 * @param end An iterator to the end of the sequence
 * @return A cursor built from the iterators
 */
template <class _Iter, typename _T = typename std::iterator_traits<_Iter>::value_type>
iterator_cursor<_Iter, _T> make_cursor(_Iter const &begin, _Iter const &end) {
	return iterator_cursor<_Iter, _T>(begin, end);
}

/*!
 * @brief A helper function to build a cursor to an existing container
 *
 * The specified container must following basic STL standards that
 * includes having a well-defined constant iterator type
 *
 * The container should not be modified while the cursor is in use.
 *
 * @param The container to reference
 * @return A cursor built from the container
 */
template <class _C, class _Iter = typename _C::const_iterator, typename _T = typename std::iterator_traits<_Iter>::value_type>
iterator_cursor<_Iter, _T> make_cursor(_C const &container) {
	return make_cursor<_Iter, _T>(container.cbegin(), container.cend());
}

template <typename _T, class _Alloc = std::allocator<_T> >
class composite_cursor: public cursor<_T>  {
public:
	typedef typename _Alloc::value_type value_type;
	typedef typename _Alloc::pointer pointer;
	typedef typename _Alloc::reference reference;
	typedef typename _Alloc::const_pointer const_pointer;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type difference_type;

	composite_cursor(std::initializer_list< cursor<_T> > parts):
		_parts(parts), _iter(_parts.cbegin()) { }
	composite_cursor(composite_cursor<_T, _Alloc> const &other):
		_parts(other.parts), _iter(other._iter) { }

	size_type size() const override {
		size_type sz = 0;
		for (auto part: _parts)
			sz += part.size();
		return sz;
	}

	size_type index() const override {
		size_type count = 0;
		for (auto iter = this->_parts.cbegin(); iter != this->_iter; iter++)
			count += (*iter).size();

		return count + (*this->_iter).index();
	}

	const_reference operator*() const override {
		return *(*this->_iter);
	}

	void rewind() noexcept {
		_iter = this->_parts.cbegin();
	}

private:
	std::list< cursor<_T> > _parts;
	typename decltype(_parts)::const_iterator _iter;
};

/*!
 * @brief A cursor of bytes
 */
typedef cursor<typename byte_sequence::value_type> byte_cursor;

/*!
 * @brief A constant byte iterator based on the compiler byte order
 */
template <typename _E>
using const_byte_iterator_t = typename std::conditional<std::is_same_v<_E, byte_order<> >,
        typename byte_sequence::const_iterator, typename byte_sequence::const_reverse_iterator>::type;

/*!
 * @brief Construct a byte cursor from a arithmetic type value
 *
 * The cursor will iterate over the precise number of bytes needed
 * to make up a value of type _Tp, but always in network byte order
 * regardless of how the compiler organizes those bytes.
 *
 * Additionally, the cursor directly uses the constant reference to the
 * value, thus it does NOT copy bytes. However, the reference must
 * be valid during the complete iteration of the cursor, or else
 * unknown behavior will occur.
 *
 * @param[in] value The value
 * @return The cursor to the bytes used to construct value
 */
template <typename _Tp>
inline enable_arithmetic<_Tp, iterator_cursor<const_byte_iterator_t<network_byte_order> > >
make_byte_cursor(_Tp const &value) {
    typedef const_byte_iterator_t<network_byte_order> type;
    typedef typename type::value_type byte_type;
    byte_sequence _bytes { reinterpret_cast<const byte_type *>(&value), byte_size_of<_Tp>() };
    if constexpr (is_network_byte_order())
        return { _bytes.begin(), _bytes.end() };
    else
        return { _bytes.rbegin(), _bytes.rend() };
}

/*!
 * @brief Construct a byte cursor from an existing character sequence
 *
 * The cursor will iterate over the precise number of bytes needed
 * to make up the character sequence, in the given order.
 *
 * Additionally, the cursor directly uses the character sequence
 * iterators directly. thus it does NOT copy bytes. However, the
 * sequence must be valid during the complete iteration of the cursor,
 * or else unknown behavior will occur.
 *
 * @param[in] str The character sequence
 * @return The cursor to the bytes used to construct the sequence
 */
template <typename _CharT>
inline iterator_cursor<typename byte_sequence::const_iterator> make_byte_cursor(char_sequence<_CharT> str) {
    auto bytes = to_byte_sequence(str);
    return { bytes.begin(), bytes.end() };
}

/*!
 * @brief Construct a byte cursor from an existing character sequence
 *
 * The cursor will iterate over the precise number of bytes needed
 * to make up the character sequence, in the given order.
 *
 * Additionally, the cursor directly uses the character sequence
 * iterators directly. thus it does NOT copy bytes. However, the
 * sequence must be valid during the complete iteration of the cursor,
 * or else unknown behavior will occur.
 *
 * @param[in] str The character sequence
 * @return The cursor to the bytes used to construct the sequence
 */
template <typename _CharT>
inline enable_string<const _CharT *, iterator_cursor<typename byte_sequence::const_iterator>, _CharT>
make_byte_cursor(const _CharT *str) {
    return make_byte_cursor(char_sequence<_CharT>(str));
}

/*!
 * @brief Construct a new arithmetic data value from the specified bytes
 *
 * The supplied byte sequence should be in network byte order,
 * therefore this function will return the value with bytes
 * correctly ordered for this compiler.
 *
 * Only up to sizeof(_Tp) bytes will be used from the cursor.
 *
 * @tparam _Tp The type of data to return
 * @param[in] bytes The byte sequence
 * @return The arithmetic value of the bytes
 */
template <typename _Tp>
inline enable_arithmetic<_Tp> get_value(byte_sequence const &bytes) {
    _Tp value = 0;
    if constexpr (is_network_byte_order())
        std::copy_n(bytes.data(), std::min(bytes.size(), sizeof(_Tp)), reinterpret_cast<byte_t *>(&value));
    else
        std::copy_n(bytes.crbegin(), std::min(bytes.size(), sizeof(_Tp)), reinterpret_cast<byte_t *>(&value));

    return value;
}

/*!
 * @brief Construct a new arithmetic data value from the specified bytes
 *
 * The supplied byte sequence should be in network byte order,
 * therefore this function will return the value with bytes
 * correctly ordered for this compiler.
 *
 * Only up to sizeof(_Tp) bytes will be used from the cursor.
 *
 * @tparam _Tp The type of data to return
 * @param[in] bytes The byte sequence
 * @return The arithmetic value of the bytes
 */
template <typename _Tp, typename _CharT>
inline enable_arithmetic<_Tp> get_value(const _CharT *bytes) {
    return get_value<_Tp>(to_byte_sequence(bytes));
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* API_INCLUDE_TMX_COMMON_TYPES_SUPPORT_CURSOR_HPP_ */
