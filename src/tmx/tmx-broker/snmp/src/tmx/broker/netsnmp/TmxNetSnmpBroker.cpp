/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxNetSnmpBroker.cpp
 *
 *  Created on: Nov 09, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/netsnmp/TmxNetSnmpBroker.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <arpa/inet.h>
#include <bitset>
#include <future>
#include <netdb.h>
#include <sys/socket.h>
#include <thread>
#include <utility>

#define BUFFER_SIZE 1024

using namespace tmx::common;
using namespace tmx::message;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace broker {
namespace netsnmp {

typedef typename tmx::common::TmxFunctor<types::Any const &, TmxMessage const &>::type::type cb_type;

enum class SNMP_VERSION: std::uint8_t {
    snmpv1 = SNMP_VERSION_1,
    snmpv2c = SNMP_VERSION_2c,
    snmpv2u = SNMP_VERSION_2u,
    snmpv3 = SNMP_VERSION_3
};

enum class SNMP_SECURITY_LEVEL: std::uint8_t {
    noAuthNoPriv = SNMP_SEC_LEVEL_NOAUTH,
    authNoPriv = SNMP_SEC_LEVEL_AUTHNOPRIV,
    authPriv = SNMP_SEC_LEVEL_AUTHPRIV
};

static const typename types::Properties_::key_t _session { "session" };
static const typename types::Properties_::key_t _handle  { "handle" };

TmxNetSnmpBrokerClient::TmxNetSnmpBrokerClient() noexcept {
    this->register_broker(enums::enum_name(SNMP_VERSION::snmpv1));
    this->register_broker(enums::enum_name(SNMP_VERSION::snmpv2c));
    this->register_broker(enums::enum_name(SNMP_VERSION::snmpv3));
    this->register_broker("ntcip");
}

TmxTypeDescriptor TmxNetSnmpBrokerClient::get_descriptor() const noexcept {
    static const auto &desc = TmxBrokerClient::get_descriptor();
    return { desc.get_instance(), typeid(*this), type_fqname(*this).data() };
}

void close_snmp(snmp_session *session = nullptr) noexcept {
    if (session) {
        snmp_close(session);

        if (session->peername) free(session->peername);
        if (session->community) free (session->community);
        if (session->securityName) free (session->securityName);
        if (session->contextName) free (session->contextName);
        if (session->securityEngineID) free (session->securityEngineID);
        if (session->contextEngineID) free (session->contextEngineID);
        if (session->securityAuthProto) free (session->securityAuthProto);
        if (session->securityPrivProto) free (session->securityPrivProto);

        session->peername = nullptr;
        session->community = nullptr;
        session->securityName = nullptr;
        session->contextName = nullptr;
        session->securityEngineID = nullptr;
        session->contextEngineID = nullptr;
        session->securityAuthProto = nullptr;
        session->securityPrivProto = nullptr;
    }
}

void TmxNetSnmpBrokerClient::initialize(TmxBrokerContext &ctx) noexcept {
    std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
    if ( !(TmxBrokerState::uninitialized == ctx.get_state()))
        return;

    init_snmp("tmx.broker.netsnmp");

    const TmxData params { ctx.get_parameters() };

    // Initialize the MIB directories
    netsnmp_get_mib_directory();
    for (std::size_t i = 0; i < params["mib-dirs"].to_uint(); i++) {
        TLOG(INFO) << "Adding MIB dir " << params["mib-dirs"][i].to_string();
        add_mibdir(params["mib-dirs"][i].to_string().c_str());
    }

    init_mib_internals();
    init_mib();

    // Create a new session
    auto &session = ctx[_session].emplace<snmp_session>();
    snmp_sess_init(&session);

    // Set the version, defaulting to SNMP version 3 (for NTCIP management support)
    auto ver = enums::enum_cast<SNMP_VERSION>(ctx.get_scheme());
    if (ver)
        session.version = enums::enum_integer(ver.value());
    else
        session.version = enums::enum_integer(SNMP_VERSION::snmpv3);

    // Add the SNMP peer
    std::string peer = ctx.get_host();
    if (!ctx.get_port().empty())
        peer += ":" + ctx.get_port();

    session.peername = strdup(peer.c_str());

    if (session.version == enums::enum_integer(SNMP_VERSION::snmpv3)) {
        // SNMP v3 argument handling, adapted from net-snmp/snmplib/snmpv3.c
        // pursuant to https://github.com/net-snmp/net-snmp/tree/master?tab=License-1-ov-file
        // all rights reserved

        // Set the security level, defaults to authentication and privacy
        session.securityLevel = enums::enum_integer(SNMP_SECURITY_LEVEL::authPriv);
        if (params["security-level"])
            if (auto s = enums::enum_cast<SNMP_SECURITY_LEVEL>(params["security-level"].to_string()))
                session.securityLevel = enums::enum_integer(s.value());

        // The security user name, which could be specified as the user portion of the context
        free(session.securityName);
        if (params["security-name"])
            session.securityName = strdup(params["security-name"].to_string().c_str());
        else if (ctx.get_user())
            session.securityName = strdup(ctx.get_user().c_str());
        else
            session.securityName = strdup("");

        session.securityNameLen = strlen(session.securityName);

        // The context name
        free(session.contextName);
        session.contextName = nullptr;
        if (params["context-name"]) {
            session.contextName = strdup(params["context-name"].to_string().c_str());
            session.contextNameLen = strlen(session.contextName);
        }

        // The security engine ID
        if (params["security-engine-id"]) {
            auto bytes = byte_string_decode(params["security-engine-id"].to_string().c_str());
            if (bytes.length() < 5 && bytes.length() > 32) {
                std::string err { "Invalid security engine ID " };
                err.append(params["security-engine-id"].to_string());
                err.append(". Must be between 10 and 64 bytes.");
                this->on_initialized(ctx, { -1 * bytes.length(), err });
                return;
            } else {
                session.securityEngineID = (uchar_t *)strdup(to_char_sequence(bytes.data()).data());
                session.securityEngineIDLen = bytes.length();
            }
        }

        // The context engine ID
        if (params["context-engine-id"]) {
            auto bytes = byte_string_decode(params["context-engine-id"].to_string().c_str());
            if (bytes.length() < 5 && bytes.length() > 32) {
                std::string err { "Invalid context engine ID " };
                err.append(params["context-engine-id"].to_string());
                err.append(". Must be between 10 and 64 bytes.");
                this->on_initialized(ctx, { -1 * bytes.length(), err });
                return;
            } else {
                session.contextEngineID = (uchar_t *)strdup(to_char_sequence(bytes.data()).data());
                session.contextEngineIDLen = bytes.length();
            }
        }

        // Authentication protocol type
        free(session.securityAuthProto);
        session.securityAuthProto = nullptr;
        if (params["authentication-protocol"]) {
            auto type = usm_lookup_auth_type(params["authentication-protocol"].to_string().c_str());
            if (type > 0) {
                const oid *auth_proto = sc_get_auth_oid(type, &session.securityAuthProtoLen);
                session.securityAuthProto = snmp_duplicate_objid(auth_proto, session.securityAuthProtoLen);
            } else {
                std::string err { "Invalid authentication protocol " };
                err.append(params["authentication-protocol"].to_string());
                err.append(". Valid values are (MD5|SHA).");

                this->on_initialized(ctx, { type, err });
                close_snmp(&session);
                return;
            }
        }

        // Privacy protocol type
        free(session.securityPrivProto);
        session.securityPrivProto = nullptr;
        if (params["privacy-protocol"]) {
            auto type = usm_lookup_priv_type(params["privacy-protocol"].to_string().c_str());
            if (type > 0) {
                const oid *priv_proto = sc_get_priv_oid(type, &session.securityPrivProtoLen);
                session.securityPrivProto = snmp_duplicate_objid(priv_proto,
                                                                 session.securityPrivProtoLen);
            } else {
                std::string err { "Invalid privacy protocol " };
                err.append(params["privacy-protocol"].to_string());
                err.append(". Valid values are (DES|AES)");

                this->on_initialized(ctx, { type, err });
                close_snmp(&session);
                return;
            }
        }

        // Authentication passphrase
        session.securityAuthKeyLen = USM_AUTH_KU_LEN;
        if (!session.securityAuthProto) {
            /*
             * get .conf set default
             */
            const oid *def = get_default_authtype(&session.securityAuthProtoLen);
            session.securityAuthProto = snmp_duplicate_objid(def, session.securityAuthProtoLen);
        }
        if (!session.securityAuthProto) {
            session.securityAuthProto = snmp_duplicate_objid(SNMP_DEFAULT_AUTH_PROTO, SNMP_DEFAULT_AUTH_PROTOLEN);
            session.securityAuthProtoLen = SNMP_DEFAULT_AUTH_PROTOLEN;
        }

        if (params["authentication-passphrase"]) {
            const auto &str = params["authentication-passphrase"].to_string();
            if (auto ret = generate_Ku(session.securityAuthProto,
                                       session.securityAuthProtoLen,
                                       (u_char *) str.c_str(), str.length(),
                                       session.securityAuthKey,
                                       &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
                std::string err{ "Error generating a key (Ku) from the supplied authentication pass phrase." };
                this->on_initialized(ctx, { ret, err });
                return;
            }
        }

        // Privacy passphrase
        session.securityPrivKeyLen = USM_PRIV_KU_LEN;
        if (!session.securityPrivProto) {
            /*
             * get .conf set default
             */
            const oid *def = get_default_privtype(&session.securityPrivProtoLen);
            session.securityPrivProto = snmp_duplicate_objid(def, session.securityPrivProtoLen);
        }
        if (!session.securityPrivProto) {
            session.securityPrivProto = snmp_duplicate_objid(SNMP_DEFAULT_PRIV_PROTO, SNMP_DEFAULT_PRIV_PROTOLEN);
            session.securityPrivProtoLen = SNMP_DEFAULT_PRIV_PROTOLEN;
        }

        if (params["privacy-passphrase"]) {
            const auto &str = params["privacy-passphrase"].to_string();
            if (auto ret = generate_Ku(session.securityAuthProto,
                                       session.securityAuthProtoLen,
                                       (u_char *) str.c_str(), str.length(),
                                       session.securityPrivKey,
                                       &session.securityPrivKeyLen) != SNMPERR_SUCCESS) {
                std::string err { "Error generating a key (Ku) from the supplied privacy pass phrase." };
                this->on_initialized(ctx, { ret, err });
                return;
            }
        }

        // Engine boots
        if (params["engine-boots"])
            session.engineBoots = params["engine-boots"].to_uint();

        // Engine time
        if (params["engine-time"])
            session.engineTime = params["engine-time"].to_uint();
    } else {
        free(session.community);
        session.community = nullptr;

        // Community string may be the secret or the username
        if (params["community"])
            session.community = (uchar_t *)strdup(params["community"].to_string().c_str());
        else if (!ctx.get_secret().empty())
            session.community = (uchar_t *)strdup(ctx.get_secret().c_str());
        else if (!ctx.get_user().empty())
            session.community = (uchar_t *)strdup(ctx.get_user().c_str());

        if (session.community)
            session.community_len = strlen((const char *)session.community);
        else
            session.community_len = 0;
    }

    // Timeout, specified in milliseconds but stored as microseconds
    if (params["timeout"])
        session.timeout = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::milliseconds(params["timeout"].to_int())).count();

    TmxBrokerClient::initialize(ctx);
}

void TmxNetSnmpBrokerClient::destroy(TmxBrokerContext &ctx) noexcept {
    shutdown_mib();

    if (this->is_connected(ctx))
        this->disconnect(ctx);

    if (ctx.count(_session)) {
        auto session = tmx::common::types::as<snmp_session>(ctx.at(_session));
        close_snmp(session.get());
    }

    ctx.erase(_session);
    TmxBrokerClient::destroy(ctx);
}

void TmxNetSnmpBrokerClient::disconnect(TmxBrokerContext &ctx) noexcept {
    ctx.erase(_handle);
    TmxBrokerClient::disconnect(ctx);
}

void TmxNetSnmpBrokerClient::connect(TmxBrokerContext &ctx, types::Any const &p) noexcept {
    std::lock_guard<std::mutex> lock(ctx.get_thread_lock());
    if (ctx.get_state() != TmxBrokerState::initialized && ctx.get_state() != TmxBrokerState::disconnected)
        return;

    auto init = tmx::common::types::as<snmp_session>(ctx.at(_session));
    if (!init) {
        this->on_connected(ctx, { 2, "Broker context " + ctx.to_string() + " not initialized properly." });
        return;
    }

    // Use the destructor function in the smart pointer
    auto session = ctx[_handle].emplace<std::shared_ptr<snmp_session> >(snmp_open(init.get()), &snmp_close);
    if (!session) {
        this->on_connected(ctx, { -1, "snmp_open() failed for unknown reason." });
        return;
    }

    TmxBrokerClient::connect(ctx, p);
}

void TmxNetSnmpBrokerClient::publish(TmxBrokerContext &ctx, message::TmxMessage const &msg) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_published(ctx, { 1, "Broker context " + ctx.to_string() + " is not connected." }, msg);
        return;
    }

    // Decode the message payload, which includes the OID list or OID:value list
    TmxData _data;
    codec::TmxCodec _codec(msg);
    auto result = _codec.decode(_data.get_container());
    if (result) {
        this->on_published(ctx, result, msg);
        this->disconnect(ctx);
        return;
    }

    if (_codec.get_message().get_source().empty())
        _codec.get_message().set_source(ctx.get_id());

    if (!_data.is_map()) {
        this->on_published(ctx, { 7, "Unable to get OID:value pairs from message" }, msg);
        this->disconnect(ctx);
        return;
    }

    auto oids = _data.to_map();
    if (std::strncmp("snmpset", msg.get_topic().c_str(), 7) == 0) {
        this->on_published(ctx, this->snmp_set(ctx, oids), msg);
        this->disconnect(ctx);
        return;
    } else if (std::strncmp("snmpget", msg.get_topic().c_str(), 7) == 0) {
        types::Any results;
        result = this->snmp_get(ctx, oids, results);
        this->on_published(ctx, result, msg);

        if (!result) {
            // Encode the message
            result = _codec.encode(results, "json");
            if (result) {
                this->on_error(ctx, result);
                this->disconnect(ctx);
                return;
            }

            types::Any _id;
            _id.emplace<std::string>(ctx.get_id());

            // Invoke the callbacks
            this->callback(ctx.get_id(), _codec.get_message());
        }
    } else {
        this->on_published(ctx, { 0, "Topic name " + msg.get_topic() + " skipped: Must start with set or get" }, msg);
        this->disconnect(ctx);
        return;
    }
}

