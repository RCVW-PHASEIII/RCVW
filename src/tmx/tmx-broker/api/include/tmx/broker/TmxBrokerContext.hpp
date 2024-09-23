/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerContext.hpp
 *
 *  Created on: Apr 7, 2023
 *      @author: Gregory M. Baumgardner
 */


#ifndef INCLUDE_TMX_BROKER_TMXBROKERURL_HPP_
#define INCLUDE_TMX_BROKER_TMXBROKERURL_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/types/String.hpp>

#include <mutex>
#include <condition_variable>

namespace tmx {
namespace broker {

enum class TmxBrokerState: std::uint8_t {
    uninitialized,
    initialized,
    disconnected,
    connected,
    registered
};

/*!
 * @brief A class to hold the broker connection URL and configuration data
 *
 * This is a generic name-value pair structure. Therefore, it can basically
 * be used in any way by the broker client. Some common client information
 * is automatically managed.
 */
class TmxBrokerContext: public common::types::Properties<common::types::Any>::value_type {
    typedef typename common::types::Properties<common::types::Any>::value_type super;
public:
    /*!
     * @brief Default constructor for an empty context
     */
    TmxBrokerContext() = default;

    /*!
     * @brief Construct a new context from the given URL string and config
     *
     * The format should always be:
     * scheme://[username:secret@]host[:port][/path]
     *
     * @param[in] url The URL string
     * @param[in] id An optional unique identifier. This defaults to a UUID
     * @param[in] params Optional default parameters
     */
    TmxBrokerContext(common::const_string, common::const_string = "",
                     common::types::Any const & = common::types::no_data()) noexcept;

    /*!
     * @brief Copy constructor
     *
     * @param[in] The context to copy
     */
    TmxBrokerContext(TmxBrokerContext const &) noexcept;

    /*!
     * @brief Default destructor
     */
    ~TmxBrokerContext();

    /*!
     * @brief Assignment operator
     */
    TmxBrokerContext &operator=(TmxBrokerContext const &) noexcept;

    /*!
     * @return True if the context is valid, false otherwise
     */
    operator bool () const noexcept;

    /*!
     * @return An executor for this context
     */
    std::shared_ptr<common::TmxTaskExecutor> get_executor() const noexcept;

    /*!
     * @brief Set a new executor for this context
     */
    void set_executor(std::shared_ptr<common::TmxTaskExecutor>) noexcept;

    /*!
     * @return The state of this context
     */
    TmxBrokerState get_state() const noexcept;

    /*!
     * @brief Set the new state of this context
     */
    void set_state(TmxBrokerState) noexcept;

    /*!
     * @return The context ID
     */
    common::types::String_ get_id() const noexcept;

    /*!
     * @return The scheme for the URL
     */
    common::types::String_ get_scheme() const noexcept;

    /*!
     * @return The user for the URL
     */
    common::types::String_ get_user() const noexcept;

    /*!
     * @return The secret or password for the URL
     */
    common::types::String_ get_secret() const noexcept;

    /*!
     * @return The host for the URL
     */
    common::types::String_ get_host() const noexcept;

    /*!
     * @return The port for the URL
     */
    common::types::String_ get_port() const noexcept;

    /*!
     * @return The path for the URL
     */
    common::types::String_ get_path() const noexcept;

    /*!
     * @return A full string representation of the URL
     */
    common::types::String_ to_string() const noexcept;

    /*!
     * @return The read/write set of connection parameters
     */
    common::types::Any &get_parameters() noexcept;

    /*!
     * @return The read-only set of default connection parameters
     */
    common::types::Any const &get_defaults() const noexcept;

    /*!
     * @return A unique mutex specifically for locking across threads on this context
     */
    std::mutex &get_thread_lock() noexcept;

    /*!
     * @return A unique mutex specifically for locking across publishes on this context
     */
    std::mutex &get_publish_lock() noexcept;

    /*!
     * @return A unique mutex specifically for locking across receiving on this context
     */
    std::mutex &get_receive_lock() noexcept;

    /*!
     * @return A unique condition variable specifically for synchronizing across threads on this context
     */
    std::condition_variable_any &get_thread_sem() noexcept;

    /*!
     * @return A unique condition variable specifically for synchronizing across receives on this context
     */
    std::condition_variable_any &get_publish_sem() noexcept;

    /*!
     * @return A unique condition variable specifically for synchronizing across publishes on this context
     */
    std::condition_variable_any &get_receive_sem() noexcept;

private:
    std::shared_ptr<common::TmxTaskExecutor> _ctxExecutor;
    std::mutex _ctxThreadLock;
    std::mutex _ctxPublishLock;
    std::mutex _ctxReceiveLock;
    std::condition_variable_any _ctxThreadCv;
    std::condition_variable_any _ctxPublishCv;
    std::condition_variable_any _ctxReceiveCv;
};

} /* End namespace broker */
} /* End namespace tmx */

#endif //INCLUDE_TMX_BROKER_TMXBROKERURL_HPP_
