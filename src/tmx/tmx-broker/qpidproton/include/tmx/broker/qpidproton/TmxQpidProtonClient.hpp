/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidProtonClient.hpp
 *
 *  Created on: Jul 17, 2023
 *      @author: Nate Clark
 */

#ifndef TMX_TMXQPIDPROTONCLIENT_HPP
#define TMX_TMXQPIDPROTONCLIENT_HPP

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>

#include <proton/messaging_handler.hpp>

namespace tmx {
namespace broker {
namespace qpidproton {

/*!
 * @brief The TMX broker client implementation for Apache Qpid Proton
 *
 * This client, much like the Qpid Proton C++ API itself, is a true
 * asynchronous implementation. Once initialized, all requests are
 * scheduled within the Proton container. No assumptions can be
 * made regarding the state of the connection, as the state may
 * change intermittently. Therefore, some requests may ultimately
 * fail due after a minimum number of retries.
 *
 * The Qpid Proton messaging handler callback functions all invoke the
 * appropriate the TMX callbacks, making this client a good example
 * for asynchronous implementations using the TMX Broker API.
 */
class TmxQpidProtonClient : public TmxBrokerClient,
                            private proton::messaging_handler {
public:
    TmxQpidProtonClient() noexcept;
    common::TmxTypeDescriptor get_descriptor() const noexcept override;

    // The TMX Broker API
    void initialize(TmxBrokerContext &ctx) noexcept override;
    void destroy(TmxBrokerContext &ctx) noexcept override;
    void connect(TmxBrokerContext &ctx, common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect(TmxBrokerContext &ctx) noexcept override;
    void publish(TmxBrokerContext &ctx, message::TmxMessage const &) noexcept override;
    void subscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void unsubscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;

    void on_initialized(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_destroyed(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_connected(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_disconnected(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_subscribed(TmxBrokerContext &ctx, common::TmxError const &,
                       common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void on_unsubscribed(TmxBrokerContext &ctx, common::TmxError const &,
                         common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void on_published(TmxBrokerContext &ctx, common::TmxError const &, message::TmxMessage const &) noexcept override;

    // Some static functions for obtaining information from or for the context
    static TmxBrokerContext &get_context(proton::container const &) noexcept;
    static common::TmxError to_error(proton::error_condition const &) noexcept;
    static proton::error_condition to_error(common::TmxError const &) noexcept;
private:
    // Functions for obtaining key Qpid Proton C++ objects
    proton::container &get_container(TmxBrokerContext &ctx) noexcept;
    proton::connection &get_connection(TmxBrokerContext &ctx, proton::connection * = nullptr) noexcept;

    // Initialization handlers
    void on_container_start(proton::container &) override;
    void on_container_stop(proton::container &) override;

    // Connection handlers
    void on_connection_open(proton::connection &) override;
    void on_connection_close(proton::connection &) override;
    void on_connection_error(proton::connection &) override;
    void on_transport_open(proton::transport &) override;
    void on_transport_close(proton::transport &) override;
    void on_transport_error(proton::transport &) override;

    // Subscription handlers
    void on_receiver_open(proton::receiver &) override;
    void on_receiver_detach(proton::receiver &) override;
    void on_receiver_close(proton::receiver &) override;
    void on_receiver_error(proton::receiver &) override;
    void on_message(proton::delivery &, proton::message &) override;

    // Publish handlers
    void on_sender_open(proton::sender &) override;
    void on_sender_close(proton::sender &) override;
    void on_sender_detach(proton::sender &) override;
    void on_sender_error(proton::sender &) override;
    void on_tracker_settle(proton::tracker &) override;
    void on_sendable(proton::sender &) override;

    // Error handler
    void on_error(const proton::error_condition &) override;
};

} /* End namesoace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */

#endif //TMX_TMXQPIDPROTONCLIENT_HPP
