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
    auto &&type_specific_storage = registry.storage<void>(status_hash);
    if(!type_specific_storage.contains(entity)){
      type_specific_storage.emplace(entity);
    }
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
  attr_info.CurrentParamValues.emplace(hash, parameter{dt, value});

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
  attr_info.CurrentParamValues.emplace(hash, parameter{dt, value});

  /*auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    return false;
  }

  specific_storage.get(entity) = parameter{dt, value};*/

  return true;
}


//=====================================
void reset_original_status(entt::registry &registry, attributes_info_snapshot &snapshot, entt::entity entity){
  attributes_info *attr_info = registry.try_get<attributes_info>(entity);
  if(!attr_info){
    return;
  }
  reset_original_status(registry, *attr_info, snapshot, entity);
}


void reset_original_status(entt::registry &registry, attributes_info &entity_attr_info, attributes_info_snapshot &snapshot, entt::entity entity){
  
  //won't need to worry about status inheritance here because ALL statuses are impacted
  snapshot.ParamValues = entity_attr_info.CurrentParamValues;
  snapshot.StatusHashes = entity_attr_info.CurrentStatusHashes;

  entity_attr_info.CurrentParamValues = entity_attr_info.OriginalParamValues;
  entity_attr_info.CurrentStatusHashes = entity_attr_info.OriginalStatusHashes;
}

void commit_attr_info(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity){

  attributes_info_changes changes;
  for(const auto &hash : attr_info.CurrentStatusHashes){
    auto &&status_specific_storage = registry.storage<void>(hash);
    if(!status_specific_storage.contains(entity) && snapshot.StatusHashes.find(hash) == snapshot.StatusHashes.end()){
      status_specific_storage.emplace(entity);
      changes.AddedStatuses.emplace(hash);
    }
  }

  for(const auto &hash : snapshot.StatusHashes){
    auto &&status_specific_storage = registry.storage<void>(hash);
    if(status_specific_storage.contains(entity) && attr_info.CurrentStatusHashes.find(hash) == attr_info.CurrentStatusHashes.end()){
      status_specific_storage.remove(entity);
      changes.RemovedStatuses.emplace(hash);
    }
  }

  for(const auto &[hash, param] : attr_info.CurrentParamValues){
    auto &&param_specific_storage = registry.storage<parameter>(hash);
    if(!param_specific_storage.contains(entity)  && snapshot.ParamValues.find(hash) == snapshot.ParamValues.end()){
      param_specific_storage.emplace(entity, param);
      changes.AddedParams.emplace(hash, param);
    } else {
      //Use patch so that registry.on_update<parameter>(entt::hashed_string::value(param_name)) will work
      //But the 'changes' struct is there to prevent us from having to trigger EnTT events all throughout this function
      parameter previous_value = param_specific_storage.get(entity);
      param_specific_storage.patch(entity, [&](auto &local_p) { local_p.DT = param.DT; local_p.Value = param.Value; });
      changes.ModifiedParams.emplace(hash, std::make_pair(previous_value, param));
    }
  }

  for(const auto &[hash, param] : snapshot.ParamValues){
    auto &&param_specific_storage = registry.storage<parameter>(hash);
    if(param_specific_storage.contains(entity) && attr_info.CurrentParamValues.find(hash) == attr_info.CurrentParamValues.end()){
      param_specific_storage.remove(entity);
      changes.RemovedParams.emplace(hash, param);
    } 
  }

  if(on_status_change_triggers *triggers = registry.try_get<on_status_change_triggers>(entity); triggers){
    for(const on_status_change_trigger_info &info : triggers->Triggers){
      info.Func(registry, changes, entity, info.TriggerOwner);
    }
  }
}