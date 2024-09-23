/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file tuple.hpp
 *
 *  Created on: Sep 06, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TUPLE_HPP_
#define PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TUPLE_HPP_

#include <tuple>

namespace tmx {
namespace common {

template <typename ...>
struct static_tuple_cat;

template <typename ... _Tp>
struct static_tuple_cat< std::tuple<_Tp...> > {
    typedef std::tuple<_Tp...> type;
};

template <typename ... _Tp>
struct static_tuple_cat< std::tuple<_Tp...>, std::tuple<> > {
    typedef std::tuple<_Tp...> type;
};

template <typename ... _Tp>
struct static_tuple_cat< std::tuple<>, std::tuple<_Tp...> > {
    typedef std::tuple<_Tp...> type;
};

template <typename ... _Tp1, typename ... _Tp2>
struct static_tuple_cat< std::tuple<_Tp1...>, std::tuple<_Tp2...> > {
    typedef std::tuple<_Tp1..., _Tp2...> type;
};

template <typename ... _Tp1, typename ... _Tp2, typename ... _Others>
struct static_tuple_cat< std::tuple<_Tp1...>, std::tuple<_Tp2...>, _Others... > {
    typedef std::tuple<_Tp1..., _Tp2...> _pt1;
    typedef typename static_tuple_cat<_Others...>::type _pt2;
    typedef typename static_tuple_cat< _pt1, _pt2 >::type type;
};

template <typename ... _Tp>
using tuple_cat = typename static_tuple_cat<_Tp...>::type;

template <typename ... _Tp>
struct static_tuple_clean;

template <>
struct static_tuple_clean< std::tuple<void> > {
    typedef std::tuple<> type;
};

template <typename _T>
struct static_tuple_clean< std::tuple<_T> > {
    typedef std::tuple<_T> type;
};

template <typename ... _Tp>
struct static_tuple_clean< std::tuple<_Tp...> > {
    typedef decltype(std::tuple_cat(std::declval<typename static_tuple_clean<_Tp>::type>()...)) type;
};

template <typename... _Tp>
using tuple_clean = typename static_tuple_clean< std::tuple<_Tp...> >::type;

}
}

#endif /* PLATFORM_INCLUDE_TMX_COMMON_PLATFORM_TYPES_TUPLE_HPP_ */
