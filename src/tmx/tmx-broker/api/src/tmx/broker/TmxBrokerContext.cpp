/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxBrokerContext.cpp
 *
 *  Created on: Apr 7, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/broker/TmxBrokerContext.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>

#include <mutex>
#include <regex>
#include <sstream>
#include <uuid/uuid.h>
#include <iostream>

using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message::codec::serializer;

namespace tmx {
namespace broker {

std::string get_uuid() {
    char _buf[37];
    uuid_t id;
    uuid_generate(id);
    uuid_unparse(id, _buf);

    return { _buf };
}

std::string replace_chars(std::string const &str) {
    static std::pair<std::string, std::string> _map[] = {
            {"excl", "!"}, {"quot", "\""}, {"num", "#"}, {"dollar", "$"}, {"percnt", "%"},
            {"amp", "&"}, {"apos", "\'"}, {"lpar", "("}, {"rpar", "}"}, {"ast", "*"},
            {"plus", "+"}, {"comma", ","}, {"period", "."}, {"sol", "/"}, {"colon", ":"},
            {"semi", ";"}, {"lt", "<"}, {"equals", "="}, {"gt", ">"}, {"quest", "?"},
            {"commat", "@"}, {"lsqb", "["}, {"bsol", "\\"}, {"rsqb", "]"}, {"hat", "^"},
            {"lowbar", "_"}, {"grave", "`"}, {"lcub", "{"}, {"verbar", "|"}, {"rcub", "}"}, {"nbsp", " "}
    };

    auto s = str;
    for (auto kv: _map) {
        std::regex pattern { "&" + kv.first + ";" };
        s = std::regex_replace(s, pattern, kv.second);
    }

    return { s.c_str() };
}

enum TmxBrokerContextField: std::uint16_t {
    id,
    state,
    scheme,
    user,
    secret,
    host,
    port,
    path
};

std::shared_ptr<const std::string> _get(const typename types::Properties<Any>::value_type &store,
                                        TmxBrokerContextField field) {
    types::Properties_::key_t key{ enums::enum_name(field).data() };
    if (store.count(key))
        return types::as<std::string>(store.at(key));

    return { };
}

template <typename _T>
auto &_set(typename types::Properties<Any>::value_type &store, TmxBrokerContextField field, _T const &value) {
    types::Properties_::key_t key{ enums::enum_name(field).data() };
    return store[key].emplace<_T>(value);
}

//TODO: I've noted that parameter values are not parsed out of the input, i.e.,
// starting with "?" and containing "&"

TmxBrokerContext::TmxBrokerContext(const_string url, const_string id, Any const &cfg) noexcept:
        super({ {"parameters", cfg}, {"defaults", cfg} }), _ctxThreadLock(), _ctxPublishLock(), _ctxReceiveLock() {

    static constexpr auto scheme_regex = TMX_STATIC_STRING("([a-zA-Z][a-zA-Z0-9+-.]*)://");
    static constexpr auto userinfo_regex = TMX_STATIC_STRING("(([^@:]+)?(:([^@]+)?)?@)?");
    static constexpr auto host_regex = TMX_STATIC_STRING("([^/:]+)");
    static constexpr auto port_regex = TMX_STATIC_STRING("(:([A-Za-z0-9]+))?");
    static constexpr auto path_regex = TMX_STATIC_STRING("(/.*)?");

    static constexpr auto url_regex = concat(static_array<char, '^'> {}, concat(scheme_regex,
                                               concat(userinfo_regex, concat(host_regex,
                                               concat(port_regex, concat(path_regex, static_array<char, '$'> {}))))));


    _set(*this, TmxBrokerContextField::id, id.empty() ? get_uuid() : std::string(id.data()));
    _set(*this, TmxBrokerContextField::state, std::string(enums::enum_name(TmxBrokerState::uninitialized).data()));

    if (url.length() == 0) return;

    std::regex urlRE { url_regex.c_str() };
    std::cmatch urlMatch;

    if ( ! std::regex_match(url.data(), urlMatch, urlRE)  )
        return;

    if ( urlMatch.size() < 9 )
        return;

    _set(*this, TmxBrokerContextField::scheme, replace_chars(urlMatch[1].str()));
    _set(*this, TmxBrokerContextField::user, replace_chars(urlMatch[3].str()));
    _set(*this, TmxBrokerContextField::secret, replace_chars(urlMatch[5].str()));
    _set(*this, TmxBrokerContextField::host, replace_chars(urlMatch[6].str()));
    _set(*this, TmxBrokerContextField::port, replace_chars(urlMatch[8].str()));
    _set(*this, TmxBrokerContextField::path, replace_chars(urlMatch[9].str()));
}

TmxBrokerContext::TmxBrokerContext(TmxBrokerContext const &copy) noexcept:
        _ctxThreadLock(), _ctxPublishLock(), _ctxReceiveLock() {
    this->operator=(copy);
}

TmxBrokerContext::~TmxBrokerContext() {
    this->clear();

    this->get_receive_sem().notify_all();
    this->get_publish_sem().notify_all();
}

TmxBrokerContext &TmxBrokerContext::operator=(TmxBrokerContext const &copy) noexcept {
    super::operator=(copy);
    return *this;
}

TmxBrokerContext::operator bool() const noexcept {
    // The scheme and the host or path are required
    return !this->get_scheme().empty() &&
            (!this->get_host().empty() || !this->get_path().empty());
}

std::shared_ptr<TmxTaskExecutor> TmxBrokerContext::get_executor() const noexcept {
    return { this->_ctxExecutor };
}

void TmxBrokerContext::set_executor(std::shared_ptr<TmxTaskExecutor> exec) noexcept {
    this->_ctxExecutor = exec;
}

std::mutex &TmxBrokerContext::get_thread_lock() noexcept {
    return this->_ctxThreadLock;
}

std::mutex &TmxBrokerContext::get_publish_lock() noexcept {
    return this->_ctxPublishLock;
}

std::mutex &TmxBrokerContext::get_receive_lock() noexcept {
    return this->_ctxReceiveLock;
}

std::condition_variable_any &TmxBrokerContext::get_thread_sem() noexcept {
    return this->_ctxThreadCv;
}

std::condition_variable_any &TmxBrokerContext::get_publish_sem() noexcept {
    return this->_ctxPublishCv;
}

std::condition_variable_any &TmxBrokerContext::get_receive_sem() noexcept {
    return this->_ctxReceiveCv;
}

Any &TmxBrokerContext::get_parameters() noexcept {
    return this->at("parameters");
}

Any const &TmxBrokerContext::get_defaults() const noexcept {
    return this->at("defaults");
}

TmxBrokerState TmxBrokerContext::get_state() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::state);
    if (val && val->c_str()) {
            auto s = enums::enum_cast<TmxBrokerState>(val->c_str());
            if (s) return s.value();
    }

