#pragma once

#include "geometry_export.h"
#include "geometry/include/position.h"
#include "geometry/include/shape.h"
#include <entt/entity/fwd.hpp>
#include <optional>
#include <set>

namespace geometry {

  struct move_request {
    std::vector<entt::entity> Entities; //TODO to support moving groups of units without them collisioning into each other, support a set of entities here, all sharing the same delta.
    glm::vec2 Delta;      //but oh no! some might move faster than others -> only group those that have the same speed. So complicated!
  };

  using move_requests_container = std::vector<move_request>;

  class geometry_API collision_processor {
    public: 
      collision_processor(entt::registry *registry);
      bool is_move_allowed(entt::entity entity, const glm::vec2 &delta, const std::set<entt::entity> &excluded_entities);
      void move_entity(entt::entity entity, const glm::vec2 &delta);

      bool do_move(const move_request &req);

      bool aabb_collision_query(const aabb &absolute_aabb, entt::entity owner, const std::set<entt::entity> &excluded_entities);
      bool circle_collision_query(const glm::vec2 &position, float radius, entt::entity owner, const std::set<entt::entity> &excluded_entities);

      bool is_circle_colliding_with_entity(const glm::vec2 &position, float radius, entt::entity entity);
      bool is_aabb_colliding_with_entity(const aabb &absolute_aabb, entt::entity);

    private:
      entt::registry *Registry{nullptr};
  };
}