void TmxNetSnmpBrokerClient::subscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &cb) noexcept {
    if (!this->is_connected(ctx)) {
        this->on_subscribed(ctx, { 1, "Broker context " + ctx.to_string() + " is not connected." }, topic, cb);
        return;
    }

    if (!cb) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not valid.");
        this->on_subscribed(ctx, { 4, err }, topic, cb);
        return;
    }

    auto callback = cb.as_instance<cb_type>();

    if (!callback) {
        std::string err { "Callback " };
        err.append(cb.get_type_name());
        err.append(" is not the correct signature. Expecting ");
        err.append(type_fqname<cb_type>());
        this->on_subscribed(ctx, { 5, err }, topic, cb);
        return;
    }

    // Save the callback
    std::string nm { topic.data() };
    callback_registry(ctx.get_id(), nm).register_handler(*callback, cb.get_typeid(), cb.get_type_short_name());
    TmxBrokerClient::subscribe(ctx, topic, cb);
}

void TmxNetSnmpBrokerClient::unsubscribe(TmxBrokerContext &ctx, const_string topic, TmxTypeDescriptor const &cb) noexcept {
    std::string nm { topic.data() };
    callback_registry(ctx.get_id(), nm).unregister(cb.get_typeid());
    TmxBrokerClient::unsubscribe(ctx, topic, cb);
}

