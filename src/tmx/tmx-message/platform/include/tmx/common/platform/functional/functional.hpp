/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file functional.hpp
 *
 *  Created on: Oct 26, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_FUNCTIONAL_FUNCTIONAL_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_FUNCTIONAL_FUNCTIONAL_HPP_

#include <tmx/common/platform/types/traits.hpp>

#include <boost/hana.hpp>
#include <functional>
#include <type_traits>
#include <utility>

namespace tmx {
namespace common {

/*!
 * @brief Detector for function signatures
 *
 * Example usage: function<decltype(&my_function)>
 *
 * This should be able to detect the return type of
 * my_function() as well as the number and types of
 * each expected parameter.
 *
 * Note that these structures are NOT intended to
 * be used directly for invocation, but only to
 * detect an appropriate signature.
 *
 * Some helper functions are provided to help invoke the
 * function safely and properly.
 *
 * @tparam _Fn The function
 */
template <typename _Fn>
struct function;

/*!
 * @brief Traits for the function parameters
 *
 * @tparam _T The function parameter type
 */
template <typename _T>
struct argument_traits {
    typedef _T argument_type;
    typedef std::decay_t<_T> data_type;
    typedef std::integral_constant<bool, std::is_reference_v<_T> > by_ref;  // Passed by reference
    typedef std::integral_constant<bool, std::is_pointer_v<_T> > by_ptr;    // Passed by pointer
    typedef std::integral_constant<bool,                                    // Mutable/Output param
        (std::is_reference_v<_T> &&                                         // If passed by reference, then
         !std::is_const_v<std::remove_reference_t<_T> >) ||                 //  make sure it is not a const reference
        (std::is_pointer_v<_T> &&                                           // otherwise if passed by reference
         !std::is_const_v<std::remove_pointer_t<_T> >)                      //  make sure it is not a const pointer
    > read_write;                                                           // otherwise pass by value is immutable
};


/*!
 * @brief Generalized function wrapper detector implementation
 *
 * This specialization detects the function signature using
 * some std::function-like template class. These all should
 * take the form MyClass<_Ret(_Args...)>.
 *
 * This parent detects result type and the parameters.
 *
 * @tparam _C The detected template class
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <template <typename, typename ...> class _C, typename _Ret, typename... _Args>
struct function< _C<_Ret(_Args...)> >: public std::function<_Ret(_Args...)> {
    typedef std::function<_Ret(_Args...)> type;

    typedef std::invoke_result_t<type, _Args...> result_type;
    typedef std::tuple<_Args...> arguments;
    typedef arguments parameters;

    static constexpr std::size_t num_arguments = sizeof...(_Args);

    template <std::size_t _I, bool>
    struct _arg_t { typedef argument_traits<void> type; };

    template <std::size_t _I>
    struct _arg_t<_I, true> {
        typedef argument_traits<typename std::tuple_element<_I, arguments>::type> type;
    };

    template <std::size_t _I>
    using arg_traits = typename _arg_t<_I, _I < num_arguments>::type;
};

/*!
 * @brief Generalized non-member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-member function. In addition to the default function signature
 * detection, this class also detects that the function is a
 * non-member, which also implies that the class is void and that
 * the function is non-const. Additionally, this specialization detects
 * that the function does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <typename _Ret, typename... _Args>
struct function<_Ret (*)(_Args...)>: public function< std::function<_Ret(_Args...)> > {
    typedef void class_type;
    typedef std::false_type is_member;
    typedef std::false_type is_const_member;
    typedef std::false_type is_noexcept;
};

/*!
 * @brief Generalized non-member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-member function. In addition to the default function signature
 * detection, this class also detects that the function is a
 * non-member, which also implies that the class is void and that
 * the function is non-const. Additionally, this specialization detects
 * that the function does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <typename _Ret, typename... _Args>
struct function<_Ret (* const)(_Args...)>: public function<_Ret(*)(_Args...)> { };

/*!
 * @brief A non-member, no exception function signature detector implementation
 *
 * This specialization of the generalized non-member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <typename _Ret, typename... _Args>
struct function<_Ret (*)(_Args...) noexcept>: public function<_Ret(*)(_Args...)> {
     typedef std::true_type is_noexcept;
};

/*!
 * @brief A non-member, no exception function signature detector implementation
 *
 * This specialization of the generalized non-member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <typename _Ret, typename... _Args>
struct function<_Ret (* const)(_Args...) noexcept>: public function<_Ret(*)(_Args...)> {
    typedef std::true_type is_noexcept;
};

/*!
 * @brief A non-const member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-const, member function. In addition to the default function signature
 * detection, this class also detects that the function belongs to a
 * member class (and which one) and that the operation may alter the
 * object. Additionally, this specialization detects that the function
 * does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::*)(_Args...)>: public function<_Ret(*)(_Args...)> {
    typedef _C class_type;
    typedef std::true_type is_member;
};

/*!
 * @brief A non-const member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-const, member function. In addition to the default function signature
 * detection, this class also detects that the function belongs to a
 * member class (and which one) and that the operation may alter the
 * object. Additionally, this specialization detects that the function
 * does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::* const)(_Args...)>: public function<_Ret(_C::*)(_Args...)> { };

/*!
 * @brief A non-const member, no exception function signature detector implementation
 *
 * This specialization of the generalized non-const member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::*)(_Args...) noexcept>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_noexcept;
};

/*!
 * @brief A non-const member, no exception function signature detector implementation
 *
 * This specialization of the generalized non-const member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::* const)(_Args...) noexcept>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_noexcept;
};

/*!
 * @brief A const member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-const, member function. In addition to the default function signature
 * detection, this class also detects that the function belongs to a
 * member class (and which one) and that the function itself promises an
 * immutable operation. Additionally, this specialization detects that the
 * function does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::*)(_Args...) const>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_const_member;
};

/*!
 * @brief A const member function signature detector implementation
 *
 * This specialization detects the function signature of a given
 * non-const, member function. In addition to the default function signature
 * detection, this class also detects that the function belongs to a
 * member class (and which one) and that the function itself promises an
 * immutable operation. Additionally, this specialization detects that the
 * function does not make execption guarantees.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::* const)(_Args...) const>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_const_member;
};

/*!
 * @brief A const member, no exception function signature detector implementation
 *
 * This specialization of the generalized const member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::*)(_Args...) const noexcept>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_const_member;
    typedef std::true_type is_noexcept;
};

/*!
 * @brief A const member, no exception function signature detector implementation
 *
 * This specialization of the generalized const member function
 * detector additionally detects that the function guarantees
 * there are no exceptions thrown.
 *
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <class _C, typename _Ret, typename... _Args>
struct function<_Ret (_C::* const)(_Args...) const noexcept>: public function<_Ret(_C::*)(_Args...)> {
    typedef std::true_type is_const_member;
    typedef std::true_type is_noexcept;
};

/*!
 * @brief A function signature detector for uniquely specified functor classes
 *
 * This specialization of the generalized function wrapper
 * detector assumes separate declaration of the return type and the
 * parameter types. These all should take the form MyClass<_Ret, _Args...)>.
 *
 * @tparam _C The template functor class
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <template <typename, typename ...> class _C, typename _Ret, typename ... _Args>
struct function< _C<_Ret, _Args...> > : public function< std::function<_Ret(_Args...)> > { };

/*!
 * @brief A function signature detector for uniquely specified functor classes
 *
 * This specialization of the generalized function wrapper
 * detector assumes separate declaration of the return type and the
 * parameter types. These all should take the form MyClass<_Ret, _Args...)>.
 *
 * @tparam _C The template functor class
 * @tparam _Ret The return type of the function
 * @tparam _Args The arguments of the function
 */
template <template <typename, typename ...> class _C, typename _Ret, typename ... _Args>
struct function< const _C<_Ret, _Args...> > : public function< std::function<_Ret(_Args...)> > { };

/*!
 * @brief A function signature detector for indeterminate functor classes
 *
 * This specialization of the generalized function wrapper
 * detector assumes no specific templated declaration of the functor
 * class, thus it can only can inspect the invocation operator
 * overload of the class to determine the signature.
 *
 * This seems to work with bound function calls such as ones produced
 * with std::bind, although it should be mentioned that the parameter
 * type traits may be different because the bound parameters obviously
 * cannot be references. It might be better to use a lambda instead.
 *
 * @tparam _C The functor class to inspect
 */
template <class _C>
struct function : public function<decltype(&_C::operator())> { };

/*!
 * @brief Automatically detect the function signature of anything
 */
template <typename _Fn>
using function_of = function< std::remove_reference_t<_Fn> >;

template <typename _Fn>
using result_of = typename function_of<_Fn>::result_type;

template <typename _Fn>
using arguments_of = typename function_of<_Fn>::arguments;

template <typename _Fn, std::size_t _I>
using ith_arg_traits = typename function_of<_Fn>::template arg_traits<_I>;

template <typename _Fn, std::size_t _I>
using ith_arg_type = typename ith_arg_traits<_Fn, _I>::argument_type;

template <typename _Fn, std::size_t _I>
using ith_arg_data_type = typename ith_arg_traits<_Fn, _I>::data_type;

template <typename _Fn, std::size_t _I>
using ith_arg_byref = typename ith_arg_traits<_Fn, _I>::by_ref;

template <typename _Fn, std::size_t _I>
using ith_arg_byptr = typename ith_arg_traits<_Fn, _I>::by_ptr;

template <typename _Fn, std::size_t _I>
using ith_arg_byval = std::integral_constant<bool,
    !ith_arg_byref<_Fn, _I>::value && !ith_arg_byptr<_Fn, _I>::value>;

template <typename _Fn, std::size_t _I>
using ith_arg_rw = typename ith_arg_traits<_Fn, _I>::read_write;

template <typename _Fn, std::size_t _I>
using ith_arg_ro = std::integral_constant<bool, !ith_arg_rw<_Fn, _I>::value>;


// Pre-define up an alias for up to eight arguments

template <typename _Fn>
using first_argument_type = ith_arg_type<_Fn, 0>;

template <typename _Fn>
using second_argument_type = ith_arg_type<_Fn, 1>;

template <typename _Fn>
using third_argument_type = ith_arg_type<_Fn, 2>;

template <typename _Fn>
using fourth_argument_type = ith_arg_type<_Fn, 3>;

template <typename _Fn>
using fifth_argument_type = ith_arg_type<_Fn, 4>;

template <typename _Fn>
using sixth_argument_type = ith_arg_type<_Fn, 5>;

template <typename _Fn>
using seventh_argument_type = ith_arg_type<_Fn, 6>;

template <typename _Fn>
using eighth_argument_type = ith_arg_type<_Fn, 7>;

template <typename _Fn>
auto make_function(_Fn &&fn) { return typename function_of<_Fn>::type { fn }; }

template <typename _Tp, class _Fn>
constexpr auto unpack(_Tp &&tuple, _Fn &&fn) {
	return boost::hana::unpack(std::forward<_Tp>(tuple), std::forward<_Fn>(tuple));
}

template <class _Fn, typename ... _Tp, std::size_t ... _I>
constexpr auto for_each(boost::hana::tuple<_Tp...> const &tuple, std::index_sequence<_I...> const &, _Fn &&fn) {
	return boost::hana::for_each(std::forward<boost::hana::tuple<_Tp...> >(tuple), std::forward<_Fn>(fn));
}

template <class _Fn, typename ... _Tp, std::size_t ... _I>
constexpr auto for_each(std::tuple<_Tp...> const &tuple, std::index_sequence<_I...> const &, _Fn &&fn) {
	return boost::hana::for_each(boost::hana::make_tuple(std::get<_I>(tuple)...), std::forward<_Fn>(fn));
}

template <class _Fn, template <typename...> class _C, typename ... _Tp>
constexpr auto for_each(_C<_Tp...> const &tuple, _Fn &&fn) {
	return for_each(std::tuple<_Tp...> { }, std::make_index_sequence<sizeof...(_Tp)> { }, std::forward<_Fn>(fn));
}


} /* End namespace common */
} /* End namespace tmx */

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_FUNCTIONAL_FUNCTIONAL_HPP_ */
