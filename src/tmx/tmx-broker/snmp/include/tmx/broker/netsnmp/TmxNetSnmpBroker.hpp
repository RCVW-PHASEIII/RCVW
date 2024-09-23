/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxNetSnmpBroker.hpp
 *
 *  Created on: Nov 09, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef SNMP_INCLUDE_TMX_BROKER_SNMP_TMXNETSNMPCLIENT_HPP
#define SNMP_INCLUDE_TMX_BROKER_SNMP_TMXNETSNMPCLIENT_HPP

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxMessage.hpp>

namespace tmx {
namespace broker {
namespace netsnmp {

class TmxNetSnmpBrokerClient: public TmxBrokerClient {
public:
    TmxNetSnmpBrokerClient() noexcept;

    common::TmxTypeDescriptor get_descriptor() const noexcept override;
    common::types::Any get_broker_info(TmxBrokerContext &ctx) const noexcept override;

    void initialize(TmxBrokerContext &ctx) noexcept override;
    void connect(TmxBrokerContext &ctx, common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect(TmxBrokerContext &ctx) noexcept override;
    void subscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void unsubscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void publish(TmxBrokerContext &ctx, message::TmxMessage const &) noexcept override;
    void destroy(TmxBrokerContext &ctx) noexcept override;

    /*!
     * @brief Make an SNMP get request
     *
     * The broker context must already be connected
     *
     * @param[in] ctx The broker context to use
     * @param[in] oids The OIDs to request
     * @param[in] results A container to hold the results
     * @return The result of the operation
     */
    common::TmxError snmp_get(TmxBrokerContext &ctx,
                              common::types::Properties<common::types::Any> const &oids,
                              common::types::Any &results) const noexcept;

    /*!
     * @brief Make an SNMP set request
     *
     * The broker context must already be connected
     *
     * @param[in] ctx The broker context to use
     * @param[in] oids The OIDs and associated values to set
     * @return The result of the operation
     */
    common::TmxError snmp_set(TmxBrokerContext &ctx,
                              common::types::Properties<common::types::Any> const &oids) const noexcept;
};

} /* End namespace netsnmp */
} /* End namespace broker */
} /* End namespace tmx */

#endif /* SNMP_INCLUDE_TMX_BROKER_SNMP_TMXNETSNMPCLIENT_HPP */
