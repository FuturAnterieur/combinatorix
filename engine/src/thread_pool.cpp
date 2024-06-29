#include "thread_pool.h"

struct thread_pool::pimpl {
  std::atomic_bool is_active {true};
  std::vector<std::thread> pool;
  std::condition_variable cv;
  std::mutex guard;
  std::deque<std::packaged_task<void()>> pending_jobs;
};

thread_pool::thread_pool(int num_threads) : _Pimpl(new pimpl)
{
  for (int i = 0; i < num_threads; i++) {
    _Pimpl->pool.emplace_back(&thread_pool::run, this);
  }
}

thread_pool::~thread_pool() {
  _Pimpl->is_active = false;
  _Pimpl->cv.notify_all();
  join();
  delete _Pimpl;
}

void thread_pool::post(std::packaged_task<void()> job) {
  std::unique_lock lock(_Pimpl->guard);
  _Pimpl->pending_jobs.emplace_back(std::move(job));
  _Pimpl->cv.notify_one();
}

void thread_pool::join() {
  for (auto& th : _Pimpl->pool) {
    th.join();
  }
}

void thread_pool::run() noexcept 
{
  while (_Pimpl->is_active)
  {
    thread_local std::packaged_task<void()> job;
    {
      std::unique_lock lock(_Pimpl->guard);
      _Pimpl->cv.wait(lock, [&]{ return !_Pimpl->pending_jobs.empty() || !_Pimpl->is_active; });
      if (!_Pimpl->is_active) break;
      job.swap(_Pimpl->pending_jobs.front());
      _Pimpl->pending_jobs.pop_front();
    }
    job();
  }
}