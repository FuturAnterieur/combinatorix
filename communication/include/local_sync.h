#pragma once

#include "comm_export.h"

struct local_sync_channel;
class communication_API local_channel_container{
  public:

    local_channel_container();
    ~local_channel_container();

    size_t add_channel();
    
  private:
    friend class local_communicator;
    friend class queue_on_receive_communicator;
    local_sync_channel *get_channel(size_t index);

    struct pimpl;
    pimpl *_Pimpl;
};