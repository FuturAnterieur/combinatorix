#include "history_manager.h"
#include "effect.h"
#include "status.h"

#include <entt/entity/registry.hpp>

namespace logistics{

  //==================================================================
  void history_manager::set_registry(entt::registry *registry){
    _Registry = registry;
    ActiveBranchName = "b0";
    CurrentAttrHistory.set_registry(registry);
    CurrentAttrHistory.set_branch_name(ActiveBranchName);
    
    IntrinsicAttrHistory.set_registry(registry);
    IntrinsicAttrHistory.set_branch_name(ActiveBranchName);

    LocalAttrHistory.set_registry(registry);
    LocalAttrHistory.set_branch_name(ActiveBranchName);

    std::string long_string_changes = ActiveBranchName  + std::string(entt::type_name<attributes_info_history>().value());
    ActiveBranchHashForStatusEffects = entt::hashed_string::value(long_string_changes.data());
  }

  //==================================================================
  void history_manager::set_stable_values(entt::entity entity, const attributes_info_short_changes &changes){
    auto cumul = cumul_changes_from_short(changes, entt::null);
    CurrentAttrHistory.set_stable_values(entity, cumul);
    IntrinsicAttrHistory.set_stable_values(entity, cumul);
    paste_changes_to_official_registry(_Registry, cumul, entity);
  }

  //==================================================================
  void history_manager::init_local_changes(entt::entity entity){
    LocalAttrHistory.get_changes_storage().remove(entity);
    LocalAttrHistory.init_history_starting_point(entity, IntrinsicAttrHistory.get_most_recent_snapshot(entity));
  }

  //==================================================================
  void history_manager::clear_local_changes(entt::entity entity){
    LocalAttrHistory.get_changes_storage().remove(entity);
  }

  //==================================================================
  void history_manager::commit_local_changes(entt::entity entity, const attributes_info_cumulative_changes &changes){
    if(!LocalAttrHistory.get_changes_storage().contains(entity)){
      LocalAttrHistory.init_history_starting_point(entity, IntrinsicAttrHistory.get_most_recent_snapshot(entity));
    }
    
    LocalAttrHistory.commit_changes(entity, changes, 0, false);
  }

  //------------------------------------
  void history_manager::commit_changes_for_current_to_active_branch(entt::entity entity, const attributes_info_cumulative_changes &changes, timing_t timing){
    CurrentAttrHistory.commit_changes(entity, changes,  timing, true);
  }

  //================================================================
  void history_manager::undo_changes_to_registry(){ //not used for now, and never tested, not even in the old API
    
    //TODO : refactor with smth like a branch_changes_storage class
    /*auto &current_changes_storage = get_active_branch_current_changes_storage();
    auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{current_changes_storage};

    for(entt::entity entity : status_changes_view){
      auto &stable_snapshot = CurrentAttrHistory.get_stable_storage().get(entity);
      
      auto &history = current_changes_storage.get(entity);
      attributes_info_changes cumulative_changes;
      attributes_info_snapshot current_snapshot = history.produce_snapshot();

      attributes_info_changes reverse_changes = compute_diff(current_snapshot, stable_snapshot);

      attributes_info_short_changes sr_changes = short_changes_from_changes(reverse_changes);
      paste_changes_to_official_registry(_Registry, sr_changes, entity);
    }*/
  }

  //=================================================================
  void history_manager::commit_changes_for_intrinsics_to_active_branch(entt::entity entity,  const attributes_info_cumulative_changes &changes, timing_t timing){
    IntrinsicAttrHistory.commit_changes(entity, changes, timing, false);
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
    CurrentAttrHistory.merge_to_reality(upper_bound, true);

    IntrinsicAttrHistory.merge_to_reality(upper_bound, false);

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
    se_storage.clear();
  }

  //===============================================================
  status_changes_storage_t &history_manager::get_active_branch_current_changes_storage(){
    
    return CurrentAttrHistory.get_changes_storage();
  }

  //=================================================================
  status_changes_storage_t &history_manager::get_active_branch_intrinsics_changes_storage(){
    
    return IntrinsicAttrHistory.get_changes_storage();
  }

  //=================================================================
  status_changes_storage_t &history_manager::get_active_branch_local_changes_storage(){
    
    return LocalAttrHistory.get_changes_storage();
  }

  //==================================================================
  status_effect_changes_storage_t &history_manager::get_active_branch_status_effects_changes_storage(){
    
    return _Registry->storage<status_effects_affecting_history>(ActiveBranchHashForStatusEffects);
  }

  //===============================================
  attributes_info_snapshot history_manager::get_most_recent_intrinsics( entt::entity entity){
    return IntrinsicAttrHistory.get_most_recent_snapshot(entity);
  }

  //===============================================
  attributes_info_snapshot history_manager::get_most_recent_currents( entt::entity entity){
    return CurrentAttrHistory.get_most_recent_snapshot(entity);
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