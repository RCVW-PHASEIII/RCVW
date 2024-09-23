/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file GPSDBrokerClient.hpp
 *
 *  Created on: Jun 14, 2023
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 7, 2024
 *      @author: Noah Limes
 */

#ifndef TMX_GPSDBROKERCLIENT_HPP
#define TMX_GPSDBROKERCLIENT_HPP

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <proton/messaging_handler.hpp>

namespace tmx {
namespace broker {
namespace gpsd {

/*!
 * @brief The TMX broker client implementation for gpsd
 *
 */
class GPSDBrokerClient: public TmxBrokerClient {
public:
    GPSDBrokerClient() noexcept;
    common::TmxTypeDescriptor get_descriptor() const noexcept override;

    //TMX Broker API
    void initialize(TmxBrokerContext &ctx) noexcept override;
    void destroy(TmxBrokerContext &ctx) noexcept override;
    void connect(TmxBrokerContext &ctx, common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect(TmxBrokerContext &ctx) noexcept override;
    void publish(TmxBrokerContext &ctx, message::TmxMessage const &) noexcept override;
    void subscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void unsubscribe(broker::TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    bool is_subscribed(TmxBrokerContext const &ctx, common::const_string) const noexcept override;

    void on_initialized(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_destroyed(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_connected(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_disconnected(TmxBrokerContext &ctx, common::TmxError const &) noexcept override;
    void on_subscribed(TmxBrokerContext &ctx, common::TmxError const &,
                       common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void on_unsubscribed(TmxBrokerContext &ctx, common::TmxError const &,
                         common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void on_published(TmxBrokerContext &ctx, common::TmxError const &, message::TmxMessage const &) noexcept override;

};

} /* End namespace gpsd */
} /* End namespace broker */
} /* End namespace tmx */

#endif /* TMX_GPSDBROKERCLIENT_HPP */
