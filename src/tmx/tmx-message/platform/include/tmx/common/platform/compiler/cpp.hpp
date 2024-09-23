/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file cpp.hpp
 *
 * This include contains basic C++ compiler support checks and definitions.
 *
 * It will include a custom header if it detects a known compiler, in
 * order to bring in that compiler-specific support.
 *
 *  Created on: Oct 5, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_CPP_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_CPP_HPP_

// TODO: Add any other known compiler support
# if defined(__clang__)
static_assert(false, "Currently TMX has no CLANG compiler support")
# elif defined(__QNXNTO__)
#include "impl/qnx.hpp"
# elif defined(__GNUC__)
#include "impl/gnu.hpp"
# elif defined(_MSC_VER)
#include "impl/msvc.hpp"
# endif

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_COMPILER_CPP_HPP_ */
