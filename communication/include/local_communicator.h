#pragma once

#include "local_sync.h"

class local_communicator {
  public: 
    void request(local_sync_channel &channel);
    void respond(local_sync_channel &channel);



};