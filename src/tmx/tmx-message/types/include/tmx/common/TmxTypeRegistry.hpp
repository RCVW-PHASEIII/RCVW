/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeRegistry.hpp
 *
 *  Created on: Jul 29, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRY_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRY_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/common/types/Array.hpp>
#include <tmx/common/types/Map.hpp>
#include <tmx/common/types/String.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>

namespace tmx {
namespace common {

/*!
 * @brief An alias to the registry implementation
 *
 * This can be used for other purposes besides in the
 * default type registry.
 */
template <class _C, typename _Key>
using TmxRegistry = typename types::Map<_Key, _C>::value_type;

/*!
 * @brief Get a static instance of the given type
 *
 * This class ensures that only one instance of the
 * type is ever created, i.e., a singleton, and returns
 * as a shared pointer with for convenience to copy around.
 * Of course, there is no deleter for the shared pointer
 * since a static instance of the object is created.
 *
 * The TMX type registry is one case where this function
 * is used. However, this function should never be used
 * directly to obtain the type registry, due to the use
 * of namespaces. Use a TmxTypeRegistry object instead.
 *
 * @see #TmxRegistry
 * @see #TmxTypeRegistry
 *
 * @return A singleton object
 */
template <class _Tp>
static inline std::shared_ptr<const _Tp> get_singleton() noexcept {
	static _Tp _singleton;
	return { static_cast<const _Tp *>(&_singleton), [](auto *) { } };
}

/*!
 * @brief Get a static instance of the given type
 *
 * This class ensures that only one instance of the
 * type is ever created, i.e., a singleton, and returns
 * as a shared pointer with for convenience to copy around.
 * Of course, there is no deleter for the shared pointer
 * since a static instance of the object is created.
 *
 * The TMX type registry is one case where this function
 * is used. However, this function should never be used
 * directly to obtain the type registry, due to the use
 * of namespaces. Use a TmxTypeRegistry object instead.
 *
 * @see #TmxRegistry
 * @see #TmxTypeRegistry
 *
 * @param[in] An type instance
 * @return A singleton object
 */
template <class _Tp>
static inline std::shared_ptr<const _Tp> get_singleton(_Tp const &) noexcept {
	return get_singleton<_Tp>();
}

/*!
 * @brief A searchable container for type information
 *
 * This container holds details regarding types available within
 * a given namespace. The namespace acts as a filter to constrain
 * the search of types across related types. These need not be
 * directly related to the C++ namespace of a type, although they
 * frequently will be.
 *
 * White space characters are fine in the namespace, but all leading
 * and trailing white space will be removed automatically. The
 * resulting trimmed namespace name cannot be an empty string, or
 * else a run-time exception will occur when constructing the
 * registry object. Likewise, no empty names can be used for the
 * types inside the namespace.
 *
 * The namespace components may be separated by any number of the
 * following string values:
 *
 * .	- Like in a Java class separation, e.g. org.example.types
 * ::	- Like in the C++ class separation, e.g. org::example::types
 * :	- Like in many markup languages, e.g. xsl:value-of
 * /	- Like for path or topic separators, e.g. /usr/local/bin
 * \	- Like for path on Windows, e.g. \windows\system32
 *
 * Within this type registry, a developer can register an prototype
 * instance of an existing type, including the instance for a class
 * member function. Types may be retrieved by the name given, or the
 * type_info structure for the actual C++ type.
 *
 * @see #std::type_info
 * @see #typeid()
 */
class TmxTypeRegistry {
public:
	typedef typename types::String_::value_type string;

	/*!
	 * @brief Construct the registry for given namespace
	 *
	 * Note that only one registry actually exists per namespace,
	 * but there can be multiple objects in this class that each
	 * point to that registry.
	 *
	 * The default constructor uses the default TMX type namespace,
	 * which is "tmx::common::types". The only types that this
	 * namespace contains are those basic TMX types, and any attempt
	 * to register a new type under that namespace will throw a
	 * run-time exception.
	 *
	 * @param[in] The namespace for this registry, null for the default
	 * @throws std::invalid_argument If the name is invalid
	 */
	TmxTypeRegistry(string && = "") noexcept;

	/*!
	 * @brief Construct the registry for given namespace
	 *
	 * Note that only one registry actually exists per namespace,
	 * but there can be multiple objects in this class that each
	 * point to that registry.
	 *
	 * @param[in] The namespace for this registry, null for the default
	 * @throws std::invalid_argument If the name is invalid
	 */
	TmxTypeRegistry(string const &) noexcept;

