/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file gnu.hpp
 *
 * The basic compiler support options for GCC
 *
 *  Created on: Oct 5, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_GNU_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_GNU_HPP_

#define TMX_PRETTY_FUNCTION __PRETTY_FUNCTION__

//#include <boost/unordered_map.hpp>
//#define TMX_HASH_ARRAY boost::unordered_map

#if __GNUC__ < 9

/*!
 * The pretty function output is a character array, but not a constant
 * expression, therefore it cannot be saved into a constant string
 * for parsing.
 */
#define TMX_PRETTY_FUNCTION_NOT_CONSTEXPR 1

#else

#define TMX_PRETTY_FUNCTION_PARSE_START "[with _T = "
#define TMX_PRETTY_FUNCTION_PARSE_END ";"

#endif

#ifdef __GXX_RTTI
#define TMX_RTTI_ENABLED 1
#endif

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_GNU_HPP_ */
