/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxXmlEncoder.cpp
 *
 *  Created on: Jul 31, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/codec/TmxCodec.hpp>

#include <pugixml.hpp>
#include <cstring>

#ifndef TMX_ARRAY_ELEMENT_NAME
#define TMX_ARRAY_ELEMENT_NAME "ArrayElement"
#endif

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

typedef pugi::xml_node &node_t;

class TmxXmlEncoder: public TmxEncoder {
public:
    TmxXmlEncoder() {
        this->register_encoder();
    }

    TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_desc = TmxEncoder::get_descriptor();
        return { _desc.get_instance(), typeid(TmxXmlEncoder), "xml" };
    }

    bool is_binary() const noexcept override {
        return false;
    }

    template <typename _Tp>
    TmxError handle(_Tp const &obj, node_t node) const {
        // By default, the value of the object is written as a text node
        // This may be different for specific types
        if (node.text().set(obj))
            return { };

        std::string err { "Unable to set the " };
        err.append(type_fqname(obj).data());
        err.append(" type data to XML node ");
        err.append(node.name());
        return { 1, err };
    }

    TmxError handle(Null const &obj, node_t node) const {
        static auto nm = TmxTypeTraits<Null>::name.data();

        // Write a null node
        if (!node.append_child(nm).empty())
            return { };

        std::string err { "Unable to set the " };
        err.append(nm);
        err.append(" XML node to ");
        err.append(node.name());
        return { 1, err };
    }

    TmxError handle(Float128 const &obj, node_t node) const {
        return this->handle(Float64(static_cast<typename Float64::value_type>((typename Float128::value_type)obj)), node);
    }

    template <typename _E>
    TmxError handle(Enum<_E> const &obj, node_t node) const {
        const auto &nm = obj.get_enum_name();
        if (nm.length() <= 0 || ::isdigit(nm[0])) {
            return this->handle(obj.get_integer_value(), node);
        } else {
            // If the string value is known, then write it as a new node.
            if (!node.append_child(nm.data()).empty())
                return { };

            std::string err { "Unable to set the enum " };
            err.append(nm);
            err.append(" XML node to ");
            err.append(node.name());
            return { 1, err };
        }
    }

    TmxError handle(Any const &data, node_t node) const {
        TmxArgList args { data, node };
        return this->execute(common::TmxTypeRegistry().get(data.type(), true), std::ref(args));
    }

    template <typename _Tp>
    TmxError handle(Array<_Tp> const &arr, node_t node) const {
        static auto nm = TMX_ARRAY_ELEMENT_NAME;

        for (std::size_t i = 0; i < arr.size(); i++){
            pugi::xml_node child = node.append_child(nm);
            if (!child) {
                std::string err { "Unable to set the " };
                err.append(nm);
                err.append(" XML node to ");
                err.append(node.name());
                return { 1, err };
            }

            auto ret = this->handle(arr[i], child);
            if (ret) return ret;
        }

        return { };
    }

    template <typename _Key, typename _Tp>
    TmxError handle(Map<_Key, _Tp> const &props, node_t node) const {
        for (auto &obj: props) {
            // Need to get the text string
            pugi::xml_document _tmp;
            auto ret = this->handle(obj.first, _tmp);
            if (ret) return ret;

            auto nm = _tmp.text().empty() ? _tmp.name() : _tmp.text().get();
            pugi::xml_node child = node.append_child(nm);
            if (!child) {
                std::string err { "Unable to set the " };
                err.append(nm);
                err.append(" XML node to ");
                err.append(node.name());
                return { 1, err };
            }

            ret = this->handle(obj.second, child);
            if (ret) return ret;
        }

        return { };
    }

    TmxError execute(TmxTypeDescriptor const &type, std::reference_wrapper<TmxArgList> args) const override {
        TmxVariant<TmxScalarTypes, String8, TmxBasicComplexTypes> var;

        static TmxTypeHandlerOverload handler {
                [this](auto const &v, TmxArgList &args) -> TmxError {
                    // TODO: Extract format from args
                    auto format = pugi::format_raw | pugi::format_no_declaration;

                    std::shared_ptr<byte_stream> *ptr;
                    if (args.size() > 1) {
                        ptr = tmx::common::any_cast<std::shared_ptr<byte_stream> >(&args[1]);

                        if (ptr && ptr->get()) {
                            // This is an execution at the root object
                            pugi::xml_document doc;
                            auto ret = this->handle(make_type(v), doc);
                            if (!ret) doc.save(*(ptr->get()), "", format);
                            return ret;
                        }

                        auto node = tmx::common::any_cast<pugi::xml_node>(&args[1]);
                        if (node)
                            // This was an internal execution on an unknown object
                            return this->handle(make_type(v), *node);
                    }

                    return { EINVAL, "Invalid argument: Missing output stream to encode to." };
                }
        };

        auto err = set_variant(var, type);
        if (!err)
            return do_handle(handler, var, args);

        // Special support for the TmxMessage bytes type
        if (err.get_code() == ENOTSUP && args.get().size() > 0) {
            auto try1 = common::types::as<std::basic_string<tmx::common::byte_t> >((args.get())[0]);
            if (try1)
                return handler(common::to_char_sequence(try1->data(), try1->length()), args.get());
        }

        return err;
    }
};

