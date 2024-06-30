#pragma once

#include "thread_pool/include/thread_pool.h"
#include "engine/include/engine.h"

struct engine::pimpl {
  pimpl(int num_threads) : ThreadPool(num_threads) {}
  std::atomic_bool IsRunning{false};
  thread_pool ThreadPool;
  entt::registry* Registry{nullptr};
};