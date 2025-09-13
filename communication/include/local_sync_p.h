#pragma once

#include <mutex>
#include <condition_variable>
#include <string>
#include <deque>

struct local_sync_channel {
  local_sync_channel() = default;
  ~local_sync_channel() = default;

  std::mutex Mutex{};
  std::condition_variable cvWaiting{};
  
  std::deque<std::string> Queue; //TODO if we want channels to be bidir, two queues are needed
  int LastMessageCommIndex{-1};
};