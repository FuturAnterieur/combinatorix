#include "status.h"

//=====================================
bool assign_status(entt::registry &registry, entt::entity entity, const std::string &type_name, bool is_original){
  return assign_status(registry, entity, entt::hashed_string::value(type_name.data()), is_original);
}

//=====================================
bool assign_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool is_original){
  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentStatusHashes.emplace(status_hash); 
  if(is_original){
    attr_info.OriginalStatusHashes.emplace(status_hash);
  }
  
  auto &&type_specific_storage = registry.storage<void>(status_hash);
  if(!type_specific_storage.contains(entity)){
    type_specific_storage.emplace(entity);
  }

  //TODO : Emplace parent types from the inheritance tree
  //registry.ctx().emplace<type_inheritance_graph>();


  return true;
}

//=====================================
bool add_original_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  entt::id_type hash = entt::hashed_string::value(param_name.data());

  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.OriginalParamValues.emplace(hash, parameter{dt, value}); 
  attr_info.CurrentParamHashes.emplace(hash);

  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
  }
  
  return true;
}

//=====================================
bool add_additional_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  entt::id_type hash = entt::hashed_string::value(param_name.data());
  
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentParamHashes.emplace(hash);

  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    return false;
  }

  specific_storage.get(entity) = parameter{dt, value};

  return true;
}


//=====================================
void reset_original_status(entt::registry &registry, entt::entity entity){
  attributes_info *attr_info = registry.try_get<attributes_info>(entity);
  if(!attr_info){
    return;
  }
  reset_original_status(registry, *attr_info, entity);
}

//=====================================
void reset_all_original_status(entt::registry &registry){
  auto view = registry.view<attributes_info>();
  view.each([&](auto entity, attributes_info &attr_info){
    reset_original_status(registry, attr_info, entity);
  });
}

void reset_original_status(entt::registry &registry, attributes_info &attr_info, entt::entity entity){
  
  //won't need to worry about status inheritance here because ALL statuses are impacted
  for(const auto &hash : attr_info.CurrentStatusHashes){
    if(attr_info.OriginalStatusHashes.find(hash) == attr_info.OriginalStatusHashes.end()){
      auto &&status_specific_storage = registry.storage<void>(hash);
      assert(status_specific_storage.contains(entity));
      status_specific_storage.remove(entity);
    }
  }

  for(const auto &hash : attr_info.OriginalStatusHashes){
    if(attr_info.CurrentStatusHashes.find(hash) == attr_info.CurrentStatusHashes.end()){
      auto &&status_specific_storage = registry.storage<void>(hash);
      assert(!status_specific_storage.contains(entity));
      status_specific_storage.emplace(entity);
    }
  }

  
  for(const auto &hash : attr_info.CurrentParamHashes){
    if(attr_info.OriginalParamValues.find(hash) == attr_info.OriginalParamValues.end()){
      auto &&param_specific_storage = registry.storage<parameter>(hash);
      assert(param_specific_storage.contains(entity));
      param_specific_storage.remove(entity);
    }
  }

  for(const auto &[hash, param] : attr_info.OriginalParamValues){
    auto &&param_specific_storage = registry.storage<parameter>(hash);
    if(attr_info.CurrentParamHashes.find(hash) == attr_info.CurrentParamHashes.end()){
      assert(!param_specific_storage.contains(entity));
      param_specific_storage.emplace(entity, param);
    } else {
      assert(param_specific_storage.contains(entity));
      param_specific_storage.get(entity) = param;
    }
  }

  attr_info.CurrentParamHashes.clear();
  for(const auto &[hash, param] : attr_info.OriginalParamValues){
    attr_info.CurrentParamHashes.insert(hash);
  }

  attr_info.CurrentStatusHashes = attr_info.OriginalStatusHashes;
}