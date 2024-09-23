/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file exception.hpp
 *
 *  Created on: Jun 16, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_COMMON_TMXEXCEPTION_HPP_
#define INCLUDE_TMX_COMMON_TMXEXCEPTION_HPP_

#ifdef _MSC_VER
#define BOOST_STACKTRACE_USE_WINDBG 1
#endif

#include "boost/exception/all.hpp"
//#include "boost/stacktrace.hpp"

#include <cstring>
#ifndef _MSC_VER
#ifndef __QNXNTO__
#include <cxxabi.h>
#include <execinfo.h>
#endif
#endif
#include <exception>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string_view>
#include <utility>

#include <iostream>
#define BT_BUFFER_SIZE 200

#define TMX_THROW(x) BOOST_THROW_EXCEPTION( x )

namespace tmx {
namespace common {

#ifdef TMX_EXCEPTION_NO_STACKTRACE
#define BOOST_STACKTRACE_USE_NOOP
#endif

/*!
 * A generic exception class that maintains a back trace of function calls for later use.
 *
 * Every TMX exception is considered a run-time error. This class may be sub-classed
 * to build a hierarchy of program-specific exceptions.
 */
class TmxException: public boost::exception {
public:
	typedef TmxException self_type;

	// TODO: typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> err_stack;
	typedef boost::error_info<struct tag_stacktrace, std::string> err_stack;
	typedef boost::error_info<struct tag_errmsg, std::string_view> err_msg;
	typedef boost::error_info<struct tag_errno, int> err_no;
	typedef boost::error_info<struct tag_source, TmxException> err_src;

	explicit TmxException(std::string_view what = ""): _what(what) {
		this->set_backtrace();
		if (!what.empty())
			*this << err_msg(what);
	}

	explicit TmxException(const std::exception &err): TmxException(err.what()) { }

	TmxException(const TmxException &&err): TmxException(err.what()) { }

	TmxException(const TmxException &err): TmxException(std::move(err)) {
		*this << err_src(err);
	}

	typedef decltype(&std::strerror) fn_def;

	TmxException(int const errno, fn_def = std::strerror): TmxException() {
		static constexpr std::size_t errmsglen = 2048;
		char errmsg[errmsglen];
#ifdef _MSVC_LANG
		strerror_s(errmsg, errmsglen, errno);
#else
		strerror_r(errno, errmsg, errmsglen);
#endif
		this->_what.append(errmsg);
		*this << err_no(errno);
		*this << err_msg(what());
	}

	const char *what() const { return _what.data(); }

/*	inline void set_backtrace(const stacktrace &stacktrace) {

	    boost::stacktrace::detail::to_string_impl impl;

		for (auto iter = stacktrace.cbegin(); iter != stacktrace.cend(); iter++) {
			set_backtrace(iter->address());
		}
		std::stringstream ss;
		ss << stacktrace;
		this->_backtrace = ss.str();
	}

	template <typename Ex>
	inline void set_backtrace(const Ex &err) {
		auto bt_ptr = boost::get_error_info<traced>(err);
		if (bt_ptr)
			set_backtrace(*bt_ptr);
	}
*/

	inline void set_backtrace(const void *callerPtr = nullptr) {
#ifndef _MSC_VER
#ifndef __QNXNTO__
		//stacktrace bt;
		//return set_backtrace(bt);
		void *callerAddr = const_cast<void *>(callerPtr);
		std::stringstream ss;

		void *btBuffer[BT_BUFFER_SIZE];
		int nptrs = backtrace(btBuffer, BT_BUFFER_SIZE);

		/* overwrite sigaction with caller's address */
		if (nptrs > 1 && callerAddr > 0)
			btBuffer[1] = callerAddr;

		char **symbols = backtrace_symbols(btBuffer, nptrs);

		/* skip first stack frame (points here) */
		for (int i = 0; symbols != NULL && i < nptrs; i++)
		{
			std::string_view s(symbols[i]);
			size_t start, end;

			start = s.find_first_of('(');
			if (start >= 0 && start + 1 < s.length())
			{
				start++;
				end = s.find_first_of('+', start);
				if (end >= 0)
				{
					std::string_view mangledName = s.substr(start, end - start);
					std::string_view demangledName = demangle_type(mangledName.data());
					if (demangledName.find("demangling error") == std::string::npos)
					{

						// No error
						ss << s.substr(0, start + 1) << demangledName << s.substr(end) << std::endl;
						continue;
					}
				}
			}

			ss << symbols[i] << std::endl;
		}

		free(symbols);

		*this << err_stack(ss.str());
#endif
#endif
	}


	/**
	 * A function to write out the exception information, including back trace, to a stream
	 */
	friend inline std::ostream &operator<<(std::ostream &os, const TmxException &ex)
	{
		os << ex.what() << std::endl;
		os << "diagnostic info (Hint: Use addr2line -C -e <exe> 0x#######) to find line number):" << std::endl << boost::diagnostic_information(ex);
		return os;
	}
private:
	std::string _what;

#ifndef _MSC_VER
#ifndef __QNXNTO__
	/**
	 * Function to demangle a C++ type name.
	 *
	 * @param mangled The mangled name as a C-style string, from typeid
	 * @return The demangled name as a C++ string.
	 */
	static inline std::string_view demangle_type(const char *mangled) {
	    int status = 0;
	    char *realname;
	    realname = abi::__cxa_demangle(mangled, 0, 0, &status);

	    std::stringstream ss;

	    if (!status)
	    	return realname;
	    else
	    	return mangled;
	}
#endif
#endif
};


} /* End namespace common */
} /* End namespace tmx */


#endif /* INCLUDE_TMX_COMMON_TMXEXCEPTION_HPP_ */
