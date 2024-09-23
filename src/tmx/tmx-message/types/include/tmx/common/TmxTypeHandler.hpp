/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeHandler.hpp
 *
 *  Created on: Aug 5, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXTYPEHANDLER_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXTYPEHANDLER_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>

#include <functional>
#include <memory>
#include <system_error>
#include <tuple>
#include <typeinfo>
#include <type_traits>

namespace tmx {
namespace common {

/*!
 * @brief A helper class to quickly build a set of handler functions
 *
 * This is based on the standard C++ overload template pattern.
 *
 * The constructor takes a set of functions, which in general should be
 * lambas in order to increase performance with in-line functions.
 *
 * An example usage with lambas:
 * auto lamba = TmxTypeHandlerOverload {
 *        [](Int8) { <Handle a "char"> },
 *        [](Int32) { <Handle an "int"> },
 *        [](Float64) { <Handle a "double"> },
 *        [](auto) { <A catch-all to handle anything else> }
 *  };
 *
 * @tparam Ts A list of types to handle
 */
template  <typename ... _Fn>
struct TmxTypeHandlerOverload: _Fn ... {
    using _Fn::operator() ...;

    static constexpr auto size = sizeof...(_Fn);

    template <std::size_t _I>
    using function = typename std::tuple_element<_I, std::tuple<_Fn...>>::type;
};
template   <class... _Fn> TmxTypeHandlerOverload(_Fn...) -> TmxTypeHandlerOverload<_Fn...>;

/*!
 * @brief A type for a variable-sized argument list
 */
typedef types::Array<types::Any> TmxArgList;

template <typename _T>
struct _TmxTypeSelector {
    typedef typename std::conditional<types::IsTmxType<_T>::value,
            std::tuple< _T, types::TmxValueTypeOf<_T> >, std::tuple<types::TmxValueTypeOf<_T> > >::type type;
};

template <std::uint8_t _Sz>
struct _TmxTypeSelector< types::Int<_Sz> > {
    typedef typename std::conditional<(_Sz == types::Int<_Sz>::numBytes * TMX_BITS_PER_BYTE),
            std::tuple< types::Int<_Sz>, typename types::Int<_Sz>::value_type >, std::tuple< types::Int<_Sz> > >::type type;
};

template <std::uint8_t _Sz>
struct _TmxTypeSelector< types::UInt<_Sz> > {
    typedef typename std::conditional<(_Sz == types::UInt<_Sz>::numBytes * TMX_BITS_PER_BYTE),
            std::tuple< types::UInt<_Sz>, typename types::UInt<_Sz>::value_type >, std::tuple< types::UInt<_Sz> > >::type type;
};

template <std::uint8_t _Sz>
struct _TmxTypeSelector< types::String<_Sz> > {
    typedef std::tuple< types::String<_Sz>, typename types::String<_Sz>::value_type, const typename types::String<_Sz>::char_t *> type;
};

template <typename ... _Fn>
struct _TmxTypeSelector< TmxTypeHandlerOverload<_Fn...> > {
    typedef std::tuple< types::TmxTypeOf< first_argument_type<_Fn> >... > type;
};

template <typename ... _Tp>
struct _TmxTypeSelector< std::tuple<_Tp...> > {
    typedef tmx::common::tuple_cat< typename _TmxTypeSelector< types::TmxTypeOf<_Tp> >::type ... > type;
};

template <typename ... _Tp>
using TmxTypeSelector = tmx::common::tuple_cat<typename _TmxTypeSelector<_Tp>::type ...>;

template <typename ... _Tp>
struct _ToVariant {
    typedef tmx::common::variant<_Tp...> type;
};

template <template <typename ...> class _C, typename ... _Tp>
struct _ToVariant< _C<_Tp...> > {
    typedef typename _ToVariant<_Tp...>::type type;
};

/*!
 * @brief A variant type containing each of the possible type declarations for the given list
 */
template <typename ... _Tp>
using TmxVariant = typename _ToVariant< TmxTypeSelector<_Tp...> >::type;

/*!
 * @brief Get the available function definitions from the overload
 *
 * Note that each function is executable through the overload.
 *
 * @param overload The type handler overload
 * return The separate functions available in the overload
 */
template <typename ... _Fn, std::size_t ... _I>
const auto &make_functions(TmxTypeHandlerOverload<_Fn...> const &overload, index_sequence<_I...> const &) {
    typedef std::tuple<_Fn...> tuple_type;
    static auto _singleton = std::make_tuple(
            tmx::common::function<typename std::tuple_element<_I, tuple_type>::type> { overload }...);
    return _singleton;
}

/*!
 * @brief An alias representing a TMX type handler operation
 */
typedef TmxFunctor<TmxTypeDescriptor const &, std::reference_wrapper<TmxArgList> > TmxTypeHandler;

/*!
 * @brief Handle any type
 *
 * This function invokes the correct handler operation for the type held in
 * the given variant. It is expected that each function in the handler
 * accepts the recovered type as the first argument and the argument list
 * as the second. Additionally, the return type must be a TmxError. Otherwise,
 * a compiler error will occur.
 *
 * If the first argument in the list matches the same type as being handled,
 * then the value passed to the handler function will be extracted from the
 * argument. Otherwise, the value will be taken from the variant.
 *
 * In this function, the variant type is directly passed in as an argument,
 * therefore the handler may contain a catch-all function with a type
 * erasure as the first argument. However, this context is only really
 * useful if the possible types are well known, such as TMX types.
 *
 * @param overload The overload handler
 * @param var The type variant
 * @param args Additional arguments to pass to the handler
 * @return The result of the operation
 */
template <typename ... _Fn, typename ... _Tp>
auto do_handle(TmxTypeHandlerOverload<_Fn...> const &overload, tmx::common::variant<_Tp...> const &var, TmxArgList &args) {
    return std::visit(TmxTypeHandlerOverload {
            [&args, &overload](auto const &val) mutable {
                typedef typename std::decay<decltype(val)>::type type;
                typedef typename types::TmxValueTypeOf<type> value_type;

                if (args.size() > 0) {
                    auto try1 = tmx::common::types::as<type>(args[0]);
                    if (try1)
                        return overload(*try1, args);

                    auto try2 = tmx::common::types::as<value_type>(args[0]);
                    if (try2)
                        return overload(*try2, args);
                }

                return overload(val, args);
            }}, var);
}

template <typename _Tp, typename _Opt>
bool _assign_optional(_Opt &v, TmxTypeDescriptor const &type) {
    if (type.get_typeid() == typeid(_Tp) &&
            (v = types::make_type(*(type.as_instance<_Tp>()))))
        return true;

    if (type.get_typeid() == typeid(std::shared_ptr<_Tp>) &&
        (v = types::make_type(*(type.as_instance<std::shared_ptr<_Tp> >()))))
        return true;

    return false;
}

/*!
 * @brief Assigns the default value of the given variant from the type descriptor
 *
 * This uses the prototype stored in the type descriptor, if that type
 * matches one of the types provided in the variant. If no type can be
 * converted, the error code returned is ENOTSUP (95), indicating that the
 * data contains none of the types supported in the variant.
 *
 * Only a direct match of types is supported. Therefore, the type variant
 * should contain all possible alternatives, such as both int and the
 * TMX Int32 type if it is possible that either may be stored in the
 * container.
 *
 * Note that this is adapted from a StackOverflow solution from Apr. 13, 2020, provided by "bolov".
 * @see https://stackoverflow.com/questions/61182946/convert-stdany-to-stdvariant
 *
 * @param var The variant type to store the value
 * @param type The type to assign
 * @return The result of the operation
 */
template <typename ... _Tp>
TmxError set_variant(tmx::common::variant<_Tp...> &var, TmxTypeDescriptor const &type) noexcept {
    std::string err { TMX_PRETTY_FUNCTION };
    err.append(": Could not assign value to variant ");
    err.append(type_fqname(var).data());

    if (!type) {
        err.append(": Type unknown");
        return { EINVAL, err };
    }

    std::optional< tmx::common::variant<_Tp...> > v = std::nullopt;
    bool found = (_assign_optional<_Tp>(v, type) || ...);

    if (found) {
        var = *v;
        return { };
    }

    err.append(": Requested type ");
    err.append(type.get_type_name());
    err.append(" is not supported by the variant.");

    return { ENOTSUP, err };
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXTYPEHANDLER_HPP_ */
