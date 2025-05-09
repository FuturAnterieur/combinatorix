#include "collision_tree.h"
#include <entt/entity/registry.hpp>

namespace geometry {
  tree_node::tree_node(){
    Parent = entt::null;
    Child1 = entt::null;
    Child2 = entt::null;
    Height = 0;
  }

  tree_node::~tree_node() {

  }

  bool tree_node::is_leaf(){
    return Child1 == entt::null;
  }


  void create_proxy(entt::registry &registry, const aabb &tight_fitting_aabb, entt::entity owner){
    
    auto &node_data = registry.emplace<tree_node>(owner);
    node_data.AABB.Min = tight_fitting_aabb.Min - glm::vec2(0.1f, 0.1f);
    node_data.AABB.Max = tight_fitting_aabb.Max + glm::vec2(0.1f, 0.1f);  
    insert_leaf(registry, owner);
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
    tree_node index_data = registry.get<tree_node>(index);
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

    balancing_loop(registry, new_parent);
  }

  void remove_leaf(entt::registry &registry, entt::entity leaf){
    if(registry.any_of<is_root>(leaf)){
      registry.remove<is_root>(leaf);
      return;
    }

    auto &leaf_data = registry.get<tree_node>(leaf);
    auto &parent_data = registry.get<tree_node>(leaf_data.Parent);
    entt::entity sibling;
    if(parent_data.Child1 == leaf){
      sibling = parent_data.Child2;
    } else {
      sibling = parent_data.Child1;
    }

    if(parent_data.Parent != entt::null){
		  // Destroy parent and connect sibling to grandParent.
      auto &gp_data = registry.get<tree_node>(parent_data.Parent);
      if(gp_data.Child1 == leaf_data.Parent){
        gp_data.Child1 = sibling;
      } else {
        gp_data.Child2 = sibling;
      }
      auto &sib_data = registry.get<tree_node>(sibling);
      sib_data.Parent = parent_data.Parent;
      registry.remove<tree_node>(leaf_data.Parent);

      balancing_loop(registry, sib_data.Parent);

    } else {
      registry.remove<is_root>(leaf_data.Parent);
      registry.emplace<is_root>(sibling);
      auto &sib_data = registry.get<tree_node>(sibling);
      sib_data.Parent = entt::null;
      registry.remove<tree_node>(leaf_data.Parent);
    }

  }

  void balancing_loop(entt::registry &registry, entt::entity start_index){
    entt::entity index = start_index;
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

  // Perform a left or right rotation if node A is imbalanced.
  // Returns the new root index.
  entt::entity balance(entt::registry &registry, entt::entity a){
    auto &a_data = registry.get<tree_node>(a);
    if(a_data.is_leaf() || a_data.Height < 2){
      return a;
    }

    entt::entity b = a_data.Child1; auto &b_data = registry.get<tree_node>(b);
    entt::entity c = a_data.Child2; auto &c_data = registry.get<tree_node>(c);

    int balance = c_data.Height - b_data.Height;

    auto swap_a_with = [&](entt::entity x, tree_node &x_data){
      x_data.Child1 = a;
      x_data.Parent = a_data.Parent;
      a_data.Parent = x;
      

      if(x_data.Parent != entt::null){
        auto &xp_data = registry.get<tree_node>(x_data.Parent);
        if(xp_data.Child1 == a){
          xp_data.Child1 = x;
        } else {
          assert(xp_data.Child2 == a);
          xp_data.Child2 = x;
        }
      } else {
        registry.remove<is_root>(a);
        registry.emplace<is_root>(x);
      }
    };

    auto rotate = [&](entt::entity x, tree_node &x_data, entt::entity y, tree_node &y_data){

      entt::entity w = x_data.Child1; auto &w_data = registry.get<tree_node>(w);
      entt::entity z = x_data.Child2; auto &z_data = registry.get<tree_node>(z);

      auto rotate_one_side = [&](entt::entity side, entt::entity other){
        auto &side_data = registry.get<tree_node>(side);
        auto &other_data = registry.get<tree_node>(other);

        x_data.Child2 = side;
        a_data.Child2 = other;
        other_data.Parent = a;
        a_data.AABB = combine(y_data.AABB, other_data.AABB);
        x_data.AABB = combine(a_data.AABB, side_data.AABB);

        a_data.Height = 1 + std::max(y_data.Height, other_data.Height);
        x_data.Height = 1 + std::max(a_data.Height, side_data.Height);
      };

      if(w_data.Height > z_data.Height){
        rotate_one_side(w, z);
      } else {
        rotate_one_side(z, w);
      }
    };

    if(balance > 1){
      swap_a_with(c, c_data);
      rotate(c, c_data, b, b_data);
      return c;
    } else if (balance < -1) {
      swap_a_with(b, b_data);
      rotate(b, b_data, c, c_data);
      return b;
    }
    return a;
  }
}
