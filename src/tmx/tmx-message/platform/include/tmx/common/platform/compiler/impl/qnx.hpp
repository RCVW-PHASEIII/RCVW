/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file qnx.hpp
 *
 * The basic compiler support for the BlackBerry QNX compiler.
 *
 * In general, the compile has GCC-like support. However, that
 * GCC version is actually older than other compilers that are needed.
 * Below are some features missing, compared to GCC 9:
 *
 * The introspection does not work the same
 *
 *  Created on: Oct 5, 2022
 *      @author: gmb
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_QNX_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_QNX_HPP_

#ifndef _QNX_SOURCE
static_assert(false, "Make sure you enable _QNX_SOURCE to compile for QNX C++17");
#endif

#define __BYTE_ORDER __BYTE_ORDER__

#include "gnu.hpp"

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_IMPL_QNX_HPP_ */