TmxError TmxNetSnmpBrokerClient::snmp_get(TmxBrokerContext &ctx,
                                          types::Properties<types::Any> const &oids,
                                          common::types::Any &results) const noexcept {
    if (!this->is_connected(ctx))
        return { 1, "Broker context " + ctx.to_string() + " is not connected."};

    auto session = tmx::common::types::as<snmp_session>(ctx.at(_handle));
    if (!session)
        return { 2, "Broker context " + ctx.to_string() + " not connected properly." };

    struct snmp_pdu *pdu = nullptr, *response = nullptr;
    struct variable_list *vars;

    oid theOid[MAX_OID_LEN];
    size_t theOid_len = MAX_OID_LEN;

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    // Return values
    TmxData values { results };

    for (auto const &oidStr: oids) {
        if (oidStr.first[0] == '.')
            read_objid(oidStr.first.c_str(), theOid, &theOid_len);
        else
            get_node(oidStr.first.c_str(), theOid, &theOid_len);

        snmp_add_null_var(pdu, theOid, theOid_len);
    }

retry:
    auto ec = snmp_synch_response(session.get(), pdu, &response);
    if (ec == STAT_TIMEOUT)
        return { ec, "Timed out waiting for SNMP response." };
    else if (ec != STAT_SUCCESS)
        return { ec, snmp_errstring(ec) };
    else if (!response)
        return { ENODATA, "No response from SNMP agent" };

    std::size_t cnt = 0;
    for (vars = response->variables; vars; vars = vars->next_variable, cnt++) {
        typename types::Properties_::key_t nm;

        if (vars->name) {
            char nameBuf[BUFFER_SIZE];
            auto chars = snprint_objid(nameBuf, BUFFER_SIZE, vars->name, vars->name_length);
            nm.assign(nameBuf, chars);
        }

        // If there is a missing object on the agent, then nothing will be set
        // Only set the value for the problem OID and then retry
        if (response->errstat && cnt != (response->errindex - 1))
            continue;

        values[nm] = types::Null();

        if (vars->type == SNMP_NOSUCHOBJECT ||
            vars->type == SNMP_NOSUCHINSTANCE ||
            vars->type == SNMP_ENDOFMIBVIEW ||
            vars->type == ASN_NULL)
            continue;

        auto &_tmp = values[nm].get_container().emplace<std::string>(4096, '\0');
        _tmp.resize(snprint_value(_tmp.data(), _tmp.capacity(), vars->name, vars->name_length, vars));

        auto idx = _tmp.find_first_of(':');
        if (idx < _tmp.length())
            _tmp = _tmp.substr(idx + 2);

        TLOG(DEBUG3) << nm << ": " << _tmp;

        if (response->errstat) {
            pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
            snmp_free_pdu(response);
            response = nullptr;

            if (!pdu)
                return { response->errstat, "Unable to obtain OID " + nm };

            goto retry;
        }
    }

    snmp_free_pdu(response);
    response = nullptr;

    return { };
}

