#pragma once

#include <entt/entity/registry.hpp>

template<typename Component>
Component &get_or_emplace(entt::registry &registry, entt::entity entity, entt::id_type hash){
  auto &&specific_storage = registry.storage<Component>(hash);
  if(!specific_storage.contains(entity)){
    return specific_storage.emplace(entity);
  } else {
    return specific_storage.get(entity);
  }
}
