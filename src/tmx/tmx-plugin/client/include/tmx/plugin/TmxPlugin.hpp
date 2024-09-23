/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerClient.hpp
 *
 *  Created on: Aug 11, 2021
 *      @author: Rich Surgenor
 */

#ifndef CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGIN_HPP_
#define CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGIN_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTaskExecutor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/plugin/TmxChannel.hpp>
#include <tmx/plugin/TmxMessageHandler.hpp>
#include <tmx/plugin/utils/async/TmxRunnable.hpp>

#include <functional>
#include <stdexcept>

namespace tmx {
namespace plugin {

class TmxPlugin: public utils::async::TmxRunnable {
public:
    /*!
     * @brief Default constructor
     */
    TmxPlugin() noexcept = default;

    /*!
     * @brief Default destructor
     */
    virtual ~TmxPlugin() = default;

    // Remove the move/copy constructors and assignments
    TmxPlugin(TmxPlugin const &) = delete;
    TmxPlugin(TmxPlugin &&) = delete;
    TmxPlugin &operator=(TmxPlugin const &) = delete;
    TmxPlugin &operator=(TmxPlugin &&) = delete;

    /*!
     * @return A registry in a namespace specific for this plugin
     */
    virtual common::TmxTypeRegistry get_registry() const noexcept;

    /*!
     * @brief Get the plugin task executor
     *
     * The plugin by default will use the Boost ASIO system context, and
     * will automatically include enough threading for a main loop and a
     * secondary asynchronous processing loop.
     *
     * @return A task executor for running asynchronous operations in this plugin
     */
    virtual common::TmxTaskExecutor &get_executor() noexcept;

    /*!
     * @brief Start this plugin
     *
     * This function should launch operations inside new threads
     * and thus will return immediately.
     */
    virtual void start() override;

    /*!
     * @brief Stop this plugin
     *
     * This function terminates any outstanding operations.
     */
    virtual void stop() override;

    /*!
     * @return The type descriptor for this plugin
     */
    virtual common::TmxTypeDescriptor get_descriptor() const noexcept;

    /*!
     * @brief Process command line arguments for this plugin
     *
     * @param[in] args The plugin program arguments
     * @return Any error that occurs during processing
     */
    virtual common::TmxError process_args(TmxRunnableArgs const &) override;

    /*!
     * @brief Run the main loop for this plugin
     *
     * It is guaranteed that the program arguments will be processed
     * and cached as config properties prior to starting the main()
     * operation. In most cases, this function will run until the
     * plugin is stopped.
     *
     * @return Any error that occurs during the main loop
     */
    virtual common::TmxError main() override;

    /*!
     * @brief Execute this plugin
     *
     * This function should not be called directly, as it is generally
     * used by program launchers to start the plugin and will block
     * until the plugin is stopped.
     *
     * @param[in] args The plugin program arguments
     * @return Any error that occurs during the launch
     */
    virtual common::TmxError execute(TmxRunnableArgs const &) override;

    /*!
     * @brief Get a configuration property from cache
     *
     * A config property represents a plug-in configuration option that
     * may be set at run-time. These are, of course, defaulted through
     * command-line arguments or manifest parameters, but are also
     * intended to be altered from some external entity, like a management
     * system or database.
     *
     * @param[in] key The key or name of the config property
     * @param[in] mtx A mutex to lock around the changes to the cache
     * @return The cached configuration value for the given key
     */
    virtual message::TmxData get_config(common::const_string, std::mutex * = nullptr) const;

    /*!
     * @brief Set a configuration property in the cache
     *
     * A config property represents a plug-in configuration option that
     * may be set at run-time. These are, of course, defaulted through
     * command-line arguments or manifest parameters, but are also
     * intended to be altered from some external entity, like a management
     * system or database.
     *
     * @param[in] key The key or name of the config property
     * @param[in] value The value to set for this config property
     * @param[in] mtx A mutex to lock around the changes to the cache
     */
    virtual void set_config(common::const_string, common::types::Any const &, std::mutex * = nullptr);

    /*!
     * @brief Set a configuration property in the cache
     *
     * A config property represents a plug-in configuration option that
     * may be set at run-time. These are, of course, defaulted through
     * command-line arguments or manifest parameters, but are also
     * intended to be altered from some external entity, like a management
     * system or database.
     *
     * @param[in] key The key or name of the config property
     * @param[in] value The string value to set for this config property
     * @param[in] mtx A mutex to lock around the changes to the cache
     */
    void set_config(common::const_string, const char *, std::mutex * = nullptr);

