#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "collision_tree.h"
#include <entt/entity/registry.hpp>

TEST_CASE("debug to see tree building"){
  using namespace geometry;
  entt::registry registry;

  aabb AABB1, AABB2, AABB3;
  AABB1.Min = glm::vec2(0.0f, 0.0f);
  AABB1.Max = glm::vec2(10.f, 10.f);

  AABB2.Min.x = 15.f; AABB2.Min.y = 15.f;
  AABB2.Max.x = 20.f; AABB2.Max.y = 20.f;

  AABB3.Min.x = 12.f; AABB3.Min.y = 12.f; //between 1 and 2
  AABB3.Max.x = 13.f; AABB3.Max.y = 13.f;
  
  entt::entity ent1 = registry.create();
  entt::entity ent2 = registry.create();
  entt::entity ent3 = registry.create();

  create_proxy(registry, AABB1, ent1);
  create_proxy(registry, AABB2, ent2);
  create_proxy(registry, AABB3, ent3);

  auto view = registry.view<tree_node>();

  std::vector<tree_node> all_tree_nodes;
  for(auto ent : view){
    auto& data = view.get<tree_node>(ent);
    all_tree_nodes.push_back(data);
  }

}