#pragma once


#pragma once

#include "geometry_export.h"
#include <glm/glm.hpp>
#include <entt/entity/fwd.hpp>

using ftype = float;

namespace geometry{

  struct geometry_API aabb {
    glm::vec2 Min;
    glm::vec2 Max;

    float perimeter() const;
    glm::vec2 center() const;
    void move_of(const glm::vec2 &displacement);
  };

  aabb combine(const aabb &lhs, const aabb &rhs);

  struct geometry_API circle_collider {
    float Radius;
  };

  struct geometry_API aabb_collider {
    aabb RelativeAABB;
  };


  struct circle_with_position {
    glm::vec2 Position;
    float Radius;
  };

  glm::vec2 circle_to_circle_d(const circle_with_position &a, const circle_with_position &b);
  float circle_to_aabb_signed_distance(const circle_with_position &a, const aabb &b);
  glm::vec2 circle_to_aabb_d(const circle_with_position &a, const aabb &b);
  glm::vec2 aabb_to_aabb_d(const aabb &a, const aabb &b);

  bool detect_circle_to_circle_collision(const circle_with_position &a, const circle_with_position &b);
  bool detect_circle_to_aabb_collision(const circle_with_position &a, const aabb &b);
  bool detect_aabb_to_aabb_collision(const aabb &a, const aabb &b);

  geometry_API aabb aabb_from_circle(const glm::vec2 &pos, float radius);
}