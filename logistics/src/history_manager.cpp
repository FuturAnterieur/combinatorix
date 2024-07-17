#include "history_manager.h"
#include "effect.h"
#include "status.h"

#include <entt/entity/registry.hpp>

namespace logistics{
  //====================================
  void history_manager::init_starting_point( status_changes_storage_t &storage, entt::entity entity, changes_category category){
    if(!storage.contains(entity)){
      attributes_info_snapshot starting_point;
      auto &stable_info = _Registry->get<attributes_info>(entity);
      
      if(category == changes_category::intrinsics) {
        starting_point.StatusHashes = stable_info.IntrinsicStatusHashes;
        starting_point.ParamValues = stable_info.IntrinsicParamValues;
      } else if (category == changes_category::current){
        starting_point.StatusHashes = stable_info.CurrentStatusHashes;
        starting_point.ParamValues = stable_info.CurrentParamValues;
      }

      storage.emplace(entity, starting_point);
    }

  }

  //------------------------------------
  void history_manager::commit_changes_for_current_to_active_branch(entt::entity entity, const attributes_info_changes &changes, entt::entity originating_entity, timing_t timing){
    //TODO  if many changes go to the same entity on the same branch : function to squash changes together
    auto &status_changes_storage = get_active_branch_current_changes_storage();
    init_starting_point(status_changes_storage, entity, changes_category::current);

    auto &history = status_changes_storage.get(entity);

    //TODO handle merge conflict (would have to revert changes to registry at least)
    attributes_info_state_at_timing state;
    state.Changes = short_changes_from_changes(changes);
    state.OriginatingEntity = originating_entity;
    //In the only use case for this function, OriginatingEntity will always be Null, and that is on purpose for now.

    history.add_changes(timing, state, PriorityCallback, _Registry);
    attributes_info_snapshot null_snapshot;
    attributes_info_reference ref(null_snapshot);
    paste_attributes_changes(*_Registry, entity, state.Changes, ref, true, false);
  }

  //================================================================
  void history_manager::undo_changes_to_registry(){ //not used for now
    
    //TODO : refactor with smth like a branch_changes_storage class
    auto &current_changes_storage = get_active_branch_current_changes_storage();
    auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{current_changes_storage};

    for(entt::entity entity : status_changes_view){
      auto &attr_info = _Registry->get<attributes_info>(entity);
      attributes_info_snapshot stable_snapshot{attr_info.CurrentStatusHashes, attr_info.CurrentParamValues};

      auto &history = current_changes_storage.get(entity);
      attributes_info_changes cumulative_changes;
      attributes_info_snapshot current_snapshot = history.produce_snapshot();

      attributes_info_changes reverse_changes = compute_diff(current_snapshot, stable_snapshot);

      attributes_info_short_changes sr_changes = short_changes_from_changes(reverse_changes);
      attributes_info_snapshot null_snapshot;
      attributes_info_reference ref(null_snapshot);
      paste_attributes_changes(*_Registry, entity, sr_changes, ref, true, false);
    }
  }

  //=================================================================
  void history_manager::commit_changes_for_intrinsics_to_active_branch( entt::entity entity,  const attributes_info_changes &changes, entt::entity originating_entity, timing_t timing){
    auto &status_changes_storage = get_active_branch_intrinsics_changes_storage();
    init_starting_point(status_changes_storage, entity, changes_category::intrinsics);

    auto &history = status_changes_storage.get(entity);

    //TODO handle merge conflict
    attributes_info_state_at_timing state;
    state.Changes = short_changes_from_changes(changes);
    //in the future : put player in ChangesContext for case of starting a simulation
    state.OriginatingEntity = originating_entity;

    history.add_changes(timing, state, PriorityCallback, _Registry);
  }

  //=============================
  void history_manager::commit_status_effects_to_active_branch( entt::entity entity, const sea_state_at_timing &info, timing_t timing){
    //This should change in the future.
    //But this also entails keeping track of combination_info, triggers, etc... on the side branch. Lots of boilerplate.
    auto &storage = get_active_branch_status_effects_changes_storage();
    if(!storage.contains(entity)){
      storage.emplace(entity);
    }

  
    auto &history = storage.get(entity);
    history.History.insert_or_assign(timing, info);
  }

