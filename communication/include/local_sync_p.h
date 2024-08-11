#pragma once

#include <mutex>
#include <condition_variable>
#include <string>

struct local_sync_channel {
  local_sync_channel() = default;
  ~local_sync_channel() = default;

  std::mutex Mutex{};
  std::condition_variable cvWaiting{};
  std::condition_variable cvReceiverAck{};
  std::condition_variable cvSendCounter{};
  bool ReceiverAck{false};
  bool InfoReady{false};
  std::string Data{}; //Here, or elsewhere?????
  size_t SendCount{0};
  size_t ReceiveCount{0};
};