#pragma once

#include "Vec2D.h"
namespace geometry{
  struct aabb {
    Vec2D<ftype> Min;
    Vec2D<ftype> Max;

    ftype perimeter() const;
  };

  aabb combine(const aabb &lhs, const aabb &rhs);
}