    /*!
     * @brief Get a status property from cache
     *
     * A status is information about the running program that is likely always
     * set by the plug-in code but is meant to be consumed externally, such as
     * by some display interface. These messages can be simply informational,
     * such as the State status indicating the current operational state of the
     * plug-in, but others may be used for diagnostic purposes by an external observer
     *
     * @param[in] key The key or name of the status property
     * @param[in] mtx A mutex to lock around the changes to the cache
     * @return The cached status value for the given key
     */
    virtual message::TmxData get_status(common::const_string, std::mutex * = nullptr) const;

    /*!
     * @brief Set a status property in the cache
     *
     * A status is information about the running program that is likely always
     * set by the plug-in code but is meant to be consumed externally, such as
     * by some display interface. These messages can be simply informational,
     * such as the State status indicating the current operational state of the
     * plug-in, but others may be used for diagnostic purposes by an external observer
     *
     * @param[in] key The key or name of the status property
     * @param[in] value The value to set for this status property
     * @param[in] mtx A mutex to lock around the changes to the cache
     */
    virtual void set_status(common::const_string, common::types::Any const &, std::mutex * = nullptr);

    /*!
     * @brief Set a status property in the cache
     *
     * A status is information about the running program that is likely always
     * set by the plug-in code but is meant to be consumed externally, such as
     * by some display interface. These messages can be simply informational,
     * such as the State status indicating the current operational state of the
     * plug-in, but others may be used for diagnostic purposes by an external observer
     *
     * @param[in] key The key or name of the status property
     * @param[in] value The string value to set for this status property
     * @param[in] mtx A mutex to lock around the changes to the cache
     */
    void set_status(common::const_string, const char *, std::mutex * = nullptr);

    /*!
     * @brief Get all the messaging channels for this plugin
     *
     * A TMX channel represents a single, managed connection to some
     * TMX broker service.
     *
     * Note that this is a constant, read-only version for external use
     *
     * @return The list of message channels for this plugin
     */
    common::types::Array<std::shared_ptr<TmxChannel> > const &get_channels() const noexcept;

    /*!
     * @return The channel for the given context ID, if it exists
     */
    std::shared_ptr<TmxChannel> get_channel(common::const_string) noexcept;

    /*!
     * @brief Construct a topic name for this plugin
     *
     * This prepends the fully-qualified plugin name to the given topic
     * name. Note that this should only be used for internal plugin
     * topics, such as the config messages since these are specific to
     * the plugin and not the data.
     *
     * @param name The topic name for this plugin
     * @return The fully-qualified topic name for this plugin
     */
    virtual typename common::types::String_::value_type get_topic(common::const_string) const;

    /*!
     * @brief Register the specified handler function for the data access object
     *
     * A DAO is just a structure of data in class form that must be constructible
     * from an Any type. The plugin will automatically construct a DAO instance
     * from the decoded message payload, and then invoke the handler function.
     *
     * Note that only one handler for the given DAO can be registered in TMX.
     * Therefore, the handler function signature must be unique. Since it is
     * often convenient to use the generic TmxData type DAO, this restriction
     * would mean that only one handler would be called no matter the message
     * topic. Therefore, the _Tag type is explicitly used to ensure that the
     * multiple handlers can use the same DAO type across different message
     * topics, provided that each registration uses a unique tag.
     *
     * @throws std::invalid_argument If the instance is null
     * @throws std::invalid_argument If a handler for that DAO already has been registered
     * @tparam _Dao The data access object type for this handler
     * @tparam _Tag The tag type for this registration
     * @tparam _Ret The return value of the handler
     * @param topic The topic for which the message is expected to be received
     * @param handler The message handler function definition
     */
    template <typename _Dao, typename _Tag, typename _Ret>
    void register_handler(common::const_string topic, TmxMessageHandler<_Dao, _Tag, _Ret> const &handler) const {
        if (!handler)
            throw std::invalid_argument("The handler instance must be valid");

        std::string nm { common::type_fqname<_Tag>().data() };
        nm.append("|handle|");
        nm.append(common::type_short_name<_Dao>().data());

        auto reg = (this->get_registry() / topic.data());

        reg.register_type(handler.get_descriptor().get_instance(), handler.get_descriptor().get_typeid(), nm);
    }

