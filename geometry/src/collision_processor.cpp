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

    glm::vec2 projected_pos = Registry->get<position>(req.Entity).Value + req.Delta;
    if (Registry->any_of<aabb_collider>(req.Entity)){
      aabb aabb_ = Registry->get<aabb_collider>(req.Entity).RelativeAABB;
      
      aabb_.move_of(projected_pos);
      return !aabb_collision_query(aabb_);

    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      circle_collider collider = Registry->get<circle_collider>(req.Entity);
      return !circle_collision_query(projected_pos, collider.Radius);
    }

    return false;
  }

  bool collision_processor::do_move(const move_request &req)
  {
    auto &starting_position = Registry->get<position>(req.Entity);
    if (Registry->any_of<aabb_collider>(req.Entity)){
      aabb initial_aabb_ = Registry->get<aabb_collider>(req.Entity).RelativeAABB;
      initial_aabb_.move_of(starting_position.Value);
      aabb aabb_ = initial_aabb_;
      glm::vec2 projected_delta = req.Delta;
      float subtracted_ratio = 0.9f;
      //TODO one day, use proper collision resolution algorithm
      while(aabb_collision_query(aabb_) && subtracted_ratio >= 0.0f){
        projected_delta = projected_delta * subtracted_ratio;
        aabb_ = initial_aabb_;
        aabb_.move_of(projected_delta);
        subtracted_ratio -= 0.1f;
      }
      starting_position.Value += projected_delta;
      aabb final_absolute_aabb = Registry->get<aabb_collider>(req.Entity).RelativeAABB;
      final_absolute_aabb.move_of(starting_position.Value);

      move_proxy(*Registry, final_absolute_aabb, req.Entity);

    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      // TODO do it
      // circle_collider collider = Registry->get<circle_collider>(req.Entity);
      // collider.Position = req.Destination.Value;

      // return circle_collision_query(collider);
    }
    return true;
  }

  //TODO for now we're only checking broad-phase AABBs, need to check for shapes themselves too!
  bool collision_processor::aabb_collision_query(const aabb &absolute_aabb)
  {
    return query(*Registry, absolute_aabb) != entt::null;
  }

  bool collision_processor::circle_collision_query(const glm::vec2 &position, float radius) {
    return query(*Registry, aabb_from_circle(position, radius)) != entt::null;
  }
}