class TmxXmlDecoder: public TmxDecoder {
public:
    TmxXmlDecoder() {
        this->register_decoder();
    }

    TmxTypeDescriptor get_descriptor() const noexcept override {
        static const auto &_desc = TmxDecoder::get_descriptor();
        return { _desc.get_instance(), typeid(TmxXmlDecoder), "xml" };
    }

    bool is_binary() const noexcept override {
        return false;
    }

    TmxError execute(TmxTypeDescriptor const &type, std::reference_wrapper<TmxArgList> args) const override {
        TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION;
        byte_sequence *bytes = nullptr;
        if (args.get().size() > 0)
            bytes = tmx::common::any_cast<byte_sequence>(&(args.get())[0]);

        if (!bytes)
            return { EINVAL, "Invalid argument: No byte sequence to decode from." };

        pugi::xml_document doc;
        auto ret = doc.load_string(to_char_sequence<pugi::char_t>(*bytes).data());
        if (!ret)
            return { ret.status, ret.description() };

        // TODO: Use type for specific conversion
        return node_to_any(doc.root().first_child(), args.get().emplace_back());
    }

private:
    TmxError node_to_any(const pugi::xml_node &node, Any &data) const noexcept {
        if (node.type() == pugi::node_element) {
            if (std::strcmp(TmxTypeTraits<Null>::name.data(), node.name()) == 0) {
                // Null element
                data.emplace<Null>();
            } else if (!node.first_child()) {
                // Assume this is some enum value
                const_string nm = node.name();
                // Can only assume the value
                data.emplace<Enum16>(-1, nm);
            } else if (node.next_sibling(node.name())) {
                // This is an array element. We can ignore the node name
                typedef Array<types::Any> array_type;

                array_type &_array = data.emplace<array_type>();
                for (pugi::xml_node n = node; n; n = n.next_sibling(node.name())) {
                    auto &val = _array.emplace_back(Null());

                    auto err = node_to_any(n.first_child(), val);
                    if (err) return err;
                }
            } else {
                // This is a probably properties element
                typedef types::Properties<types::Any> properties_type;
                typedef typename properties_type::key_t key_type;

                auto &_props = data.emplace<properties_type>();

                // Reverse the order of insertion due the unordered map
                // This should get it close to the same order
                pugi::xml_node n = node;
                for (; n.next_sibling(); n = n.next_sibling());
                for (; n; n = n.previous_sibling()){
                    key_type key { n.name() };
                    _props[key] = Null();

                    auto err = node_to_any(n.first_child(), _props[key]);
                    if (err) return err;
                }
            }
        } else {
            const auto &txt = node.text();

            // Check for a number
            if (std::isdigit(txt.as_string()[0])) {
                if (std::strpbrk(".", txt.as_string()))
                    data.emplace<Float64>(txt.as_double());
                else
                    data.emplace<Intmax>(txt.as_llong());
            } else {
                if (std::strcmp(txt.as_string(), "true") == 0 ||
                    std::strcmp(txt.as_string(), "True") == 0 ||
                    std::strcmp(txt.as_string(), "false") == 0 ||
                    std::strcmp(txt.as_string(), "False") == 0) {
                    data.emplace<Boolean>(txt.as_bool());
                } else {
                    data.emplace<String8>(txt.as_string());
                }
            }
        }

        return { };
    }
};

static TmxTypeRegistrar< TmxXmlEncoder > _xml_encoder_registrar;
static TmxTypeRegistrar< TmxXmlDecoder > _xml_decoder_registrar;

}
}
}