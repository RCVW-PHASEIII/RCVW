/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFunctor.hpp
 *
 *  Created on: Mar 1, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXFUNCTOR_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXFUNCTOR_HPP_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <functional>

namespace tmx {
namespace common {

template <typename _Ret>
struct safe_return {
    _Ret operator()(int err, const char *) { return (_Ret)err; }
};

template <> struct safe_return<void> { void operator()(int, const char *) { } };
template <> struct safe_return<bool> { bool operator()(int err, const char *) { return (err == 0); } };
template <> struct safe_return<common::TmxError> {
    common::TmxError operator()(int err, const char *msg) { return { err, msg }; }
};
template <typename _CharT>
struct safe_return<std::basic_string<_CharT> > {
    std::basic_string<_CharT> operator()(int err, const char *msg) { return { msg }; }
};

/*!
 * @brief A base class for polymorphic function calls
 *
 * This is a type of std::function implementation, except
 * that the a TMX functor class can be invoked in an
 * object-mutable (non-const) manner. This allows the functor
 * class to be extended in a way more suitable for TMX
 * operations, especially plugin handlers, which will almost
 * certainly be modifying itself upon callback.
 *
 * The default operations are also left unimplemented in
 * this parent class, unless the object actually is storing
 * a function. Therefore, sub-class must override at least
 * the immutable operation implementation. The operations
 * return failures if left unimplemented.
 *
 * A functor instance can also wrap existing function, just
 * like a std::function, assuming that the signatures line up.
 * Since the TMX dispatchers can only register and invoke a
 * Functor class, wrapping global and/or member functions allows
 * for direct invocation of those functions from the dispatchers.
 *
 * @see #make_functor
 */
template <typename _Ret, typename... _Args>
class Functor {
    typedef Functor<_Ret, _Args...> self_type;
public:
    typedef tmx::common::function_of< _Ret(Functor::*)(_Args...) > type;

    Functor() noexcept: _function() { }
    Functor(types::Null::value_type fn) noexcept: _function(fn) { }

    Functor(self_type const &copy): _function(copy._function) { };
    Functor(type_unboxer<type> const &copy): _function(copy) { }
    Functor(type_unboxer<type> &&move): _function(std::forward<type_unboxer<type> >(move)) { }

    virtual ~Functor() = default;

    /*!
     * @brief Execute an immutable functor
     *
     * By default, this operation is not supported unless
     * the functor was created from an existing function.
     *
     * @param args The arguments
     * @return The return value of the functor execution
     */
    virtual _Ret execute(_Args... args) const {
        if (this->_function.operator bool())
            return (_Ret)this->_function(args...);

        // Not implemented
        return this->unsupported(TMX_PRETTY_FUNCTION);
    }

    /*!
     * @brief Execute a mutable functor
     *
     * By default, this operator simply executes itself
     * as an immutable functor.
     *
     * #see execute() const
     * @param args The arguments
     * @return The return value of the functor execution
     */
    virtual _Ret execute(_Args... args) {
        // If this is not overridden explicitly, then use the const version
        return const_cast<self_type const *>(this)->execute(args...);
    }

    /*!
     * @brief Execute a immutable functor
     *
     * @param args The arguments
     * @return The return value of the functor execution
     */
    _Ret operator()(_Args... args) const {
        return this->execute(args...);
    }

    /*!
     * @brief Execute a mutable functor
     *
     * @param args The arguments
     * @return The return value of the functor execution
     */
    _Ret operator()(_Args... args) {
        return this->execute(args...);
    }

