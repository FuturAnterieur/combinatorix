#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <entt/entity/registry.hpp>
#include <entt/entity/runtime_view.hpp>

struct has_name {
  std::string Name;
};


TEST_CASE("Storage sanity"){
  entt::registry registry;
  
  const entt::entity mermelionos = registry.create();
  registry.emplace<has_name>(mermelionos, "Mermelionos");
  const entt::entity taliesin = registry.create();
  registry.emplace<has_name>(taliesin, "Taliesin");

  auto &&wizzy_storage = registry.storage<void>(entt::hashed_string::value("wizard"));
  wizzy_storage.emplace(mermelionos);
  wizzy_storage.emplace(taliesin);

  auto &&roby_storage = registry.storage<void>(entt::hashed_string::value("has_robe"));
  roby_storage.emplace(mermelionos);
  
  entt::runtime_view rt_view{};
  rt_view.iterate(wizzy_storage).iterate(roby_storage).iterate(registry.storage<has_name>());
  
  std::vector<std::string> result;
  for(auto entity : rt_view){
    has_name &hn = registry.get<has_name>(entity);
    result.push_back(hn.Name);
  }
  CHECK(result.size() == 1);
  CHECK(result[0] == "Mermelionos");
  
  auto yes_view = entt::view<entt::get_t<void>>{wizzy_storage};
  auto yes_view_2 = entt::view<entt::get_t<has_name>>(registry.storage<has_name>());
  auto no_view = entt::view<entt::get_t<>, entt::exclude_t<void>>{roby_storage};

  result.clear();
  for(auto entity : yes_view | yes_view_2 | no_view){
    has_name &hn = registry.get<has_name>(entity);
    result.push_back(hn.Name);
  }
  CHECK(result.size() == 1);
  CHECK(result[0] == "Taliesin");
}