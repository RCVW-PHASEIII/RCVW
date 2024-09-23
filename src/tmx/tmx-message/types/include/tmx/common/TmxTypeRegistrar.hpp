/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxTypeRegistrar.hpp
 *
 *  Created on: May 17, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRAR_HPP_
#define TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRAR_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>

namespace tmx {
namespace common {

template <typename _Tp>
class TmxRegistrar;

template <char... _V>
class TmxRegistrar< static_array<char, _V...> >: public TmxFunctor<> {
public:
    static TmxTypeRegistry get_registry() {
        return { static_array<char, _V...>::c_str() };
    }
};

/*!
 * @brief A helper structure to self-register a type instance
 *
 * Declare a static instance of this templated type in order to
 * create the shared instance pointer, and optionally register
 * it under the default name and namespace of _Tp. Typically,
 * this is what you, however, setting the boolean template
 * parameter to false gives more control over where and how to
 * register the type, yet still building the instance for you.
 */
template <typename _Tp>
class TmxTypeRegistrar: public TmxRegistrar< decltype(introspect_namespace<_Tp>()) > {
public:
    TmxTypeRegistrar(common::const_string nm = common::empty_string()) {
        if (this->get_registry().get_namespace() != TmxTypeRegistry(default_namespace().data()).get_namespace())
            this->get_registry().register_instance(this->instance(), nm);
    }

    auto instance() {
        return get_singleton<_Tp>();
    }

    auto const &descriptor() {
        static const auto descr = this->get_registry().get(typeid(_Tp));
        return descr;
    }
};

} /* End namespace common */
} /* End namespace tmx */

#endif /* TYPES_INCLUDE_TMX_COMMON_TMXTYPEREGISTRAR_HPP_ */
