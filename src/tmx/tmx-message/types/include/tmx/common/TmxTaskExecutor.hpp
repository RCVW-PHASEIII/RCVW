/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTaskExecutor.hpp
 *
 *  Created on: May 13, 2024
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXTASKEXECUTOR_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXTASKEXECUTOR_HPP_

#include <tmx/platform.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>

#include <future>
#include <list>
#include <type_traits>

#ifndef TMX_SYNC_DEFAULT_WAIT_NS
#define TMX_SYNC_DEFAULT_WAIT_NS 1
#endif

#ifndef TMX_SYNC_DEFAULT_WAIT_PCT
#define TMX_SYNC_DEFAULT_WAIT_PCT 0.1
#endif

namespace tmx {
namespace common {

/*
 * @brief A class for running tasks in a specific execution context
 *
 * This abstract base class is meant to define the structure for running
 * tasks in TMX. A task is defined as any operation that is to be
 * executed as a single atomic unit. This class controls exactly how
 * that operation is to be performed, which can generally be summarized
 * as a new process, as a new thread within the current process, or as
 * a new task within the current thread.
 *
 * All tasks executions are assumed to be long-running, thus are run
 * asynchronously, meaning that the return of the operation is kept
 * within a std::shared_future to track when the task is complete.
 * The precise implementation of each TMX executor is meant to be
 * encapsulated inside a running TMX plugin in order to ensure
 * consistency across the library.
 *
 * Only functions with no arguments that return a void or a TmxError can be
 * directly executed by a task executor. However, some helper routines are
 * available that allow for using a supplied std::promise instance to obtain
 * other return values asynchronously.
 */
class TmxTaskExecutor {
    template <typename _Fn>
    using valid_fn = std::integral_constant<bool,
                        std::is_void<tmx::common::result_of<_Fn> >::value ||
                        std::is_same<tmx::common::result_of<_Fn>, TmxError>::value>;

public:
    template <typename _Ret>
    using future = std::shared_future<_Ret>;

    template <typename _Fn>
    using future_ret = future<tmx::common::result_of<_Fn> >;

    template <typename _Ret, typename ... _Args>
    struct _callback_t {
        typedef Functor<void, _Ret const &, _Args...> type;
    };

    template <typename ... _Args>
    struct _callback_t<void, _Args...> {
        typedef Functor<void, _Args...> type;
    };

    template <typename _Ret, typename ... _Args>
    using callback_t = typename _callback_t<_Ret, _Args...>::type;

    template <typename _Ret>
    auto exec_error(int err, const char *msg) const {
        static safe_return<_Ret> _singleton;
        return std::async(std::launch::deferred, &safe_return<_Ret>::operator(), _singleton, err, msg).share();
    }

protected:
    TmxTaskExecutor() = default;

    template <typename _Ret>
    auto unsupported() const {
        std::string err { type_fqname(*this) };
        err.append("::execute(");
        err.append(type_fqname<Functor<_Ret> >());
        err.append(" &&) method has not been implemented for this executor.");

        return exec_error<_Ret>(ENOEXEC, err.c_str());
    }

    template <typename _Ret, typename ... _Args>
    struct no_op: public callback_t<_Ret, _Args...> {
        void execute(_Ret const &, _Args &&...) const override { }
    };

    template <typename ... _Args>
    struct no_op<void, _Args...>: public callback_t<void, _Args...> {
        void execute(_Args &&...) const override { }
    };
public:

    /*
     * @brief Execute a no-argument, TmxError return function
     *
     * @param[in] The function to execute
     * @return A future return value
     */
    virtual future<TmxError> exec_async(Functor<TmxError> &&) { return unsupported<TmxError>(); };

    /*
     * @brief Execute a no-argument, no-return function
     *
     * @param[in] The function to execute
     * @return A future return value
     */
    virtual future<void> exec_async_noreturn(Functor<void> &&) { return unsupported<void>(); }

    /*!
     * @param ret The function return object
     * @return True if the function has stopped running and the return value can be obtained
     */
    template <typename _Ret, class _Duration = std::chrono::nanoseconds>
    bool exec_stopped(future<_Ret> &ret, _Duration const &wait = _Duration(TMX_SYNC_DEFAULT_WAIT_NS)) noexcept {
        return ret.valid() &&
               (ret.wait_for(wait) == std::future_status::ready || ret.wait_for(wait) == std::future_status::deferred);
    }

    /*!
     * @param ret The function return object
     * @return True if the function is still running
     */
    template <typename _Ret, class _Duration = std::chrono::nanoseconds>
    bool exec_running(future<_Ret> &ret, _Duration const &wait = _Duration(TMX_SYNC_DEFAULT_WAIT_NS)) noexcept {
        return ret.valid() && ret.wait_for(wait) == std::future_status::timeout;
    }

