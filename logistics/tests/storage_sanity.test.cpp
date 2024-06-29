#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <entt/entity/registry.hpp>
#include <entt/entity/runtime_view.hpp>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/snapshot.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include "status.h"

struct has_name {
  std::string Name;
};

struct wizard_info {
  entt::entity Entity;
  std::string Description;
  bool is_updating{false};
};

using wizards_info = std::vector<wizard_info>;

constexpr auto param_hash = entt::hashed_string::value("description");
constexpr auto param_updated_hash = entt::hashed_string::value("description_updated");

template<typename Archive>
void serialize(Archive &archive, parameter &param){
  archive(param.DT, param.Value);
}

void serialize_round_trip(entt::registry &registry, entt::continuous_loader &loader){
  std::stringstream data_store;
  {
    cereal::BinaryOutputArchive output{data_store};
    entt::snapshot{registry}
      .get<entt::entity>(output)
      .get<parameter>(output, param_updated_hash);
  }

  cereal::BinaryInputArchive input{data_store};
  loader.get<entt::entity>(input)
        .get<parameter>(input, param_updated_hash);
}

void update_wizards(entt::registry &registry, wizards_info &info){
  info.clear();
  auto &&my_update_storage = registry.storage<parameter>(param_updated_hash);
  auto &&my_stable_storage = registry.storage<parameter>(param_hash);
  for(size_t i = 0; i < my_update_storage.size(); i++ ){
    //Update our wizards info
    auto entity = my_update_storage.data()[i];
    parameter &update_param = my_update_storage.get(entity);
    //Update registry stable data
    parameter *stable_param{nullptr};
    if(my_stable_storage.contains(entity)){
      stable_param = &my_stable_storage.get(entity);
    } else {
      stable_param = &my_stable_storage.emplace(entity);
    }
    stable_param->DT = update_param.DT;
    stable_param->Value = update_param.Value;
  }  
  for(size_t i = 0; i < my_stable_storage.size(); i++){
    auto entity = my_stable_storage.data()[i];
    parameter &stable_param = my_stable_storage.get(entity);
    info.push_back(wizard_info{entity, stable_param.Value});
  }
}

void compare_wizards(wizards_info &wizards1, wizards_info &wizards2){
  CHECK(wizards1.size() == wizards2.size());
  for(size_t i = 0; i < wizards1.size(); i++){
    CHECK(wizards1[i].Entity == wizards2[i].Entity);
    CHECK(wizards2[i].Description == wizards2[i].Description);
  }
}

TEST_CASE("Storage sanity"){
  entt::registry registry;
  
  const entt::entity mermelionos = registry.create();
  registry.emplace<has_name>(mermelionos, "Mermelionos");
  const entt::entity taliesin = registry.create();
  registry.emplace<has_name>(taliesin, "Taliesin");
  const entt::entity merlin = registry.create();

  auto &&wizzy_storage = registry.storage<void>(entt::hashed_string::value("wizard"));
  wizzy_storage.emplace(merlin);
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

  //This launches an assert. Cannot have the same name assigned to pools of different types.
  //auto &&my_param_storage = registry.storage<parameter>(entt::hashed_string::value("has_robe"));
  

  //Using storage.data()
  
  auto &&my_update_storage = registry.storage<parameter>(param_updated_hash);
  my_update_storage.emplace(mermelionos, data_type::string, "Le roi des paranoiaques");
  my_update_storage.emplace(taliesin, data_type::string, "Celui qui a ecrit le Livre");
  my_update_storage.emplace(merlin, data_type::string, "en vacances avec viviane");
  
  //Beware of order!
  wizards_info wizards1;
  update_wizards(registry, wizards1);
  
  //COPYING REGISTRY PARTS
  entt::registry registry2;
  entt::continuous_loader loader(registry2);
  serialize_round_trip(registry, loader);

  wizards_info wizards2;
  update_wizards(registry2, wizards2);
  compare_wizards(wizards1, wizards2);
  
  //auto sorcier2_only = registry2.create();
  auto sorcier1_only = registry.create();
  my_update_storage.clear();
  my_update_storage.emplace(sorcier1_only, data_type::string, "Le nouveau-venu");
  
  update_wizards(registry, wizards1);

  serialize_round_trip(registry, loader);
  update_wizards(registry2, wizards2);

  compare_wizards(wizards1, wizards2);

  my_update_storage.clear();
  my_update_storage.emplace(taliesin, data_type::string, "Entité 1 avec nouvelle desc");
  update_wizards(registry, wizards1);

  serialize_round_trip(registry, loader);
  update_wizards(registry2, wizards2);

  compare_wizards(wizards1, wizards2);
}

