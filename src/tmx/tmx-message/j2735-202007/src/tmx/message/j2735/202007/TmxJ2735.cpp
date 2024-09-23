/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxJ2735.cpp
 *
 *  Created on: Mar 18, 2024
 *      @author: Gregory M. Baumgardner
 */

#define TMX_ENUM_RANGE_MIN 0
#define TMX_ENUM_RANGE_MAX 300

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/j2735/202007/MessageFrame.h>

#include <tuple>

#define RELEASE 202007
#define RELEASE_NM TMX_CPP_QUOTE(RELEASE)
#define RELEASE_NS(X) TMX_CPP_CAT(R, X)
#define RELEASE_VAR(X, Y) TMX_CPP_CAT(X, Y)

namespace tmx {
namespace message {
namespace J2735 {

struct RELEASE_VAR(_tmp_, RELEASE) { };

namespace RELEASE_NS(RELEASE) {

static auto _registry =
        common::TmxTypeRegistry(common::type_namespace<struct RELEASE_VAR(_tmp_, RELEASE)>().data()) / RELEASE_NM;

// Pulled these directly from the ASN, since they are not generated into a header
enum class J2735_MESSAGE: DSRCmsgID_t {
    MAP = 18,
    SPAT = 19,
    BSM = 20,
    CSR = 21,
    EVA = 22,
    ICA = 23,
    NMEA = 24,
    PDM = 25,
    PVD = 26,
    RSA = 27,
    RTCM = 28,
    SRM = 29,
    SSM = 30,
    TIM = 31,
    PSM = 32,
    TEST00 = 240,
    TEST01,
    TEST02,
    TEST03,
    TEST04,
    TEST05,
    TEST06,
    TEST07,
    TEST08,
    TEST09,
    TEST10,
    TEST11,
    TEST12,
    TEST13,
    TEST14,
    TEST15
};

template <J2735_MESSAGE _Id>
class TmxJ2735Message: public common::Functor<const void *> {
    typedef TmxJ2735Message<_Id> self_type;

public:
    TmxJ2735Message() {
        std::shared_ptr<self_type> _instance { this, [](auto *) { } };

        _registry.register_instance(_instance, this->get_message_name());
        auto td = this->get_type_descriptor();
        if (td)
            _registry.register_instance(_instance, td->name);

        _registry.register_instance(_instance, std::to_string(this->get_message_id()));
    }

    const void *execute() const override {
        return this->get_type_descriptor();
    }

    DSRCmsgID_t get_message_id() const {
        return common::enums::enum_integer(_Id);
    }

    common::const_string get_message_name() const {
        return common::enums::enum_name(_Id);
    }

    asn_TYPE_descriptor_t const *get_type_descriptor() const {
        static MessageFrame _frame;
        if (!_frame.messageId)
            _frame.messageId = get_message_id();

        return asn_DEF_MessageFrame.elements[1].type_selector(&asn_DEF_MessageFrame, &_frame).type_descriptor;
    }
};

} /* End namespace R202007 */

using namespace RELEASE_NS(RELEASE);

class TmxJ2735 {
    struct _getMsgId { };
    struct _getMsgNm { };
    struct _getTypeD { };
    struct _getTypeNm { };
public:
    template <J2735_MESSAGE ... _Id>
    auto get_messages(common::static_array<J2735_MESSAGE, _Id...> const &) {
        static auto _tuple = std::make_tuple(common::get_singleton< TmxJ2735Message<_Id> >()...);
        return _tuple;
    }

    TmxJ2735() {
        auto reg = _registry.get_parent();
        std::shared_ptr<TmxJ2735> _instance { this, [](auto *) { } };

        // Register this helper as the J2735 release
        reg.register_instance(_instance, RELEASE_NM);

        // The messages self register in the release namespace
        static constexpr auto message_ids = common::enums::enum_sequence<J2735_MESSAGE>();
        get_messages(message_ids);

        // Register the functions
        static auto getMsgId = common::make_function(&TmxJ2735::get_message_id);
        static auto getMsgNm = common::make_function(&TmxJ2735::get_message_name);
        static auto getTypeD = common::make_function(&TmxJ2735::get_type_descriptor);
        static auto getTypeNm = common::make_function(&TmxJ2735::get_type_name);

        _registry.register_handler(getMsgId, typeid(_getMsgId), "get-message-id");
        _registry.register_handler(getMsgNm, typeid(_getMsgNm), "get-message-name");
        _registry.register_handler(getTypeD, typeid(_getTypeD), "get-type-descriptor");
        _registry.register_handler(getTypeNm, typeid(_getTypeNm), "get-type-name");

        // Check to see if this is the latest version
        if (auto const &descr = reg.get("latest"))
            if (RELEASE < std::atoi(descr.get_type_short_name().c_str()))
                return;

        // Copy all the registered names
        for (auto const &descr: _registry.get_all())
            reg.register_type(descr.get_instance(), descr.get_typeid(), descr.get_type_short_name());

        reg.register_instance(_instance, "latest");
    }

    static DSRCmsgID_t get_message_id(std::string name) {
        auto const &descr = _registry.get(name);
        if (descr) {
            for (auto const &msg: _registry.get_all(descr.get_typeid())) {
                auto val = common::enums::enum_cast<J2735_MESSAGE>(msg.get_type_short_name());
                if (val)
                    return common::enums::enum_integer(val.value());
            }
        }

        return 0;
    }

    static std::string get_message_name(std::string name) {
        auto const &descr = _registry.get(name);
        if (descr) {
            for (auto const &msg: _registry.get_all(descr.get_typeid())) {
                auto val = common::enums::enum_cast<J2735_MESSAGE>(msg.get_type_short_name());
                if (val)
                    return { common::enums::enum_name(val.value()).data() };
            }
        }

        return { };
    }

    static const void *get_type_descriptor(std::string name) {
        auto fn = _registry.get(name).as_instance<common::Functor<const void *> >();
        if (fn && fn->operator bool())
            return fn->execute();

        return nullptr;
    }

    static std::string get_type_name(std::string name) {
        auto ptr = (const asn_TYPE_descriptor_t *)get_type_descriptor(name);
        if (ptr)
            return { ptr->name };
        else
            return { };
    }
};

static TmxJ2735 RELEASE_VAR(_j2735_, RELEASE);

} /* End namespace J2735 */
} /* End namespace message */
} /* End namespace tmx */