/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerClient.hpp
 *
 *  Created on: Aug 11, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_BROKER_TMXCLIENT_HPP_
#define INCLUDE_TMX_BROKER_TMXCLIENT_HPP_

#include <tmx/platform.hpp>

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/types/Int.hpp>
#include <tmx/common/types/String.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <atomic>
#include <memory>

namespace tmx {
namespace broker {

/*!
 * @brief The abstract base class for all clients of TMX brokers
 *
 * In order to facilitate clients to different brokers, this class must
 * be extended to provide specific connectivity. Connectivity to a specific
 * broker scheme may be subject to licensing.
 */
class TmxBrokerClient {
public:
	virtual ~TmxBrokerClient() = default;

    static std::shared_ptr<TmxBrokerClient> get_broker(TmxBrokerContext const &) noexcept;

    /*!
     * @return The type descriptor for this broker client
     */
    virtual common::TmxTypeDescriptor get_descriptor() const noexcept;

    /*!
     * @return A broker-specifc construct with information about the broker connection
     */
    virtual common::types::Any get_broker_info(TmxBrokerContext &) const noexcept;

    /*!
     * @brief Initialize the broker with the given context
     *
     * The specified context should be saved within the client fpr
     * connectivity.
     *
     * @param[in] The broker context
     * @see #on_initialized
     */
    virtual void initialize(TmxBrokerContext &) noexcept;

    /*!
     * @brief Connect to the broker using the specified parameters
     *
     * The broker connection information is passed in. The default values
     * for the host and port and obtained from the construction URL, but may
     * be overridden here. The client name must be unique for the broker. If
     * one is not specified (i.e., and empty string), then a UUID-like name
     * is automatically generated. If the group name is not specified, then
     * then this client will not be capable of sharing topics amongst other
     * client instances. In other words, it will be isolated and will receive
     * all messages it subscribes to. If more than one client use the same
     * group name for the same broker, then subscribed messages are intended
     * to be distributed evenly across each. The additional connection parameters
     * override any default settings for the broker scheme.
     *
     * Note that this operation is intended to be asynchronous. Thus, it returns
     * immediately, and the result of the connection attempt will be passed
     * to the onConnected() call-back.
     *
     * @param[in] Additional connection parameters, which is null by default
     * @see #on_connected
     */
    virtual void connect(TmxBrokerContext &, common::types::Any const & = common::types::no_data()) noexcept;

    /*!
    * @brief Manually disconnect from the broker
    *
    * While network issues may also cause the client to become disconnected, this
    * function specifically forces the issue.
    *
    * Note that this operation is intended to be asynchronous. Thus, it returns
    * immediately, and the result of the disconnection attempt will be passed to
    * the onDisconnected() call-back.
    */
    virtual void disconnect(TmxBrokerContext &) noexcept;

    /*!
     * @brief Manually disconnect and reconnect to the broker
     */
    virtual void reconnect(TmxBrokerContext &) noexcept;

    /*!
	 * @return True if this client is currently connected to the broker. False otherwise
	 */
	virtual bool is_connected(TmxBrokerContext const &) const noexcept;

	/*!
	 * @brief Subscribe to a topic
	 *
	 * The given call-back function is invoked whenever a message arrives on that
	 * topic. It is up to the call-back implementation to handle that message as
	 * it sees fit, including decoding the payload.
	 *
	 * Note that this operation is intended to be asynchronous. Thus, it returns
	 * immediately, and the result of the subscribe attempt will be passed to
	 * the onSubscribed() call-back.
	 *
	 * @param[in] The topic name to subscribe to
	 * @param[in] The call-back to use for this subscription
	 * @see #on_subscribed
	 */
	virtual void subscribe(TmxBrokerContext &, common::const_string, common::TmxTypeDescriptor const &) noexcept;

	/*!
	 * @brief Unsubscribe to a topic
	 *
	 * Removes a specified call-back function from the subscription, or
	 * all of them by default.
	 *
	 * Note that this operation is intended to be asynchronous. Thus, it returns
	 * immediately, and the result of the unsubscribe attempt will be passed to
	 * the onUnsubscribed() call-back.
	 *
	 * @param[in] The topic name to unsubscribe to
	 * @param[in] The call-back to unsubscribe, or null for all call-backs
	 * @see #on_unsubscribed
	 */
	virtual void unsubscribe(TmxBrokerContext &, common::const_string, common::TmxTypeDescriptor const & = empty_descriptor()) noexcept;

    /*!
     * @param[in] The topic name
     * @return True if this client is currently subscribed with the broker for the topic. False otherwise
     */
    virtual bool is_subscribed(TmxBrokerContext const &, common::const_string) const noexcept;