TmxError TmxNetSnmpBrokerClient::snmp_set(TmxBrokerContext &ctx,
                                          types::Properties<common::types::Any> const &oids) const noexcept {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;

    if (!this->is_connected(ctx))
        return { 1, "Broker context " + ctx.to_string() + " is not connected."};

    auto session = tmx::common::types::as<snmp_session>(ctx.at(_handle));
    if (!session)
        return { 2, "Broker context " + ctx.to_string() + " not connected properly." };

    struct snmp_pdu *pdu = nullptr, *response = nullptr;
    oid theOid[MAX_OID_LEN];
    size_t theOid_len = MAX_OID_LEN;

    pdu = snmp_pdu_create(SNMP_MSG_SET);

    for (auto &pair: oids) {
        if (pair.first[0] == '.')
            read_objid(pair.first.c_str(), theOid, &theOid_len);
        else
            get_node(pair.first.c_str(), theOid, &theOid_len);

        auto tp = get_tree(theOid, theOid_len, get_tree_head());
        if (!tp)
            continue;

        const TmxData value { pair.second };
        switch (tp->type) {
            case ASN_OCTET_STR:
            {
                std::string copy = value.to_string();
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)copy.c_str(), copy.length());
                break;
            }
            case ASN_BOOLEAN:
            {
                int copy = (int)(value.to_bool());
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)&copy, sizeof(copy));
                break;
            }
            case ASN_INTEGER:
            {
                long copy = value;
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)&copy, sizeof(copy));
                break;
            }
            case ASN_TIMETICKS:
            case ASN_COUNTER:
            {
                uint32_t copy = value;
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)&copy, sizeof(copy));
                break;
            }
            case ASN_BIT_STR:
            {
                const std::bitset<BUFFER_SIZE> bits(value.to_uint());
                std::string copy = bits.to_string();
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)&copy, sizeof(copy));
                break;
            }
            case ASN_DOUBLE:
            case ASN_FLOAT:
            {
                double copy = value;
                snmp_pdu_add_variable(pdu, theOid, theOid_len, tp->type, (const void *)&copy, sizeof(copy));
            }
        }
    }

    auto ec = snmp_synch_response(session.get(), pdu, &response);
    if (ec != STAT_SUCCESS)
        return { ec, snmp_errstring(ec) };

    snmp_free_pdu(response);
    snmp_free_pdu(pdu);

    return { };
}

