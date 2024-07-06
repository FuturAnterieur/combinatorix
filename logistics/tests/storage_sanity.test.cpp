#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <entt/entity/registry.hpp>
#include <entt/entity/runtime_view.hpp>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/snapshot.hpp>
#include <entt/meta/resolve.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/archives/binary.hpp>
#include "status.h"

struct has_name {
  std::string Name;
};

struct wizard_info {
  entt::entity Entity;
  std::string Description{};
  bool is_updating{false};
};

using wizards_info = std::vector<wizard_info>;

constexpr auto desc_hash = entt::hashed_string::value("description");
constexpr auto desc_updated_hash = entt::hashed_string::value("description_updated");

void serialize_round_trip(entt::registry &registry, entt::continuous_loader &loader){
  std::stringstream data_store;
  {
    cereal::BinaryOutputArchive output{data_store};
    entt::snapshot{registry}
      .get<entt::entity>(output)
      .get<parameter>(output, desc_updated_hash);
  }

  cereal::BinaryInputArchive input{data_store};
  loader.get<entt::entity>(input)
        .get<parameter>(input, desc_updated_hash)
        .orphans();
}

void update_wizards(entt::registry &registry, wizards_info &info){
  info.clear();
  //Using storage.data()

  auto &&my_entity_storage = registry.storage<entt::entity>();
  auto &&my_update_storage = registry.storage<parameter>(desc_updated_hash);
  auto &&my_stable_storage = registry.storage<parameter>(desc_hash);

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
    *stable_param = update_param;
  }  

  /*for(size_t i = 0; i < my_entity_storage.size(); i++){
    info.push_back(wizard_info{my_entity_storage.data()[i]});
  }*/

  for(size_t i = 0; i < my_stable_storage.size(); i++){
    auto entity = my_stable_storage.data()[i];
    parameter &stable_param = my_stable_storage.get(entity);
    info.push_back(wizard_info{entity, std::get<std::string>(stable_param.value())});
  }
}

//entt::meta_type type = entt::resolve(desc_hash);

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
  
  //COPYING REGISTRY PARTS
  //registry is the central/server registry
  //registry2 is a client registry
  //desc_updated_hash is for the pool of items that are to be updated
  //desc_hash is for the pool of commited items
  auto &&my_update_storage = registry.storage<parameter>(desc_updated_hash);
  my_update_storage.emplace(mermelionos, "Le roi des paranoiaques");
  my_update_storage.emplace(taliesin, "Celui qui a ecrit le Livre");
  my_update_storage.emplace(merlin, "en vacances avec viviane");
  
  wizards_info wizards1;
  update_wizards(registry, wizards1);
  
  
  //see behavior when...

  //we deserialize reg1's 'params to update' output to reg2
  entt::registry registry2;
  entt::continuous_loader loader(registry2);
  serialize_round_trip(registry, loader);

  wizards_info wizards2;
  update_wizards(registry2, wizards2);
  compare_wizards(wizards1, wizards2);
  
  //We create a new wizard in the central registry
  my_update_storage.clear();
  auto sorcier1_only = registry.create();
  my_update_storage.emplace(sorcier1_only, "Le nouveau-venu");
  


  update_wizards(registry, wizards1);

  serialize_round_trip(registry, loader);
  update_wizards(registry2, wizards2);

  compare_wizards(wizards1, wizards2);

  //We update an existing wizard in the central registry
  my_update_storage.clear();
  my_update_storage.emplace(taliesin, "Entité 1 avec nouvelle desc");
  update_wizards(registry, wizards1);

  serialize_round_trip(registry, loader);
  update_wizards(registry2, wizards2);

  compare_wizards(wizards1, wizards2);

  //We create a new wizard in the client registry (tagging it 'to be updated')
  //AND we create a new wizard in the server at the same time.
  my_update_storage.clear();
  auto &&my_update_storage2 = registry2.storage<parameter>(desc_updated_hash);
  auto ron_weasley = registry2.create();
  my_update_storage2.emplace(ron_weasley,  "Ron weasley, entite 4 sur le client");

  auto hermione_granger = registry.create();
  my_update_storage.emplace(hermione_granger, "Hermione Granger, entity 4 sur le serveur");

  update_wizards(registry, wizards1);

  serialize_round_trip(registry, loader);
  update_wizards(registry2, wizards2);

  CHECK(entt::to_integral(ron_weasley) == 4);
  CHECK(entt::to_integral(hermione_granger) == 4);

  //loader.map makes the cnversion "entityID in its (serialized) source -> entityID in destination registry"
  CHECK(entt::to_integral(loader.map(ron_weasley)) == 5); //makes no 'sense' to make this request
  CHECK(entt::to_integral(loader.map(hermione_granger)) == 5);
  //So we know entity 4 maps to entity 5.


  //What about the previous ones?
  for(std::uint32_t i = 0; i < 4; i++){
    CHECK(entt::to_integral(loader.map(static_cast<entt::entity>(i))) == i);
  }

  auto entity_maps_to_null = [&](uint32_t n){return loader.map(static_cast<entt::entity>(n)) == entt::null;};

  //And finally...
  CHECK(entity_maps_to_null(5));

  //Then, test what happens when an entity is destroyed:
  registry.destroy(taliesin); //TALIESIN NOOOOOOO. Taliesin is entity 1

  update_wizards(registry, wizards1);
  serialize_round_trip(registry, loader);

  update_wizards(registry2, wizards2);
  //Wow. Entity 1 is now absent from wizards2

  CHECK(entity_maps_to_null(1));
  my_update_storage.clear();
  my_update_storage.emplace(hermione_granger, "Hermione Granger MODIFIEE, entity 4 sur le serveur");

  update_wizards(registry, wizards1);
  serialize_round_trip(registry, loader);

  update_wizards(registry2, wizards2);

  my_update_storage.clear();

  auto &&my_stable_storage_2 = registry2.storage<parameter>(desc_hash);
  std::string hg_desc_on_client = std::get<std::string>(my_stable_storage_2.get(loader.map(hermione_granger)).value());

  CHECK(hg_desc_on_client == "Hermione Granger MODIFIEE, entity 4 sur le serveur");
}

