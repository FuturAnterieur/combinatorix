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
    void move(const glm::vec2 &displacement);
  };

  aabb combine(const aabb &lhs, const aabb &rhs);

  struct geometry_API circle_collider {
    glm::vec2 Position;
    float Radius;
  };

  struct geometry_API aabb_collider {
    aabb AABB;
  };

  bool detect_circle_to_circle_collision(const circle_collider &a, const circle_collider &b);
  bool detect_circle_to_aabb_collision(const circle_collider &a, const aabb_collider &b);
  bool detect_aabb_to_aabb_collision(const aabb_collider &a, const aabb_collider &b);

  geometry_API aabb aabb_from_circle(const circle_collider &c);
}