    /*!
     * @brief Register the specified handler function for the data access object
     *
     * A DAO is just a structure of data in class form that must be constructible
     * from an Any type. The plugin will automatically construct a DAO instance
     * from the decoded message payload, and then invoke the handler function.
     *
     * Note that only one handler for the given DAO can be registered in TMX.
     * Therefore, the handler function signature must be unique. Since it is
     * often convenient to use the generic TmxData type DAO, this restriction
     * would mean that only one handler would be called no matter the message
     * topic. Therefore, the _Tag type is explicitly used to ensure that the
     * multiple handlers can use the same DAO type across different message
     * topics, provided that each registration uses a unique tag.
     *
     * @throws std::invalid_argument If the instance is null
     * @throws std::invalid_argument If a handler for that DAO already has been registered
     * @tparam _Tag The tag type for this registration
     * @tparam _Dao The data access object type for this handler
     * @tparam _Handler The plugin or other class in which the member function belongs
     * @tparam _Ret The return type of the handler
     * @param topic The topic for which the message is expected to be received
     * @param instance The message handler object instance
     * @param handler The message handler function definition
     */
    template <typename _Tag, typename _Dao, class _Handler, typename _Ret>
    void register_handler(common::const_string topic, _Handler *instance,
                          _Ret (_Handler::*handler)(_Dao &, message::TmxMessage const &)) const {
        static const auto &_hndlr = make_handler<_Tag, _Dao, _Ret>(instance, handler);
        this->register_handler(topic, _hndlr);
    }

    /*!
     * @brief Remove a handler for the given topic
     *
     * @tparam _Dao The data access object type for this handler
     * @tparam _Tag The tag type for this registration
     * @param topic The topic name to unregister
     */
    template <typename _Dao, typename _Tag>
    void unregister_handler(common::const_string topic) const {
        std::string nm { common::type_fqname<_Tag>().data() };
        nm.append("|handle|");
        nm.append(common::type_short_name<_Dao>().data());

        auto reg = (this->get_registry() / topic.data());
        reg.unregister(nm);
    }

    /*!
     * @brief A generic template message receiver for the plugin
     *
     * Requires template specialization within the tmx::plugin
     * namespace for use. This function provides a generic
     * template for building TMX message handlers without
     * explicitly declaring handler functions in the plugin
     * header, thus wholly encapsulating the details of what
     * messages the plugin may be consuming.
     *
     * Some caveats do exist, however. First, one must complete
     * the template specialization definition in the tmx::plugin
     * namespace prior to calling register_handler(), which
     * generally is done in the constructor of the plugin.
     * Secondly, the tag becomes most important when using this
     * function correctly. Therefore, the tag type name should
     * generally represent what the handler is doing in help
     * in debug logging. This is the case with the internal plugin
     * config, status and error handlers.
     *
     * Note that unless the encapsulation benefits are needed, it
     * is often much easier just to declare a unique handler
     * function within the plugin. However, even so the signature
     * of the handler MUST be:
     *
     * <return_type> handle(<DAO type> &, TmxMessage const &);
     *
     * @param[in] DAO The message payload to handle, decoded from message
     * @param[in] message The received TMX message
     * @return The return of the handler, generally a void
     */
    template <typename _Dao, typename _Tag = _Dao, typename _Ret = void>
    _Ret on_message_received(_Dao &, message::TmxMessage const &);

    /*!
     * @brief Asynchronously calls the registered handlers for the topic
     *
     * Note that this function expects the message payload to have already
     * been decoded to an Any type. Each handler may have a unique DAO
     * type, which must be convertible from this Any data, which includes
     * the generic TmxData class that can be used for extracting scalars,
     * arrays and/or properties.
     *
     * Note that this function can be used to short-cut the decoding process
     * if the data is already known. The incoming message is only passed
     * thru and may be ignored by the handler. Also note that only the
     * registered handlers will be invoked, including any registered handlers
     * that belong to objects external to this plugin class.
     *
     * @param data The decoded data from the message
     * @param message The received TMX message
     * @param topic The topic name to use, defaults to the message topic
     */
    virtual void invoke_handlers(common::types::Any const &, message::TmxMessage const &,
                                 common::const_string = "");

    /*!
     * @brief The main call-back for message being received on a channel
     *
     * This function first decodes the message, the invokes the handlers.
     *
     * Any errors that occur at any point in the receipt, decode or handling
     * of the message should be broadcast to the error channel, where
     * it can be handled by special functions dedicated to errors.
     *
     * @param[in] message The received TMX message
     */
    virtual void on_message_received(message::TmxMessage const &);

    /*!
     * @brief Asynchronously send a message across every channel
     *
     * Any errors that occur at any point in the construction, encoding
     * or sending of the message should be broadcast to the error
     * channel, where it can be handled by special functions dedicated
     * to errors.
     *
     * @param[in] message The TMX message to send
     */
    virtual void broadcast(message::TmxMessage const &);

