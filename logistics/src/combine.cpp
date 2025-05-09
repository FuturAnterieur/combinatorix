#include "combine.h"

bool combine(entt::registry &registry, entt::entity a, entt::entity b){
  if(!registry.any_of<combination_info>(a) || !registry.any_of<combination_info>(b)){
    return false;
  }
  
  combination_info &combinable_of_a = registry.get<combination_info>(a);
  combination_info &combinable_of_b = registry.get<combination_info>(b);

  bool match_found = false;
  for(const auto kind : combinable_of_a.AcceptedCombinations){
    auto it_b = combinable_of_b.AcceptedCombinations.find(kind);
    if(it_b == combinable_of_b.AcceptedCombinations.end()){
      continue;
    }    
    combinable_of_a.CurrentCombinations[kind].insert(b);
    if(registry.any_of<on_combine_trigger>(a)){
      for(const auto &func : registry.get<on_combine_trigger>(a).Funcs){
        func(registry, kind, a, b);
      }
    }
    
    if(registry.any_of<on_combine_trigger>(b)){
      for(const auto &func : registry.get<on_combine_trigger>(b).Funcs){
        func(registry, kind, b, a);
      }
    }

    if(const on_combine_trigger *global_triggers = registry.ctx().find<on_combine_trigger>(); global_triggers){
      for(const auto &func : global_triggers->Funcs){
        func(registry, kind, a, b);
      }
    }
    match_found = true;
  }
  return match_found;
}

void link(entt::registry &registry, entt::entity a, entt::entity b){
  combination_info &combinable_of_a = registry.get_or_emplace<combination_info>(a);
  
  combinable_of_a.CurrentCombinations[combination_kind::link].insert(b);
}

void unlink(entt::registry &registry, entt::entity a, entt::entity b){
  combination_info &combinable_of_a = registry.get_or_emplace<combination_info>(a);
  combinable_of_a.CurrentCombinations[combination_kind::link].erase(b);
}

void add_combine_trigger(entt::registry &registry, entt::entity e, combine_trigger_t func) 
{
  if(!registry.any_of<on_combine_trigger>(e)){
    registry.emplace<on_combine_trigger>(e);
  } 
  on_combine_trigger &es_funcs = registry.get<on_combine_trigger>(e);
  es_funcs.Funcs.push_back(func);
}