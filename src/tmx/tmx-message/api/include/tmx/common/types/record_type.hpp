/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file record_type.hpp
 *
 *  Created on: Jul 19, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_COMMON_TYPES_RECORD_TYPE_HPP_
#define API_INCLUDE_TMX_COMMON_TYPES_RECORD_TYPE_HPP_

#include <tmx/common/types/enum_type.hpp>

#include <istream>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace tmx {
namespace common {
namespace types {

template <class... _Types>
class record_type: public std::tuple<_Types...> {
public:
	typedef record_type<_Types...> self_type;

	static constexpr const size_t count = sizeof...(_Types);

private:
	static constexpr auto _base = TMX_CHAR_TUPLE_STRING("record");
	static constexpr auto _lt = TMX_CHAR_TUPLE_STRING("<");
	static constexpr auto _gt = TMX_CHAR_TUPLE_STRING(">");
	static constexpr auto _concat = support::type_id_name<self_type>();

public:

	/*!
	 * @brief This is a tuple type
	 */
	typedef std::tuple<_Types...> type;

	/*!
	 * @brief The name depends on the type count
	 *
	 * @todo Can we do better by concatenating all the type names?
	 */
	static constexpr const auto name = _concat.c_str();

	/*!
	 * @brief Construct an empty record
	 */
	constexpr explicit record_type(): type() { }

	/*!
	 * @brief Construct a record from the supplied tuple
	 *
	 * @param[in] value The tuple to use
	 */
	explicit record_type(type &&value): type(std::forward<type>(value)) { }

	/*!
	 * @brief Construct a record from a copy of the supplied tuple
	 *
	 * @param[in] value The tuple to use
	 */
	explicit record_type(const type &value): type(value) { }

	/*!
	 * @brief Construct a record from the supplied type list
	 *
	 * @param[in] value The types to use
	 */
	explicit record_type(const _Types&... elems): type(elems...) { }

	template <size_t _I>
	using element = std::tuple_element<_I, type>;

	template <size_t _I>
	using element_t = typename element<_I>::type;

	/*!
	 * @return The tuple representation of this record
	 */
	operator type () {
		return static_cast<type &>(*this);
	}

	using type::operator =;

private:
	// Some type trait tests
	template <size_t _I>
	static inline constexpr bool type_exists() {
		return count > 0 && _I < count;
	}

	template <typename _T, class _C>
	void do_set(_T &elem, const _C value) {
		elem = value;
	}

	template <typename _T, class _C>
	void do_set(std::reference_wrapper<_T> &elem, const _C value) {
		elem.get() = value;
	}

public:
	/*!
	 * @brief Copy the _Ith element of the record to the supplied value
	 *
	 * @param[in] value A reference to copy the data to
	 */
	template <size_t _I, class _C = element_t<_I> >
	typename std::enable_if<type_exists<_I>(), void>::type get(_C &value) {
		value = std::get<_I>(*this);
	}

	/*!
	 * @brief Set the _Ith element of the record to the supplied value
	 *
	 * @param[in] value The value to set
	 */
	template <size_t _I, class _C = element_t<_I> >
	typename std::enable_if<type_exists<_I>(), void>::type set(const _C &value) {
		auto &elem = std::get<_I>(*this);
		do_set(elem, value);
	}

};

template <class _C>
auto make_record(_C &data) {
	// TODO: implement a default or throw an exception
}

// Serialization operations
/*
template <class... _Types>
std::ostream &operator<<(std::ostream &os, const record_type<_Types...> &obj) {
	(..., (os << (I == 0 ? "" : ", ") << support::type_id_name(get<I>(tp))));
	return os;
}

template <class... _Types>
std::istream &operator>>(std::istream &is, record_type<_Types...> &obj) {
	typename float_type<_Sz>::type value;
	is >> value;
	obj = value;
	return is;
}*/

} /* End namespace types */
} /* End namespace common */
} /* End namespace tmx */



#endif /* API_INCLUDE_TMX_COMMON_TYPES_RECORD_TYPE_HPP_ */
