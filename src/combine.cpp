#include "combination_components.h"

bool combine(entt::registry &registry, entt::entity a, entt::entity b){
  if(!registry.any_of<combination_info>(a) || !registry.any_of<combination_info>(b)){
    return false;
  }
  
  combination_info &combinable_of_a = registry.get<combination_info>(a);
  combination_info &combinable_of_b = registry.get<combination_info>(b);

  bool match_found = false;
  for(const auto [kind, type_set] : combinable_of_a.AcceptedCombinations){
    auto it_b = combinable_of_b.AcceptedCombinations.find(kind);
    if(it_b == combinable_of_b.AcceptedCombinations.end()){
      continue;
    }

    //TODO : do reciprocal check
    //Maybe TODO : place current combinations map in another component for more modularity
    for(const auto type : type_set){
      auto storage_ptr = registry.storage(type.hash()); //storage_ptr should not be null, but just in case
      if(storage_ptr && storage_ptr->contains(b)){
        
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
        return true;
      }
    }
  }

  return match_found;
}
