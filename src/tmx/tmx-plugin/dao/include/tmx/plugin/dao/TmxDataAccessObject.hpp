/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataAccessObject.hpp
 *
 *  Created on: Mar 9, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDATAACCESSOBJECT_HPP_
#define DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDATAACCESSOBJECT_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxError.hpp>
#include <tmx/common/types/Any.hpp>

#include <typeinfo>

namespace tmx {
namespace plugin {
namespace dao {

class TmxDataAccessObject {
public:
    TmxDataAccessObject();
    virtual ~TmxDataAccessObject() = default;

    TmxDataAccessObject(common::types::Any &);
    TmxDataAccessObject(common::types::Any const &);
    TmxDataAccessObject(TmxDataAccessObject const &);

    common::const_string get_contents() const noexcept;
    std::type_info const &get_typeid() const noexcept;

    operator typename common::types::Null::value_type () const;
    operator typename common::types::Boolean::value_type () const;
    operator typename common::types::UIntmax::value_type () const;
    operator typename common::types::Intmax::value_type () const;
    operator typename common::types::Floatmax::value_type () const;
    operator typename common::types::String8::value_type () const;
    operator const typename common::types::String8::char_t * () const;

    template <typename _Tp>
    common::TmxError as_type(common::types::TmxDataType<_Tp> &out) const;

    TmxDataAccessObject &operator[](const typename common::types::UIntmax::value_type);
    TmxDataAccessObject &operator[](const typename common::types::UIntmax::value_type) const;
    TmxDataAccessObject &operator[](const typename common::types::String8::value_type &);
    TmxDataAccessObject &operator[](const typename common::types::String8::value_type &) const;
    TmxDataAccessObject &operator[](const typename common::types::String8::char_t *);
    TmxDataAccessObject &operator[](const typename common::types::String8::char_t *) const;

    template <typename _E>
    typename std::enable_if<common::is_enum_type<_E>(), TmxDataAccessObject &>::type operator[](const _E);

    template <typename _E>
    typename std::enable_if<common::is_enum_type<_E>(), TmxDataAccessObject &>::type operator[](const _E) const;

private:
    common::types::Any _data;
};

} /* End namespace dao */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDATAACCESSOBJECT_HPP_ */
