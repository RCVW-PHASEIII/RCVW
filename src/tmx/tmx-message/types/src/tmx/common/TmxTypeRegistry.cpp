/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeRegistry.cpp
 *
 *  Created on: Jul 29, 2021
 *      @author: gmb
 */

#include <tmx/platform.hpp>

#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/types/TmxDataType.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>
#include <typeindex>
#include <utility>

#ifndef TMX_INIT_REGISTRY_SIZE
#define TMX_INIT_REGISTRY_SIZE 2048
#endif

using namespace tmx::common::types;

namespace tmx {
namespace common {

typedef std::shared_ptr<const void> type_descriptor;
typedef TmxRegistry<TmxTypeDescriptor, std::type_index> ById;
typedef TmxRegistry<const TmxTypeDescriptor *, TmxValueTypeOf<typename TmxTypeRegistry::string> > ByName;

// Using the filesystem notation, this
static constexpr char _namespace_sep = filesystem::path::preferred_separator;

ById &byId() {
	static ById _singleton { TMX_INIT_REGISTRY_SIZE };
	return _singleton;
}

ByName &byName() {
	// Expect at least one alias per type
	static ByName _singleton { 2 * TMX_INIT_REGISTRY_SIZE };
	return _singleton;
}

TmxTypeRegistry::TmxTypeRegistry(typename TmxTypeRegistry::string const &_ns) noexcept {
    static typename TmxTypeRegistry::string _repl { _namespace_sep };
    static constexpr auto _delims = static_array<char, '.', ':', '/', '\\'>::c_str();

    auto _tmp = TmxTypeOf<std::string>(_ns).trim();
	if (_tmp.empty())
		_tmp = default_namespace().data();

    // Unify the namespace separator character
    std::filesystem::path path;
    std::size_t pos = 0;
    do {
        pos = _tmp.find_first_of(_delims);
        path /= _tmp.substr(0, pos);

        if (pos != std::string::npos)
            _tmp = _tmp.substr(pos + 1);
    } while (pos != std::string::npos);

	this->nmspace = path.native();
}

TmxTypeRegistry::TmxTypeRegistry(typename TmxTypeRegistry::string &&_ns) noexcept: TmxTypeRegistry(_ns) { }

TmxTypeRegistry::TmxTypeRegistry(TmxTypeRegistry const &other) noexcept {
	this->operator=(other);
}

TmxTypeRegistry &TmxTypeRegistry::operator =(TmxTypeRegistry const &other) noexcept {
	this->nmspace = other.nmspace;
	return *this;
}

TmxTypeRegistry TmxTypeRegistry::operator/(typename TmxTypeRegistry::string const &nmspace) const noexcept {
    filesystem::path _path { this->nmspace };
    _path /= nmspace;
    return { _path.native() };
}

TmxTypeRegistry TmxTypeRegistry::get_parent() const noexcept {
    filesystem::path _path { this->nmspace };
    return { _path.parent_path().native() };
}

const_string TmxTypeRegistry::get_namespace() const noexcept {
	return { this->nmspace.c_str() };
}

TmxTypeDescriptor TmxTypeRegistry::get(const std::type_info &type, bool ignoreNs) const noexcept {
	if (byId().count(type)) {
		TmxTypeDescriptor const &descriptor = byId().at(type);
        if (ignoreNs)
            return { descriptor };

		// The type descriptor name is intentionally a constant string view in order
		// for fast copies, etc. However, that string view must be backed by an
		// existing character array, which in this case is stored in the ByName registry.
		// So, we must obtain that fully-qualified name value.
        filesystem::path _path { this->nmspace };
        _path /= descriptor.get_type_short_name();

		ByName::const_iterator iter = byName().find(_path.native());
		if (iter != byName().end()) {
			return { descriptor.get_instance(), descriptor.get_typeid(),
					 iter->first };
		}
	}

	return { std::shared_ptr<const void> {}, type, std::string(empty_string()) };
}

TmxTypeDescriptor TmxTypeRegistry::get(const_string nm) const noexcept {
	// The type descriptor name is intentionally a constant string view in order
	// for fast copies, etc. However, that string view must be backed by an
	// existing character array, which in this case is stored in the ByName registry.
	// So, we must obtain that fully-qualified name value.
    filesystem::path _path { this->nmspace };
    _path /= String8(nm).trim();

	ByName::const_iterator iter = byName().find(_path.native());
	if (iter != byName().end()) {
		return { iter->second->get_instance(), iter->second->get_typeid(),
				 iter->first };
	}

	return { std::shared_ptr<const void> {}, typeid(void), std::string(nm) };
}

Array<TmxTypeDescriptor> TmxTypeRegistry::get_all(std::type_info const &id) const noexcept {
	const_string _ns = this->get_namespace();
	Array<TmxTypeDescriptor> _ret;

	for (auto &type: byName()) {
        if (id != typeid(void) && type.second->get_typeid() != id)
            continue;

		if (::strncmp(_ns.data(), type.first.c_str(), _ns.length()) == 0)
			_ret.emplace_back(type.second->get_instance(), type.second->get_typeid(), type.first.c_str());
	}

	return _ret;
}

void _register(TmxTypeDescriptor const &descriptor, const_string nmspace) {
	if (!descriptor)
		return;

    filesystem::path _path { nmspace };
    _path /= descriptor.get_type_short_name();

    // Never register the type ID more than once, as it would override the default type name.
    // The fully qualified path name of the initial registration is used as the default type
    // name in order to ensure that aliases will always reference an original C++ type or class.
	// Also, emplace the information into newly constructed objects so they stay after any
	// temporaries are gone.
	if (!byId().count(descriptor.get_typeid()))
		byId().emplace(descriptor.get_typeid(),
                       TmxTypeDescriptor(descriptor.get_instance(), descriptor.get_typeid(), _path.native()));

    // The objects in the Name registry should contain only pointers to objects in the ID registry
	TmxTypeDescriptor const &_stored = byId().at(descriptor.get_typeid());
	byName().emplace(_path.native(), &(_stored));
};

void TmxTypeRegistry::register_type(std::shared_ptr<const void> instance, std::type_info const &id, const_string nm) const {
	if (::strncmp(this->get_namespace().data(), default_namespace().data(), default_namespace().length()) == 0)
		throw std::invalid_argument("registerType: No new types can be added to the default namespace");

	filesystem::path _path { TmxTypeOf<const_string>(nm).trim() };

	if (_path.empty())
		throw std::invalid_argument("registerType: name cannot be empty");

	TmxTypeDescriptor _tmp(instance, id, _path.native());
	_register(_tmp, this->get_namespace());
}

void TmxTypeRegistry::unregister(std::type_info const &id) const noexcept {
	auto descriptor = this->get(id);
	if (descriptor)
		this->unregister(descriptor.get_type_name());
}

void TmxTypeRegistry::unregister(const_string nm) const noexcept {
	auto descriptor = this->get(nm);
	if (descriptor)
		byName().erase(typename ByName::key_type(descriptor.get_type_name()));
}

static TmxTypeRegistry &defaultRegistry() {
    static std::string _defaultNs { default_namespace().data() };
    static TmxTypeRegistry _default { _defaultNs };
    return _default;
}

struct _RegisterAllTypes {
	static TmxTypeDescriptor _registerAlias(TmxTypeDescriptor const &descriptor, typename ByName::key_type &&name) {
		if (descriptor) {
			TmxTypeDescriptor _tmp(descriptor.get_instance(), descriptor.get_typeid(), name);
			_register(_tmp, defaultRegistry().get_namespace());

            return _tmp;
		}

        return { descriptor.get_instance(), typeid(void), "" };
	}

