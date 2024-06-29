#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <set>
#include <string>

struct has_name {
  std::string Name;
};

struct has_types {
  std::set<std::string> Types;
};

struct scene_3d_manager {
  int InternalData{420};

  scene_3d_manager(entt::registry *registry, int data) {
    registry->on_update<has_name>(entt::hashed_string::value("marker:request_update")).connect<&scene_3d_manager::on_marker_request_updated>(*this);
    InternalData = data;
  }

  void on_marker_request_updated(entt::registry &registry, entt::entity entity){
    std::cout << "Manager #" << InternalData << " updating a marker for entity " << entt::to_integral(entity)  << "\n";
    auto &&marker_update_storage = registry.storage<has_name>(entt::hashed_string::value("marker:request_update"));
    const has_name &updated_name = marker_update_storage.get(entity);
    //Notice : do not delete the name component from request_update, EnTT expects the component to still exist (i.e. entity still in its storage) after this function returns.
    if(updated_name.Name != "Forbidden") {
      has_name &current_real_name = registry.get<has_name>(entity);
      current_real_name.Name = updated_name.Name;
    }
  }

  ~scene_3d_manager(){
    
  }
};

TEST_CASE("Storage insanity"){
  entt::registry registry;

  scene_3d_manager manager(&registry, 2501);
  
  const entt::entity ent1 = registry.create();
  const entt::entity ent2 = registry.create();

  registry.emplace<has_types>(ent1, std::set<std::string>{"marker"});
  registry.emplace<has_types>(ent2, std::set<std::string>{"marker"});

  registry.emplace<has_name>(ent1, "Initial");
  registry.emplace<has_name>(ent2, "Initial2");

  auto &&marker_update_name_storage = registry.storage<has_name>(entt::hashed_string::value("marker:request_update"));
  marker_update_name_storage.emplace(ent1, has_name{});
  marker_update_name_storage.emplace(ent2, has_name{});
  
  //....
  //Then in the drawer
  //(marker_update_storage still being fetchable from the registry)
  //i.e. using this:
  auto ent1_types = registry.get<has_types>(ent1);
  REQUIRE(ent1_types.Types.size() == 1);
  std::string type_name = *ent1_types.Types.begin();
  REQUIRE(type_name == "marker");
  auto &&retrieved_marker_storage = registry.storage<has_name>(entt::hashed_string::value((type_name + ":request_update").data()));

  retrieved_marker_storage.patch(ent1, [](has_name &hn){hn.Name = "OK";});
  CHECK(registry.get<has_name>(ent1).Name == "OK");

  retrieved_marker_storage.patch(ent1, [](has_name &hn){hn.Name = "Forbidden";});
  CHECK(registry.get<has_name>(ent1).Name == "OK");

  retrieved_marker_storage.patch(ent1, [](has_name &hn){hn.Name = "OK again";});
  CHECK(registry.get<has_name>(ent1).Name == "OK again");

  retrieved_marker_storage.patch(ent2, [](has_name &hn){hn.Name = "On another entity";});
  CHECK(registry.get<has_name>(ent2).Name == "On another entity");

}