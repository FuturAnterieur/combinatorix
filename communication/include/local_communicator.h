#pragma once

#include "local_sync.h"
#include "comm_export.h"
#include <string>
#include <deque>

class communication_API blocking_on_receive_communicator {
  private:
    local_channel_container *Container{nullptr};
    int Index;

  public: 
    blocking_on_receive_communicator(int index);
    void set_container(local_channel_container *container);

    void send(size_t channel_id, const std::string &data);
    void receive(size_t channel_id, std::string &data);
};

// todo replace with strategy pattern for send and receive functions
class communication_API queue_on_receive_communicator {
  private:
    local_channel_container *Container{nullptr};
    int Index;

  public: 
    queue_on_receive_communicator(int index);
    void set_container(local_channel_container *container);

    void send(size_t channel_id, const std::string &data);
    void receive(size_t channel_id, std::deque<std::string> &data);
};