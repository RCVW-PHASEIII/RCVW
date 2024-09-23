/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file msvc.hpp
 *
 * The basic compiler support for Visual Studio
 *
 *  Created on: Oct 5, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_MSVC_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_MSVC_HPP_

#define TMX_PRETTY_FUNCTION __FUNCSIG__

#define TMX_PRETTY_FUNCTION_PARSE_START "<"
#define TMX_PRETTY_FUNCTION_PARSE_END ",0x00"

#define TMX_MAX_FLOAT long double

#if __has_include(<Windows.h>)

#include <Windows.h>

#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#define TMX_BYTE_ORDER TMX_LITTLE_ENDIAN
#elif REG_DWORD == REG_DWORD_BIG_ENDIAN
#define TMX_BYTE_ORDER TMX_BIG_ENDIAN
#endif

#endif

#ifdef _CPPRTTI
#define TMX_RTTI_ENABLED 1
#endif

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_MSVC_HPP_ */
