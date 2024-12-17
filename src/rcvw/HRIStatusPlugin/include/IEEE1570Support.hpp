/*!
 * @file IEEE1570Support.hpp
 *
 *  Created on: Dec 04, 2024
 *      Author: Gregory M. Baumgardner
 */

#ifndef IEEE1570SUPPORT_HPP
#define IEEE1570SUPPORT_HPP

#include <tmx/platform.hpp>

#include <tmx/common/types/Int.hpp>
#include <tmx/message/TmxData.hpp>

namespace tmx {
namespace message {
namespace ieee {
namespace std1570 {

inline TmxData decode_4904(common::byte_sequence const &bytes) noexcept {
    using namespace tmx::common;

    TmxData json;
    std::basic_string<common::byte_t> byteStr { bytes.data(), bytes.length() };

    types::UInt1 res1;
    types::UInt2 res2;
    types::UInt3 res3;
    types::UInt4 res4;

    // Decode the 4904 message bytes
    auto header = get_value<typename types::UInt8::value_type>(to_byte_sequence(byteStr.data(), 1));

    types::UInt1 rhba;
    types::UInt4 xings;

    types::unpack(header, res3, rhba, xings);
    json["heartbeat"] = (*rhba != 0);

    byteStr.erase(0, 1);

    typedef typename types::UInt32 x_frame;
    static const auto x_sz = x_frame::numBits / TMX_BITS_PER_BYTE;
    typedef typename types::UInt24 t_frame;
    static const auto t_sz = t_frame::numBits / TMX_BITS_PER_BYTE;

    for (std::size_t i = 0; i < xings && x_sz <= byteStr.length(); i++) {
        auto xing = get_value<typename x_frame::value_type>(to_byte_sequence(byteStr.data(), x_sz));
        types::UInt4 trains;
        types::UInt4 xingSeq;
        types::UInt1 pea;
        types::UInt1 wsa;
        types::UInt1 tpd;
        types::UInt1 so;
        types::UInt1 ngd;
        types::UInt1 ngu;
        types::UInt1 ngp;
        types::UInt1 xgd;
        types::UInt1 xgu;
        types::UInt1 xgp;
        types::UInt8 time;

        unpack(xing, trains, xingSeq, res4, pea, wsa, tpd, so, res2, ngd, ngu, ngp, xgd, xgu, xgp, time);
        json["crossings"][i]["preemption-design-time"] = *time;
        json["crossings"][i]["egress-gates"] = (*xgp != 0);
        json["crossings"][i]["egress-gates-up"] = (*xgu != 0);
        json["crossings"][i]["egress-gates-down"] = (*xgd != 0);
        json["crossings"][i]["ingress-gates"] = (*ngp != 0);
        json["crossings"][i]["ingress-gates-up"] = (*ngu != 0);
        json["crossings"][i]["ingress-gates-down"] = (*ngd != 0);
        json["crossings"][i]["preemption"] = (*pea != 0);
        json["crossings"][i]["flashers"] = (*wsa != 0);
        json["crossings"][i]["detection"] = (*tpd != 0);
        json["crossings"][i]["operational"] = (*so != 0);
        json["crossings"][i]["sequence"] = *xingSeq;

        byteStr.erase(0, x_sz);

        for (std::size_t j = 0; j < trains && t_sz <= byteStr.length(); j++) {
            auto train = get_value<typename t_frame::value_type>(to_byte_sequence(byteStr.data(), t_sz));
            types::UInt1 ico;
            types::UInt2 dir;
            types::UInt4 trainSeq;
            types::UInt8 designTime;
            types::UInt8 estTime;

            unpack(train, res1, ico, dir, trainSeq, designTime, estTime);
            json["crossings"][i]["trains"][j]["estimated-warning-time"] = *estTime;
            json["crossings"][i]["trains"][j]["warning-design-time"] = *designTime;
            json["crossings"][i]["trains"][j]["island-occupied"] = (*ico != 0);
            json["crossings"][i]["trains"][j]["sequence"] = *trainSeq;
            switch (*dir) {
            case 0:
                json["crossings"][i]["trains"][j]["direction"] = std::string("East/North");
                break;
            case 1:
                json["crossings"][i]["trains"][j]["direction"] = std::string("West/South");
                break;
            default:
                json["crossings"][i]["trains"][j]["direction"] = std::string("Unknown");
            }

            byteStr.erase(0, t_sz);
        }
    }

    return std::move(json);
}

} /* End namespace std1570 */
} /* End namespace ieee */
} /* End namespace message */
} /* End namespace ieee */



#endif //IEEE1570SUPPORT_HPP
