#pragma once

#include "local_sync.h"
#include "comm_export.h"

class communication_API local_communicator {
  private:
    local_channel_container *Container{nullptr};

  public: 
    void set_container(local_channel_container *container);

    void wait_receiver_ready(size_t channel_id);
    void send(size_t channel_id);
    void receive(size_t channel_id);



};