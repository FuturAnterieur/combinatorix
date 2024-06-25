
#include "split.h"
#include "effect.h"
#include "combination_components.h"
#include "status.h"
#include <entt/entity/registry.hpp>
#include <entt/entity/runtime_view.hpp>
#include <iostream>
#include <string>


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

namespace couverturable  {
  void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    std::cout << "couverturable has been combined\n";
  }
}

void my_on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other){
  std::cout << "le_monstre has been combined\n";
}

namespace couverture {
  void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    std::cout << "couverture has been combined\n";
  }
}

void display(entt::registry &registry){
  auto view = registry.view<has_name>();

  for(auto entity : view){
    has_name &hn = view.get<has_name>(entity);
    std::cout << hn.Name << "\n";
  }
}

int main(int argc, char* argv[]){

  entt::registry registry;

  std::cout << "Testing storage (should write Mermelionos) : ";
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
  for(auto entity : rt_view){
    has_name &hn = registry.get<has_name>(entity);
    std::cout << hn.Name;
  }
  std::cout << "\n";
  std::cout << "Again (should write Taliesin) : ";
  auto yes_view = entt::view<entt::get_t<void>>{wizzy_storage};
  auto yes_view_2 = entt::view<entt::get_t<has_name>>(registry.storage<has_name>());
  auto no_view = entt::view<entt::get_t<>, entt::exclude_t<void>>{roby_storage};

  for(auto entity : yes_view | yes_view_2 | no_view){
    has_name &hn = registry.get<has_name>(entity);
    std::cout << hn.Name;
  }
  std::cout << "\n";

  const auto the_factory = registry.create();
  registry.emplace<has_name>(the_factory, "Gurren Lagann");
  entt::entity feff = factory_effect::init(registry, the_factory);
  

  for(int i = 0; i < 10; i++) {
      const auto entity = registry.create();
      registry.emplace<has_name>(entity, std::to_string(i));
      if(i % 2 == 0) {
        combination_info fable_info;
        fable_info.AcceptedCombinations.emplace(combination_kind::attached);
        registry.emplace<combination_info>(entity, fable_info);
        combine(registry, feff, entity);
      }
  }
  
  std::cout << "Before producing : \n";
  display(registry);

  use(registry, feff, entt::null);
  
  std::cout << "After producing : \n";
  display(registry);

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

  if(combine(registry, la_couverte, le_monstre)){
    std::cout << "combination success" << std::endl;
  }

  split_rule my_rule;
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<has_name>()}));
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<combination_info>()}));

  std::vector<entt::entity> split_result;
  split(registry, la_couverte, my_rule, split_result);

  std::cout << "split result : ";
  auto &combines = registry.get<has_name>(split_result[0]).Name;
  std::cout << combines << "\n";

  return 0;
}