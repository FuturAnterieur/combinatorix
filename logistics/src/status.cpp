#include "status.h"
#include "combine.h"
#include "effect.h"
#include "simulation_engine.h"
#include "local_change_tracker.h"
#include "change_merger.h"
#include "entt_utils.h"

//=====================================
bool assign_active_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value){
  using namespace logistics;
  auto &storage = logistics::get_active_branch_local_changes_storage(registry);
  assert(storage.contains(entity));
  attributes_info_history &history = storage.get(entity);
  attributes_info_changes changes;
  changes.ModifiedStatuses.emplace(status_hash, status_value ? smt::added : smt::removed);
  history.add_changes(0, changes);
  
  
  //TODO : Emplace parent types from the inheritance tree
  //registry.ctx().emplace<type_inheritance_graph>();

  return true;
}

//=====================================
bool init_intrinsic_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value){
  if(!registry.any_of<attributes_info>(entity)){
    registry.emplace<attributes_info>(entity);
  }
  
  
  auto &&type_specific_storage = registry.storage<void>(status_hash);
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  if(status_value){
    attr_info.CurrentStatusHashes.emplace(status_hash); 
    attr_info.IntrinsicStatusHashes.emplace(status_hash);
    if(!type_specific_storage.contains(entity)) {
      type_specific_storage.emplace(entity);
    }
  } else {
    attr_info.CurrentStatusHashes.erase(status_hash); 
    attr_info.IntrinsicStatusHashes.erase(status_hash);
    if(type_specific_storage.contains(entity)){
      type_specific_storage.remove(entity);
    }
  }
  
  return true;
}

//=====================================
bool get_active_value_for_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash){
  using namespace logistics;
  

  //Then check in the current simulation branch
  attributes_info_snapshot snapshot = get_active_snapshot(registry, entity);
  return snapshot.StatusHashes.find(status_hash) != snapshot.StatusHashes.end();
   

  //If we are not actively changing nor even running a simulation, that means we can fall back on the current value.
  //Invariant : entity.CurrentStatusHashes will be a perfect mirror of the <void> storage

  
}

//=====================================
parameter get_active_value_for_parameter(entt::registry &registry, entt::entity entity, entt::id_type param_hash){
  using namespace logistics;
 
  attributes_info_snapshot snapshot_to_use = get_active_snapshot(registry, entity);
  auto it = snapshot_to_use.ParamValues.find(param_hash);
  if(it == snapshot_to_use.ParamValues.end()){
    return parameter{};
  } else {
    return it->second;
  }
    
}



//=====================================
//To be called from modifiers
bool add_or_set_active_parameter(entt::registry &registry, entt::entity entity, entt::id_type param_hash, const parameter &param){
  auto &storage = logistics::get_active_branch_local_changes_storage(registry);
  assert(storage.contains(entity));
  auto &history = storage.get(entity);
  
  parameter previous = get_active_value_for_parameter(registry, entity, param_hash); //hmm, might be redundant if the status effect code already fetched this value
  attributes_info_changes changes; 
  changes.ModifiedParams.emplace(param_hash, std::make_pair(previous, param));
  history.add_changes(0, changes);

  return true;
}

//=====================================
bool init_intrinsic_parameter(entt::registry &registry, entt::entity entity, entt::id_type hash, const parameter &value){
  
  
  attributes_info &attr_info = registry.get_or_emplace<attributes_info>(entity);
  attr_info.CurrentParamValues.insert_or_assign(hash, parameter(value));
  attr_info.IntrinsicParamValues.insert_or_assign(hash, parameter(value)); //also set original values because it isn't edited through a Modifier

  parameter new_param(value);
  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, new_param);
  } else {
    parameter &old_param = specific_storage.get(entity);
    old_param = new_param;
  }

  return true;
}


