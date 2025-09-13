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

  glm::vec2 circle_to_circle_d(const circle_with_position &a, const circle_with_position &b)
  {
    float length = (a.Position - b.Position).length() - (a.Radius + b.Radius);
    return length * glm::normalize(a.Position - b.Position);
  }

  float circle_to_aabb_signed_distance(const circle_with_position &a, const aabb &b)
  {
    // Formula taken from https://iquilezles.org/articles/distfunctions2d/
    glm::vec2 aabb_center = b.center();
    glm::vec2 virtual_box = b.Max - aabb_center;
    glm::vec2 virtual_p = a.Position - aabb_center;

    glm::vec2 d = glm::abs(virtual_p) - virtual_box;
    return glm::length(glm::max(d, 0.0f)) + glm::min(glm::max(d.x, d.y), 0.f);
  }

  glm::vec2 circle_to_aabb_d(const circle_with_position &a, const aabb &b)
  {
    //TODO proper 1-component vector unless at corner
    glm::vec2 aabb_center = b.center();
    glm::vec2 virtual_box = b.Max - aabb_center;
    glm::vec2 virtual_p = a.Position - aabb_center;

    return virtual_p - virtual_box;
  }

  bool detect_circle_to_circle_collision(const circle_with_position &a, const circle_with_position &b)
  {
    return (a.Position - b.Position).length() < (a.Radius + b.Radius);
  }

  // supposing b has a center that may differ from 0
  bool detect_circle_to_aabb_collision(const circle_with_position &a, const aabb &b)
  {
    return circle_to_aabb_signed_distance(a, b) < a.Radius;
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
}
