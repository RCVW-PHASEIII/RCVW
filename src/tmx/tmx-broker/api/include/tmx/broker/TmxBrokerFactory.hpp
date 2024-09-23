/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerFactory.hpp
 *
 *  Created on: Apr 17, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_BROKER_TMXBROKERFACTORY_HPP_
#define INCLUDE_TMX_BROKER_TMXBROKERFACTORY_HPP_

#include <tmx/platform.hpp>

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>

#include <memory>
#include <type_traits>

namespace tmx {
namespace broker {

/*!
 * @brief A factory class for constructing and initializing TMX broker clients
 */
class TmxBrokerFactory {
public:
    /*!
     * @brief Default destructor
     */
    virtual ~TmxBrokerFactory() = default;

    /*!
     * @brief No copy constructor
     */
    TmxBrokerFactory(TmxBrokerFactory const &) = delete;

    /*!
     * @brief No assignment operator
     */
     TmxBrokerFactory &operator=(TmxBrokerFactory const &) = delete;

    /*!
     * @brief Initialize a new client with the given context
     *
     * The type of client used is determined by the scheme identified
     * in the context.
     *
     * This function may or may not create a new client object, or
     * reuse an existing, uninitialized one.
     *
     * @param[in] The context to use to initialize
     * @return A self-managing pointer to the client instance
     */
    static std::shared_ptr<TmxBrokerClient> initialize(TmxBrokerContext const &) noexcept;

protected:
    /*!
     * @brief Default constructor
     *
     * Protected so only sub-class may use
     */
    TmxBrokerFactory() noexcept = default;

    /*!
     * @brief Register this factory
     *
     * @param[in] The scheme name
     */
    void register_factory(common::const_string) const noexcept;

    /*!
     * @brief Get the broker instance pointer for the context
     *
     * The factory may choose to construct a new broker client or reuse
     * and existing, but uninitalized one. The shared pointer must be
     * reset to self-manage the destruction of the client.
     *
     * @param[out] The pointer to the client instance for the context, passed in as empty
     * @return The result of the operation
     */
    virtual common::TmxError get_instance(std::shared_ptr<TmxBrokerClient> &) const noexcept;
};


} /* End namespace broker */
} /* End namespace tmx */

#endif //INCLUDE_TMX_BROKER_TMXBROKERFACTORY_HPP_
