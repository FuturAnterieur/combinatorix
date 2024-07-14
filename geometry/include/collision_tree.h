#pragma once

#include "geometry_export.h"
#include "aabb.h"
#include <entt/entity/fwd.hpp>

namespace geometry {
  struct geometry_API tree_node {
    tree_node();
    ~tree_node();

    aabb AABB; //enlarged, following Box2D

    entt::entity Parent;
    entt::entity Child1;
    entt::entity Child2;

    int Height;
    bool is_leaf();
  };

  struct is_root {};

  void geometry_API create_proxy(entt::registry &registry, const aabb &tight_fitting_aabb, entt::entity owner);

  void insert_leaf(entt::registry &registry, entt::entity leaf);
  entt::entity balance(entt::registry &registry, entt::entity node);
}
