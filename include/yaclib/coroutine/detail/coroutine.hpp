#pragma once

#if defined(__clang__)
#include <experimental/coroutine>

namespace yaclib_std {

using std::experimental::coroutine_handle;
using std::experimental::coroutine_traits;
using std::experimental::suspend_always;
using std::experimental::suspend_never;

}  // namespace yaclib_std

#else
#include <coroutine>

namespace yaclib_std {

using std::coroutine_handle;
using std::coroutine_traits;
using std::suspend_always;
using std::suspend_never;

}  // namespace yaclib_std
#endif
