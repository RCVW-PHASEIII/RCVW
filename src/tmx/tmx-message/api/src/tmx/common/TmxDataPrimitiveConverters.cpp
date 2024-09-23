/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataPrimitiveConverters.cpp
 *
 *  Created on: Jul 21, 2021
 *      @author: gmb
 */

#include <tmx/common/TmxData.hpp>
#include <tmx/common/TmxTypeVisitor.hpp>

#include <cstdlib>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/algorithm/string.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

using namespace tmx::common::types;

namespace tmx {
namespace common {

template <class _T>
class _NumericVisitor: public TmxTypeVisitor {
public:
	using TmxTypeVisitor::dispatch;

	string::view_type get_name() const override {
		return _T::name;
	}

	typedef typename _T::type type;
	static_assert(std::is_arithmetic_v<type>, "expecting an arithmetic type");

	type value = (type) 0;

	template <typename _Val = type>
	std::error_code ec(_Val val = 0) { value = (type) val; return std::error_code(); }

	std::error_code dispatch(const null_type &) override {
		return ec(0);
	}
	std::error_code dispatch(const boolean_type &b) override {
		return ec(*b ? 1 : 0);
	}
	std::error_code dispatch(const uint1 &i) override {
		return ec(*i);
	}
	std::error_code dispatch(uint64 &&i, uint8_t bits) override {
		return ec(*i);
	}
	std::error_code dispatch(int64 &&i, uint8_t bits) override {
		return ec(*i);
	}
#ifdef TMX_FLOAT128
	std::error_code dispatch(const float128 &f) override {
		return ec(*f);
	}
#endif
	std::error_code dispatch(const string8 &s) override {
		// Special cases for boolean type
		// If the value is non-numeric, check specifically
		// for the false. Every other non-null string is true
		if (std::is_same_v<type, bool>) {
			if (boost::iequals(s.c_str(), "false") ||
					boost::iequals(s.c_str(), "f") ||
					boost::iequals(s.c_str(), "no") ||
					boost::iequals(s.c_str(), "n") ||
					boost::iequals(s.c_str(), "off"))
				return ec(0);
			else
				return ec(s.length());
		}

		if (std::is_floating_point_v<type>) {
			return ec(std::strtold(s.c_str(), nullptr));
		} else {
			if (std::numeric_limits<type>::is_signed) {
				return ec(std::strtoll(s.c_str(), nullptr, 0));
			} else {
				return ec(std::strtoull(s.c_str(), nullptr, 0));
			}
		}
	};
	std::error_code dispatch(const array_type<TmxData> &s) override {
		return ec(s.size());
	}
	std::error_code dispatch(const map_type<string, TmxData> &m) override {
		return ec(m.size());
	}
};

TmxData::operator typename null_type::type () const {
	// No matter what is contained, convert to a null pointer
	return null_type::value;
}

template <typename _V>
typename _V::type convert(const TmxData &a) {
	_V visitor;
	visitor.dispatch(a);
	return visitor.value;
}

template <typename _T>
using num_v = _NumericVisitor<_T>;

TmxData::operator typename int64::type() const {
	return convert< num_v<int64> >(*this);
}

TmxData::operator typename int32::type() const {
	return convert< num_v<int32> >(*this);
}

TmxData::operator typename int16::type() const {
	return convert< num_v<int16> >(*this);
}

TmxData::operator typename int8::type() const {
	return convert< num_v<int8> >(*this);
}

TmxData::operator typename uint64::type() const {
	return convert< num_v<uint64> >(*this);
}

TmxData::operator typename uint32::type() const {
	return convert< num_v<uint32> >(*this);
}

TmxData::operator typename uint16::type() const {
	return convert< num_v<uint16> >(*this);
}

TmxData::operator typename uint8::type() const {
	return convert< num_v<uint8> >(*this);
}

TmxData::operator typename float32::type() const {
	return convert< num_v<float32> >(*this);
}

TmxData::operator typename float64::type() const {
	return convert< num_v<float64> >(*this);
}

#ifdef TMX_FLOAT128
TmxData::operator typename float128::type() const {
	return convert< num_v<float128> >(*this);
}
#endif

TmxData::operator typename boolean_type::type () const {
	return convert< num_v<boolean_type> >(*this) != 0;
}


} /* End namespace common */
} /* End namespace tmx */
