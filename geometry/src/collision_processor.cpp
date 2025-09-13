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
      return !aabb_collision_query(aabb_, req.Entity);

    } else if (Registry->any_of<circle_collider>(req.Entity)) {
      circle_collider collider = Registry->get<circle_collider>(req.Entity);
      return !circle_collision_query(projected_pos, collider.Radius, req.Entity);
    }

    return false;
  }

  bool collision_processor::do_move(const move_request &req)
  {
    auto &starting_position = Registry->get<position>(req.Entity);
    if (Registry->any_of<aabb_collider>(req.Entity)){
      aabb initial_aabb_ = Registry->get<aabb_collider>(req.Entity).RelativeAABB;
      glm::vec2 projected_delta = req.Delta;
      initial_aabb_.move_of(starting_position.Value);
      aabb aabb_ = initial_aabb_;
      aabb_.move_of(projected_delta);
      float subtracted_ratio = 0.9f;
      //TODO one day, use proper collision resolution algorithm
      // use smth like signed 2D distance between both objects
      // BUT we might have more than one collision object to check
      while(aabb_collision_query(aabb_, req.Entity) && subtracted_ratio >= 0.0f){
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
      const circle_collider &collider = Registry->get<circle_collider>(req.Entity);
      glm::vec2 projected_delta = req.Delta;
      float subtracted_ratio = 1.0f;
      while(circle_collision_query(starting_position.Value + projected_delta * subtracted_ratio, collider.Radius, req.Entity) && subtracted_ratio > 0.0f) {
        subtracted_ratio -= 0.1f;
      }
      starting_position.Value += projected_delta;
      move_proxy(*Registry, aabb_from_circle(starting_position.Value, collider.Radius), req.Entity);
    }
    return true;
  }

  //TODO for now we're only checking broad-phase AABBs, need to check for shapes themselves too!
  bool collision_processor::aabb_collision_query(const aabb &absolute_aabb, entt::entity owner)
  {
    entt::entity colliding = query(*Registry, absolute_aabb);
    return colliding != entt::null && colliding != owner && is_aabb_colliding_with_entity(absolute_aabb, colliding);
  }

  bool collision_processor::circle_collision_query(const glm::vec2 &position, float radius, entt::entity owner) {
    entt::entity colliding = query(*Registry, aabb_from_circle(position, radius));
    return colliding != entt::null && colliding != owner && is_circle_colliding_with_entity(position, radius, colliding);
  }

  bool collision_processor::is_circle_colliding_with_entity(const glm::vec2 &position, float radius, entt::entity entity)
  {
    geometry::position other_pos = Registry->get<geometry::position>(entity);
    if (Registry->any_of<aabb_collider>(entity)){
      aabb absolute_aabb = Registry->get<aabb_collider>(entity).RelativeAABB;
      absolute_aabb.move_of(other_pos.Value);
      return detect_circle_to_aabb_collision(circle_with_position{position, radius}, absolute_aabb);
    } else if (Registry->any_of<circle_collider>(entity)) {
      const circle_collider &collider = Registry->get<circle_collider>(entity);
      return detect_circle_to_circle_collision(circle_with_position{position, radius}, circle_with_position{other_pos.Value, collider.Radius});
    }
    return false;
  }

  bool collision_processor::is_aabb_colliding_with_entity(const aabb &absolute_aabb_, entt::entity entity)
  {
    geometry::position other_pos = Registry->get<geometry::position>(entity);
    if (Registry->any_of<aabb_collider>(entity)){
      aabb absolute_aabb = Registry->get<aabb_collider>(entity).RelativeAABB;
      absolute_aabb.move_of(other_pos.Value);
      return detect_aabb_to_aabb_collision(absolute_aabb_, absolute_aabb);
    } else if (Registry->any_of<circle_collider>(entity)) {
      const circle_collider &collider = Registry->get<circle_collider>(entity);
      return detect_circle_to_aabb_collision(circle_with_position{other_pos.Value, collider.Radius}, absolute_aabb_);
    }
    return false;
  }
}
