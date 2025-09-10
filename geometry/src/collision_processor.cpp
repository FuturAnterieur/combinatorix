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
      return !aabb_collision_query(aabb_collider{aabb_});

    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      circle_collider collider = Registry->get<circle_collider>(req.Entity);
      collider.Position = req.Destination.Value;

      return !circle_collision_query(collider);
    }

    return false;
  }

  bool collision_processor::do_move(const move_request &req)
  {
    if (Registry->any_of<aabb_collider>(req.Entity)){
      aabb initial_aabb_ = Registry->get<aabb_collider>(req.Entity).AABB;
      aabb aabb_ = initial_aabb_;
      glm::vec2 projected_delta = req.Destination.Value - aabb_.center();
      aabb_.move(projected_delta);
      float subtracted_ratio = 0.9f;
      while(aabb_collision_query(aabb_collider{aabb_}) && subtracted_ratio >= 0.0f){
        projected_delta = projected_delta * subtracted_ratio;
        aabb_ = initial_aabb_;
        aabb_.move(projected_delta);
        subtracted_ratio -= 0.1f;
      }
      aabb &final_aabb =  Registry->get<aabb_collider>(req.Entity).AABB;
      final_aabb.move(projected_delta);
      move_proxy(*Registry, final_aabb, req.Entity);

    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      // TODO do it
      // circle_collider collider = Registry->get<circle_collider>(req.Entity);
      // collider.Position = req.Destination.Value;

      // return circle_collision_query(collider);
    }
    return true;
  }

  //TODO for now we're only checking broad-phase AABBs, need to check for shapes themselves too!
  bool collision_processor::aabb_collision_query(const aabb_collider &collider)
  {
    return query(*Registry, collider.AABB) != entt::null;
  }

  bool collision_processor::circle_collision_query(const circle_collider &collider) {
    return query(*Registry, aabb_from_circle(collider)) != entt::null;
  }
}

