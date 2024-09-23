/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file endian.hpp
 *
 *  Created on: Oct 5, 2022
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_ENDIAN_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_ENDIAN_HPP_

#if __has_include(<endian.h>)

// This optional header file defines the byte order
#include <endian.h>

#define TMX_BYTE_ORDER __BYTE_ORDER

#define TMX_LITTLE_ENDIAN LITTLE_ENDIAN
#define TMX_BIG_ENDIAN BIG_ENDIAN
#define TMX_PDP_ENDIAN PDP_ENDIAN

#else

// No endian.h, so explicitly specify the byte order
// From endian.h on Linux...
/* Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

#define	TMX_LITTLE_ENDIAN 1234
#define TMX_BIG_ENDIAN 4321
#define	TMX_PDP_ENDIAN 3412

#endif

#define TMX_NETWORK_BYTE_ORDER TMX_BIG_ENDIAN

#ifndef TMX_BYTE_ORDER
static_assert(false, "Cannot determine compiler byte order. Please define TMX_BYTE_ORDER");
#else
#if TMX_BYTE_ORDER != TMX_LITTLE_ENDIAN && TMX_BYTE_ORDER != TMX_BIG_ENDIAN
static_assert(false, "Unknown compiler byte order. TMX only supports a big-endian or little-endian compiler");
#endif
#endif

#ifndef htobe32
#if TMX_BYTE_ORDER == TMX_LITTLE_ENDIAN
#if __has_include(<arpa/inet.h>)	// For the hton and ntoh operations
#include <arpa/inet.h>

#define htobe32(x) htonl(x)
#elif TMX_BYTE_ORDER == TMX_BIG_ENDIAN
#define htobe32(x) (x)
#endif
#endif
#endif

#ifndef be32toh
#if TMX_BYTE_ORDER == TMX_LITTLE_ENDIAN
#if __has_include(<arpa/inet.h>)	// For the hton and ntoh operations
#include <arpa/inet.h>

#define be32toh(x) ntohl(x)
#elif TMX_BYTE_ORDER == TMX_BIG_ENDIAN
#define be32toh(x) (x)
#endif
#endif
#endif

#ifndef htobe64
#if TMX_BYTE_ORDER == TMX_LITTLE_ENDIAN
#if __has_include(<arpa/inet.h>)	// For the hton and ntoh operations
#include <arpa/inet.h>

#define htobe64(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#elif TMX_BYTE_ORDER == TMX_BIG_ENDIAN
#define htobe64(x) (x)
#endif
#endif
#endif

#ifndef be64toh
#if TMX_BYTE_ORDER == TMX_LITTLE_ENDIAN
#if __has_include(<arpa/inet.h>)	// For the hton and ntoh operations
#include <arpa/inet.h>

#define be64toh(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#elif TMX_BYTE_ORDER == TMX_BIG_ENDIAN
#define be64toh(x) (x)
#endif
#endif
#endif

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_ENDIANNESS_ENDIAN_HPP_ */
