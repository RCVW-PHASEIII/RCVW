
#include <atomic>
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxData.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/message/codec/serializer/TmxDataSerializer.hpp>
#include <tmx/message/codec/thirdparty/pugixml.hpp>
#include <tmx/plugin/TmxPlugin.hpp>
#include <tmx/plugin/TmxPluginDataUpdate.hpp>
#include <tmx/plugin/utils/FrequencyThrottle.hpp>

#include <MapData.h>
#include <MessageFrame.h>

#include "utils/common.h"
#include "utils/map.h"

using namespace std;
using namespace tmx;
using namespace tmx::common;
using namespace tmx::message::codec::serializer;
using namespace tmx::plugin;
using namespace tmx::plugin::utils;

namespace tmx {
namespace plugin {
namespace v2x {
namespace Map {

volatile int gMessageCount = 0;

class MapPlugin : public TmxPlugin {
public:
    MapPlugin();
    virtual ~MapPlugin();

    TmxTypeDescriptor get_descriptor() const noexcept override;
    TmxError main() override;

    // Config message handler
    void handle_config_update(TmxPluginDataUpdate const &, message::TmxMessage const &);

protected:
    common::types::Array<common::types::Any> get_config_description() const noexcept override;

private:
    std::atomic<int> _mapAction{ -1 };
    std::atomic<bool> _isMapFileNew{ false };

    message::TmxData _mapFiles;
    std::mutex _dataLock;

    FrequencyThrottle<int> throttle;

    void LoadMapFiles(message::TmxData &);

