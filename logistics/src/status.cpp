#include "status.h"
#include "combine.h"
#include "effect.h"
#include "simulation_engine.h"
#include "local_change_tracker.h"

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
    attr_info.IntrinsicStatusHashes.emplace(status_hash);
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


//=====================================
bool add_original_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  entt::id_type hash = entt::hashed_string::value(param_name.data());

  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.IntrinsicParamValues.emplace(hash, parameter{dt, value}); 
  attr_info.CurrentParamValues.emplace(hash, parameter{dt, value});

  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
  }
  
  return true;
}

//=====================================
//To be called from modifiers
bool add_or_set_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  entt::id_type hash = entt::hashed_string::value(param_name.data());
  if(logistics::local_change_tracker *tracker = registry.try_get<logistics::local_change_tracker>(entity); tracker){
    tracker->apply_param_edit(hash, parameter{dt, value});
  }
  
  return true;
}

//=====================================
logistics_API bool add_or_set_parameter_and_trigger_on_change(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  
  entt::id_type hash = entt::hashed_string::value(param_name.data());
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentParamValues.insert_or_assign(hash, parameter{dt, value});
  attr_info.IntrinsicParamValues.insert_or_assign(hash, parameter{dt, value}); //also set original values because it isn't edited through a Modifier
  //TODO implement the ChangeTracker (with methods) so that editing statuses/params is less cumbersome

  const parameter param_null = parameter{data_type::null, ""};
  attributes_info_changes changes;
  parameter new_param{dt,value};
  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
    changes.ModifiedParams.emplace(hash, std::make_pair(param_null, new_param));
  } else {
    parameter &old_param = specific_storage.get(entity);
    auto &pair = changes.ModifiedParams.emplace(hash, std::make_pair(old_param, new_param));
    old_param = new_param;
  }

  if(on_status_change_triggers *triggers = registry.try_get<on_status_change_triggers>(entity); triggers){
    for(const on_status_change_trigger_info &info : triggers->Triggers){
      if(info.Filter(registry, changes, entity, info.TriggerOwner)){
        info.Func(registry, changes, entity, info.TriggerOwner);
      }
    }
  }

  //For now we can still go fetch global triggers here if they exist
  const on_status_change_triggers *global_triggers = registry.ctx().find<on_status_change_triggers>();
  if(global_triggers){
    for(const on_status_change_trigger_info &info : global_triggers->Triggers){
      if(info.Filter(registry, changes, entity, info.TriggerOwner)){
        info.Func(registry, changes, entity, info.TriggerOwner);
      }
    }
  }

  if(registry.any_of<combination_info>(entity)){
    auto &info = registry.get<combination_info>(entity);
    for(const auto &[kind, entities] : info.CurrentCombinations){
      for(entt::entity entity : entities){
        update_status_effects(registry, entity);
      }
    }
  }
  
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

//=========================================
void reset_original_status(entt::registry &registry, attributes_info &entity_attr_info, attributes_info_snapshot &snapshot, entt::entity entity){
  
  //won't need to worry about status inheritance here because ALL statuses are impacted
  snapshot.ParamValues = entity_attr_info.CurrentParamValues;
  snapshot.StatusHashes = entity_attr_info.CurrentStatusHashes;

  entity_attr_info.CurrentParamValues = entity_attr_info.IntrinsicParamValues;
  entity_attr_info.CurrentStatusHashes = entity_attr_info.IntrinsicStatusHashes;
}

//=========================================
bool changes_empty(attributes_info_changes &changes){
  return changes.ModifiedStatuses.empty() && changes.ModifiedParams.empty();
}

