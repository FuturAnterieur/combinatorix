#pragma once

#include <entt/entity/registry.hpp>

namespace utils {
  template<typename Component>
  Component &get_or_emplace(entt::registry &registry, entt::entity entity, entt::id_type hash){
    auto &&specific_storage = registry.storage<Component>(hash);
    if(!specific_storage.contains(entity)){
      return specific_storage.emplace(entity);
    } else {
      return specific_storage.get(entity);
    }
  }

  template<typename Component>
  void emplace_or_replace(entt::registry &registry, entt::entity entity, entt::id_type hash, const Component &value){
    auto &&specific_storage = registry.storage<Component>(hash);
    if(!specific_storage.contains(entity)){
      specific_storage.emplace(entity, value);
    } else {
      auto &existing = specific_storage.get(entity);
      existing = value;
    }
  }

  template<typename Component>
  Component get_or_default(entt::registry &registry, entt::entity entity, entt::id_type hash, const Component &default_val){
    auto &&specific_storage = registry.storage<Component>(hash);
    if(!specific_storage.contains(entity)){
      return default_val;
    } else {
      return specific_storage.get(entity);
    }
  }
}