types::Any TmxNetSnmpBrokerClient::get_broker_info(TmxBrokerContext &ctx) const noexcept {
    auto tp = std::chrono::system_clock::now();

    typedef types::Properties_::key_t key_t;
    static types::Properties<types::Any> host_oids;
    if (host_oids.empty()) {
        for (auto const &oid: {
                "SNMPv2-MIB::sysDescr.0",
                "SNMPv2-MIB::sysUpTime.0",
                "SNMPv2-MIB::sysContact.0",
                "SNMPv2-MIB::sysName.0",
                "SNMPv2-MIB::sysLocation.0",
                "HOST-RESOURCES-MIB::hrSystemUptime.0",
                "HOST-RESOURCES-MIB::hrSystemDate.0",
                "UCD-SNMP-MIB::memTotalReal.0",
                "UCD-SNMP-MIB::memAvailReal.0",
                "UCD-SNMP-MIB::ssCpuIdle.0"
        }) host_oids[key_t(oid)] = true;
    }

    // TODO: Work in "HOST-RESOURCES-MIB::hrSystemDate.0" for current-time
    // This a string that follows DateAndTime type in RFC 2579. It needs
    // decoded, but for time sake I did not do that yet.

    // This is a const function, so need my own copy of the broker client
    TmxData data = TmxBrokerClient::get_broker_info(ctx);

    if (this->is_connected(ctx)) {
        // Get the SNMP data
        TmxData output;
        auto err = this->snmp_get(ctx, host_oids, output.get_container());
        if (err) {
            typedef common::types::Properties<common::types::Any> props_type;

            TLOG(ERR) << "Unable to connect to SNMP host: " << err.get_message();
            data["host"].get_container().emplace<props_type>(const_cast<const TmxError &>(err));
        } else {
            // Convert the context host to IP
            struct hostent *host_entry = gethostbyname(ctx.get_host().c_str());
            if (host_entry)
                output["ip"] = std::string(inet_ntoa(*((struct in_addr *) host_entry->h_addr_list[0])));

            if (output["HOST-RESOURCES-MIB::hrSystemDate.0"]) {
                // TODO: Follow RFC 2579
                std::tm tm = { };

                // Determine if DST is set locally
                auto _now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                tm.tm_isdst = localtime(&_now)->tm_isdst;

                auto str = output["HOST-RESOURCES-MIB::hrSystemDate.0"].to_string();
                auto idx = str.find_first_of('.');

                // Retrieve the rest of the parts
                int ds = 0, tzh = 0, tzm = 0;

                std::sscanf(str.c_str(), "%d-%d-%d,%d:%d:%d.%d,%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                            &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &ds, &tzh, &tzm);

                // Year is since 1900
                tm.tm_year -= 1900;
                // January is 0 month
                tm.tm_mon--;

                tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

                tp += std::chrono::milliseconds(100 * ds);
                tp += std::chrono::hours(tzh);
                tp += std::chrono::minutes(tzm);
            }

            auto rawtime = std::chrono::system_clock::to_time_t(tp);
            output["current-time"].get_container().emplace<std::string>(std::asctime(std::gmtime(&rawtime))).pop_back();

            rawtime = 0;
            if (output["HOST-RESOURCES-MIB::hrSystemUptime.0"])
                sscanf(output["HOST-RESOURCES-MIB::hrSystemUptime.0"].to_string().c_str(), "(%ld)", &rawtime);
            else
                sscanf(output["SNMPv2-MIB::sysUpTime.0"].to_string().c_str(), "(%ld)", &rawtime);

            if (rawtime) {
                rawtime = std::chrono::system_clock::to_time_t(tp - std::chrono::milliseconds(10 * rawtime));
                output["up-since"].get_container().emplace<std::string>(std::asctime(std::gmtime(&rawtime))).pop_back();
            }

            data["host"] = output;
        }
    }

    return std::move(data.get_container());
}

TmxTypeRegistrar<TmxNetSnmpBrokerClient> _client_instance;

} /* End namespace netsnmp */
} /* End namespace broker */
} /* End namespace tmx */