#include "types.h"

bool assign_status(entt::registry &registry, entt::entity entity, const std::string &type_name){
  return assign_status(registry, entity, entt::hashed_string::value(type_name.data()));
}

bool assign_status(entt::registry &registry, entt::entity entity, entt::id_type type_hash){
  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentHashes.emplace(type_hash); 
  
  auto &&type_specific_storage = registry.storage<void>(type_hash);
  if(!type_specific_storage.contains(entity)){
    type_specific_storage.emplace(entity);
  }

  //TODO : Emplace parent types from the inheritance tree
  //registry.ctx().emplace<type_inheritance_graph>();


  return true;
}

bool add_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  entt::id_type hash = entt::hashed_string::value(param_name.data());

  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentHashes.emplace(hash); 

  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
  }
  
  return true;
}