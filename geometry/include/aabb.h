#pragma once

#include "geometry_export.h"
#include "Vec2D.h"
namespace geometry{
  struct geometry_API aabb {
    Vec2D<ftype> Min;
    Vec2D<ftype> Max;

    ftype perimeter() const;
  };

  aabb combine(const aabb &lhs, const aabb &rhs);
}