    template <typename _Ret, class _Duration = std::chrono::nanoseconds, typename ... _Args>
    TmxError exec_callback(future<_Ret> ret, _Duration const &wait = _Duration(TMX_SYNC_DEFAULT_WAIT_NS),
                           callback_t<_Ret, _Args...> const &callback = no_op<_Ret, _Args...> {},
                           _Args &&...args) noexcept {
        typedef typename Functor<_Ret, _Args...>::type::type fn_type;
        static future<_Ret> _invalid;

        std::lock_guard<std::mutex> lock(this->_wait_lock);
        auto &list = this->get_list(ret);
        if (ret.valid())
            list.push_back(ret);

        std::size_t cnt = 0;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            if (this->exec_stopped(*iter, wait)) {
                // Invoke the callback
                if TMX_CONSTEXPR_FN (std::is_same<_Ret, void>::value) {
                    iter->wait();
                    callback.execute(args...);
                } else {
                    callback.execute(iter->get(), args...);
                }

                cnt++;
            }
        }

        if (cnt == list.size())
            list.clear();
        else
            // Use a percentage additive increase so not to bog down the processor with rapid checks
            this->schedule(&TmxTaskExecutor::template exec_callback<_Ret, _Duration, _Args...>, this, _invalid,
                    _Duration((typename _Duration::rep)(wait.count() + wait.count() * (1 + TMX_SYNC_DEFAULT_WAIT_PCT))),
                    callback, std::forward<_Args>(args)...);

        return { };
    }

    /*!
     * @brief Asynchronously invoke any function using this executor
     *
     * This function will execute the operation in the executor and
     * return the future value to track.
     *
     * @param fn The function
     * @param args The arguments to the function
     * @return The future return value of the function
     */
    template <typename _Fn, typename ... _Args>
    typename std::enable_if<std::is_void<tmx::common::result_of<_Fn> >::value, future_ret<_Fn> >::type
    schedule(_Fn &&fn, _Args &&... args) {
        std::function<void()> function = std::bind(fn, args...);
        return this->exec_async_noreturn(make_functor(function));
    }

    /*!
     * @brief Asynchronously invoke any function using this executor
     *
     * This function will execute the operation in the executor and
     * return the future value to track.
     *
     * @param fn The function
     * @param args The arguments to the function
     * @return The future return value of the function
     */
    template <typename _Fn, typename ... _Args>
    typename std::enable_if<std::is_same<tmx::common::result_of<_Fn>, TmxError>::value, future_ret<_Fn> >::type
    schedule(_Fn &&fn, _Args &&... args) {
        std::function<TmxError()> function = std::bind(fn, args...);
        return this->exec_async(make_functor(std::move(function)));
    }

    /*!
     * @brief Asynchronously invoke any function using this executor
     *
     * This function expects a std::promise for the return value as
     * input, which is used to execute the operation in the executor and
     * return the future value to track.
     *
     * @param fn The function
     * @param promise The promise argument
     * @param args The arguments to the function
     * @return The future return value of the function
     */
    template <typename _Fn, typename ... _Args>
    typename std::enable_if<!valid_fn<_Fn>::value, future_ret<_Fn> >::type
    schedule(_Fn &&fn, std::promise<tmx::common::result_of<_Fn> > &&promise, _Args &&...args) {
        std::function<tmx::common::result_of<_Fn>()> function = std::bind(fn, args...);
        auto lambda = [&function, &promise]() -> void {
            promise.set_value(function());
        };
        this->exec_async_noreturn(make_functor(lambda));
        return promise.get_future().share();
    }

    /*!
     * @brief Obtain the underlying execution context implementation
     *
     * This is specifically for cases in which the implementation may
     * matter, such as for a TMX broker that explicitly requires some
     * asynchronous context. Those instances must validate that the
     * type is correct through a static cast, and somehow return an
     * error if the types do not match.
     *
     * @return A pointer to some underlying execution context implementation
     */
    virtual void *get_implementation() { return nullptr; }

private:
    std::mutex _wait_lock;
    std::list<std::shared_future<void> > _waiting_no_return;
    std::list<std::shared_future<TmxError> > _waiting;

    auto &get_list(future<void> const &) {
        return this->_waiting_no_return;
    }

    auto &get_list(future<TmxError> const &) {
        return this->_waiting;
    }
};

} /* End namespace common */
} /* End namespace tmx */

#endif // TYPES_INCLUDE_TMX_COMMON_TMXTASKEXECUTOR_HPP_
