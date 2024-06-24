
#include "split.h"
#include "combination_components.h"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <string>


//======================================
struct has_name {
  std::string Name;
};

struct factoryable{

};

struct factory{
  
  static void produce(entt::registry &registry, entt::entity me){
    if(!registry.any_of<combination_info>(me)){
      return;
    }

    combination_info &info = registry.get<combination_info>(me);
    auto map_it = info.CurrentCombinations.find(combination_kind::production);
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
};

//==================================

struct couverturable  {
  static void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    std::cout << "couverturable has been combined\n";
  }
};

struct material {
  
};

struct spell {
};

void my_on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other){
  std::cout << "le_monstre has been combined\n";
}

struct couverture {
  static void on_combined_to(entt::registry &registry, combination_kind kind, entt::entity me, entt::entity other) {
    std::cout << "couverture has been combined\n";
  }
};

void display(entt::registry &registry){
  auto view = registry.view<factoryable, has_name>();

  for(auto entity : view){
    has_name &hn = view.get<has_name>(entity);
    std::cout << hn.Name << "\n";
  }
}

int main(int argc, char* argv[]){

  entt::registry registry;

  const auto the_factory = registry.create();
  registry.emplace<factory>(the_factory);
  registry.emplace<has_name>(the_factory, "Gurren Lagann");
  combination_info fci;
  fci.AcceptedCombinations.emplace(combination_kind::production, std::set<entt::type_info>{entt::type_id<factoryable>()});
  registry.emplace<combination_info>(the_factory, fci);

  for(int i = 0; i < 10; i++) {
      const auto entity = registry.create();
      registry.emplace<has_name>(entity, std::to_string(i));
      if(i % 2 == 0) {
        registry.emplace<factoryable>(entity);
        combination_info fable_info;
        fable_info.AcceptedCombinations.emplace(combination_kind::production, std::set<entt::type_info>{entt::type_id<factory>()});
        registry.emplace<combination_info>(entity, fable_info);
        combine(registry, the_factory, entity);
      }
  }
  
  std::cout << "Before producing : \n";
  display(registry);

  factory::produce(registry, the_factory);
  
  std::cout << "After producing : \n";
  display(registry);

  const auto la_couverte = registry.create();
  registry.emplace<on_combine_trigger>(la_couverte);
 
  combination_info starting_couverture_combinable;
  starting_couverture_combinable.AcceptedCombinations.emplace(combination_kind::equipping, std::set<entt::type_info>({entt::type_id<couverturable>()}));
  starting_couverture_combinable.AcceptedCombinations.emplace(combination_kind::substance_of, std::set<entt::type_info>({entt::type_id<spell>(), entt::type_id<material>()}));
  registry.emplace<combination_info>(la_couverte, starting_couverture_combinable);
  emplace_combination_reactive_component<couverture>(registry, la_couverte);

  const auto le_monstre = registry.create();
  registry.emplace<on_combine_trigger>(le_monstre); 
  
  combination_info monster_info;
  monster_info.AcceptedCombinations.emplace(combination_kind::equipping, std::set<entt::type_info>({entt::type_id<couverture>()}));
  registry.emplace<combination_info>(le_monstre, monster_info);
  emplace_combination_reactive_component<couverturable>(registry, le_monstre);

  if(combine(registry, la_couverte, le_monstre)){
    std::cout << "combination success" << std::endl;
  }

  split_rule my_rule;
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<couverture>()}));
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<combination_info>()}));

  std::vector<entt::entity> split_result;
  split(registry, la_couverte, my_rule, split_result);

  std::cout << "split result" << std::endl;
  auto it = registry.get<combination_info>(split_result[1]).AcceptedCombinations.at(combination_kind::equipping).begin();
  std::cout << entt::type_id<couverturable>().index() << std::endl;
  std::cout << it->index() << std::endl;

  return 0;
}