//=====================================
bool paste_attributes_changes(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes, attributes_info_reference &ref, bool affect_registry)
{
  for(const auto &[hash, param_pair] : changes.ModifiedParams){
    auto &&storage = registry.storage<parameter>(hash);
    if(param_pair.second.dt() == data_type::null){ //deletion
      ref.ParamValues.erase(hash);
      if(affect_registry)
        bool ret = storage.remove(entity);
      //assert(ret);
    } else  { //modification
      ref.ParamValues.insert_or_assign(hash, param_pair.second);
      if(affect_registry)
        utils::emplace_or_replace<parameter>(registry, entity, hash, param_pair.second);
    }
  }

  for(const auto &[hash, smt_val] : changes.ModifiedStatuses){
    auto &&storage = registry.storage<void>(hash);
    if(smt_val == smt::removed){
      ref.StatusHashes.erase(hash);
      if(affect_registry)
        storage.remove(entity);
      //assert(ret);
    } else {
      ref.StatusHashes.insert(hash);
      if(affect_registry && !storage.contains(entity))
        storage.emplace(entity);
    }
  }
  return true;
}

//====================================
//To be called from outside status effect modifier functions
bool assign_intrinsic_attributes_changes(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes){
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

  logistics::commit_changes_for_intrinsics_to_active_branch(registry, entity, changes);
  update_status_effects(registry, entity); //assumes this launches commit after calculating status effect consequences

  return true;
}

//=====================================
void init_history_for_local_changes(entt::registry &registry, entt::entity entity){
  
  attributes_info_snapshot intrinsics = logistics::get_most_recent_intrinsics(registry, entity, logistics::changes_request::working_copy);

  auto &local_storage = logistics::get_active_branch_local_changes_storage(registry);
  local_storage.emplace(entity, intrinsics);
}

//=========================================
bool changes_empty(attributes_info_changes &changes){
  return changes.ModifiedStatuses.empty() && changes.ModifiedParams.empty();
}

//=========================================
void activate_status_change_triggers(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes){
  logistics::simulation_engine *eng = logistics::get_simulation_engine(registry);
  

  if(on_status_change_triggers *triggers = registry.try_get<on_status_change_triggers>(entity); triggers){
    for(const on_status_change_trigger_info &info : triggers->Triggers){
      if(info.Filter(registry, changes, entity, info)){
        logistics::add_edge(registry, entity, info.TriggerOwner);
        eng->enqueue_trigger(info, entity, changes);
        //info.Func(registry, changes, entity, info);
      }
    }
  }

  //For now we can still go fetch global triggers here if they exist
  const on_status_change_triggers *global_triggers = registry.ctx().find<on_status_change_triggers>();
  if(global_triggers){
    for(const on_status_change_trigger_info &info : global_triggers->Triggers){
      if(info.Filter(registry, changes, entity, info)){
        logistics::add_edge(registry, entity, info.TriggerOwner);
        eng->enqueue_trigger(info, entity, changes);
        //info.Func(registry, changes, entity, info);
      }
    }
  }

  if(registry.any_of<combination_info>(entity)){
    auto &info = registry.get<combination_info>(entity);
    for(const auto &[kind, entities] : info.CurrentCombinations){
      for(entt::entity target : entities){
        //We are causing an update on another entity here, so add an edge to the graph.
        logistics::add_edge(registry, entity, target);
        eng->enqueue_update(target, entity, DEFAULT_TIMING_DELTA);
      }
    }
  }
}

//=========================================
void commit_attr_info_to_branch(entt::registry &registry, entt::entity entity){
  auto &local_storage = logistics::get_active_branch_local_changes_storage(registry);
  auto &status_effects_history = local_storage.get(entity);

  //get most recent version of intrinsics - ask the intrinsics storage if needed. 
  //Needs to be the same as when the snapshot (of current) was produced, i.e. in init_history_for_local_changes
  attributes_info_snapshot working_copy = status_effects_history.produce_snapshot();
  attributes_info_snapshot previous_current = logistics::get_most_recent_currents(registry, entity, logistics::changes_request::working_copy);

  attributes_info_changes changes = compute_diff(previous_current, working_copy);

  logistics::commit_changes_for_current_to_active_branch(registry, entity, changes);
 
  local_storage.remove(entity);

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