/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsynchronousIOBroker.hpp
 *
 *  Created on: May 01, 2024
 *      @author: Gregory M. Baumgardner
 */

#ifndef ASYNC_INCLUDE_TMX_BROKER_ASYNC_ASYNCHRONOUSIOBROKER_HPP
#define ASYNC_INCLUDE_TMX_BROKER_ASYNC_ASYNCHRONOUSIOBROKER_HPP

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <boost/asio.hpp>

namespace tmx {
namespace broker {
namespace async {

/*!
 * @brief A class to
 */
class TmxAsynchronousIOBrokerClient: public TmxBrokerClient {
public:
    virtual ~TmxAsynchronousIOBrokerClient() = default;

    virtual void subscribe(TmxBrokerContext &, common::const_string,
                           common::TmxTypeDescriptor const &) noexcept override;
    virtual void unsubscribe(TmxBrokerContext &, common::const_string,
                             common::TmxTypeDescriptor const &) noexcept override;

protected:
    typedef boost::asio::strand<boost::asio::thread_pool::executor_type> strand_t;

    TmxAsynchronousIOBrokerClient() = default;

    /*!
     * @brief Write the incoming buffer contents as a TmxMessage
     *
     * Note that the only guarantee is that the payload value is
     * correctly set. If the bytes read in are actually a complete
     * TMX message, as determined by the preamble, then every
     * possible part of the message is decoded. Otherwise, this
     * function assumes that the bytes are all payload, but will
     * still attempt to determine the encoding and some metadata.
     *
     * Any sub-class and any user of this broker should attempt to
     * fill in more information in the message, such as the topic
     * and source.
     *
     * @param[in] buf The buffer to read from
     * @param[in] message The message to fill out
     * @return An errors that occured during the operation
     */
    virtual common::TmxError to_message(boost::asio::const_buffer const &,
                                        TmxBrokerContext &ctx, message::TmxMessage &) const noexcept;

    /*!
     * @param[in] ctx The TMX broker context
     * @return The thread pool to use for this broker context
     */
    boost::asio::thread_pool &get_context(TmxBrokerContext &) const noexcept;

    /*!
     * @brief Create a new strand to use in the thread pool
     *
     * A strand ensures sequential operation of related tasks, thus
     * eliminating the need for explicit synchronization across the
     * tasks. This is most likely necessary for connect().
     *
     * Note that the new strand is NOT saved to the broker context
     * since using more than one strand may be necessary. It is
     * more effective to store a strand within a stored asynchronous
     * object itself and bind that executor to operations which
     * require the strand.
     *
     * @param[in] ctx The TMX broker context
     * @return A new strand to use for this context
     */
     strand_t make_strand(TmxBrokerContext &) const noexcept;

     template <typename _Context, typename _T>
     auto run_in(_Context &ctx, _T &&t) {
         return boost::asio::bind_executor(ctx, std::forward<_T>(t));
     }

    // Read/write handlers that are necessary to bridge the ASIO framework
    // to TMX

    /*!
     * @brief A handler for when write operations complete
     *
     * This callback should be used to fulfill the ASIO WriteToken
     * requirements, with the additional TMX context parameter.
     *
     * @param[in] ec The error code
     * @param[in] bytes The number of bytes written out
     * @param[in] ctx The TMX context to use
     * @param[in] msg The TMX message that was sent, if applicable
     */
    virtual void on_write(boost::system::error_code const &, std::size_t,
                          std::reference_wrapper<TmxBrokerContext>, message::TmxMessage const & = {}) noexcept;

    /*!
     * @brief A handler for when read operations complete
     *
     * This callback should be used to fulfill the ASIO ReadToken
     * requirements, with the additional TMX context parameter.
     * By default, this function checks for an error and executes
     * the callbacks.
     *
     * @param[in] ec The error code
     * @param[in] bytes The number of bytes written out
     * @param[in] buffer The buffer stream containing the bytes read
     * @param[in] ctx The TMX context to use
     */
    virtual void on_read(boost::system::error_code const &, std::size_t,
                         std::shared_ptr<boost::asio::streambuf>, std::reference_wrapper<TmxBrokerContext>) noexcept;
};

} /* End namespace async */
} /* End namespace broker */
} /* End namespace tmx */

#endif /* ASYNC_INCLUDE_TMX_BROKER_ASYNC_ASYNCHRONOUSIOBROKER_HPP */