	/*!
	 * @brief Construct the registry from the copy
	 */
	TmxTypeRegistry(TmxTypeRegistry const &) noexcept;

	/*!
	 * @brief No special destructor need for the registry
	 */
	virtual ~TmxTypeRegistry() = default;

	/*!
	 * @brief Re-assign this registry based on the other
	 *
	 * @return This registry instance
	 */
	TmxTypeRegistry &operator=(TmxTypeRegistry const &) noexcept;

    /*!
     * @brief Construct a registry in the namespace under the current one
     *
     * @param[in] The sub-namespace
     * @return The registry for the sub-namespace
     */
    TmxTypeRegistry operator/(string const &) const noexcept;

    /*!
     * @return The type registry for the parent namespace
     */
    TmxTypeRegistry get_parent() const noexcept;

	/*!
	 * @brief Register a new type instance
	 *
	 * If an instance is already registered under the name, then
	 * the old value will be replaced. The use of a shared pointer
	 * in this context implies that the object will then be deleted
	 * if no further references to it exist.
	 *
	 * The same pointer instance can also be registered under a
	 * different name, thus creating an alias. This is particularly
	 * useful in creating shorter names that are easier to remember
	 * than the C++ type name. The default name is the same as the
	 * non-qualified name of the type. For example:
	 *
	 * "Boolean" for tmx::common::types::Boolean
	 * "string"  for std::string
	 *
	 * @param[in] instance The instance to register
	 * @param[in] name The name of the type, null for the default
	 * @throws std::invalid_argument If the instance is null
	 * @throws std::invalid_argument If the namespace is the default
	 */
	template <typename _Tp>
	void register_instance(std::shared_ptr<_Tp> instance, const_string name = empty_string()) const {
		if (instance == nullptr)
			throw std::invalid_argument("register_instance: instance must be a valid pointer");

		if (name == empty_string())
			name = type_short_name(*instance);

		this->register_type(std::static_pointer_cast<const void>(instance), typeid(_Tp), name);
	}

	/*!
	 * @brief Register a new handler for any type
	 *
	 * This takes a public member function of any object and registers it under the
	 * given name, which can then be used to invoke the handler function by name.
	 *
	 * For simplicity sake, every TMX function handler only accepts a single argument,
	 * which can be any available C++ type, but defaults to the TMX Any type. This is
	 * convenient for the developer that just wishes to pass representative TMX data,
	 * such as a primative type, string data, an array of values, or a key-value map.
	 * This covers most information passing.  However, it additionally may cause a
	 * challenge if most robust support is required. The solution for this problem,
	 * however, is simply to wrap that data inside a class.
	 *
	 * For this version of function handler, the argument type is passed by an lvalue
	 * reference, which implies that argument is intended to be by-referece, i.e.,
	 * an input/output parameter, in order for the function to alter the contents of
	 * the reference. There is also a registration version for handlers that accepts the
	 * argument by-value, i.e., an input only parameter, and thus the function will
	 * ultimately receive a copy of the data supplied. Consider using the former
	 * case exclusively for the TMX data types since those all support references
	 * to some existing variable, thus reducing the amount of data copying that
	 * is done.
	 *
	 * @param[in] function The handler function to invoke for this function
	 * @param[in] id The unique type id to use for this function
	 * @param[in] name The name to use for the function
	 * @throws std::invalid_argument If the name is invalid
	 * @throws std::invalid_argument If the namespace is the default
	 */
	template <typename... _Args, typename _Ret = TmxError>
	void register_handler(std::function<_Ret(_Args...)> const &function, std::type_info const &id, const_string name) const {
        this->register_type(std::shared_ptr<const void>(static_cast<const void *>(&function), [](auto *){}),
                            id, name);
	}

	/*!
	 * @brief Remove the type from the registry
	 *
	 * Note that this only really removes the name from the
	 * namespace, since the type may be registered in other
	 * namespaces as well.
	 *
	 * @param[in] id The type identifier to unregister
	 */
	void unregister(std::type_info const &id) const noexcept;

	/*!
	 * @brief Remove the type from the registry
	 *
	 * Note that this only really removes the name from the
	 * namespace, since the type may be registered in other
	 * namespaces as well.
	 *
	 * @param[in] id The type name to unregister
	 */
	void unregister(const_string name) const noexcept;

