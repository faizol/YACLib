#pragma once

#include <yaclib/coroutine/detail/coroutine.hpp>
#include <yaclib/coroutine/detail/promise_type.hpp>
#include <yaclib/executor/executor.hpp>
#include <yaclib/executor/task.hpp>

namespace yaclib::detail {

class ViaAwaiter {
 public:
  explicit ViaAwaiter(IExecutor& e);

  bool await_ready();
  void await_resume();

  template <typename V, typename E>
  void await_suspend(yaclib_std::coroutine_handle<PromiseType<V, E>> handle) {
    _executor.Submit(handle.promise());
  }

 private:
  IExecutor& _executor;
};

}  // namespace yaclib::detail
