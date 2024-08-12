#pragma once

#include <mutex>
#include <condition_variable>
#include <string>

struct local_sync_channel {
  local_sync_channel() = default;
  ~local_sync_channel() = default;

  std::mutex Mutex{};
  std::condition_variable cvWaiting{};
  std::condition_variable cvSendCounter{};
  std::condition_variable cvReceiverAck{};

  bool InfoReady{false};
  bool ReceiverAck{false};
  std::string Data{};
  size_t SendCount{0};
  size_t ReceiveCount{0};
};