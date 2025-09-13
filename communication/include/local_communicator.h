#pragma once

#include "local_sync.h"
#include "comm_export.h"
#include <string>
#include <deque>

class communication_API local_communicator {
  private:
    local_channel_container *Container{nullptr};
    int Index;

  public: 
    local_communicator(int index);
    void set_container(local_channel_container *container);

    void send(size_t channel_id, const std::string &data);
    void receive_blocking(size_t channel_id, std::string &data);
    void receive_non_blocking(size_t channel_id, std::deque<std::string> &data);
};