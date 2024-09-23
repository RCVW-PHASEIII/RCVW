//
// Created by cas on 6/28/24.
//

#include <tmx/plugin/TmxPlugin.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/message/TmxData.hpp>

#include <tmx/message/j2735/202007/MessageFrame.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#define HELPER_TREE_ROOT "TreeRepair."

using namespace tmx::common;

namespace tmx {
namespace plugin {
namespace rcvw {
namespace cvi {

/**
 * Converter function to change XML arrays to JSON arrays, removing the extra key
 */
struct fix_xml_arrays {
    template<typename Tree>
    void operator()(Tree &pt) {
        Tree copy(pt);

        pt.clear();
        for (typename Tree::iterator i = copy.begin(); i != copy.end(); i++) {
            pt.push_back(
                    std::pair<typename Tree::key_type, Tree>("", i->second));
        }
    }
};

/**
 * Converter function to take an entire sub-tree up one level, effectively deleting unnecessary keys
 */
struct del_unnecessary_nodes {
    template<typename Tree>
    void operator()(Tree &pt) {
        Tree copy(pt);

        pt.clear();
        for (typename Tree::iterator i = copy.begin(); i != copy.end(); i++) {
            for (typename Tree::iterator j = i->second.begin();
                 j != i->second.end(); j++) {
                pt.push_back(
                        std::pair<typename Tree::key_type, Tree>(j->first,
                                                                 j->second));
            }
        }

    }
};

/**
 * Converter function to take make the key of a sub-tree a new value in the parent level,
 * effectively deleting unnecessary sub-objects
 */
struct flatten_node {
    template<typename Tree>
    void operator()(Tree &pt) {
        Tree copy(pt);
        pt.clear();
        pt.put_value(copy.begin()->first);
    }
};

class CVInspectorPlugin: public TmxPlugin {
    typedef boost::property_tree::basic_ptree<std::string, std::string> tree_type;
    typedef typename tree_type::iterator it_type;
    typedef typename tree_type::key_type key_type;
    typedef typename tree_type::value_type value_type;
    typedef typename tree_type::path_type path_type;

public:
    CVInspectorPlugin(): TmxPlugin() {
        this->register_handler<struct SPAT>("J2735/SPAT", this, &CVInspectorPlugin::handle_incoming);
        this->register_handler<struct MAP>("J2735/MAP", this, &CVInspectorPlugin::handle_incoming);
        this->register_handler<struct BSM>("J2735/BSM", this, &CVInspectorPlugin::handle_incoming);
        this->register_handler<struct TIM>("J2735/TIM", this, &CVInspectorPlugin::handle_incoming);
    }
    virtual ~CVInspectorPlugin() = default;

    TmxTypeDescriptor get_descriptor() const noexcept override {
        static auto const &descr = TmxPlugin::get_descriptor();
        return { descr.get_instance(), typeid(*this), type_fqname(*this).data() };
    }

    types::Array<types::Any> get_config_description() const noexcept override {
        message::TmxData cfg;
        cfg[0]["key"] = std::string("repair-file");
        cfg[0]["default"] = std::string("treerepair.xml");
        cfg[0]["description"] = std::string("The XML repair file used to correct the formatting of the J2735 JSON.");

        return cfg.to_array();
    }

    void init() override {
        TmxPlugin::init();

        TLOG(INFO) << "Reading in repair file " << this->get_config("repair-file").to_string();
        boost::property_tree::read_xml(this->get_config("repair-file").to_string(), this->repair_tree,
                                       boost::property_tree::xml_parser::trim_whitespace);
    }

    void on_message_received(const message::TmxMessage &msg) override {
        if (std::strncmp("J2735", msg.get_topic().c_str(), 4) == 0)
            // No decoding here
            this->invoke_handlers(types::Null(), msg);
        else
            TmxPlugin::on_message_received(msg);
    }