    /*!
     * @brief Asynchronously send a message across every channel
     *
     * This functions builds and encodes a new message from the given
     * payload. Any errors that occur at any point in the construction,
     * encoding or sending of the message should be broadcast to the error
     * channel, where it can be handled by special functions dedicated
     * to errors.
     *
     * Note that the source of the message is automatically prepended by
     * the hostname and the plugin name.
     *
     * @param[in] data The payload to use
     * @param[in] topic The topic name to use in the message
     * @param[in] source The source of the message, generally the function
     * @param[in] encoding The encoding to use, defaults to JSON
     */
    virtual void broadcast(common::types::Any const &, common::const_string,
                           common::const_string = common::empty_string(),
                           common::const_string = common::empty_string());

    /*!
     * @brief Asynchronously send a message across every channel
     *
     * This functions builds and encodes a new message from the given
     * payload. Any errors that occur at any point in the construction,
     * encoding or sending of the message should be broadcast to the error
     * channel, where it can be handled by special functions dedicated
     * to errors.
     *
     * Note that the source of the message is automatically prepended by
     * the hostname and the plugin name.
     *
     * @param[in] data The payload to use
     * @param[in] topic The topic name to use in the message
     * @param[in] source The source of the message, generally the function
     * @param[in] encoding The encoding to use, defaults to JSON
     */
    template <typename _Dao>
    void broadcast(_Dao const &data, common::const_string topic,
                               common::const_string source = common::empty_string(),
                               common::const_string encoding = common::empty_string()) {
        if (common::types::IsTmxType<_Dao>::value) {
            this->broadcast(common::types::Any(data), topic, source, encoding);
            return;
        }

        // TODO: Use Dao traits to convert to Any
        common::types::Any _data { data };
        this->broadcast(_data, topic, source, encoding);
    }

    /*!
     * @brief Get the list of all registered plugins in this program
     *
     * @return The list of registered plugins
     */
    static common::types::Array<std::shared_ptr<TmxPlugin> > get_all_plugins() noexcept;

protected:
    /*!
     * @brief Initialize this plugin
     *
     * This function is explicitly meant to occur after arguments were
     * processed, but before the main() function is called. By default,
     * this step registers the default plugin handlers, and sets up
     * topic callbacks with all registered brokers. Since there are
     * cases where this registration process must occur during run-time,
     * this function must also be re-runnable during the main()
     * processing.
     *
     * While this function may be overridden for plugin specific
     * initialization, the default functionality MUST be invoked first.
     */
    virtual void init();

    /*!
     * Note that this is a read-write version for internal plugin use
     *
     * @return The list of message channels for this plugin
     */
    common::types::Array<std::shared_ptr<TmxChannel> > &get_channels() noexcept;

    /*!
     * @brief Returns a description of the configuration to use for this plugin
     *
     * The expected format for each parameter is a Map that contains a
     * "name" of the parameter, a text "description" for the parameter, and
     * a "default" value. Each parameter description will be translated
     * into plugin program arguments, and the assigned value will be saved
     * as a config parameter upon inialization of the plugin.
     *
     * The default arguments provided by this parent class are added outside
     * the base class, thus this function returns an empty structure by default.
     *
     * @return The list of parameters for
     */
    virtual common::types::Array<common::types::Any> get_config_description() const noexcept;

private:
    // Config and status property caches
	message::TmxData _config;
	message::TmxData _status;

    // The channels for this plugin
    common::types::Array<std::shared_ptr<TmxChannel> > _channels;
 };

/*!
 * @brief Specialization for handling an error in the plugin
 *
 * This captures error messages and invokes the error handler as
 * well as broadcasting a message to the specified topic.
 */
template <>
inline void TmxPlugin::broadcast<common::TmxError>(common::TmxError const &error, common::const_string topic,
                                                   common::const_string source, common::const_string encoding) {
    typedef common::types::Properties<common::types::Any> props_type;
    props_type const &err = static_cast<const props_type &>(error);

    TLOG(ERR) << message::TmxData(err).to_string();

    // Broadcast error message to outgoing channels
    this->broadcast(common::types::Any(err), this->get_topic("error"), source, encoding);

    message::TmxMessage msg;
    msg.set_id(type_fqname(error).data());
    msg.set_topic(topic.data());
    msg.set_source(source.data());
    msg.set_encoding("json");
    msg.set_timepoint();
    msg.set_payload("null");

    // Also, call local error handlers.
    // Note that this should be done last in case the error terminates the plugin
    this->invoke_handlers(err, msg, topic);
}

} /* namespace plugin */
} /* namespace tmx */

#endif /* CLIENT_INCLUDE_TMX_PLUGIN_TMXPLUGIN_HPP_ */
