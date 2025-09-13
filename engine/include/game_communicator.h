#pragma once

#include "communication/include/local_communicator.h"
#include "engine_export.h"

class engine_API game_communicator{
  public:
    game_communicator(local_communicator *comm);

    std::string ask_question(const std::string &question_data, size_t chan_idx);

  private:
    local_communicator *_Communication;
    
};