#pragma once

#include "geometry_export.h"
#include "geometry/include/position.h"
#include "geometry/include/shape.h"
#include <entt/entity/fwd.hpp>
#include <optional>

namespace geometry {

  struct move_request {
    entt::entity Entity;
    // float Velocity; //would get it from the entity's move speed, which is a parameter
    glm::vec2 Delta;
  };

  using move_requests_container = std::vector<move_request>;

  class geometry_API collision_processor {
    public: 
      collision_processor(entt::registry *registry);
      bool is_move_allowed(const move_request &req);

      bool do_move(const move_request &req);

      bool aabb_collision_query(const aabb &absolute_aabb);
      bool circle_collision_query(const glm::vec2 &position, float radius);


      //void process_move_requests(move_requests_container &req, float duration, float delta_time);
    private:
      entt::registry *Registry{nullptr};
  };
}


