#include "status.h"
#include "combine.h"
#include "effect.h"
#include "simulation_engine.h"
#include "local_change_tracker.h"
#include "entt_utils.h"

//=====================================
bool assign_active_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value){
  using namespace logistics;
  if(!registry.all_of<attributes_info, local_change_tracker>(entity)){
    return false;
  }

  local_change_tracker &tracker = registry.get<local_change_tracker>(entity);
  tracker.apply_status_edit(status_hash, status_value);
  
  
  //TODO : Emplace parent types from the inheritance tree
  //registry.ctx().emplace<type_inheritance_graph>();

  return true;
}

//=====================================
bool assign_intrinsic_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value){
  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  attributes_info_changes changes;
  logistics::start_simulating(registry, entity);
  auto &&type_specific_storage = registry.storage<void>(status_hash);
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  if(status_value){
    attr_info.CurrentStatusHashes.emplace(status_hash); 
    attr_info.IntrinsicStatusHashes.emplace(status_hash);
    
    if(!type_specific_storage.contains(entity)){
      changes.ModifiedStatuses.emplace(status_hash, smt::added);
      type_specific_storage.emplace(entity);
    }
  } else {
    attr_info.CurrentStatusHashes.erase(status_hash); 
    attr_info.IntrinsicStatusHashes.erase(status_hash);
    if(type_specific_storage.contains(entity)){
      changes.ModifiedStatuses.emplace(status_hash, smt::removed);
      type_specific_storage.remove(entity);
    }
  }
  
  activate_status_change_triggers(registry, entity, changes);
  //end simulation here (merge if OK... well to be confirmed on actual mechanic)
  logistics::merge_active_branch_to_reality(registry);

  return true;
}

//=====================================
bool get_active_value_for_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash){
  using namespace logistics;
  if(local_change_tracker *tracker = registry.try_get<local_change_tracker>(entity); tracker){
    return tracker->get_active_value_for_status(status_hash);
  }

  //Then check in the current simulation branch
  if(simulation_engine *sim = registry.ctx().find<simulation_engine>(); sim){
    auto &&attr_storage = get_active_branch_status_changes_storage(registry);
    if(attr_storage.contains(entity)){
      auto &hashes = attr_storage.get(entity).ModifiedStatuses;
      auto it = hashes.find(status_hash);
      if(it == hashes.end()){
        return registry.storage<void>(status_hash).contains(entity);
      } else {
        return it->second == smt::added;
      }
      
    }
  }

  //If we are not actively changing nor even running a simulation, that means we can fall back on the current value.
  //Invariant : entity.CurrentStatusHashes will be a perfect mirror of the <void> storage

  return registry.storage<void>(status_hash).contains(entity);
}

//=====================================
parameter get_active_value_for_parameter(entt::registry &registry, entt::entity entity, entt::id_type param_hash){
  using namespace logistics;
  if(local_change_tracker *tracker = registry.try_get<local_change_tracker>(entity); tracker){
    return tracker->get_active_value_for_parameter(param_hash);
  }

  //Then check in the current simulation branch
  if(simulation_engine *sim = registry.ctx().find<simulation_engine>(); sim){
    auto &&attr_storage = get_active_branch_status_changes_storage(registry);
    if(attr_storage.contains(entity)){
      auto &mod_params = attr_storage.get(entity).ModifiedParams;
      auto it = mod_params.find(param_hash);
      if(it == mod_params.end()){
        return utils::get_or_default(registry, entity, param_hash, parameter{});
      } else {
        return it->second.second;
      }
      
    }
  }

  //If we are not actively changing nor even running a simulation, that means we can fall back on the current value.
  //Invariant : entity.CurrentParameters will be a perfect mirror of the <parameter>() storage

  return utils::get_or_default(registry, entity, param_hash, parameter{});
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
bool add_or_set_intrinsic_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  
  entt::id_type hash = entt::hashed_string::value(param_name.data());
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

  logistics::start_simulating(registry, entity);
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentParamValues.insert_or_assign(hash, parameter{dt, value});
  attr_info.IntrinsicParamValues.insert_or_assign(hash, parameter{dt, value}); //also set original values because it isn't edited through a Modifier

  attributes_info_changes changes;
  parameter new_param{dt,value};
  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
    changes.ModifiedParams.emplace(hash, std::make_pair(parameter{}, new_param));
  } else {
    parameter &old_param = specific_storage.get(entity);
    auto &pair = changes.ModifiedParams.emplace(hash, std::make_pair(old_param, new_param));
    old_param = new_param;
  }

  activate_status_change_triggers(registry, entity, changes);
  
  //end simulation here (merge if OK... well to be confirmed on actual mechanic)
  logistics::merge_active_branch_to_reality(registry);
  
  return true;
}

//=====================================
void reset_original_status(entt::registry &registry, attributes_info_snapshot &snapshot, entt::entity entity){
  //assert(registry.all_of<attributes_info, logistics::local_change_tracker>(entity));
  
  auto &entity_attr_info = registry.get<attributes_info>(entity);
  auto &local_change_tracker = registry.get<logistics::local_change_tracker>(entity);

  //TODO : take snapshot from current branch if in a simulation and snapshot exists for this entity (case of going through the same entity again, basically)
  //won't need to worry about status inheritance here because ALL statuses are impacted
  snapshot.ParamValues = entity_attr_info.CurrentParamValues;
  snapshot.StatusHashes = entity_attr_info.CurrentStatusHashes;

  local_change_tracker.set_starting_point(attributes_info_snapshot{entity_attr_info.IntrinsicStatusHashes, entity_attr_info.IntrinsicParamValues});
}

//=========================================
bool changes_empty(attributes_info_changes &changes){
  return changes.ModifiedStatuses.empty() && changes.ModifiedParams.empty();
}

//=========================================
void activate_status_change_triggers(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes){
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

//=========================================
void commit_attr_info_to_branch(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity){

  
  auto &local_change_tracker = registry.get<logistics::local_change_tracker>(entity);
  attributes_info_snapshot new_snapshot = local_change_tracker.produce_active_snapshot();

  attributes_info_changes changes = compute_diff(snapshot, new_snapshot);
  
  logistics::commit_changes_to_active_branch(registry, entity, changes);
  registry.remove<logistics::local_change_tracker>(entity);

  if(changes_empty(changes)){
    return;
  }

  activate_status_change_triggers(registry, entity, changes);
}

//==============================================================
void add_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info){
  on_status_change_triggers &triggers = registry.get_or_emplace<on_status_change_triggers>(entity);
  add_on_status_change_trigger(registry, triggers, info);
}

//==============================================================
void add_global_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info){
  on_status_change_triggers *triggers = registry.ctx().find<on_status_change_triggers>();
  if(!triggers){
    registry.ctx().emplace<on_status_change_triggers>();
    triggers = &registry.ctx().get<on_status_change_triggers>();
  }
  add_on_status_change_trigger(registry, *triggers, info);
}

//==============================================================
void add_on_status_change_trigger(entt::registry &registry, on_status_change_triggers &triggers, on_status_change_trigger_info &info){
  triggers.Triggers.push_back(info);
}