	/*!
	 * @return The namespace for this registry
	 */
	const_string get_namespace() const noexcept;

	/*!
	 * @brief Get the type descriptor for the given id
	 *
	 * Note that by default this operation only searches the precise
	 * namespace for this registry. In other words, if the type is registered
	 * under "org.example.handlers", then a search for the type under
	 * the "org.example" namespace will not find anything. If the
	 * type was not registered, an empty type descriptor will be
	 * returned, which will return a false boolean value.
	 *
	 * The second parameter is for ignoring alias names. Ignoring an alias
	 * name in the query will return the type in the namespace it was originally
	 * registered with, which should always match the C++ type name. Otherwise,
	 * this function will by default only return a valid type descriptor if the
	 * unqualified name of the type is aliased inside the namespace of this registry.
	 * Note that in this case, the descriptor might actually reference a different
	 * type. This facilitates finding templated classes as is the case with the
	 * TMX types.
	 *
	 * For example:
	 * Assuming the tmx::common::types::UInt32 class is implemented by a
	 * "std::uint32_t" which in turn is actually the primitive "unsigned int"...
	 *
	 * TmxTypeRegistry().get(typeid(UInt32), true|false)
	 * returns the "tmx::common::types::UInt32" type descriptor. As would:
	 * TmxTypeRegistry().get(typeid(std::uint32), false)
	 * or
	 * TmxTypeeRegistry().get(typeid(unsigned int), false)
	 * because both the aliases "tmx::common::types::uint32" and
	 * "tmx::common::types::unsigned int" will be linked to the TMX
	 * type "tmx::common::types::UInt32".
	 *
	 * However,
	 * TmxTypeRegistry().get(typeid(std::uint32), true)
	 * or
	 * TmxTypeeRegistry().get(typeid(unsigned int), true)
	 * actually returns the "unsigned int" type descriptor instead since
	 * both are actually the same type.
	 *
	 * @see #std::type_info
	 * @see #typeid()
	 * @see #getAll()
	 * @see #TmxTypeDescriptor::operator bool()
	 * @param The type identifier to search for in this namespace
	 * @param Ignore the namespace of this registry in the search
	 * @return The type descriptor
	 */
	TmxTypeDescriptor get(std::type_info const &, bool = false) const noexcept;

	/*!
	 * @brief Get the type descriptor for the given name
	 *
	 * Note that this operation only searches the precise namespace
	 * for this registry. In other words, if the type is registered
	 * under "org.example.handlers", then a search for the type under
	 * the "org.example" namespace will not find anything. If the
	 * type was not registered, an empty type descriptor will be
	 * returned, which will return a false boolean value.
	 *
	 * @see #getAll()
	 * @see #TmxTypeDescriptor::operator bool()
	 * @param The type name to search for in this namespace
	 * @return The type descriptor
	 */
	TmxTypeDescriptor get(const_string name) const noexcept;

	/*!
	 * @brief Get all the type descriptors in this namespace
	 *
	 * If a type identifier is provided, then this only matches
	 * descriptors of that given type. By default, this returns
	 * descriptors of all types.
	 *
	 * This function returns every descriptor that exists
	 * somewhere at or below the current namespace hierarchy.
	 * In other words, the "org.example" registry will include
	 * every type registered in "org.example", "org.example.helpers",
	 * "org.example.handlers", etc.
	 *
	 * Note that because of this filtering, this operation is
	 * much slower performing that the standard get()
	 * operations, and should be used only sparingly.
	 *
	 * @see #get()
	 * @return An array of types found for this namespace
	 */
	types::Array<TmxTypeDescriptor> get_all(std::type_info const & = typeid(void)) const noexcept;

    /*!
     * @brief Register new type information
     *
     * @param[in] ptr The instance of the type
     * @param[in] id The type identifier
     * @param[in] name The name for this type
     * @throws std::invalid_argument If the instance is null
     * @throws std::invalid_argument If the name is invalid
     * @throws std::invalid_argument If the namespace is the default
     */
    void register_type(std::shared_ptr<const void> instance, std::type_info const &id, const_string name) const;

private:
	/*!
	 * @brief The namespace for this registry
	 */
	 string nmspace;
};

} /* namespace common */
} /* namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRY_HPP_ */
