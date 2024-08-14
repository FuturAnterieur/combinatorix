#pragma once

#include "geometry/include/position.h"
#include <entt/entity/fwd.hpp>

namespace geometry {
  struct move_request {
    entt::entity Entity;
    position Destination;
  };
}
