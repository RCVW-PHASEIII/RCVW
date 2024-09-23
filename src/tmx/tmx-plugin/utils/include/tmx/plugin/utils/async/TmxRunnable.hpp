/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxRunnable.hpp
 *
 *  Created on: Mar 2, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXRUNNABLE_HPP_
#define UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXRUNNABLE_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Array.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/message/TmxData.hpp>

#include <atomic>
#include <exception>
#include <future>
#include <iostream>

namespace tmx {
namespace plugin {

using TmxRunnableArgs = common::types::Array<typename common::types::String8::value_type>;

namespace utils {
namespace async {

/*!
 * @brief A base class for runnable objects or commands
 *
 * A runnable object is anything that executes stand-alone. Each has
 * the option for processing a list of command arguments, plus
 * a main() loop, but require an execution environment to run within.
 * The execution environment may be a separate process, thread, or
 * some execution container.
 */
class TmxRunnable : public common::TmxFunctor<const TmxRunnableArgs &> {
public:
    struct TmxNewTask;
    struct TmxNewThread;
    struct TmxNewProcess;

    /*!
     * @brief Default constructor
     */
    TmxRunnable() = default;

    /*!
     * @brief Default desctructor
     */
    virtual ~TmxRunnable() = default;

    /*!
     * @brief Execute the runnable command from start to finish
     *
     * This function allows the runnable object to be initialized and
     * executed in one atomic operation. First, the arguments are processed.
     * Secondly, the runnable object is started. Note that this function
     * blocks until the operation returns. Programmatically use start/stop
     * functions if
     *
     * @see TmxFunctor::operator()()
     * @return Any
     */
    virtual common::TmxError execute(TmxRunnableArgs const &) override;

    /*!
     * @brief Process the command arguments
     *
     * The default implementation does nothing
     *
     * @param[in] The arguments as a list of strings
     * @return Any errors encountered during processing
     */
    virtual common::TmxError process_args(TmxRunnableArgs const &);

    /*!
     * @brief The main loop for this runnable object
     *
     * The default implementation
     * @return Any errors encountered in the loop
     */
    virtual common::TmxError main();

    /*!
     * @return True if the runnable is still active. False otherwise.
     */
    virtual bool is_running() const;

    /*!
     * @brief Start this runnable object
     */
    virtual void start();

    /*!
     * @brief Stop this runnable object, and which point the result should be available
     */
    virtual void stop();

protected:
    std::atomic<bool> _running { false };
};

struct _Log_Initializer {
    _Log_Initializer() {
        common::TmxLogger::enable();
    }
};

} /* End namespace async */
} /* End namespace utils */

template <class _C>
int run(_C &functor, int argc, char **argv) {
    static utils::async::_Log_Initializer _log_initializer;
    tmx::common::TmxError error;

    try {
        error = functor.execute({argv, argv + argc});
    } catch (std::exception &ex) {
        error = tmx::common::TmxError(ex);
    }

    typedef tmx::common::types::Properties<tmx::common::types::Any> props_type;
    auto &err = static_cast<const typename props_type::value_type &>(error);

    if (error) {
        TLOG(ERR) << tmx::message::TmxData(err).to_string();

        std::cerr << error.get_message() << std::endl;
    } else {
        TLOG(DEBUG1) << tmx::message::TmxData(err).to_string();
    }

    return (int) error;
}

} /* End namespace plugin */
} /* End namespace tmx */

#endif /* UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXRUNNABLE_HPP_ */
