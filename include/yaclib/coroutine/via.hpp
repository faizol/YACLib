#pragma once

#include <yaclib/coroutine/detail/via_awaiter.hpp>
#include <yaclib/executor/executor.hpp>

namespace yaclib {

/**
 *
 * @param e
 * @return
 */
detail::ViaAwaiter Via(IExecutor& e);

}  // namespace yaclib
