/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file KafkaBroker.hpp
 *
 *  Created on: Aug 30, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_BROKER_APACHE_KAFKA_KAFKABROKER_HPP_
#define API_INCLUDE_TMX_BROKER_APACHE_KAFKA_KAFKABROKER_HPP_

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>

#include <memory>
#include <librdkafka/rdkafkacpp.h>

namespace tmx {
namespace broker {
namespace apache {
namespace kafka {

class TmxKafkaBroker: public TmxBrokerClient,
	public virtual RdKafka::EventCb,
	public virtual RdKafka::DeliveryReportCb,
    public virtual RdKafka::RebalanceCb,
    public virtual RdKafka::ConsumeCb {
public:
	TmxKafkaBroker() noexcept;
	virtual ~TmxKafkaBroker() = default;

    common::TmxTypeDescriptor get_descriptor() const noexcept override;

    common::types::Any get_broker_info(TmxBrokerContext &ctx) const noexcept override;

    void initialize(TmxBrokerContext &ctx) noexcept override;
    void connect(TmxBrokerContext &ctx, common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect(TmxBrokerContext &ctx) noexcept override;
    void subscribe(TmxBrokerContext &ctx, common::const_string topicName, common::TmxTypeDescriptor const &) noexcept override;
    void unsubscribe(TmxBrokerContext &ctx, common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void publish(TmxBrokerContext &ctx, message::TmxMessage const &) noexcept override;
    void destroy(TmxBrokerContext &ctx) noexcept override;

private:
    void consume_cb(RdKafka::Message &, void *) override;
	void event_cb(RdKafka::Event &event) override;
	void dr_cb(RdKafka::Message &) override;
    void rebalance_cb(RdKafka::KafkaConsumer *, RdKafka::ErrorCode, std::vector<RdKafka::TopicPartition *> &) override;
};

} /* namespace kafka */
} /* namespace apache */
} /* namespace broker */
} /* namespace tmx */

#endif /* API_INCLUDE_TMX_BROKER_APACHE_KAFKA_KAFKABROKER_HPP_ */
