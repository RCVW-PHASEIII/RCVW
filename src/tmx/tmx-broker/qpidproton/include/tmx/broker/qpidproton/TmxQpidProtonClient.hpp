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

#include <proton/messaging_handler.hpp>
#include <proton/container.hpp>

#include <thread>

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
class TmxQpidProtonClient : public TmxBrokerClient {
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
};

/*!
 * A class for handling the Qpid Proton connection
 */
class TmxQpidProtonConnection: public TmxQpidProtonClient, public proton::messaging_handler {
public:
    TmxQpidProtonConnection(TmxBrokerContext &ctx);

    TmxBrokerContext &context() noexcept;
    proton::container &container() noexcept;
    std::string const &url() const noexcept;
private:
    void on_container_start(proton::container &) override;
    void on_container_stop(proton::container &) override;
    void on_transport_open(proton::transport &) override;
    void on_transport_close(proton::transport &) override;
    void on_transport_error(proton::transport &) override;
    void on_connection_open(proton::connection &) override;
    void on_connection_close(proton::connection &) override;
    void on_connection_error(proton::connection &) override;
    void on_connection_wake(proton::connection &) override;
    void event_loop() noexcept;

    TmxBrokerContext &_context;
    std::shared_ptr<proton::container> _container;
    std::string _url;
    std::thread _thread;
};

// Some functions for obtaining information from or for the context
TmxBrokerContext &to_context(proton::container const &) noexcept;
common::TmxError to_error(proton::error_condition const &) noexcept;
proton::error_condition to_error(common::TmxError const &) noexcept;

} /* End namesoace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */

#endif //TMX_TMXQPIDPROTONCLIENT_HPP