  //=============================
  void history_manager::merge_active_branch_to_reality( timing_t upper_bound){
    
    //TODO : refactor with smth like a branch_changes_storage class
    auto &current_changes_storage = get_active_branch_current_changes_storage();
    auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{current_changes_storage};

    for(entt::entity entity : status_changes_view){
      apply_history_to_entity(status_changes_view.get<attributes_info_history>(entity), entity, changes_category::current, upper_bound);
    }

    auto &intrinsic_changes_storage = get_active_branch_intrinsics_changes_storage();
    auto view = entt::view<entt::get_t<attributes_info_history>>{intrinsic_changes_storage};

    for(entt::entity entity : view){
      apply_history_to_entity(view.get<attributes_info_history>(entity), entity, changes_category::intrinsics, upper_bound);
    }

    auto& se_storage = get_active_branch_status_effects_changes_storage();
    auto se_view = entt::view<entt::get_t<status_effects_affecting_history>>{se_storage};

    for(entt::entity entity : se_view){
      auto &new_se = se_view.get<status_effects_affecting_history>(entity);
      auto &real_se = _Registry->get_or_emplace<status_effects_affecting>(entity);
      auto it = new_se.History.begin();
      auto scout = it;
      scout++;
      while(scout != new_se.History.end() && scout->first < upper_bound){
        it++;
        scout++;
      }
      real_se.EffectEntities = it->second.EffectEntities;
    }
  
    //TODO : other stuff
    current_changes_storage.clear();
    intrinsic_changes_storage.clear();
    se_storage.clear();
  }

  //================================
  void history_manager::apply_history_to_entity( const attributes_info_history &history, entt::entity entity, changes_category category, timing_t upper_bound){
    assert(_Registry->all_of<attributes_info>(entity));
    auto &attr_info = _Registry->get<attributes_info>(entity);

  
    attributes_info_short_changes cumulative_changes;
    history.cumulative_changes(cumulative_changes, upper_bound);

    if(category == changes_category::current){
      attributes_info_reference ref{attr_info.CurrentStatusHashes, attr_info.CurrentParamValues};
      paste_attributes_changes(*_Registry, entity, cumulative_changes, ref, true, true);
    } else if (category == changes_category::intrinsics){
      attributes_info_reference ref{attr_info.IntrinsicStatusHashes, attr_info.IntrinsicParamValues};
      paste_attributes_changes(*_Registry, entity, cumulative_changes, ref, false, true);
    }
  }
  
  //===============================================================
  status_changes_storage_t &history_manager::get_active_branch_current_changes_storage(){
    
    return _Registry->storage<attributes_info_history>(ActiveBranchHashForCurrentStatusChanges);
  }

  //=================================================================
  status_changes_storage_t &history_manager::get_active_branch_intrinsics_changes_storage(){
    
    return _Registry->storage<attributes_info_history>(ActiveBranchHashForIntrinsicStatusChanges);
  }

  //=================================================================
  status_changes_storage_t &history_manager::get_active_branch_local_changes_storage(){
    
    return _Registry->storage<attributes_info_history>(ActiveBranchHashForLocalStatusChanges);
  }

  //==================================================================
  status_effect_changes_storage_t &history_manager::get_active_branch_status_effects_changes_storage(){
    
    return _Registry->storage<status_effects_affecting_history>(ActiveBranchHashForStatusEffects);
  }

  //===============================================
  attributes_info_snapshot history_manager::get_most_recent_intrinsics( entt::entity entity){
    attributes_info_snapshot stable_values;
    auto &info = _Registry->get<attributes_info>(entity);
    stable_values.StatusHashes = info.IntrinsicStatusHashes;
    stable_values.ParamValues = info.IntrinsicParamValues;
    
    auto &storage = get_active_branch_intrinsics_changes_storage();
    if(storage.contains(entity)){
      
      auto &intrinsic_history = storage.get(entity);
      return intrinsic_history.produce_snapshot(); 
    }

    return stable_values;
  }

  //===============================================
  attributes_info_snapshot history_manager::get_most_recent_currents( entt::entity entity){
    attributes_info_snapshot stable_values;
    auto &info = _Registry->get<attributes_info>(entity);
    stable_values.StatusHashes = info.CurrentStatusHashes;
    stable_values.ParamValues = info.CurrentParamValues;
    
    auto &storage = get_active_branch_current_changes_storage();
    if(storage.contains(entity)){
      
      auto &current_history = storage.get(entity);
      return current_history.produce_snapshot(); 
    }

    return stable_values;
  }

  //===============================================
  attributes_info_snapshot history_manager::get_active_snapshot(entt::entity entity){
    auto &local_storage = get_active_branch_local_changes_storage();
    if(local_storage.contains(entity)){
      auto &history = local_storage.get(entity);
      return history.produce_snapshot(); //most recent
    }
    return get_most_recent_currents(entity);
  }

  
  //===============================================
  status_effects_affecting history_manager::get_most_recent_status_effects( entt::entity entity){
    auto &storage = get_active_branch_status_effects_changes_storage();
    if(storage.contains(entity)){
      
      auto &current_history = storage.get(entity);
      return current_history.History.rbegin()->second;
    }
    
    status_effects_affecting stable_values;
    auto &info = _Registry->get_or_emplace<status_effects_affecting>(entity);
    stable_values.EffectEntities = info.EffectEntities;
    
    return stable_values;
  }
}