	template <typename _Tp>
	static void _registerAlias(const_string name) {
        // Register a new instance of _Tp if it is not already registered to its appropriate namespace
		if (!byId().count(typeid(_Tp))) {
            auto _ptr = get_singleton<_Tp>();

            TmxTypeRegistry _helper { type_fqname<_Tp>().data() };
            TmxTypeDescriptor _tmp(std::static_pointer_cast<const void>(_ptr), typeid(_Tp),
                                   _helper.get_namespace().data());
            _register(_tmp, _helper.get_parent().get_namespace());
        }

        auto descr = defaultRegistry().get(typeid(TmxTypeOf<_Tp>));
        if (!descr) return;

        // Add the alias
		_registerAlias(descr, typename ByName::key_type(name));
	}

	template <typename _Tp>
	static auto isSzTemplate(_Tp const &v) {
		return TMX_CHECK_FOR_STATIC_CONSTEXPR(_Tp, numBits);
	}

	static constexpr auto _registerDefaultType = [](auto &&instance) {
		typedef typename ByName::key_type key_type;
        typedef TmxTypeTraits<decltype(instance)> traits_type;
        typedef TmxValueTypeOf<typename traits_type::type> value_type;

		auto _ptr = get_singleton(instance);
		TmxTypeDescriptor _tmp(std::static_pointer_cast<const void>(_ptr), typeid(decltype(instance)),
		        std::string((defaultRegistry() / traits_type::name.data()).get_namespace()));

		_register(_tmp, defaultRegistry().get_namespace());

        // Register the underlying value type by ID under the short name
        if (std::is_same<typename traits_type::type, TmxTypeOf<value_type> >::value)
            _registerAlias<value_type>(type_short_name<value_type>());

		key_type _name { _tmp.get_type_name() };
		if (isSzTemplate(instance)) {
            _registerAlias(_tmp, key_type(std::regex_replace(_name, std::regex("[<>]"), "")));
        }

		std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
		auto lcAlias = _registerAlias(_tmp, typename ByName::key_type(_name));
		if (isSzTemplate(instance)) {
            _registerAlias(_tmp, typename ByName::key_type(std::regex_replace(_name, std::regex("[<>]"), "")));
        }
	};

