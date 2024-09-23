/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxQpidddProtonClient.cpp
 *
 *  Created on: Jul 17, 2023
 *      @author: Nate Clark
 */

#include <tmx/broker/qpidproton/TmxQpidProtonClient.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/message/TmxData.hpp>

#include <proton/error_condition.hpp>
#include <proton/map.hpp>
#include <proton/scalar.hpp>
#include <proton/value.hpp>

#include <thread>

using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace qpidproton {

TmxQpidProtonClient::TmxQpidProtonClient() noexcept {
    this->register_broker("amqps");
    this->register_broker("amqp");
}

TmxTypeDescriptor TmxQpidProtonClient::get_descriptor() const noexcept {
    static const auto &desc = TmxBrokerClient::get_descriptor();
    return { desc.get_instance(), typeid(*this), "qpid-proton" };
}

TmxError TmxQpidProtonClient::to_error(const proton::error_condition &ec) noexcept {
    TmxError err { (ec ? -1 : 0), ec.name() + ": " + ec.description() };
    try {
        proton::map<typename types::String_::value_type, proton::scalar> map;
        proton::get(ec.properties(), map);

        if (map.exists("code"))
            err.operator[]("code") = message::TmxData(map.get("code")).to_int();
        if (map.exists("stacktrace"))
            err.operator[]("stacktrace") = map.get("stacktrace");
        if (map.exists("category"))
            err.operator[]("category") = map.get("category");
    } catch (proton::conversion_error ex) {
        // Ignore
    }

    return { err };
}

proton::error_condition TmxQpidProtonClient::to_error(const common::TmxError &err) noexcept {
    proton::map<typename types::String_::value_type, proton::scalar> map;
    for (auto const &kv: err)
        map.put(kv.first, message::TmxData(kv.second).to_string());

    return { type_fqname(err).data(), err.get_message(), map };
}

void TmxQpidProtonClient::on_error(const proton::error_condition &err) {
    static TmxBrokerContext _error_context;
    TmxBrokerClient::on_error(_error_context, this->to_error(err));
}

// Register the broker client
static TmxTypeRegistrar< TmxQpidProtonClient > _qpid_proton_registrar;

} /* End namesoace qpidproton */
} /* End namespace broker */
} /* End namespace tmx */
