/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxFileSystemBroker.cpp
 *
 *  Created on: Sep 8, 2021
 *      @author: gmb
 */

#include <tmx/broker/file/TmxFileSystemBroker.hpp>

#include <tmx/message/codec/TmxCodec.hpp>

#include <filesystem>
#include <fstream>
#include <thread>

using namespace tmx::common;
using namespace tmx::common::types;
using namespace tmx::message;

namespace tmx {
namespace broker {
namespace file {

void TmxFileSystemBroker::initialize(TmxBrokerContext const &ctx) noexcept {
    if (this->get_state() != TmxBrokerState::uninitialized) return;

    TmxBrokerClient::initialize(ctx);
}

void TmxFileSystemBroker::connect(Any const &cfg) noexcept {
	// Start up a thread for checking if the file is updated
    filesystem::path p { this->get_context().get_path() };
    if (filesystem::exists(p) && filesystem::is_regular_file(p)) {
        std::ifstream f;
        f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        TmxMessage &msg = this->_data.emplace<TmxMessage>();

        try {
            auto tp = filesystem::last_write_time(p);
            auto sz = filesystem::file_size(p);
            typename Array<char_t>::value_type buffer { sz };

            f.open(p.c_str());
            f.read(buffer.data(), buffer.size());
            f.close();

            // Convert the data to a message
            msg.set_source(p.native());
            msg.set_payload(to_byte_sequence(buffer.data(), buffer.size()));

            // Check if the file name extension specifies the encoder
            std::string enc;
            if (p.has_extension()) {
                enc = p.extension().string().substr(1);
            }

            if (enc == "hex" || enc == "b16")
                enc = "base-16";
            else if (enc == "b32")
                enc = "base-32";
            else if (enc == "b64")
                enc = "base-64";
            else if (enc == "txt")
                enc = "string";
            msg.set_encoding(enc);
        } catch (std::ios_base::failure &ex) {
            this->on_connected({ ex });
            return;
        }
    }

    TmxBrokerClient::connect(cfg);
}

void TmxFileSystemBroker::subscribe(common::const_string topic, const common::TmxTypeDescriptor &callback) noexcept {
    typedef typename tmx::common::TmxFunctor<TmxBrokerContext const, TmxMessage const>::type cb_type;

    if (!callback) {
        std::string err { "Callback " };
        err.append(callback.get_type_name());
        err.append(" is not valid.");

        this->on_subscribed({ 4, err }, callback);
        return;
    }

    TmxMessage msg;
    msg.set_topic(topic.data());
    msg.set_source(filesystem::path(this->get_context().get_path()).native());

    auto ret = dispatch(callback, this->get_context(), )
    auto cb = callback.as_instance<cb_type>();
    if (!cb) {
        std::string err { "Callback " };
        err.append(callback.get_type_name());
        err.append(" is not the correct signature. Expecting ");
        err.append(type_fqname<cb_type>());

        this->on_subscribed({ 5, err }, callback);
        return;
    }

	any err;

	if (cb) {
		std::lock_guard<std::mutex> lock(this->_mutex);

		// Add to the list of callbacks
		size_t pos = this->_callbacks.size();
		this->_callbacks.push_back(cb);

		// Remember the topic name for each position
		(*this->_parameters)["params"]["callback"][pos] = topic;
	} else {
		err["error"] = "No callback specified";
	}

	async_invoke(&TmxFileSystemBroker::onSubscribed, this, this, cb, err).reset();
}

void TmxFileSystemBroker::publish(const TmxMessage &msg) {
	TLOG(debug3) << "TmxFileSystemBroker: publish() invoked";

	if (!this->is_connected())
		return;

	// Get the directory path to write to
	any &parameters = _null_data;
	{
		std::lock_guard<std::mutex> lock(this->_mutex);
		parameters = this->get_parameters();
	}

	any err;
	std::filesystem::path p;
	if (parameters["url"]["path"])
		p = (std::string)parameters["url"]["path"].to_string();

	if (!p.has_extension())
		p.extension().assign("." + msg.encoding());
	else if ((p.extension().string() == ".txt" && msg.encoding() != "string") ||
			 (p.extension().string() == ".hex" && msg.encoding() != "base-16") ||
			 (p.extension().string() == ".b16" && msg.encoding() != "base-16") ||
			 (p.extension().string() == ".b32" && msg.encoding() != "base-32") ||
			 (p.extension().string() == ".b64" && msg.encoding() != "base-64") ||
			 (p.extension().string() != "." + msg.encoding()))
		TLOG(warning) << "File name " << p.c_str() << " extension is not consistent with encoding " << msg.encoding();

	if (!p.empty()) {
		std::ios_base::openmode mode = std::ios_base::out;
		if (parameters["params"]["append"])
			mode |= std::ios_base::app;

		std::ofstream f;
		f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			TLOG(debug2) << "Opening file " << p.c_str() << " for writing";
			f.open(p.c_str());
			f << std::string((const char *)msg.get_payload().data(), msg.get_length());
			f.flush();
			f.close();
		} catch (std::ios_base::failure &ex) {
			err["code"] = ex.code().value();
			err["error"] = ex.code().message();
		}
	} else {
		err["error"] = "No path specified";
	}

	if (err)
		TLOG(error) << err;

	async_invoke(&TmxFileSystemBroker::onPublished, this, this, msg, err).reset();
}

static TmxFileSystemBroker _file_broker { "file" };

} /* namespace file */
} /* namespace broker */
} /* namespace tmx */
