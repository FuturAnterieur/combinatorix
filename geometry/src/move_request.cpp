#include "move_request.h"
#include "position.h"
#include "velocity.h"
#include "collision_tree.h"

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>

namespace geometry{



  move_request_processor::move_request_processor(entt::registry *registry)
  {
    Registry = registry;
  }

  bool move_request_processor::is_move_allowed(const move_request &req) {

    if (Registry->any_of<aabb_collider>(req.Entity)){
      aabb aabb_ = Registry->get<aabb_collider>(req.Entity).AABB;
      glm::vec2 projected_delta = req.Destination.Value - aabb_.center();
      aabb_.Min += projected_delta;
      aabb_.Max += projected_delta;

      return !query(*Registry, aabb_);
    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      //TODO aabb construction from circle
    }

    return false;
  }
}