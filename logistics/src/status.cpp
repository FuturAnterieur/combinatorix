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
bool init_intrinsic_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value){
  
  entt::id_type hash = entt::hashed_string::value(param_name.data());
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

 
  attributes_info &attr_info = registry.get<attributes_info>(entity);
  attr_info.CurrentParamValues.insert_or_assign(hash, parameter{dt, value});
  attr_info.IntrinsicParamValues.insert_or_assign(hash, parameter{dt, value}); //also set original values because it isn't edited through a Modifier

  parameter new_param{dt,value};
  auto &&specific_storage = registry.storage<parameter>(hash);
  if(!specific_storage.contains(entity)){
    specific_storage.emplace(entity, dt, value);
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
    if(param_pair.second.DT == data_type::null){ //deletion
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

//=====================================
bool assign_intrinsic_attributes_changes(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes){
  if(!registry.any_of<attributes_info>(entity)){
    return false;
  }

  logistics::start_simulating(registry, entity);

  attributes_info &stable_info = registry.get<attributes_info>(entity);

  //attributes_info_snapshot snapshot_of_current{stable_info.CurrentStatusHashes, stable_info.CurrentParamValues};
  attributes_info_reference ref_to_intrinsic{stable_info.IntrinsicStatusHashes, stable_info.IntrinsicParamValues};

  paste_attributes_changes(registry, entity, changes, ref_to_intrinsic, false);
  update_status_effects(registry, entity); //assumes this launches commit after calculating status effect consequences

  logistics::simulation_engine *eng = logistics::get_simulation_engine(registry);
  eng->execute_stuff();
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
        eng->enqueue_update(entity, 1); //WHAT VALUE 2 GIVE
        //update_status_effects(registry, target);
      }
    }
  }
}

//=========================================
void commit_attr_info_to_branch(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity){

  auto &local_change_tracker = registry.get<logistics::local_change_tracker>(entity);
  attributes_info_snapshot new_snapshot = local_change_tracker.produce_active_snapshot();

  attributes_info_changes changes = compute_diff(snapshot, new_snapshot);

  auto &storage = logistics::get_active_branch_status_changes_storage(registry);
  
  if(storage.contains(entity)){
    attributes_info_changes already_commited_changes = storage.get(entity);
    attributes_info_changes merged_changes;
    logistics::simple_change_merger merger;
    logistics::merge_result ret = merger.merge_changes(already_commited_changes, changes, merged_changes);
    if(ret == logistics::merge_result::conflict){
      //TODO : restoration logic in case of conflict
      //namely, status effects!!!!
      return;
    }
    attributes_info_changes &final_changes = storage.get(entity);
    final_changes = merged_changes;

    attributes_info_changes difference = compute_changes_diff(already_commited_changes, merged_changes);
    std::swap(changes, difference);
  } else {
    logistics::commit_changes_to_active_branch(registry, entity, changes);
  }
  
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