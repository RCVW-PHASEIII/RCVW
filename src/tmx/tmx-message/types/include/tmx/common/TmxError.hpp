/*
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxError.hpp
 *
 *  Created on: Feb 6, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXERROR_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXERROR_HPP_

#ifndef TMX_NO_STACKTRACE
#define BOOST_STACKTRACE_USE_BACKTRACE

#include <boost/stacktrace.hpp>
#ifndef TMX_STACKTRACE_MAX_BUFFER_SIZE
#define TMX_STACKTRACE_MAX_BUFFER_SIZE 4096
#endif
#else
#define BOOST_STACKTRACE_USE_NOOP
#endif

#include <tmx/common/types/Any.hpp>

#include <exception>
#include <sstream>
#include <system_error>

namespace tmx {
namespace common {

/*!
 * @brief A class that represents TMX errors
 *
 * This class is really just a simple name-value pair of
 * error codes, messages, exceptions, and backtraces, if
 * available. It does, however, contain an interface that
 * is analogous to the std::error_code.
 *
 * Note that each property has a string name, but can be
 * Any type. This includes the integer error code or the
 * exception object.
 */
class TmxError: public common::types::Properties<common::types::Any>::value_type {
	typedef typename common::types::Properties<common::types::Any>::value_type super;
public:
	template <typename _T, typename = enable_scalar<_T> >
	inline TmxError(_T code, std::string message = "", int skip = 1) {
		this->operator[](key_type("code")).emplace<int>(code);

        if (!message.empty())
		    this->operator[](key_type("message")).emplace<std::string>(message.data());

        if (code) {
            char buffer[TMX_STACKTRACE_MAX_BUFFER_SIZE];
            boost::stacktrace::safe_dump_to(skip, buffer, TMX_STACKTRACE_MAX_BUFFER_SIZE);

            std::ostringstream os;
            os << boost::stacktrace::stacktrace::from_dump(buffer, TMX_STACKTRACE_MAX_BUFFER_SIZE);

            this->operator[](key_type("stacktrace")).emplace<std::string>(os.str());
        }
	}

    TmxError(): TmxError(EXIT_SUCCESS, std::strerror(EXIT_SUCCESS)) { }

	inline TmxError(std::exception const &ex): TmxError((int)common::types::TmxTypeOf<int>::limits().max(), ex.what(), 2) {
		this->operator[](key_type("exception")).emplace<std::exception>(ex);
	}

	inline TmxError(std::error_code const &ec): TmxError(ec.value(), ec.message(), 2) {
		this->operator[](key_type("category")) = ec.category().name();
	}

    inline TmxError(std::error_condition const &ec): TmxError(ec.value(), ec.message(), 2) {
        this->operator[](key_type("category")) = ec.category().name();
    }

    inline TmxError(TmxError const &copy): super(dynamic_cast<const super &>(copy)) { }

	virtual ~TmxError() = default;

	inline TmxError &operator=(TmxError const &copy) {
		super::operator=(dynamic_cast<const super &>(copy));
		return *this;
	}

	inline int get_code() const noexcept {
        static const key_type param { "code" };
		if (this->count(param)) {
			auto v = tmx::common::any_cast<int>(&(this->at(param)));
			if (v) return *v;
		}

		if (this->size())
			return 1;

		return 0;
	}

	inline std::string get_message() const noexcept {
        static const key_type param { "message" };
		if (this->count(param)) {
			auto v = tmx::common::any_cast<std::string>(&(this->at(param)));
			if (v) return *v;
		}
		return "";
	}

    inline operator int() const {
        return this->get_code();
    }

	inline operator bool() const {
		return this->get_code() != 0;
	}
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXERROR_HPP_ */
