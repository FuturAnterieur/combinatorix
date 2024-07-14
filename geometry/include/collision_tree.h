#pragma once

#include "aabb.h"
#include <entt/entity/fwd.hpp>

namespace geometry {
  struct tree_node {
    tree_node();
    ~tree_node();

    aabb AABB; //enlarged, following Box2D

    entt::entity Parent;
    entt::entity Child1;
    entt::entity Child2;

    entt::entity Owner; //probably an entity - game object - with a collider component

    int Height;
    bool is_leaf();
  };

  struct is_root {};

  void create_proxy(entt::registry &registry, const aabb &tight_fitting_aabb, entt::entity owner);

  void insert_leaf(entt::registry &registry, entt::entity leaf);
}
