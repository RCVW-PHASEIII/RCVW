/*
 * TmxModule.h
 *
 *  Created on: Jan 2, 2019
 *      Author: gmb
 */

#ifndef SRC_TMXMODULE_H_
#define SRC_TMXMODULE_H_

#include <cstring>
#include <memory>
#include <list>

#include <tmx/TmxException.hpp>

#define TMX_MODULE_SECTION "TMX"

namespace tmx {
namespace module {

/**
 * A TMX platform module is basically an extended feature set that is added to TMX platform
 * during execution of the TMX plugin or core process.  It is intended that these features
 * introduce no compile time dependencies to the plugin or core process, thus must extend
 * a specific interface, which is detailed below.  Some base functionality is provided, but
 * all functions are virtual.
 *
 * Some examples of plugable modules would include: a DB implementation, such as MySQL or Mongo DB;
 * the Plugin API implementation, like for the IVP implementation, Cisco Kinetic or Apache Kafka;
 * log writer implementations, such as for standard output or for Syslog.  It is suggested that each
 * of these module types provide a small, compiled in default module to use, provided that these do
 * not introduce undesired compile time dependencies.
 *
 * Because there are no symbols compiled into the program using the module, the TMX platform must
 * be able to find and hold a reference to the module.  Any module that is compiled in must self-
 * register with the platform.  For dynamically loaded modules, each must belong inside a shared
 * library that is accessible from the running program.  The name of the library file must begin
 * with "libtmxmod" and must end in ".so" or ".dll".  Additionally, the symbol loading is handled
 * by Boost DLL, which requires that symbols be noted for export explicitly in the shared object.
 * This is done using BOOST_DLL_ALIAS_SECTIONED, or some derivative.  The exported symbol must be a
 * no-argument factory function that builds an instance to the TmxModule object and returns a shared
 * pointer to that object.  This shared pointer is the assumed owner of the module instance, and thus
 * responsible for memory cleanup.  The resultant object is stored in the TMX platform.  Additionally,
 * the factory method export should apply to the section "TMX" for easy locating.  The TMX platform
 * automatically will find all available modules in shared libraries that are located under the directory
 * of existing shared objects for the program, or under any path specified by ldconfig or in LD_LIBRARY_PATH.
 *
 * All platform modules must have a unique identifying name, which is returned via the Name() function.
 * The module will be enabled or disabled automatically when constructed, based on the use of environment
 * variables.  If the boolean TMX_MODULE_<NAME> variable is set to some false value (such as numeric 0,
 * "NO" or "FALSE" or "OFF"), then the module is considered disabled by default.  Otherwise, the module
 * is considered enabled by default.  Once the module is enabled, the Setup() function is called to perform
 * initialization for the module.  If the function returns false, then the module is automatically disabled.
 * When a module is disabled, the Teardown() function is automatically invoked to release resources.
 */
class TmxModule {
public:
	/**
	 * Default constructor does nothing.  The module will be enabled
	 * or disabled after constructed, during registration of the module.
	 */
	TmxModule() { };

	/**
	 * Default destructor does nothing.  In reality, this should not
	 * be called until the program exits.  If the module requires it,
	 * however, then the module must be disabled and torn down before
	 * the object can be destroyed.
	 */
	virtual ~TmxModule() { };

	// Virtual methods that MUST be overridden

	/**
	 * The pure virtual method that sets the name of the module
	 */
	virtual const char *Name() noexcept = 0;

	/*
	 * Setup the module for use in this program.  Called automatically
	 * once the module is enabled.  If a failure occurs, a TmxException
	 * should be thrown.
	 */
	virtual void Setup() = 0;

	/**
	 * Clean up the module after being used in the program.  Called
	 * automatically when the module is disabled.  If a failure occurs, a
	 * TmxException should be thrown.
	 */
	virtual void Teardown() = 0;

	// Virtual methods that CAN be overridden

	/**
	 * @return The type of module, used solely for grouping purposes
	 *
	 */
	virtual const char *Type() noexcept;

	/**
	 * Enable the module for this program
	 */
	virtual void Enable() noexcept;

	/**
	 * Disable the module for this program
	 */
	virtual void Disable() noexcept;

	/**
	 * @return True if the module is enabled, false otherwise
	 */
	virtual bool IsEnabled() noexcept;
};

extern void registerModule(std::shared_ptr<TmxModule>);
extern std::list<std::shared_ptr<TmxModule> >::const_iterator TmxModuleBegin();
extern std::list<std::shared_ptr<TmxModule> >::const_iterator TmxModuleEnd();
extern size_t TmxModuleSize();

/**
 * Fills a container with shared pointers to the available modules of the given type
 * @param Module A template parameter which should be a sub-class of TmxModule
 * @param out An iterator for a container to write the module pointers to
 * @return The number of items inserted
 */
template <class Module = TmxModule, class OutputIterator = typename std::list<std::shared_ptr<Module> >::iterator>
static inline size_t AllModules(OutputIterator out) {
	size_t count = 0;

	for (auto iter = TmxModuleBegin(); iter != TmxModuleEnd(); iter++) {
		std::shared_ptr<Module> tmp = std::dynamic_pointer_cast<Module>(*iter);
		if (tmp) {
			*out = tmp;
			out++; count++;
		}
	}

	return count;
}

/**
 * @param Module A template parameter which should be a sub-class of TmxModule
 * @param name The name of the module to find
 * @return The instance for the named module as a pointer to the specified type, or NULL if not found
 */
template <class Module = TmxModule>
static std::shared_ptr<Module> GetModule(const char *name) {
	for (auto iter = TmxModuleBegin(); iter != TmxModuleEnd(); iter++) {
		if (*iter && strcmp((*iter)->Name(), name) == 0)
			return std::dynamic_pointer_cast<Module>(*iter);
	}

	return std::shared_ptr<Module>();
}

/**
 * A templated structure that can be used to assist management of the module instance by
 * creating a singleton object.
 *
 * A simple export of the create() method should be enough.
 */
template <class Module>
struct TmxModuleFactory {
	/**
	 * The constructor creates the pointer instance
	 */
	TmxModuleFactory(bool autoRegister = true): instance(new Module(), *this) {
		if (autoRegister)
			registerModule(instance);
	}

	/**
	 * The factory method to export by default
	 */
	static std::shared_ptr<TmxModule> create() {
		static TmxModuleFactory<Module> _factory(false);
		return _factory.instance;
	}

	// A simple cleanup routine to delete the pointer this factory created.
	void operator()(Module *ptr) { delete ptr; }

	/**
	 * The actual pointer
	 */
	std::shared_ptr<Module> instance;
};

} /* namespace module */
} /* namespace tmx */

#endif /* SRC_TMXMODULE_H_ */
