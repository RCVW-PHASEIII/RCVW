/*!
 * Copyright (c) 2024 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file executor.hpp
 *
 *  Created on: May 03, 2024
 *      @author: Gregory M. Baumgardner
 */


#ifndef EXEC_INCLUDE_TMX_COMMON_FORK_EXECUTOR_HPP
#define EXEC_INCLUDE_TMX_COMMON_FORK_EXECUTOR_HPP

#include "../executor.hpp"

#ifdef TMX_UX_POSIX
#define TMX_ASYNC_EXEC_FORK tmx::common::exec::fork

#include <wait.h>

namespace tmx {
namespace common {
namespace exec {

struct fork { };

} /* End namespace exec */

/*!
 * This only works for integral types (or void) since there is no other way for the parent process
 */
template <>
template <typename _Fn, typename ... _Args>
exec::future_ret<_Fn> executor<TMX_ASYNC_EXEC_FORK>::execute(_Fn &&fn, _Args && ... args) {
    std::promise<exec::result_of<_Fn> > _promise;
    auto pid = fork();
    if (pid == 0) {
        exec::synchronous ctx;

        // In the child process, execute the function
        exit(exec_sync(ctx, std::forward<_Fn>(fn), std::forward<_Args>(args)...));
    } else {
        // In the parent process, wait for the return type
        int status;
        waitpid(pid, &status, 0);

        if constexpr (!std::is_same_v<result_of<_Fn>, void>)
            _promise.set_value(result_of<_Fn>(status));

        return _promise.get_future().share();
    }
}

} /* End namespace common */
} /* End namespace tmx */

#endif
#endif // EXEC_INCLUDE_TMX_COMMON_FORK_EXECUTOR_HPP
