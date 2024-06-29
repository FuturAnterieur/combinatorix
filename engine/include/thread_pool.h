//Thread pool implementation shamelessly taken from 
//https://nixiz.github.io/yazilim-notlari/2023/10/07/thread_pool-en 

#pragma once
#include "engine/include/engine_export.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <deque>
#include <vector>

class engine_API thread_pool
{
private:
  struct pimpl;
  pimpl *_Pimpl;

public:
  explicit thread_pool(int num_threads = 1);
  ~thread_pool();
  
  void post(std::packaged_task<void()> job);

private:
  void join();
  void run() noexcept;
};

template <class Executor, class Fn>
void post(Executor& exec, Fn&& func)
{
  using return_type = decltype(func());
  static_assert(std::is_void_v<return_type>, "posting functions with return types must be used with \"use_future\" tag.");
  std::packaged_task<void()> task(std::forward<Fn>(func));
  exec.post(std::move(task));
}

struct use_future_tag {};
template <class Fn>
constexpr auto use_future(Fn&& func) {
  return std::make_tuple(use_future_tag{}, std::forward<Fn>(func));
}

template<class Fn>
struct forwarder_t {
  using return_type = std::invoke_result_t<Fn>;
  forwarder_t(Fn&& fn) : tsk(std::forward<Fn>(fn)) {}
  void operator()(std::shared_ptr<std::promise<return_type>> promise) noexcept
  {
    promise->set_value(tsk());
  }
private:
  std::decay_t<Fn> tsk;
};

template <class Executor, class Fn>
[[nodiscard]] decltype(auto) post(Executor& exec, std::tuple<use_future_tag, Fn>&& tpl)
{
  using return_type = std::invoke_result_t<Fn>;
  auto&& [_, func] = tpl;
  if constexpr (std::is_void_v<return_type>) 
  {
    std::packaged_task<void()> tsk(std::move(func));
    auto ret_future = tsk.get_future();
    exec.post(std::move(tsk));
    return ret_future;
  }
  else
  {
    forwarder_t forwarder(std::forward<Fn>(func));
    auto promise = std::make_shared<std::promise<return_type>>();
    auto ret_future = promise->get_future();
    std::packaged_task<void()> tsk([promise = std::move(promise), forwarder = std::move(forwarder)] () mutable {
      forwarder(promise);
    });
    exec.post(std::move(tsk));
    return ret_future;
  }
}
