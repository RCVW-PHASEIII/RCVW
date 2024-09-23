/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxMessageHandler.hpp
 *
 *  Created on: Mar 2, 2023
 *      @author: Gregory M. Baumgardner
 */

#ifndef CLIENT_INCLUDE_TMX_PLUGIN_TMXMESSAGEHANDLER_HPP_
#define CLIENT_INCLUDE_TMX_PLUGIN_TMXMESSAGEHANDLER_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/TmxFunctor.hpp>
#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/TmxTypeDescriptor.hpp>
#include <tmx/common/TmxTypeHandler.hpp>
#include <tmx/common/TmxTypeRegistrar.hpp>
#include <tmx/common/TmxTypeRegistry.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <tmx/message/codec/TmxCodec.hpp>
#include <tmx/plugin/dao/TmxDaoTraits.hpp>
#include <tmx/plugin/utils/Uuid.hpp>

#include <thread>
#include <type_traits>

namespace tmx {
namespace plugin {

/*!
 * @brief The basic interface for a handler of TMX messages
 *
 * The handler is registered into the namespace based on the
 * _Tag parameter. Note, however, that this parameter is optional.
 * For singular handlers or for multiple instances that each
 * handle different _Dao types, then this default will suffcice.
 * However, t is important for separating the handler into a
 * different namespace and with unique run-time data, in order
 * to avoid any ambiguity for multiple handlers of the same _Dao type.
 */
template <class _Dao, typename _Tag = _Dao, typename _Ret = common::TmxError>
class TmxMessageHandler: public common::Functor<_Ret, common::types::Any const &, message::TmxMessage const &> {
    typedef TmxMessageHandler<_Dao, _Tag> self_type;
    typedef common::Functor<_Ret, _Dao &, message::TmxMessage const &> fn_type;

    common::TmxTypeRegistry _reg { common::type_namespace(*this).data() };
public:
    TmxMessageHandler() = delete;
    TmxMessageHandler(fn_type const &functor): _functor(functor) {
        // Register the handler function instance
        std::shared_ptr<const void> _hPtr { static_cast<const void *>(this), [](auto *) { } };
        _reg.register_type(_hPtr, typeid(*this), common::type_short_name(*this));
    }
    TmxMessageHandler(self_type const &copy): _functor(copy._functor) { }

    self_type &operator=(self_type const &copy) {
        this->_functor = copy._functor;
        return *this;
    }

    common::TmxTypeDescriptor get_descriptor() const {
        return _reg.get(typeid(*this));
    }

    operator bool() const {
        return (bool)_functor;
    }

    _Ret execute(common::types::Any const &data, message::TmxMessage const &msg) override {
        // Construct the DAO
        // TODO: Dependent on DOA traits
        auto _dao = dao::make_dao<_Dao>(data);
        if (_functor)
            return (_Ret) _functor.execute(_dao, msg);

        std::string err { "Functor function " };
        err.append(TMX_PRETTY_FUNCTION);
        err.append(" has not been properly set.");

        return (_Ret) common::TmxError(EOPNOTSUPP, err);
    }

    _Ret execute(common::types::Any const &data, message::TmxMessage const &msg) const override {
        // Construct the DAO
        // TODO: Dependent on DOA traits
        auto _dao = dao::make_dao<_Dao>(data);
        if (_functor)
            return (_Ret) _functor.execute(_dao, msg);

        std::string err { "Functor function " };
        err.append(TMX_PRETTY_FUNCTION);
        err.append(" has not been properly set.");

        return (_Ret) common::TmxError(EOPNOTSUPP, err);
    }

private:
    fn_type _functor;
};

template <typename _Tag, typename _Dao, typename _Ret>
auto make_handler(std::function<_Ret(_Dao &, message::TmxMessage const &)> &&fn) {
    return tmx::common::make_functor(fn);
}

template <typename _Dao, typename _Ret>
auto make_handler(std::function<_Ret(_Dao &, message::TmxMessage const &)> &&fn) {
    return make_handler<_Dao, _Dao, _Ret>(std::forward<decltype(fn)>(fn));
}

template <typename _Tag, typename _Dao, typename _Ret, class _Handler>
TmxMessageHandler<_Dao, _Tag, _Ret> make_handler(_Handler *instance,
                                                 _Ret (_Handler::*handler)(_Dao &, message::TmxMessage const &)) {
    // The function must be static to preserve the instance pointer
    typename tmx::common::function_of<decltype(handler)>::type _fn =
            std::bind(handler, instance, std::placeholders::_1, std::placeholders::_2);
    return make_handler<_Tag, _Dao, _Ret>(std::move(_fn));
}

template <typename _Dao, typename _Ret, class _Handler>
auto make_handler(_Handler *instance, _Ret (_Handler::*handler)(_Dao &, message::TmxMessage const &)) {
    return make_handler<_Dao, _Dao, _Ret>(instance, handler);
}

} /* End namespace plugin */
} /* End namespace tmx */

#endif //CLIENT_INCLUDE_TMX_PLUGIN_TMXMESSAGEHANDLER_HPP_