    void handle_incoming(types::Any const &, message::TmxMessage const &msg) {
        TLOG(DEBUG) << "Enter " << TMX_PRETTY_FUNCTION << " on message " << msg.to_string();

        // Use the topic name from the message
        std::string topic = std::filesystem::path(msg.get_topic()).native();

        // Dump the entire message frame
        auto payloadBytes = byte_string_decode(msg.get_payload_string());

        MessageFrame *frame = nullptr;
        auto ret = uper_decode_complete(nullptr, &asn_DEF_MessageFrame, (void **) &frame,
                                        payloadBytes.data(), payloadBytes.length());
        if (ret.code != RC_OK) {
            ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);

            this->broadcast<TmxError>(
                    { ret.code, "Decoding failed after " + std::to_string(ret.consumed) + " bytes" },
                    this->get_topic("error"), __FUNCTION__);
            return;
        }

        // Now dump to XML

        char *buffer = nullptr;
        size_t bufSize;
        const_string xml;

        FILE *mStream = open_memstream(&buffer, &bufSize);
        if (mStream) {
            if (xer_fprint(mStream, &asn_DEF_MessageFrame, frame) == 0)
                xml = to_char_sequence(buffer, bufSize);

            fclose(mStream);
        }

        if (xml.length()) {
            std::istringstream is { std::string(xml) };

            tree_type msgTree;
            boost::property_tree::xml_parser::read_xml(is, msgTree,
                                                       boost::property_tree::xml_parser::trim_whitespace);
            auto subTree = msgTree.get_child_optional("MessageFrame.value");
            if (subTree.has_value()) {
                this->cleanupTree(subTree.value());

                std::ostringstream content;
                boost::property_tree::json_parser::write_json(content, subTree.value(), false);

                // Using previous TMX forwarding format
                std::ostringstream os;
                os.str().clear();
                os << "{\"typeId\":\"" << topic;
                if (topic == "MAP" || topic == "SPAT")
                    os << "_P";
                os << "\",\"contentType\":\"JSON\"";
                os << ",\"contentLength\":" << content.str().length();
                os << ",\"content\":" << content.str() << "}}";

                message::TmxMessage newMsg { msg };
                newMsg.set_id("Properties<any>");
                newMsg.set_encoding("json");
                newMsg.set_topic("Decoded/" + topic);
                newMsg.set_payload(os.str());

                this->broadcast(newMsg);
            }
        }

        free(buffer);
        buffer = nullptr;

        ASN_STRUCT_FREE(asn_DEF_MessageFrame, frame);
    }

private:
    template<typename Functor>
    void repairAlongTree(Functor &fn, tree_type &pt, path_type path)
    {
        if (path.empty())
        {
            return;
        }

        if (path.single())
        {
            boost::optional<tree_type &> child = pt.get_child_optional(path);
            if (child)
                fn(child.get());
        }
        else
        {
            key_type head = path.reduce();
            for (it_type i = pt.begin(); i != pt.end(); i++)
            {
                if (i->first == head)
                {
                    repairAlongTree<Functor>(fn, i->second, path);
                }
            }
        }
    }

    std::string get_value(tree_type &msgTree, std::string field, bool *found = 0)
    {
        value_type &root = msgTree.front();
        auto id = this->repair_tree.get_optional<std::string>(
                path_type(HELPER_TREE_ROOT + root.first + "." + field));

        if (found)
            *found = (id.has_value() ? true : false);

        if (id.has_value())
            return id.value();
        else
            return root.first;
    }

    template <typename AttrType>
    std::string get_value(tree_type &msgTree)
    {
        return get_value(msgTree, common::type_short_name<AttrType>().data());
    }

    template<typename Functor>
    void cleanupTree(tree_type &msgTree)
    {
        Functor fn;
        value_type &root = msgTree.front();
        it_type i;

        boost::optional<tree_type &> subTree = this->repair_tree.get_child_optional(
                path_type(HELPER_TREE_ROOT + root.first + "." + common::type_short_name(fn).data()));
        if (subTree.has_value())
        {
            for (i = subTree.value().begin(); i != subTree.value().end(); i++)
                repairAlongTree(fn, msgTree,
                                path_type(i->second.template get_value<std::string>()));
        }
    }

    virtual void cleanupTree(tree_type &msgTree)
    {
        cleanupTree<fix_xml_arrays>(msgTree);
        cleanupTree<del_unnecessary_nodes>(msgTree);
        cleanupTree<flatten_node>(msgTree);
    }

    tree_type repair_tree;
};

}
}
}
}

int main(int argc, char **argv) {
    tmx::plugin::rcvw::cvi::CVInspectorPlugin plugin;
    return tmx::plugin::run(plugin, argc, argv);
}