    /*!
	 * @brief Publish a message to the broker
	 *
	 * The encoded message should not be modified in any way for consistent
	 * use across the application. The topic to publish to is embedded
	 * within the message, and may have to be created on the broker.
	 *
	 * Note that this operation is intended to be asynchronous. Thus, it returns
	 * immediately, and the result of the publish attempt will be passed to
	 * the onPublished() call-back.
	 *
	 * @param[in] The message to publish
	 * @see #on_published
	 */
	virtual void publish(TmxBrokerContext &, message::TmxMessage const &) noexcept;

    /*!
     * @brief Clean up all resources used in this client
     *
     * Note that this operation is intended to be asynchronous. Thus, it returns
     * immediately, and the result of the destruction attempt will be passed to
     * the onDestroyed() call-back.
     */
    virtual void destroy(TmxBrokerContext &) noexcept;

    // The call-back APIs
    // Default implementations are provided for all of these

    /*!
     * @brief Call-back for when the client is initialized
     *
     * @param[in] The result of the initialize operation
     */
    virtual void on_initialized(TmxBrokerContext &, common::TmxError const &) noexcept;

    /*!
     * @brief Call-back for when the client has tried to connect
     *
     * @param[in] The result of the connect operation
     */
    virtual void on_connected(TmxBrokerContext &, common::TmxError const &) noexcept;

    /*!
     * @brief Call-back for when the client has been disconnected
     *
     * @param[in] The result or reason for the disconnection
     */
    virtual void on_disconnected(TmxBrokerContext &, common::TmxError const &) noexcept;

    /*!
     * @brief Call-back for when a topic is subscribed to
     *
     * @param[in] The result of the subscribe operation
     * @param[in] The topic name that was used
     * @param[in] The call-back function that was used
     */
    virtual void on_subscribed(TmxBrokerContext &, common::TmxError const &,
                               common::const_string, common::TmxTypeDescriptor const &) noexcept;

    /*!
     * @brief Call-back for when a topic is unsubscribed to
     *
     * @param[in] The result of the unsubscribe operation
     * @param[in] The topic name that was used
     * @param[in] The call-back function that was used
     */
    virtual void on_unsubscribed(TmxBrokerContext &, common::TmxError const &,
                                 common::const_string, common::TmxTypeDescriptor const &) noexcept;

    /*!
     * @brief Call-back for when a publish operation is attempted
     *
     * @param[in] The result of the publish operation
     * @param[in] The message that was used
     */
    virtual void on_published(TmxBrokerContext &, common::TmxError const &, message::TmxMessage const &) noexcept;

    /*!
     * @brief Call-back for when a client is destroyed
     *
     * @param[in] The result of the destroy operation
     */
    virtual void on_destroyed(TmxBrokerContext &, common::TmxError const &) noexcept;

    /*!
     * @brief Call-back for when some unexpected error occurs
     *
     * Some examples that might invoke this call-back would include:
     * 	An exception was caught
     * 	The server returned some strange error
     * 	The client context was somehow lost
     *
     * @param[in] True if the error is fatal. False otherwise
     * @param[in] The result of the error callback
     */
    virtual void on_error(TmxBrokerContext &, common::TmxError const &, typename common::types::Boolean::value_type = false) noexcept;

    /*!
     * @brief Execute the callbacks registered in the context for the message topic
     *
     * Note that this function by default will synchronously invoke the call against
     * the default task executor for the context. This behavior can be modified by
     * overriding the method.
     *
     * @param[in] id The context id
     * @param[in] message The message to send
     * @return The result of the call
     */
    virtual void callback(common::const_string id, message::TmxMessage const &message) noexcept;

protected:
	/*!
	 * @brief The default constructor
	 *
	 * Protected so only derived classes may use
	 */
	TmxBrokerClient() noexcept = default;

    /*!
     * @brief Register this broker for use
     */
    void register_broker(common::const_string nm = common::empty_string()) const noexcept;

    /*!
     * @param id The context id
     * @return The context for the given ID, assuming it was registered, or null if not found
     */
    TmxBrokerContext *get_context(common::const_string id) const noexcept;

    /*!
     * @param[in] id The context id
     * @param[in] topic The topic name
     * @return A registry for callbacks registered to the context within the given topic
     */
    common::TmxTypeRegistry callback_registry(common::const_string id, common::const_string topic) const noexcept;

    /*!
     * @brief Register the context for later use
     *
     * This is done automatically in the base class within initialize()
     */
    void register_context(TmxBrokerContext &);

    /*!
     * @brief Unregister the context
     *
     * This is done automatically in the base class within destroy()
     */
    void unregister_context(TmxBrokerContext &);

 private:
    static const common::TmxTypeDescriptor &empty_descriptor();
};

} /* namespace broker */
} /* namespace tmx */

#endif /* INCLUDE_TMX_BROKER_TMXCLIENT_HPP_ */
