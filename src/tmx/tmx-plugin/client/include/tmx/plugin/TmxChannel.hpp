/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxChannel.hpp
 *
 *  Created on: May 08, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef CLIENT_INCLUDE_TMX_PLUGIN_TMXCHANNEL_HPP_
#define CLIENT_INCLUDE_TMX_PLUGIN_TMXCHANNEL_HPP_

#include <tmx/platform.hpp>

#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <future>
#include <memory>

namespace tmx {
namespace plugin {

/*!
 * @brief A container class for a given TMX broker client
 *
 * This class automatically maintains a stable connection to the
 * client, only offering necessary read/write capabilities. It
 * also behaves as a functor itself to facilitate callbacks into
 * this channel.
 */
class TmxChannel: public common::TmxFunctor<common::TmxTypeDescriptor const &, message::TmxMessage const &> {
public:
    /*!
     * @brief Construct a new channel for the given plugin
     *
     * All channel configuration data, including the broker
     * context information and specific broker parameters
     * must be passed in through the config parameter.
     *
     * @param[in] plugin The plugin type descriptor to which this channel belongs
     * @param[in] parameters The channel configuration parameters
     */
    TmxChannel(common::TmxTypeDescriptor const &, common::types::Any const &) noexcept;
    TmxChannel(TmxChannel &&) noexcept;

    /*!
     * @brief Destructor
     */
    virtual ~TmxChannel();

    // Remove copy constructor and assignment operator
    TmxChannel(TmxChannel const &) = delete;
    TmxChannel &operator=(TmxChannel const &) = delete;

    /*!
     * @brief Disconnect the channel
     */
    void disconnect() noexcept;

    /*!
     * @brief Connect the channel
     *
     * @param[in] parameters Additional connection parameters
     */
    void connect(common::types::Any const & = { }) noexcept;

    /*!
     * @brief Write the TMX message to the channel
     *
     * @param[in] message The message to write
     */
    void write_message(message::TmxMessage const &) noexcept;

    /*!
     * @brief Set up the channel to receive messages at the given topic
     *
     * @param[in] topic The topic to read from
     */
    void read_messages(common::const_string) noexcept;

    /*!
     * @brief Execute a messaging operation on this channel
     *
     * By default, a TMX channel uses the priority of the TMX message,
     * as determined by the metadata, for ordering the assignment of
     * operations. Likewise, the message metadata can explicitly control
     * which worker in a multi-threaded channel should be assigned to
     * execute the operation. By utilizing this function directly, one
     * ensure that the operation is given the proper assignment and
     * priority.
     *
     * The channel internally uses this function to ensure proper
     * assignment and priority of messages coming from or going to the
     * configured broker.
     *
     * @param[in] callback The call-back descriptor
     * @param[in] message The message to handle
     */
    common::TmxError execute(common::TmxTypeDescriptor const &, message::TmxMessage const &) override;

    /*!
     * @return A read-write reference to the broker context for this channel
     */
    broker::TmxBrokerContext &get_context() noexcept;

    /*!
     * @return A read-only reference to the broker context for this channel
     */
    broker::TmxBrokerContext const &get_context() const noexcept;

private:
    /*!
     * @brief The context data for this channel
     */
    common::types::Any _data;
};

} /* End namespace plugin */
} /* End namespace tmx */

#endif /* CLIENT_INCLUDE_TMX_PLUGIN_TMXCHANNEL_HPP_ */
