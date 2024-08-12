#pragma once

#include "local_sync.h"
#include "comm_export.h"
#include <string>

class communication_API local_communicator {
  private:
    local_channel_container *Container{nullptr};

  public: 
    void set_container(local_channel_container *container);

    void send(size_t channel_id, const std::string &data, bool wait_before = true, bool wait_after = false);
    void receive(size_t channel_id, std::string &data);
};