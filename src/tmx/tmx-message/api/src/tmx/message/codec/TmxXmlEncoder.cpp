/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxXmlEncoder.cpp
 *
 *  Created on: Aug 17, 2021
 *      @author: gmb
 */

#include <tmx/message/codec/TmxCodec.hpp>

#include <tmx/common/TmxTypeRegistry.hpp>

#include <pugixml/pugixml.hpp>
#include <sstream>

#include <iostream>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace message {
namespace codec {

std::error_code any_to_xml(const any &, pugi::xml_document &);
std::error_code xml_to_any(const pugi::xml_document &, any &);

class TmxXmlEncoder: public TmxCodec {
public:
	explicit TmxXmlEncoder(): TmxCodec() { }
	explicit TmxXmlEncoder(bool): TmxCodec(this) { }

	using TmxTypeVisitor::dispatch;

	typename string8::view_type get_name() const override {
		return "xml";
	}

	std::error_code encode(const any &data, byte_stream &theBytes) const override {
		pugi::xml_document xml;
		auto ret = any_to_xml(data, xml);
		if (!ret) {
			std::stringstream ss;
			xml.save(ss, "", pugi::format_raw | pugi::format_no_declaration);
			for (auto i = 0; i < ss.str().length(); i++)
				theBytes.push_back((byte_t) ss.str()[i]);
		}

		return ret;
	}

	std::error_code decode(const byte_stream &theBytes, any &data) const override {
		std::error_code _ec;
		types::bytes_type<typename types::string::char_t> _bytes { theBytes.data(), theBytes.size() };

		auto dec = new TmxXmlEncoder();
		std::stringstream ss { _bytes.to_string() };
		dec->_doc.load(ss);

		// TODO: load returns a result
		if (!_ec) {
			auto ret = xml_to_any(dec->_doc, data);
			if (ret)
				return ret;
		}

		delete dec;
		return _ec;
	}

	template <typename _T>
	std::error_code ec(_T &&val, typename string::view_type const &type) {
		auto result = _node.text().set(val);
		if (!result) {
			// TODO: Create error
		}

		if (!_node.append_attribute("type").set_value(type.data())) {
			// TODO: Create error
		}

		return std::error_code();
	}

	std::error_code dispatch(const boolean_type &b) override {
		return ec(*b, boolean_type::name);
	}

	std::error_code dispatch(int32 &&i, bitcount_type bits) override {
		return ec(*i, int32::name);
	}

	std::error_code dispatch(int64 &&i, bitcount_type bits) override {
		return ec(*i, int64::name);
	}

	std::error_code dispatch(uint32 &&i, bitcount_type bits) override {
		return ec(*i, uint32::name);
	}

	std::error_code dispatch(uint64 &&i, bitcount_type bits) override {
		return ec(*i, uint64::name);
	}

	std::error_code dispatch(const float32 &f) override {
		return ec(*f, float32::name);
	}

	std::error_code dispatch(const float64 &f) override {
		return ec(*f, float64::name);
	}

#ifdef TMX_FLOAT128
	std::error_code dispatch(const float128 &f) override {
		// Only up to double is supported
		return ec(*f, float128::name);
	}
#endif

	std::error_code dispatch(const string8 &s) override {
		return ec(s.c_str(), string8::name);
	}

	std::error_code dispatch(const array_type<TmxData> &a) override {
		// Each item in the array must be contained in a separate node element
		for (auto item: a) {
			_node = _node.append_child("element");

			auto ret = this->dispatch(item);
			if (ret) return ret;
		}

		return std::error_code();
	}

	std::error_code dispatch(const map_type<string8, TmxData> &m) override {
		// For the first node, if there is more than one key, then
		// a root node is required
		if (_node == _doc.root())
			_node = _doc.append_child("root");

		pugi::xml_node baseNode = _node;
		for (auto item: m) {

			_node = baseNode.append_child(item.first.c_str());
			auto ret = this->dispatch(item.second);
			if (ret) return ret;
		}

		return std::error_code();
	}

	pugi::xml_document _doc;
	pugi::xml_node _node = _doc.root();
};

std::error_code any_to_xml(const any &data, pugi::xml_document &xml) {
	auto enc = new TmxXmlEncoder();

	// Always treat the data as a record type
	auto record = data.to_map();
	auto ret = enc->dispatch(record);
	if (ret) return ret;

	xml.reset(enc->_doc);

	delete enc;
	return std::error_code();
}

std::error_code node_to_any(const pugi::xml_node &xml, any &data) {
	size_t i = 0;
	for (auto node: xml.children()) {
		if (node.type() != pugi::node_element)
			continue;

		auto &val = data[node.name()];
		if (node.first_child().type() == pugi::node_pcdata) {
			val.operator =(node.first_child().value());
		} else if (string("element") == node.first_child().name()) {
			auto ret = node_to_any(node.first_child(), val[i++]);
			if (ret) return ret;
		} else {
			auto ret = node_to_any(node.first_child(), val);
			if (ret) return ret;
		}
	}

	return std::error_code();
}

std::error_code xml_to_any(const pugi::xml_document &xml, any &data) {
	if (string("root") == xml.first_child().name()) {
		return node_to_any(xml.first_child(), data);
	} else {
		return node_to_any(xml, data);
	}
}

static TmxXmlEncoder _xml_encoder { true };

}
}
}