    _RegisterAllTypes() {
        // Register all the simple types, but only the basic complex types in the default namespace
		tmx::common::for_each(TmxAllSimpleTypes {}, _registerDefaultType);
        tmx::common::for_each(TmxBasicComplexTypes {}, _registerDefaultType);

		// Alias registration
        // Some aliases are just new names for existing TMX types
        // Others are new types altogether, but reference the default TMX type name
		_registerAlias<Null>("nullptr");
		_registerAlias<Null>("nil");
		//_registerAlias<byte_t>("Byte");
        //_registerAlias<byte_t>("byte");
        //_registerAlias<byte_sequence>("bytes");
        //_registerAlias<Array<byte_t> >("Bytes");
        //_registerAlias<Array<byte_t> >("bytes");
		_registerAlias<Array<Any> >("Array");
		_registerAlias<Array<Any> >("Array<Any>");
		_registerAlias<Array<Any> >("array");
		_registerAlias<Array<Any> >("array<any>");
		_registerAlias<Array<Any> >("List");
		_registerAlias<Array<Any> >("List<Any>");
		_registerAlias<Array<Any> >("list");
		_registerAlias<Array<Any> >("list<any>");
        _registerAlias<std::list<Any> >("Array");
		_registerAlias<Array<Any> >("Queue");
		_registerAlias<Array<Any> >("Queue<Any>");
		_registerAlias<Array<Any> >("queue");
		_registerAlias<Array<Any> >("queue<Any>");
        _registerAlias<std::queue<Any> >("Array");
		_registerAlias<Array<Any> >("Vector");
		_registerAlias<Array<Any> >("Vector<Any>");
		_registerAlias<Array<Any> >("vector");
		_registerAlias<Array<Any> >("vector<any>");
        _registerAlias<std::vector<Any> >("Array");
        _registerAlias<Properties<Any> >("Properties");
        _registerAlias<Properties<Any> >("Properties<Any>");
        _registerAlias<Properties<Any> >("properties");
        _registerAlias<Properties<Any> >("properties<any>");
        _registerAlias<Properties<Any> >("Dictionary");
        _registerAlias<Properties<Any> >("Dictionary<Any>");
        _registerAlias<Properties<Any> >("dictionary");
        _registerAlias<Properties<Any> >("dictionary<any>");
		_registerAlias<char_t>("char");
		_registerAlias<TMX_CHAR8>("char8");
		_registerAlias<TMX_CHAR8>("char<8>");
		_registerAlias<TMX_CHAR8>("Char8");
		_registerAlias<TMX_CHAR8>("Char<8>");
		_registerAlias<TMX_CHAR16>("char16");
		_registerAlias<TMX_CHAR16>("char<16>");
		_registerAlias<TMX_CHAR16>("Char16");
		_registerAlias<TMX_CHAR16>("Char<16>");
		_registerAlias<TMX_CHAR32>("char32");
		_registerAlias<TMX_CHAR32>("char<32>");
		_registerAlias<TMX_CHAR32>("Char32");
		_registerAlias<TMX_CHAR32>("Char<32>");
		_registerAlias<String<8> >("String<Char8>");
		_registerAlias<String<8> >("string<char8>");
        _registerAlias<const TMX_CHAR8 *>("String<8>");
        _registerAlias<char_sequence<TMX_CHAR8> >("String<8>");
		_registerAlias<String<16> >("String<Char16>");
		_registerAlias<String<16> >("string<char16>");
        _registerAlias<const TMX_CHAR16 *>("String<16>");
        _registerAlias<char_sequence<TMX_CHAR16> >("String<16>");
		_registerAlias<String<32> >("String<Char32>");
		_registerAlias<String<32> >("string<char32>");
        _registerAlias<const TMX_CHAR32 *>("String<32>");
        _registerAlias<char_sequence<TMX_CHAR32> >("String<32>");
	}
};

static _RegisterAllTypes _tmxTypes;

} /* End namespace common */
} /* End namespace tmx */
