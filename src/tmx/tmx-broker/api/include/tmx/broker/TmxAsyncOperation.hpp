/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxAsyncOperation.hpp
 *
 *  Created on: Aug 30, 2021
 *      @author: gmb
 */

#ifndef API_INCLUDE_TMX_BROKER_TMXASYNCOPERATION_HPP_
#define API_INCLUDE_TMX_BROKER_TMXASYNCOPERATION_HPP_

#include <functional>
#include <future>

namespace tmx {
namespace broker {

/*!
 * @brief Execute a non-return function using std::async
 *
 * This returns a pointer to the std::future object created by the
 * asynchronous invocation. For many TMX functions, which return a void,
 * this pointer can be ignored simply by invoking the reset() operation
 * on it.
 *
 * @param[in] fn The function to execute
 * @param[in] args The arguments to the function, including a "this" pointer for member functions
 * @return The future to use in order to access the return value or to synchronize
 * @see std::async
 * @see std::future
 */
template <class Fn, class... Args>
auto async_invoke(Fn &&fn, Args&&... args) {
	typedef typename std::function<Fn(Args...)>::result_type return_type;

	return std::make_unique<std::future<return_type>*>(new auto(std::async(fn, args...)));
}

} /* namespace broker */
} /* namespace tmx */

#endif /* API_INCLUDE_TMX_BROKER_TMXASYNCOPERATION_HPP_ */
