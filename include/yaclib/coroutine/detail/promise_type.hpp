#pragma once

#include <yaclib/async/detail/result_core.hpp>
#include <yaclib/coroutine/detail/coroutine_deleter.hpp>
#include <yaclib/util/detail/atomic_counter.hpp>
#include <yaclib/util/intrusive_ptr.hpp>

#include <exception>

namespace yaclib::detail {

template <typename V, typename E>
class PromiseType;

template <typename V, typename E>
class Destroy {
 public:
  Destroy() noexcept = default;

  bool await_ready() noexcept {
    return false;
  }

  void await_resume() noexcept {
  }

  void await_suspend(yaclib_std::coroutine_handle<PromiseType<V, E>> handle) noexcept {
    handle.promise().DecRef();
  }
};

template <typename V, typename E>
class BasePromiseType : public AtomicCounter<ResultCore<V, E>, CoroutineDeleter> {
  using Base = AtomicCounter<ResultCore<V, E>, CoroutineDeleter>;

 public:
  BasePromiseType() : Base{2} {  // get_return_object is gonna be invoked right after ctor
  }

  Future<V, E> get_return_object() {
    return {ResultCorePtr<V, E>{NoRefTag{}, this}};
  }

  yaclib_std::suspend_never initial_suspend() noexcept {
    return {};
  }

  Destroy<V, E> final_suspend() noexcept {
    return {};
  }

  void unhandled_exception() noexcept {
    Base::Set(std::current_exception());
  }

 private:
  void Call() noexcept final {
    auto handle = yaclib_std::coroutine_handle<PromiseType<V, E>>::from_promise(static_cast<PromiseType<V, E>&>(*this));
    assert(handle);
    assert(!handle.done());
    handle.resume();
  }

  void Cancel() noexcept final {
    auto handle = yaclib_std::coroutine_handle<PromiseType<V, E>>::from_promise(static_cast<PromiseType<V, E>&>(*this));
    assert(handle);
    assert(!handle.done());
    Base::Set(StopTag{});
    Base::Cancel();
  }
};

template <typename V, typename E>
class PromiseType : public BasePromiseType<V, E> {
  using Base = BasePromiseType<V, E>;

 public:
  void return_value(const V& value) noexcept(std::is_nothrow_copy_constructible_v<V>) {
    Base::Set(value);
  }

  void return_value(V&& value) noexcept(std::is_nothrow_move_constructible_v<V>) {
    Base::Set(std::move(value));
  }
};

template <typename E>
class PromiseType<void, E> : public BasePromiseType<void, E> {
  using Base = BasePromiseType<void, E>;

 public:
  void return_void() noexcept {
    Base::Set(Unit{});
  }
};

}  // namespace yaclib::detail
