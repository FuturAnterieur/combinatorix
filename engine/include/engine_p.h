#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <entt/entity/registry.hpp>

struct engine_pimpl {
  std::thread EngineThread;
  std::mutex EngineMutex;
  std::condition_variable CVServer;
  std::condition_variable CVClient;
  entt::registry Registry;
};