    return TmxBrokerState::uninitialized;
}

void TmxBrokerContext::set_state(TmxBrokerState state) noexcept {
    _set(*this, TmxBrokerContextField::state, std::string(enums::enum_name(state).data()));
}

String_ TmxBrokerContext::get_id() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::id);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_scheme() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::scheme);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_user() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::user);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_secret() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::secret);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_host() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::host);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_port() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::port);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::get_path() const noexcept {
    auto const val = _get(*this, TmxBrokerContextField::path);
    if (val) return { *val };
    return { };
}

String_ TmxBrokerContext::to_string() const noexcept {
    std::stringstream ss;

    if ( 0 < this -> get_scheme().length() )
        ss << this->get_scheme() << "://";

    if (!this->get_user().empty()) {
        ss << this->get_user();

        if (!this->get_secret().empty())
            ss << ":" << this->get_secret();

        ss << "@";
    }

    ss << this->get_host();

    if (!this->get_port().empty())
        ss << ":" << this->get_port();

    //NTC: I took out the slash. Prior test code seemed to imply that the leading "/" was
    // intended to be part of the path so putting it in here causes the test against
    // the input string and result string to fail.

    if (!this->get_path().empty())
        //ss << "/" << this->get_path();
        ss << this -> get_path();

    return ss.str();
}

} /* End namespace broker */
} /* End namespace tmx */
