#include "collision_tree.h"
#include <entt/entity/registry.hpp>

namespace geometry {
  tree_node::tree_node(){
    Parent = entt::null;
    Child1 = entt::null;
    Child2 = entt::null;
    Owner = entt::null;
    Height = 0;
  }

  bool tree_node::is_leaf(){
    return Child1 == entt::null;
  }


  void create_proxy(entt::registry &registry, const aabb &tight_fitting_aabb, entt::entity owner){
    entt::entity node = registry.create();
    auto &node_data = registry.emplace<tree_node>(node);
    node_data.Owner = owner;
    node_data.AABB.Min = tight_fitting_aabb.Min - Vec2D<ftype>(0.1f, 0.1f);
    node_data.AABB.Max = tight_fitting_aabb.Max + Vec2D<ftype>(0.1f, 0.1f);  
    insert_leaf(registry, node);
  }

  void insert_leaf(entt::registry &registry, entt::entity leaf){
    auto &node_data = registry.get<tree_node>(leaf);
    auto root_view = registry.view<is_root>();
    if(root_view.empty()){
      registry.emplace<is_root>(leaf);
      node_data.Parent = entt::null;
      return;
    }
    tree_node &leaf_data = registry.get<tree_node>(leaf);

    entt::entity index = root_view.front(); //there can only be one root at a time
    tree_node &index_data = registry.get<tree_node>(index);
    while(!index_data.is_leaf()){
      entt::entity child1 = index_data.Child1;
      entt::entity child2 = index_data.Child2;

      ftype area = index_data.AABB.perimeter();
      aabb combinedAABB = combine(leaf_data.AABB, index_data.AABB);

      ftype combined_area = combinedAABB.perimeter();

      // Cost of creating a new parent for this node and the new leaf
      ftype cost = 2.f * combined_area;

      // Minimum cost of pushing the leaf further down the tree
		  ftype inheritance_cost = 2.0f * (combined_area - area);

      auto eval_child_cost = [&](tree_node &c_data) -> ftype {
        if(c_data.is_leaf()){
          aabb AABB = combine(leaf_data.AABB, c_data.AABB);
          return AABB.perimeter() + inheritance_cost;
        } else {
          aabb AABB = combine(leaf_data.AABB, c_data.AABB);
          ftype old_area = c_data.AABB.perimeter();
          ftype new_area = AABB.perimeter();
          return (new_area - old_area) + inheritance_cost;
        }
      };

      
      tree_node &c1_data = registry.get<tree_node>(index_data.Child1);
      ftype cost1 = eval_child_cost(c1_data);

      tree_node &c2_data = registry.get<tree_node>(index_data.Child2);
      ftype cost2 = eval_child_cost(c2_data);

      if(cost < cost1 && cost < cost2){
        break;
      }

      if(cost1 < cost2){
        index = child1;
      } else {
        index = child2;
      }

      index_data = registry.get<tree_node>(index);
    }

    entt::entity sibling = index;
    auto &sibling_data = registry.get<tree_node>(sibling);
    entt::entity old_parent = sibling_data.Parent;
    entt::entity new_parent = registry.create();
    auto &np_data = registry.emplace<tree_node>(new_parent);
    np_data.Parent = old_parent;
    np_data.AABB = combine(leaf_data.AABB, sibling_data.AABB);
    np_data.Height = sibling_data.Height + 1;

    if(old_parent != entt::null){
      //sibling was not the root
      auto &op_data = registry.get<tree_node>(old_parent);
      if(op_data.Child1 == sibling){
        op_data.Child1 = new_parent;
      } else {
        op_data.Child2 = new_parent;
      }
      
    } else {
      registry.remove<is_root>(sibling);
      registry.emplace<is_root>(new_parent);
    }

    np_data.Child1 = sibling;
    np_data.Child2 = leaf;
    sibling_data.Parent = new_parent;
    leaf_data.Parent = new_parent;

    index = new_parent;
    while(index != entt::null){
      index = balance(registry, index);
      auto &index_data = registry.get<tree_node>(index);

      entt::entity child1 = index_data.Child1; auto &c1_d = registry.get<tree_node>(child1);
      entt::entity child2 = index_data.Child2; auto &c2_d = registry.get<tree_node>(child2);

      index_data.Height = 1 + std::max(c1_d.Height, c2_d.Height);
      index_data.AABB = combine(c1_d.AABB, c2_d.AABB);

      index = index_data.Parent;
    }
  }
}
