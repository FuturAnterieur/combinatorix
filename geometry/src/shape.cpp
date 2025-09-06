#include "shape.h"

namespace geometry {
  float aabb::perimeter() const{
    return (Max.x - Min.x + Max.y - Min.y) * 2.f;
  }

  glm::vec2 aabb::center() const
  {
    return {(Max.x + Min.x) / 2.f, (Max.y + Min.y) / 2.f};
  }

  aabb combine(const aabb &lhs, const aabb &rhs){
    return aabb{min(lhs.Min, rhs.Min), max(lhs.Max, rhs.Max)};
  }

  bool detect_circle_to_circle_collision(const circle_collider &a, const circle_collider &b)
  {
    return (a.Position - b.Position).length() < (a.Radius + b.Radius);
  }

  
  bool detect_aabb_to_aabb_collision(const aabb_collider &a, const aabb_collider &b)
  {
    return (b.AABB.Min.x < a.AABB.Max.x && b.AABB.Max.x > a.AABB.Min.x
      && b.AABB.Min.y < a.AABB.Max.y && b.AABB.Max.y > a.AABB.Min.y);
  }

  aabb aabb_from_circle(const circle_collider &c)
  {
    return aabb{{c.Position.x - c.Radius, c.Position.y - c.Radius}, {c.Position.x + c.Radius, c.Position.y + c.Radius}};
  }

  bool detect_circle_to_aabb_collision(const circle_collider &a, const aabb_collider &b)
  {
    glm::vec2 aabb_center = b.AABB.center();
    return false;
  }
    
}
