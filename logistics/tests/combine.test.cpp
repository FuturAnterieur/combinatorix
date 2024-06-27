#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "combine.h"
#include "effect.h"

#include <string>
#include <sstream>

//======================================
struct has_name {
  std::string Name;
};

namespace factory_effect{
  void produce(entt::registry &registry, entt::entity me, entt::entity target){
    if(!registry.any_of<combination_info>(me)){
      return;
    }

    combination_info &info = registry.get<combination_info>(me);
    auto map_it = info.CurrentCombinations.find(combination_kind::attached);
    if(map_it == info.CurrentCombinations.end()){
      return;
    }

    for(const entt::entity &source : map_it->second){
      const entt::entity other = registry.create();
      for(const auto [id, storage] : registry.storage()){
        if(storage.contains(source)){
          storage.push(other, storage.value(source));
        }
      }
    }
  }

  entt::entity init(entt::registry &registry, entt::entity owner){
    combination_info base_info;
    base_info.AcceptedCombinations.emplace(combination_kind::attached);
    return add_ability(registry, owner, factory_effect::produce, base_info);
  }
}

//==================================
static std::stringstream on_combined_trigger_stream;

namespace couverturable  {
  void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    on_combined_trigger_stream << "b";
  }
}

namespace couverture {
  void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    on_combined_trigger_stream << "a";
  }
}

std::string display(entt::registry &registry){
  auto view = registry.view<has_name>();
  std::stringstream result;

  for(auto entity : view){
    has_name &hn = view.get<has_name>(entity);
    result << hn.Name;
  }
  return result.str();
}



TEST_CASE("Combination functions"){
  entt::registry registry;

  const auto the_factory = registry.create();
  entt::entity feff = factory_effect::init(registry, the_factory);
  

  for(int i = 0; i < 10; i++) {
      const auto entity = registry.create();
      registry.emplace<has_name>(entity, std::to_string(i));
      if(i % 2 == 0) {
        combination_info fable_info;
        fable_info.AcceptedCombinations.emplace(combination_kind::attached);
        registry.emplace<combination_info>(entity, fable_info);
        REQUIRE(combine(registry, feff, entity));
      }
  }
  
  CHECK(display(registry) == "9876543210");

  use(registry, feff, entt::null);
  
  CHECK(display(registry) == "864209876543210");
  const auto la_couverte = registry.create();
  registry.emplace<has_name>(la_couverte, "Couverture Name!!!");
  combination_info couverture_info;
  couverture_info.AcceptedCombinations.emplace(combination_kind::equipping);
  couverture_info.AcceptedCombinations.emplace(combination_kind::substance_of);
  registry.emplace<combination_info>(la_couverte, couverture_info);
  add_combine_trigger(registry, la_couverte, &couverture::on_combined_to);

  const auto le_monstre = registry.create();
  combination_info monster_info;
  monster_info.AcceptedCombinations.emplace(combination_kind::equipping);
  registry.emplace<combination_info>(le_monstre, monster_info);
  add_combine_trigger(registry, le_monstre, &couverturable::on_combined_to);

  CHECK(combine(registry, la_couverte, le_monstre));
  CHECK(on_combined_trigger_stream.str() == "ab");
}


