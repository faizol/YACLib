#include <util/mpsc_stack.hpp>

#include <yaclib/config.hpp>
#include <yaclib/executor/executor.hpp>
#include <yaclib/executor/strand.hpp>
#include <yaclib/executor/task.hpp>
#include <yaclib/fault/atomic.hpp>
#include <yaclib/util/helper.hpp>
#include <yaclib/util/intrusive_ptr.hpp>

#include <cstdint>
#include <utility>

namespace yaclib {
namespace {

class alignas(kCacheLineSize) Strand : public IExecutor, public ITask {
  // Inheritance from two IRef's, but that's okay, because they are pure virtual
 public:
  explicit Strand(IExecutorPtr executor) : _executor{std::move(executor)} {
  }

  ~Strand() override {
    assert(_tasks.load(std::memory_order_acquire) == Mark());
  }

 private:
  [[nodiscard]] Type Tag() const final {
    return Type::Strand;
  }

  void Submit(ITask& task) noexcept final {
    auto* old = _tasks.load(std::memory_order_relaxed);
    do {
      task.next = old == Mark() ? nullptr : old;
    } while (!_tasks.compare_exchange_weak(old, &task, std::memory_order_release, std::memory_order_relaxed));
    if (old == Mark()) {
      static_cast<ITask&>(*this).IncRef();
      _executor->Submit(*this);
    }
  }

  void Call() noexcept final {
    auto* node = _tasks.exchange(nullptr, std::memory_order_acquire);
    Node* prev = nullptr;
    while (node != nullptr) {
      auto* next = node->next;
      node->next = prev;
      prev = node;
      node = next;
    }
    while (prev != nullptr) {
      auto* next = prev->next;
      static_cast<ITask*>(prev)->Call();
      prev = next;
    }
    if (_tasks.compare_exchange_strong(node, Mark(), std::memory_order_release, std::memory_order_relaxed)) {
      static_cast<ITask&>(*this).DecRef();
    } else {
      _executor->Submit(*this);
    }
  }

  void Cancel() noexcept final {
    auto* node = _tasks.exchange(Mark(), std::memory_order_acquire);
    while (node != nullptr) {
      auto* next = node->next;
      static_cast<ITask*>(node)->Cancel();
      node = next;
    }
    static_cast<ITask&>(*this).DecRef();
  }

  Node* Mark() noexcept {
    return static_cast<Node*>(this);
  }

  IExecutorPtr _executor;
  std::atomic<Node*> _tasks{Mark()};
};

}  // namespace

IExecutorPtr MakeStrand(IExecutorPtr executor) {
  return MakeIntrusive<Strand, IExecutor>(std::move(executor));
}

}  // namespace yaclib
