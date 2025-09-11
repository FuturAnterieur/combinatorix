#pragma once

#include "geometry/include/position.h"

// intended to be a registry component

struct move_request_history {
  std::vector<glm::vec2> PendingRequests;
  
};