    /*!
     * @return True if this functor can be invoked
     */
    operator bool() const {
        return true;
    }

protected:
    // Unsupported operation error
    _Ret unsupported(const char *function) const {
        static safe_return<_Ret> _singleton;

        std::string err { "Functor function " };
        err.append(function);
        err.append(" has not been implemented.");

        return _singleton(EOPNOTSUPP, err.c_str());
    }

private:
    type_unboxer<type> _function;
};

template <typename ... _Args>
struct _TmxFunctor {
    typedef Functor<TmxError, _Args...> type;
};

template <typename _Ret, typename ... _Args>
struct _TmxFunctor< std::function<_Ret(_Args...)> > {
    typedef Functor<_Ret, _Args...> type;
};

template <typename ... _Args>
using TmxFunctor = typename _TmxFunctor<_Args...>::type;

/*!
 * @brief Create a functor from a moved function (e.g., lambda)
 *
 * @return The functor
 */
template <typename _Fn>
TmxFunctor<typename tmx::common::function_of<_Fn>::type> make_functor(_Fn function) {
    return { function };
}

/*
 * Some functions to help dispatch invocation to the appropriate functor
 */

/*!
 * @brief Invoke the function with the given arguments
 *
 * This function assumes the descriptor represents an actual functor class
 * instance that matches the return argument types. Failures in this function
 * generally mean that the programmer has some mis-matched argument types.
 *
 * For example, a <const MyType &> Functor parameter expects the associated
 * argument to be an immutable reference to a MyType instance. Note that a
 * non-const input to the function may cause a compiler error or worse a
 * segmentation fault. Therefore, take care to use the appropriate
 * arguments when calling invoke.
 *
 * Output parameters are easily supported by using the std::reference_wrapper
 * class. The implementing function, however, must accept that type of
 * object, and the argument passed to invoke should be std::ref(myVariable).
 *
 * @param[in] functor The functor to dispatch to
 * @param[in] args The arguments to supply to the operation
 * @return The future result of the operation
 */
template <typename _Ret = TmxError, typename... _Args>
_Ret dispatch(TmxTypeDescriptor const &descriptor, _Args &&...args) {
    typedef Functor<_Ret, _Args...> fn_type;
    typedef type_unboxer<fn_type> function_type;

//    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << descriptor.get_type_name();

    if (descriptor) {
        auto functor = descriptor.as_instance<fn_type>();
        if (functor && functor->operator bool())
            return (*functor)(std::forward<_Args>(args)...);
    }

    std::string err { "Dispatch failed because " };
    err.append(descriptor.get_type_name());
    err.append(" is not any known instance of function signature ");
    err.append(type_fqname<type_unboxer<fn_type> >());
    err.append(".");

    safe_return<_Ret> _ret;
    return _ret(EPROTOTYPE, err.c_str());
}

/*!
 * @brief Invoke the function with the specified name using the given arguments
 *
 * This function assumes the descriptor represents an actual functor class
 * instance that matches the return argument types. Failures in this function
 * generally mean that the programmer has some mis-matched argument types.
 *
 * For example, a <const MyType &> Functor parameter expects the associated
 * argument to be an immutable reference to a MyType instance. Note that a
 * non-const input to the function may cause a compiler error or worse a
 * segmentation fault. Therefore, take care to use the appropriate
 * arguments when calling invoke.
 *
 * Output parameters are easily supported by using the std::reference_wrapper
 * class. The implementing function, however, must accept that type of
 * object, and the argument passed to invoke should be std::ref(myVariable).
 *
 * @param[in] name The function name to search for
 * @param[in] nmspace The namespace to search within
 * @param[in] args The arguments to supply to the operation
 * @return The future result of the operation
 */
template <typename _Ret = TmxError, typename... _Args>
_Ret dispatch_byname(const_string name, const_string nmspace, _Args &&...args) {
    auto functor = TmxTypeRegistry(nmspace.data()).get(name);
    if (functor)
        return tmx::common::dispatch(functor, std::forward<_Args>(args)...);

    std::string err{ "TmxFunctor " };
    err.append(name);
    err.append(" is not found within namespace ");
    err.append(nmspace);

    safe_return<_Ret> _ret;
    return _ret(EPROTONOSUPPORT, err.c_str());
}

} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXFUNCTOR_HPP_ */
