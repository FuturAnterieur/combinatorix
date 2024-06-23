
#include "split.h"

bool split(entt::registry &registry, entt::entity x, const split_rule &rule, std::vector<entt::entity> &output){
  
  //TODO: If x is part of a combination, uncombine it (at least partly)
  for(const auto &group : rule.TypeGroups){
    const auto y = registry.create();
    for(const auto type_info : group){
      auto storage_ptr = registry.storage(type_info.hash());
      if(storage_ptr && storage_ptr->contains(x)){
        storage_ptr->push(y, storage_ptr->value(x));
      }
    }
    output.push_back(y);
  }

  registry.destroy(x);
  return true;
}
