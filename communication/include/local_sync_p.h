#pragma once

#include <mutex>
#include <condition_variable>
#include <string>

struct local_sync_channel {
  local_sync_channel() = default;
  ~local_sync_channel() = default;

  //local_sync_channel(local_sync_channel const&) noexcept;

  /*local_sync_channel(local_sync_channel&& other) = default;
  local_sync_channel& operator=(local_sync_channel&& other) = default;*/

  std::mutex Mutex{};
  std::condition_variable cvWaiting{};
  std::condition_variable cvReceiverReady{};
  bool ReceiverReady{false};
  bool InfoReady{false};
  std::string Data{}; //Here, or elsewhere?????
};