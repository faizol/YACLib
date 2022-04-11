#include <yaclib/coroutine/detail/via_awaiter.hpp>
#include <yaclib/executor/executor.hpp>
#include <yaclib/executor/task.hpp>

namespace yaclib::detail {

ViaAwaiter::ViaAwaiter(IExecutor& e) : _executor{e} {
}

bool ViaAwaiter::await_ready() {
  return false;
}

void ViaAwaiter::await_resume() {
}

}  // namespace yaclib::detail
