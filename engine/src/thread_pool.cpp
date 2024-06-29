#include "thread_pool.h"

struct thread_pool::pimpl {
  std::atomic_bool is_active;
  std::vector<std::thread> pool;
  std::condition_variable cv;
  std::mutex guard;
  std::deque<std::packaged_task<void()>> pending_jobs;
  size_t num_threads;
};

thread_pool::thread_pool(int num_threads) : _Pimpl(new pimpl)
{
  _Pimpl->num_threads = num_threads;
  _Pimpl->pool.reserve(num_threads);
}

void thread_pool::launch() {
  if(_Pimpl->is_active){
    return;
  }
  _Pimpl->is_active = true;
  for (int i = 0; i < _Pimpl->num_threads; i++) {
    _Pimpl->pool.emplace_back(&thread_pool::run, this);
  }
}

thread_pool::~thread_pool() {
  abort();
  
  delete _Pimpl;
}

void thread_pool::abort() {
  if(_Pimpl->is_active){
    _Pimpl->is_active = false;
    _Pimpl->cv.notify_all();
    join();
  }
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