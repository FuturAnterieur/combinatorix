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

  bool collision_processor::is_move_allowed(entt::entity entity, const glm::vec2 &delta, const std::set<entt::entity> &excluded_entities) {

    glm::vec2 projected_pos = Registry->get<position>(entity).Value + delta;
    if (Registry->any_of<aabb_collider>(entity)){
      aabb aabb_ = Registry->get<aabb_collider>(entity).RelativeAABB;
      
      aabb_.move_of(projected_pos);
      return !aabb_collision_query(aabb_, entity, excluded_entities);

    } else if (Registry->any_of<circle_collider>(entity)) {
      circle_collider collider = Registry->get<circle_collider>(entity);
      return !circle_collision_query(projected_pos, collider.Radius, entity, excluded_entities);
    }

    return false;
  }

  void collision_processor::move_entity(entt::entity entity, const glm::vec2 &delta)
  {
    glm::vec2 &position_ = Registry->get<position>(entity).Value;
    position_ += delta;

    if (Registry->any_of<aabb_collider>(entity)){
      aabb aabb_ = Registry->get<aabb_collider>(entity).RelativeAABB;
      aabb_.move_of(position_);
      move_proxy(*Registry, aabb_, entity);
      
    } else if (Registry->any_of<circle_collider>(entity)) {
      circle_collider collider = Registry->get<circle_collider>(entity);
      move_proxy(*Registry, aabb_from_circle(position_, collider.Radius), entity);
    }
  }

  bool collision_processor::do_move(const move_request &req)
  {
    std::set<entt::entity> entities_excluded_from_queries;
    entities_excluded_from_queries.insert(req.Entities.begin(), req.Entities.end());

    float ratio = 0.f;
    bool move_allowed = true;
    while (move_allowed && ratio < 1.f) {
      bool ok_this_round = true;
      for (size_t i = 0; i < req.Entities.size(); i++) {
        if (!is_move_allowed(req.Entities[i], req.Delta * ratio, entities_excluded_from_queries)) {
          ok_this_round = false;
          break;
        }
      }
      move_allowed = ok_this_round;
      if (move_allowed) {
        ratio += 0.1f;
      } else {
        ratio -= 0.1f;
      }
    }

    for (const auto entity : req.Entities) {
      move_entity(entity, req.Delta * ratio);
    }
  
    return true;
  }

  bool collision_processor::aabb_collision_query(const aabb &absolute_aabb, entt::entity owner, const std::set<entt::entity> &excluded_entities)
  {
    entt::entity colliding = query(*Registry, absolute_aabb, owner, excluded_entities);
    return colliding != entt::null && colliding != owner && is_aabb_colliding_with_entity(absolute_aabb, colliding);
  }

  bool collision_processor::circle_collision_query(const glm::vec2 &position, float radius, entt::entity owner, const std::set<entt::entity> &excluded_entities) {
    entt::entity colliding = query(*Registry, aabb_from_circle(position, radius), owner, excluded_entities);
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
