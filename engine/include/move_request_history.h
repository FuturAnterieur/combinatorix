#pragma once

#include "geometry/include/position.h"
#include "geometry/include/collision_processor.h"

// intended to be a registry component

struct move_request_history {
  std::vector<geometry::move_request> PendingRequests;
};