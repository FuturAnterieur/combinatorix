#pragma once

#include "geometry_export.h"
#include "shape.h"
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
  void geometry_API move_proxy(entt::registry &registry, const aabb &new_tight_fitting_aabb, entt::entity owner);
  void geometry_API destroy_proxy(entt::registry &registry, entt::entity owner);
  // In box 2d, tree queries take a callback for further testing in case of AABB match on a leaf
  entt::entity geometry_API query(entt::registry &registry, const aabb &aabb_, entt::entity owner);

  void insert_leaf(entt::registry &registry, entt::entity leaf);
  void remove_leaf(entt::registry &registry, entt::entity leaf);

  void balancing_loop(entt::registry &registry, entt::entity start_index);
  entt::entity balance(entt::registry &registry, entt::entity node);
}
