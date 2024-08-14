#pragma once

#include "geometry/include/position.h"
#include <entt/entity/fwd.hpp>
#include <optional>

namespace geometry {

  struct move_request {
    entt::entity Entity;
    float Velocity; //would get it from the entity's move speed, which is a parameter
    position Destination;
  };

  using move_requests_container = std::vector<move_request>;

  class move_request_processor {
    public: 
      move_request_processor(entt::registry *registry);
      void process_move_requests(move_requests_container &req, float duration, float delta_time);
    private:
      entt::registry *Registry{nullptr};
  };
}


