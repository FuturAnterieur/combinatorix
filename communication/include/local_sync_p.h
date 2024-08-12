#pragma once

#include <mutex>
#include <condition_variable>
#include <string>

struct local_sync_channel {
  local_sync_channel() = default;
  ~local_sync_channel() = default;

  std::mutex Mutex{};
  std::condition_variable cvWaiting{};
  
  bool InfoReady{false};
  std::string Data{};
  int LastMessageCommIndex{-1};
};