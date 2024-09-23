/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxGpsdBrokerClient.hpp
 *
 *  Created on: Jun 14, 2023
 *      @author: Gregory M. Baumgardner
 *  Edited on: May 7, 2024
 *      @author: Noah Limes
 */

#ifndef GPSD_INCLUDE_TMX_BROKER_GPSD_TMXGPSDBROKERCLIENT_HPP_
#define GPSD_INCLUDE_TMX_BROKER_GPSD_TMXGPSDBROKERCLIENT_HPP_

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/broker/async/TmxAsynchronousIOBroker.hpp>

namespace tmx {
namespace broker {
namespace gpsd {

/*!
 * @brief The TMX broker client implementation for connecting to gpsd
 */
class TmxGpsdBrokerClient: public async::TmxAsynchronousIOBrokerClient {
    typedef TmxGpsdBrokerClient self_type;
    typedef async::TmxAsynchronousIOBrokerClient super;

public:
    TmxGpsdBrokerClient() noexcept;
    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    common::types::Any get_broker_info(TmxBrokerContext &ctx) const noexcept override;

    //TMX Broker API
    void initialize(TmxBrokerContext &ctx) noexcept override;
    void destroy(TmxBrokerContext &ctx) noexcept override;
    void connect(TmxBrokerContext &ctx, common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect(TmxBrokerContext &ctx) noexcept override;
    void publish(TmxBrokerContext &ctx, message::TmxMessage const &) noexcept override;

private:
    common::TmxError to_message(boost::asio::const_buffer const &,
                                TmxBrokerContext &ctx, message::TmxMessage &) const noexcept override;

    // Asynchronous callbacks
    void on_gps_open(boost::system::error_code const &, std::reference_wrapper<TmxBrokerContext>) noexcept;
    void on_gps_stream(boost::system::error_code const &, std::reference_wrapper<TmxBrokerContext>) noexcept;
    void on_read(boost::system::error_code const &, std::size_t,
                 std::shared_ptr<boost::asio::streambuf>, std::reference_wrapper<TmxBrokerContext>) noexcept override;
};


} /* End namespace gpsd */
} /* End namespace broker */
} /* End namespace tmx */

#endif //GPSD_INCLUDE_TMX_BROKER_GPSD_TMXGPSDBROKERCLIENT_HPP_
