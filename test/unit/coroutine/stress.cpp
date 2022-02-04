#include <util/time.hpp>

#include <yaclib/async/run.hpp>
#include <yaclib/coroutine/await.hpp>
#include <yaclib/coroutine/future_coro_traits.hpp>
#include <yaclib/executor/thread_pool.hpp>
#include <yaclib/fault/atomic.hpp>
#include <yaclib/fault/thread.hpp>

#include <array>
#include <exception>
#include <random>
#include <stack>
#include <utility>

#include <gtest/gtest.h>

namespace {
using namespace std::chrono_literals;
using namespace yaclib;

yaclib_std::atomic_size_t state = 0;

Future<size_t> incr(uint64_t delta) {
  size_t old = state.fetch_add(delta, std::memory_order_acq_rel);
  co_return old;
}

TEST(StressCoro, IncAtomicSingleThread) {
  constexpr size_t kTestCases = 100'000;
  constexpr size_t kAwaitedFutures = 20;
  constexpr size_t kMaxDelta = 10;

  size_t acum = 0;

  size_t control_value = 0;

  for (size_t tc = 0; tc < kTestCases; ++tc) {
    std::mt19937 rng(tc);
    auto coro = [&]() -> Future<size_t> {
      std::array<Future<size_t>, kAwaitedFutures> futures;
      for (size_t i = 0; i < kAwaitedFutures; ++i) {
        size_t arg = rng() % kMaxDelta;
        control_value += state.load(std::memory_order_relaxed);
        futures[i] = incr(arg);
      }
      co_await Await(futures.begin(), futures.end());
      size_t loc_accum = 0;
      for (auto&& future : futures) {
        loc_accum += std::move(future).Get().Ok();
      }
      co_return loc_accum;
    };
    control_value -= coro().Get().Ok();
  }
  EXPECT_EQ(control_value, 0);
}

}  // namespace
