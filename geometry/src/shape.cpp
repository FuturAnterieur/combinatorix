#include "shape.h"

namespace geometry {
  float aabb::perimeter() const{
    return (Max.x - Min.x + Max.y - Min.y) * 2.f;
  }

  glm::vec2 aabb::center() const
  {
    return {(Max.x + Min.x) / 2.f, (Max.y + Min.y) / 2.f};
  }

  void aabb::move_of(const glm::vec2 &displacement)
  {
    Max += displacement;
    Min += displacement;
  }

  aabb combine(const aabb &lhs, const aabb &rhs){
    return aabb{min(lhs.Min, rhs.Min), max(lhs.Max, rhs.Max)};
  }

  bool detect_circle_to_circle_collision(const circle_with_position &a, const circle_with_position &b)
  {
    return (a.Position - b.Position).length() < (a.Radius + b.Radius);
  }

  
  bool detect_aabb_to_aabb_collision(const aabb &a, const aabb &b)
  {
    return (b.Min.x < a.Max.x && b.Max.x > a.Min.x
      && b.Min.y < a.Max.y && b.Max.y > a.Min.y);
  }

  aabb aabb_from_circle(const glm::vec2 &position, float radius)
  {
    return aabb{{position.x - radius, position.y - radius}, {position.x + radius, position.y + radius}};
  }

  bool detect_circle_to_aabb_collision(const circle_collider &a, const aabb_collider &b)
  {
    glm::vec2 aabb_center = b.RelativeAABB.center();
    return false;
  }
    
}
