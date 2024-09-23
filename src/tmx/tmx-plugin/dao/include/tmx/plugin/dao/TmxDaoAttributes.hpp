/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDaoAttributes.hpp
 *
 *  Created on: Jun 13, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDAOATTRIBUTES_HPP_
#define DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDAOATTRIBUTES_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxData.hpp>

#include <thread>
#include <type_traits>

namespace tmx {
namespace plugin {
namespace dao {

template <typename _Dao>
common::TmxTypeRegistry get_attributes() noexcept {
    static common::TmxTypeRegistry _base { common::type_fqname<_Dao>().data() };
    return _base / "attributes";
}

template <typename _Dao>
common::TmxTypeRegistry get_attributes(_Dao const &dao) noexcept {
    return get_attributes<_Dao>();
}

template <class _Dao>
class TmxDaoAttribute {
public:
    TmxDaoAttribute(const TmxDaoAttribute &) = delete;
    TmxDaoAttribute &operator=(const TmxDaoAttribute &) = delete;

    virtual common::TmxTypeDescriptor get_descriptor() const noexcept {
        static std::shared_ptr<const void> _ptr { static_cast<const void *>(this), [](auto *) {} };
        return { _ptr, typeid(void), "" };
    }

protected:
    TmxDaoAttribute() = default;

    void register_attribute() {
        auto _desc = this->get_descriptor();
        get_attributes<_Dao>().register_type(_desc.get_instance(), _desc.get_typeid(), _desc.get_type_name() );
    }
};

template <class _Dao, typename _Tp, typename _Name>
class TmxDaoAttributeImpl: public TmxDaoAttribute<_Dao> {
public:
    typedef common::types::TmxTypeTraits<_Tp> traits_type;
    typedef typename std::conditional<traits_type::TMX::value, typename traits_type::type, _Tp>::type type;
    typedef common::types::TmxValueTypeOf<_Tp> value_type;

    static constexpr auto name = _Name::c_str();

    TmxDaoAttributeImpl(): TmxDaoAttribute<_Dao>() {
        // Register non-TMX type
        static common::TmxTypeRegistrar<type> _registar;
        static bool _registered = false;

        // Register the attribute by name
        if (!_registered)
            this->register_attribute();

        _registered = true;
    }

    common::TmxTypeDescriptor get_descriptor() const noexcept {
        static common::TmxTypeDescriptor _tmp = TmxDaoAttribute<_Dao>::get_descriptor();
        static common::TmxTypeDescriptor _ret { _tmp.get_instance(), typeid(_Tp), name };
        return _ret;
    }
};

template <class _Dao, typename _Tp, typename _Name>
auto _make_momento(_Dao const &obj, std::tuple< TmxDaoAttributeImpl<_Dao, _Tp, _Name> > const &attr) {
    typedef TmxDaoAttributeImpl<_Dao, _Tp, _Name> type;


}

template <class _Dao, typename _Tp, typename _Name>
auto _make_record(message::TmxData const &obj,
                  std::tuple< TmxDaoAttributeImpl<_Dao, _Tp, _Name> > const &attr) {
    typedef TmxDaoAttributeImpl<_Dao, _Tp, _Name> type;
    common::TmxTypeDescriptor _descr = std::get<0>(attr).get_descriptor();

    if (_descr)
        return std::make_tuple((typename type::type) obj.operator[](_descr.get_type_short_name().data()));
    else
        return std::make_tuple(typename type::type { });
}

template <class _Dao, typename _Tp, typename _Name, class ... _Others>
auto _make_record(message::TmxData const &obj,
                  std::tuple<TmxDaoAttributeImpl<_Dao, _Tp, _Name>, _Others...> const &attrs) {
    return std::tuple_cat(_make_record(obj, std::tuple< TmxDaoAttributeImpl<_Dao, _Tp, _Name> > { }),
                          _make_record(obj, std::tuple<_Others...> { }));
}

template <class ... _Attrs, class ... _Vals>
common::TmxError tie_attributes(common::types::Any const &obj, std::tuple<_Attrs...> const &attrs, _Vals &...vals) {
    std::tie(vals...) = _make_record( { obj }, attrs);
    return { };
}

}
}
}

#define tmx_dao_attribute_def(T, N, D) \
public:                                                                                                     \
    static constexpr const auto N ## _s = TMX_STATIC_STRING(#N);                                            \
    typedef tmx::plugin::dao::TmxDaoAttributeImpl<self_type, T, decltype(N ## _s)> N ## _attr;              \
private:                                                                                                    \
    typename N ## _attr::type _ ## N { D };

#define tmx_dao_attribute_getter(N)                                                                         \
    inline auto get_ ## N() const noexcept {                                                                \
        return this->_ ## N;                                                                                \
    }

#define tmx_dao_attribute_setter(N)                                                                         \
    inline void set_ ## N(typename N ## _attr::value_type const &v) noexcept {                              \
        this->_ ## N = v;                                                                                   \
    }

#define tmx_dao_ro_attribute(T, N, D)                                                                       \
    tmx_dao_attribute_def(T, N, D)                                                                          \
private:                                                                                                    \
    tmx_dao_attribute_setter(N)                                                                             \
public:                                                                                                     \
    tmx_dao_attribute_getter(N)

#define tmx_dao_attribute(T, N, D)                                                                          \
    tmx_dao_attribute_def(T, N, D)                                                                          \
public:                                                                                                     \
    tmx_dao_attribute_getter(N)                                                                             \
    tmx_dao_attribute_setter(N)

#endif /* DAO_INCLUDE_TMX_PLUGIN_DAO_TMXDAOATTRIBUTES_HPP_ */
