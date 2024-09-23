//
// Created by cas on 6/18/24.
//

#ifndef UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXTASKWORKER_HPP_
#define UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXTASKWORKER_HPP_

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/plugin/utils/async/TmxRunnable.hpp>

#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>

namespace tmx {
namespace plugin {
namespace utils {
namespace async {

template <typename _ExecContext>
class TmxTaskWorker: public common::TmxTaskExecutor,
                     public TmxRunnable {
public:
    TmxTaskWorker(_ExecContext *ctx): _context(ctx), _id(), _count(0),
                                      _guard(boost::asio::make_work_guard(*_context)) { }
    virtual ~TmxTaskWorker() = default;

    TmxTaskWorker(TmxTaskWorker const &copy): _context(copy._context), _id(copy._id), _count((std::size_t)copy._count),
                                              _guard(boost::asio::make_work_guard(*_context)) { }
    TmxTaskWorker &operator=(TmxTaskWorker const &) = delete;

    std::thread::id const &get_id() const noexcept { return this->_id; }
    std::size_t size() const noexcept { return this->_count; }

    future<void> exec_async_noreturn(common::Functor<void> &&fn) override {
        this->_count++;
        return boost::asio::post(this->get_context().get_executor(),
                                 std::packaged_task<void()>([this, fn]() {
            fn();
            this->_count--;
        })).share();
    }

    future<common::TmxError> exec_async(common::Functor<tmx::common::TmxError> &&fn) override {
        this->_count++;
        return boost::asio::post(this->get_context().get_executor(),
                                 std::packaged_task<common::TmxError()>([this, fn]() {
            auto ret = fn();
            this->_count--;
            return ret;
        })).share();
    }

    _ExecContext &get_context() {
        return *(this->_context);
    }

    auto get_future() const {
        return this->_future;
    }

    void start() override;
    void stop() override;
private:
    std::shared_ptr<_ExecContext> _context;

    std::thread::id _id;
    std::atomic<std::size_t> _count;
    std::shared_future<void> _future;
    boost::asio::executor_work_guard<typename _ExecContext::executor_type> _guard;
};

template <>
inline void TmxTaskWorker<boost::asio::io_context>::start() {
    this->_future = std::async([this]() {
        TmxRunnable::start();
        this->_id = std::this_thread::get_id();

        TLOG(NOTICE) << this->_id << ": " << common::type_short_name(*this) << " has started";

        while (this->is_running()) {
            // Give a chance for more scheduling to occur
            std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            auto cnt = this->get_context().run();
            this->get_context().reset();
        }

        TLOG(NOTICE) << this->_id << ": " << common::type_short_name(*this) << " has terminated";
    }).share();
}

template <>
inline void TmxTaskWorker<boost::asio::io_context>::stop() {
    TmxRunnable::stop();
    this->get_context().stop();

    if (this->_future.valid())
        this->_future.wait_for(std::chrono::seconds(1));
}

template <>
inline void TmxTaskWorker<boost::asio::system_context>::start() {
    TmxRunnable::start();
    this->_id = std::this_thread::get_id();
}

template <>
inline void TmxTaskWorker<boost::asio::system_context>::stop() {
    TmxRunnable::stop();
    this->get_context().stop();
    this->get_context().join();
}

template <>
inline void TmxTaskWorker<boost::asio::thread_pool>::start() {
    TmxRunnable::start();
    this->_id = std::this_thread::get_id();
}

template <>
inline void TmxTaskWorker<boost::asio::thread_pool>::stop() {
    TmxRunnable::stop();
    this->get_context().stop();
    this->get_context().join();
}

} /* End namespace async */
} /* End namespace utils */
} /* End namespace plugin */
} /* End namespace tmx */

#endif // UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXTASKWORKER_HPP_