//=========================================
void commit_attr_info(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity){

  attributes_info_changes changes;

  const parameter param_null = parameter{data_type::null, ""};
  //the new "current status hashes" would be the local_change_tracker's StartingPoint + all the changes in the local tracker
  for(const auto &hash : attr_info.CurrentStatusHashes){
    auto &&status_specific_storage = registry.storage<void>(hash);
    if(!status_specific_storage.contains(entity) && snapshot.StatusHashes.find(hash) == snapshot.StatusHashes.end()){
      status_specific_storage.emplace(entity);
      changes.ModifiedStatuses.emplace(hash, smt::added);
    }
  }

  for(const auto &hash : snapshot.StatusHashes){
    auto &&status_specific_storage = registry.storage<void>(hash);
    if(status_specific_storage.contains(entity) && attr_info.CurrentStatusHashes.find(hash) == attr_info.CurrentStatusHashes.end()){
      status_specific_storage.remove(entity);
      changes.ModifiedStatuses.emplace(hash, smt::removed);
    }
  }

  for(const auto &[hash, param] : attr_info.CurrentParamValues){
    auto &&param_specific_storage = registry.storage<parameter>(hash);
    if(!param_specific_storage.contains(entity)  && snapshot.ParamValues.find(hash) == snapshot.ParamValues.end()){
      param_specific_storage.emplace(entity, param);
      changes.ModifiedParams.emplace(hash, std::make_pair(param_null, param));
    } else {
      //Use patch so that registry.on_update<parameter>(entt::hashed_string::value(param_name)) will work
      //But the 'changes' struct is there to prevent us from having to trigger EnTT events all throughout this function
      parameter previous_value = param_specific_storage.get(entity);
      if(previous_value.Value != param.Value){
        param_specific_storage.patch(entity, [&](auto &local_p) { local_p.DT = param.DT; local_p.Value = param.Value; });
        changes.ModifiedParams.emplace(hash, std::make_pair(previous_value, param));
      }
    }
  }

  for(const auto &[hash, param] : snapshot.ParamValues){
    auto &&param_specific_storage = registry.storage<parameter>(hash);
    if(param_specific_storage.contains(entity) && attr_info.CurrentParamValues.find(hash) == attr_info.CurrentParamValues.end()){
      param_specific_storage.remove(entity);
      changes.ModifiedParams.emplace(hash, std::make_pair(param, param_null));
    } 
  }

  //logistics::commit_status_to_active_branch(registry, entity, snapshot);

  if(changes_empty(changes)){
    return;
  }
  
  if(on_status_change_triggers *triggers = registry.try_get<on_status_change_triggers>(entity); triggers){
    for(const on_status_change_trigger_info &info : triggers->Triggers){
      if(info.Filter(registry, changes, entity, info.TriggerOwner)){
        info.Func(registry, changes, entity, info.TriggerOwner);
      }
    }
  }

  //For now we can still go fetch global triggers here if they exist
  const on_status_change_triggers *global_triggers = registry.ctx().find<on_status_change_triggers>();
  if(global_triggers){
    for(const on_status_change_trigger_info &info : global_triggers->Triggers){
      if(info.Filter(registry, changes, entity, info.TriggerOwner)){
        info.Func(registry, changes, entity, info.TriggerOwner);
      }
    }
  }

  if(registry.any_of<combination_info>(entity)){
    auto &info = registry.get<combination_info>(entity);
    for(const auto &[kind, entities] : info.CurrentCombinations){
      for(entt::entity entity : entities){
        update_status_effects(registry, entity);
      }
    }
  }
}

void add_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info){
  on_status_change_triggers &triggers = registry.get_or_emplace<on_status_change_triggers>(entity);
  add_on_status_change_trigger(registry, triggers, info);
}

void add_global_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info){
  on_status_change_triggers *triggers = registry.ctx().find<on_status_change_triggers>();
  if(!triggers){
    registry.ctx().emplace<on_status_change_triggers>();
    triggers = &registry.ctx().get<on_status_change_triggers>();
  }
  add_on_status_change_trigger(registry, *triggers, info);
}


void add_on_status_change_trigger(entt::registry &registry, on_status_change_triggers &triggers, on_status_change_trigger_info &info){
  triggers.Triggers.push_back(info);
}