#pragma once

#include <mutex>
#include <condition_variable>
#include <string>

struct local_sync_channel {
  std::mutex Mutex;
  std::condition_variable cvAnswerer;
  std::condition_variable cvRequester;
  bool InfoReady{false};
  std::string Data; //Here, or elsewhere?????
};