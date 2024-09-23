/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxRunnable.cpp
 *
 *  Created on: Mar 2, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/utils/async/TmxRunnable.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>

#include <chrono>
#include <thread>

#if __has_include(<boost/asio/io_context.hpp>)
#define TMX_ASYNC_EXEC_PROACTOR boost::asio::io_context
#define TMX_ASYNC_EXEC_POOLED boost::asio::thread_pool

#include <boost/asio.hpp>
#endif

#ifdef TMX_UX_POSIX
#include <wait.h>
#endif

#ifndef TMX_ASYNC_THREAD_POOL_SIZE
#define TMX_ASYNC_THREAD_POOL_SIZE 1
#endif

using namespace tmx::common;

namespace tmx {
namespace plugin {
namespace utils {
namespace async {

template <std::launch _Launch>
class TmxAsyncTaskExecutor : public TmxTaskExecutor {
public:
    future<void> exec_async_noreturn(Functor<void> &&function) override {
        return std::async(_Launch, function);
    }

    future <TmxError> exec_async(Functor <TmxError> &&function) override {
        return std::async(_Launch, function);
    }
};

static TmxAsyncTaskExecutor<std::launch::deferred> _deferred_executor;
static TmxAsyncTaskExecutor<std::launch::async> _thread_executor;

#ifdef TMX_ASYNC_EXEC_PROACTOR

class TmxProactorTaskExecutor : public TmxTaskExecutor {
public:
    future<void> exec_async_noreturn(Functor<void> &&function) override {
        return boost::asio::post(this->context, std::packaged_task<void()>(function)).share();
    }

    future <TmxError> exec_async(Functor <TmxError> &&function) override {
        return boost::asio::post(this->context, std::packaged_task<TmxError()>(function)).share();
    }

    TMX_ASYNC_EXEC_PROACTOR context;

    void run() {
        this->context.run();
    }

    void stop() {
        this->context.stop();
    }

    void *get_implementation() override { return static_cast<void *>(&this->context); }
};

static TmxProactorTaskExecutor _proactor_executor;
#endif

#ifdef TMX_ASYNC_EXEC_POOLED

class TmxPooledTaskExecutor : public TmxTaskExecutor {
public:
    future<void> exec_async_noreturn(Functor<void> &&function) override {
        return boost::asio::post(this->context.get_executor(), std::packaged_task<void()>(function)).share();
    }

    future <TmxError> exec_async(Functor <TmxError> &&function) override {
        return boost::asio::post(this->context.get_executor(), std::packaged_task<TmxError()>(function)).share();
    }

    TMX_ASYNC_EXEC_POOLED context{ TMX_ASYNC_THREAD_POOL_SIZE };

    void start() {
        this->context.attach();
    }

    void stop() {
        this->context.stop();
        this->context.join();
    }

    void *get_implementation() override { return static_cast<void *>(&this->context); }
};

static TmxPooledTaskExecutor _pooled_executor;
#endif

#ifdef TMX_UX_POSIX
class TmxForkTaskExecutor : public TmxTaskExecutor {
    future<TmxError> exec_async(Functor <TmxError> &&function) override {
        auto pid = fork();
        if (pid == 0) {
            // In the child process, execute the function
            exit(function());
        } else {
            // In the parent process, need wait for the return type in a new thread
            auto _lambda = [pid]() -> TmxError {
                int status;
                waitpid(pid, &status, 0);
                return { status };
            };
            return _thread_executor.schedule(_lambda);
        }
    }
};

static TmxForkTaskExecutor _fork_executor;
#endif

//common::TmxTaskExecutor &TmxRunnable::new_task() noexcept {
//#if defined(TMX_ASYNC_EXEC_PROACTOR) && !defined(TMX_ASYNC_NO_PROACTOR)
//#if TMX_ASYNC_THREAD_POOL_SIZE > 1
//    return _pooled_executor;
//#else
//    return _proactor_executor;
//#endif
//#else
//    return _deferred_executor;
//#endif
//}
//
//common::TmxTaskExecutor &TmxRunnable::new_thread() noexcept {
//#ifdef TMX_ASYNC_EXEC_POOLED
//    // If we are running in a thread pool, then the best
//    // strategy is to create an additional thread in the
//    // pool and assign to task to one of those workers
//    // because then that ASIO executor can be used across
//    // all the threads for asynchronous operations.
//    if (&(this->new_task()) == &_pooled_executor) {
//        std::thread([]() ->void { _pooled_executor.context.attach(); }).detach();
//        return _pooled_executor;
//    }
//#endif
//
//    // Just launch a new separate thread
//    return _thread_executor;
//}
//
//common::TmxTaskExecutor &TmxRunnable::new_process() noexcept {
//#ifdef TMX_UX_POSIX
//    return _fork_executor;
//#else
//    return _thread_executor;
//#endif
//}

TmxError TmxRunnable::execute(TmxRunnableArgs const &args) {
    auto err = this->process_args(args);
    if (err) return err;

    // Launch the main thread
    this->start();

    try {
        err = this->main();
        this->stop();
        return err;
    } catch (std::exception &ex) {
        return { ex };
    }
}

bool TmxRunnable::is_running() const {
    return this->_running;
}

void TmxRunnable::start() {
    this->_running = true;
}

void TmxRunnable::stop() {
    this->_running = false;
}

TmxError TmxRunnable::process_args(TmxRunnableArgs const &) {
    return { };
}

TmxError TmxRunnable::main() {
    return { };
}

} /* End namespace async */
} /* End namespace utils */
} /* End namespace plugin */
} /* End namespace tmx */

