/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFileSystemBroker.hpp
 *
 *  Created on: Sep 8, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_BROKER_FILE_TMXFILESYSTEMBROKER_HPP_
#define API_INCLUDE_TMX_BROKER_FILE_TMXFILESYSTEMBROKER_HPP_

#include <tmx/broker/TmxBrokerClient.hpp>
#include <tmx/broker/TmxBrokerContext.hpp>
#include <tmx/common/types/Any.hpp>

#include <istream>
#include <ostream>

namespace tmx {
namespace broker {
namespace file {

class TmxFileSystemBroker: public TmxBrokerClient {
public:
    TmxFileSystemBroker() noexcept = default;

    void initialize(TmxBrokerContext const &) noexcept override;
    void connect(common::types::Any const & = common::types::no_data()) noexcept override;
    void disconnect() noexcept override;
    void subscribe(common::const_string, common::TmxTypeDescriptor const &) noexcept override;
    void publish(message::TmxMessage const &) noexcept override;

protected:
    void destroy() noexcept override;

private:
    common::types::Any _data;
};

} /* namespace file */
} /* namespace broker */
} /* namespace tmx */

#endif /* API_INCLUDE_TMX_BROKER_FILE_TMXFILESYSTEMBROKER_HPP_ */
