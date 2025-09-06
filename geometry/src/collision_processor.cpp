#include "collision_processor.h"
#include "position.h"
#include "velocity.h"
#include "collision_tree.h"

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include "collision_processor.h"

namespace geometry{



  collision_processor::collision_processor(entt::registry *registry)
  {
    Registry = registry;
  }

  bool collision_processor::is_move_allowed(const move_request &req) {

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

  //TODO for now we're only checking broad-phase AABBs, need to check for shapes themselves too!
  bool collision_processor::aabb_collision_query(const aabb_collider &collider)
  {
    return query(*Registry, collider.AABB);
  }

  bool collision_processor::circle_collision_query(const circle_collider &collider) {
    return query(*Registry, aabb_from_circle(collider));
  }
}

