/*
 * TmxModule.cpp
 *
 *  Created on: Jan 2, 2019
 *      Author: gmb
 */

#include "TmxModule.h"

#include "System.h"
#include "tmx/TmxException.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 106100
#include <boost/dll.hpp>
#endif
#include <boost/filesystem.hpp>

#include <atomic>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <link.h>
#include <map>

using namespace boost::filesystem;

namespace tmx {
namespace module {

struct TmxModuleRegistry {
	TmxModuleRegistry();
	~TmxModuleRegistry();

	// The search path for the modules
	std::map<path, bool> modulePaths;

	std::list<std::shared_ptr<TmxModule> > &loadedModules();
	std::map<std::string, std::atomic<bool> > &modulesEnabled();

	static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data);
};

static TmxModuleRegistry registry;

bool checkEnabled(const char *name) {
	// Determine if this module should be enabled.  The default is always
	// yes, unless a variable is specifically set to disable
	std::string envName = "TMX_MODULE_";
	envName += name;

	// Convert to all CAPS
	std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);

	// Also, replace any spaces with underscores
	for (size_t i = 10; i < envName.length(); i++)
		if (envName[i] == ' ') envName[i] = '_';

	const char *env = ::getenv(envName.c_str());

	// No environment variable, auto set to true
	if (!env) return true;

	// A numeric value, check its value
	if (::isdigit(env[0])) return ::atoi(env);

	char buf[6];
	size_t c;

	// A string value, compare to specific values without case sensitivity
	for (c = 0; c < 6 && c < ::strlen(env); c++)
		buf[c] = ::toupper(env[c]);

	buf[c] = '\0';

	return !(::strcmp("FALSE", buf) == 0 ||
			 ::strcmp("NO", buf)    == 0 ||
			 ::strcmp("OFF", buf)   == 0);
}

void registerModule(std::shared_ptr<TmxModule> module) {
	std::string name(module->Name());

	// Only register once
	if (module && registry.modulesEnabled().count(name) == 0) {
		// Add the module to the registry
		registry.loadedModules().push_back(module);

		// Add the module as disabled so TearDown is not called before Setup
		registry.modulesEnabled()[name] = false;

		if (checkEnabled(name.c_str()))
			module->Enable();
		else
			module->Disable();

	}
}

TmxModuleRegistry::TmxModuleRegistry() {
	// Load all available modules dynamically by checking the existing shared object directories
	dl_iterate_phdr(&TmxModuleRegistry::dl_iterate_callback, static_cast<void *>(this));

	// Add the directories from ldconfig
	std::string cmd = "ldconfig -p 2>&1 | awk '{print $NF}' | grep -v \"'\" | sed -e 's/\\/[^\\/]*$//' | sort -u";
	std::string result = tmx::utils::System::ExecCommand(cmd.c_str());
	if (!result.empty()) {
		std::vector<std::string> lines;
		boost::split(lines, result, boost::is_any_of("\n"));
		for (size_t i = 0; i < lines.size(); i++)
			modulePaths[lines[i]] = true;
	}

	// Add the directories from LD_LIBRARY_PATH
	char *env = ::getenv("LD_LIBRARY_PATH");
	if (env) {
		char *tmp = ::strdup(env);
		for (char *c = ::strtok(env, ":"); c != NULL; c = ::strtok(NULL, ":"))
			modulePaths[path(env)] = true;
		free(tmp);
	}

	for (auto pr: modulePaths) {
		path p = pr.first;
		if (is_directory(p)) {

			directory_iterator dir(p);

			for (; dir != directory_iterator(); dir++) {
				path lib = (*dir).path();

				if (::strncmp("libtmx", lib.stem().c_str(), 6) == 0 &&
						(lib.extension().string() == ".so" || lib.extension().string() == ".dll")) {

					// Found a shared library.  Check for a symbol section name TMX
#if BOOST_VERSION >= 106100
					// NOTE: This is only valid with Boost:DLL, introduced in 1.61.0
					boost::dll::library_info info(lib);
					boost::dll::shared_library so(lib);

					std::vector<std::string> symbols = info.symbols(TMX_MODULE_SECTION);
					for (std::string s: symbols)
						registerModule(so.get_alias<std::shared_ptr<TmxModule>(void)>(s)());
#endif
				}
			}
		}
	}
}

TmxModuleRegistry::~TmxModuleRegistry() {
	loadedModules().clear();
	modulesEnabled().clear();
}

std::list<std::shared_ptr<TmxModule> > &TmxModuleRegistry::loadedModules() {
	static std::list<std::shared_ptr<TmxModule> > _instance;
	return _instance;
}

std::map<std::string, std::atomic<bool> > &TmxModuleRegistry::modulesEnabled() {
	static std::map<std::string, std::atomic<bool> > _instance;
	return _instance;
}

int TmxModuleRegistry::dl_iterate_callback(dl_phdr_info *info, size_t size, void *data) {
	TmxModuleRegistry *me = static_cast<TmxModuleRegistry *>(data);
	if (!info || !data) return 1;

	boost::filesystem::path p = info->dlpi_name;
	if (exists(p) && exists(p.parent_path()))
		me->modulePaths[p.parent_path()] = true;

	return 0;
}

const char *TmxModule::Type() noexcept { return "Unspecified Module"; }

void TmxModule::Enable() noexcept {
	// Do nothing if the module was already enabled
	if (registry.modulesEnabled()[this->Name()].exchange(true)) return;

	// Try to initialize the module, or else disable it
	try {
		this->Setup();
	} catch (tmx::TmxException &ex) {
		std::cerr << "WARNING: Unable to initialize module " << this->Name() << ": " << ex << std::endl;
		this->Disable();
	} catch (std::exception &ex) {
		std::cerr << "WARNING: Unable to initialize module " << this->Name() << ": " << ex.what() << std::endl;
		this->Disable();
	}
}

void TmxModule::Disable() noexcept {
	// Do nothing if the module was already disabled
	if (registry.modulesEnabled()[this->Name()].exchange(false)) {
		try {
			this->Teardown();
		} catch (tmx::TmxException &ex) {
			std::cerr << "Unable to tear down module " << this->Name() << ": " << ex << std::endl;
		} catch (std::exception &ex) {
			std::cerr << "Unable to tear down module " << this->Name() << ": " << ex.what() << std::endl;
		}
	}
}

bool TmxModule::IsEnabled() noexcept {
	return registry.modulesEnabled()[this->Name()];
}

std::list<std::shared_ptr<TmxModule> >::const_iterator TmxModuleBegin() {
	return registry.loadedModules().begin();
}

std::list<std::shared_ptr<TmxModule> >::const_iterator TmxModuleEnd() {
	return registry.loadedModules().end();
}

size_t TmxModuleSize() {
	return registry.loadedModules().size();
}

} /* namespace utils */
} /* namespace tmx */
