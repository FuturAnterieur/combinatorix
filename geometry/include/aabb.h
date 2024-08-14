#pragma once


#include "geometry_export.h"
#include <glm/glm.hpp>

using ftype = float;

namespace geometry{
  struct geometry_API aabb {
    glm::vec2 Min;
    glm::vec2 Max;

    float perimeter() const;
  };

  aabb combine(const aabb &lhs, const aabb &rhs);
}