    // A private tag for the handler
    struct on_config_update { };
};

MapPlugin::MapPlugin() {
    this->register_handler<on_config_update>(this->get_topic("config/Frequency"), this, &MapPlugin::handle_config_update);
    this->register_handler<on_config_update>(this->get_topic("config/MapFiles"), this, &MapPlugin::handle_config_update);
    this->register_handler<on_config_update>("TSC/Action", this, &MapPlugin::handle_config_update);
}

MapPlugin::~MapPlugin() { }

common::types::Array<common::types::Any> MapPlugin::get_config_description() const noexcept {
    message::TmxData _data;
    _data[0]["key"] = std::string("Frequency");
    _data[0]["default"] = std::string("1000");
    _data[0]["description"] = std::string("The frequency to send the MAP message in milliseconds.");
    _data[1]["key"] = std::string("MapFiles");
    _data[1]["description"] = std::string("JSON array string defining a list of MAP files.  "
                                          "One map file for each action set specified by the TSC.");

    return tmx::common::any_cast<common::types::Array<common::types::Any> >(_data.get_container());
}

TmxTypeDescriptor MapPlugin::get_descriptor() const noexcept {
    auto _descr = TmxPlugin::get_descriptor();
    return { _descr.get_instance(), typeid(MapPlugin), type_fqname(*this).data() };
}

void MapPlugin::handle_config_update(TmxPluginDataUpdate const &data, message::TmxMessage const &msg) {
    TLOG(DEBUG) << "handle_config_updated invoked with " << msg.get_container();
    TLOG(DEBUG) << "Topic is " << message::TmxData(msg.get_container())["topic"].to_string();
    TLOG(DEBUG) << "Topic is " << msg.get_topic().c_str();

    const message::TmxData newVal { data.get_value() };
    // Handle the action status change
    if (msg.get_topic() == "TSC/Action") {
        this->_mapAction = newVal.to_int();
        return;
    }

    auto str = data.get_key();
    if (str.empty()) {
        this->broadcast<TmxError>({ 1, "Invalid configuration update message: Missing key." }, this->get_topic("error"),
                                  __FUNCTION__);
        return;
    }

    if (newVal.is_empty()) {
        this->broadcast<TmxError>({ 2, "Invalid configuration update message: Missing new value." }, this->get_topic("error"),
                                  __FUNCTION__);
        return;
    }

    if (strcmp("Frequency", str.c_str()) == 0) {
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        throttle.set_Frequency(chrono::milliseconds(newVal.to_uint()));

        TLOG(DEBUG) << "Message frequency set to " <<
                    chrono::duration_cast<chrono::milliseconds>(throttle.get_Frequency()).count() << " ms";
    } else if (strcmp("MapFiles", str.c_str()) == 0) {
        std::lock_guard<std::mutex> _lock(this->_dataLock);
        this->_mapFiles = data.get_value();
        this->_isMapFileNew = true;
    }
}

TmxError MapPlugin::main() {
    this->set_status("State", "Running");

    bool mapFilesOk = false;

    int activeAction = -1;
    message::TmxMessage _msg;
    message::TmxData _maps;

    while (this->is_running()) {
        if (this->_isMapFileNew) {
            _maps.get_container().reset();
            _msg.get_source().clear();

            this->LoadMapFiles(_maps);
            this->_isMapFileNew = false;
        }

        activeAction = this->_mapAction;

        if (activeAction < 0) {
            // No action set yet, so just wait
            sleep(1);
            continue;
        }

        for (std::size_t i = 0; _msg.get_source().empty() && _maps.is_array() && i < (std::size_t) _maps; i++) {
            message::TmxData mapInfo { _maps[i] };
            if (mapInfo["Action"].to_int() == activeAction) {
                TLOG(INFO) << "Building MAP message for action " << activeAction;

                _msg.set_id(type_fqname<MapData>().data());
                _msg.set_topic("J2735/MAP");
                _msg.set_timepoint();
                _msg.set_payload(mapInfo["Bytes"].to_string());
                _msg.set_encoding("asn.1-uper");

                if (_msg.get_length()) {
                    const message::TmxData intxn { mapInfo["Decoded"]["MapData"]["intersections"] };

                    // The IntersectionGeometry value may be an array. If so, use the first vale
                    enum class _IG: std::uint8_t { IntersectionGeometry = 0 };
                    if (intxn[_IG::IntersectionGeometry]["name"])
                        _msg.set_source(intxn[_IG::IntersectionGeometry]["name"].to_string());
                    else
                        _msg.set_source(intxn[_IG::IntersectionGeometry]["id"]["id"].to_string());

                }
            }
        }

        if (!_msg.get_source().empty() && this->throttle.Monitor(activeAction)) {
            _msg.set_timepoint();
            this->broadcast(_msg);

            this->set_status("ActiveMap", _msg.get_source().c_str());
        }

        std::this_thread::sleep_for(this->throttle.get_Frequency() / 100);
    }

    this->set_status("State", "Terminated");

    return { };
}

void MapPlugin::LoadMapFiles(message::TmxData &_maps) {
    TLOG(DEBUG3) << "Enter " << TMX_PRETTY_FUNCTION << " with " << _mapFiles.to_int()
                 << (_mapFiles.is_array() ? " array" : " non-array");

    if (!_mapFiles.is_array() || _mapFiles.to_int() <= 0)
        return;

    lock_guard<mutex> lock(this->_dataLock);
    for (std::size_t i = 0; _mapFiles.is_array() && i < _mapFiles.to_int(); i++) {
        MessageFrame *msg = (MessageFrame *) calloc(1, sizeof(MessageFrame));
        if (!msg) {
            TLOG(ERR) << "Unable to allocate memory: " << strerror(errno);
            continue;
        }

        msg->messageId = 0x12;
        msg->value.present = MessageFrame__value_PR_MapData;
        MapData *mapData = &msg->value.choice.MapData;

        if (_mapFiles[i]["FilePath"] || _mapFiles[i]["Bytes"]) {
            string fn = _mapFiles[i]["FilePath"].to_string();

            // Fill in the bytes for each map file
            try {
                if (_mapFiles[i]["Bytes"] || fn.substr(fn.size() - 4) == ".txt") {
                    string line = _mapFiles[i]["Bytes"].to_string();
                    if (line.empty()) {
                        ifstream in{ fn };
                        std::getline(in, line);
                    }

                    auto bytes = byte_string_decode(line);

                    asn_dec_rval_t ret;
                    if (line.length() > 4 && line.substr(0, 4) == "0012") {
                        // This message is framed. Skip the 4 byte header
                        ret = uper_decode_complete(nullptr, &asn_DEF_MessageFrame, (void **) &msg,
                                                   bytes.data(), bytes.length());
                    } else {
                        ret = uper_decode_complete(nullptr, &asn_DEF_MapData, (void **) &mapData,
                                                   bytes.data(), bytes.length());
                    }

                    if (ret.code == RC_OK) {
                        TLOG(INFO) << "Loading MAP from " << line << " succeeded. Decoded " <<
                                   ret.consumed << " bytes.";
                        mapData = &msg->value.choice.MapData;
                    } else {
                        TLOG(ERR) << "Loading MAP from " << line << " failed. Decoded only " <<
                                  ret.consumed << " bytes.";
                    }
                } else if (fn.substr(fn.size() - 4) == ".xml") {
                    pugi::xml_document doc;
                    auto result = doc.load_file(fn.c_str());
                    if (result) {
                        std::ostringstream os;
                        pugi::xml_node mapNode = doc.first_child();
                        mapNode.print(os, "\t", pugi::format_raw | pugi::format_no_declaration);
                        asn_dec_rval_t chk;
                        chk.code = RC_FAIL;

                        if (std::string(asn_DEF_MessageFrame.name) == mapNode.name())
                            mapNode = mapNode.last_child().first_child();

                        if (std::string(asn_DEF_MapData.name) == mapNode.name())
                            chk = xer_decode(nullptr, &asn_DEF_MapData, (void **) &mapData,
                                             os.str().c_str(), os.str().length());


                        if (chk.code != RC_OK) {
                            TLOG(ERR) << "Loading MAP from " << os.str() << " failed. Decoded only " <<
                                      chk.consumed << " bytes.";
                        }
                    } else {
                        TLOG(ERR) << "Loading MAP from " << fn << "failed. " << result.description();
                    }
                }
            }
            catch (exception &ex) {
                TLOG(ERR) << "Unable to convert " << fn << ": " << ex.what();
            }
        }

        _maps[i]["Action"] = _mapFiles[i]["Action"];

        // Serialize the Map for status printout
        char *buffer;
        size_t bufSize;
        FILE *mStream = open_memstream(&buffer, &bufSize);
        if (mStream) {
            if (xer_fprint(mStream, &asn_DEF_MapData, mapData) == 0) {
                const_string tmp { buffer, bufSize };
                TLOG(INFO) << "Map for action " << _mapFiles[i]["Action"].to_int() <<
                           " is: " << endl << tmp;

                auto decoder = message::codec::TmxDecoder::get_decoder("xml");
                if (decoder)
                    decoder->decode(_maps[i]["Decoded"].get_container(), tmp);
            }
            fclose(mStream);

            unsigned char *bytes;
            auto result = uper_encode_to_new_buffer(&asn_DEF_MessageFrame, nullptr, msg, (void **) &bytes);
            if (result > 0) {
                _maps[i]["Bytes"] = byte_string_encode(to_byte_sequence(bytes, result));

                if (this->_mapAction < 0)
                    this->_mapAction = _maps[i]["Action"];
            } else {
                TLOG(ERR) << "Unable to encode MAP";
            }

            free(bytes);
        }

        free(buffer);
        buffer = nullptr;

        ASN_STRUCT_FREE(asn_DEF_MessageFrame, msg);
    }

    TLOG(DEBUG3) << "Exit " << TMX_PRETTY_FUNCTION;
}

} /* End namespace Map */
} /* End namespace v2x */
} /* End namespace plugin */
} /* End namespace tmx */

int main(int argc, char *argv[]) {
    v2x::Map::MapPlugin _plugin;
    return plugin::run(_plugin, argc, argv);
}
