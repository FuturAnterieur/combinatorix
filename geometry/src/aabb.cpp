#include "aabb.h"

namespace geometry {
  float aabb::perimeter() const{
    return (Max.x - Min.x + Max.y - Min.y) * 2.f;
  }

  aabb combine(const aabb &lhs, const aabb &rhs){
    return aabb{min(lhs.Min, rhs.Min), max(lhs.Max, rhs.Max)};
  }
}
