#include "game_logic.h"
#include "logistics/include/combine.h"
#include "status_structs.h"
#include "action.h"
#include "logistics/include/history_storage.h"

#define DEFAULT_UPDATE_DELTA 0

namespace engine{
  
  game_logic::game_logic(entt::registry *registry){
    HistoryManager.reset(new logistics::history_manager());
    set_registry(registry);
  }

  //==============================================================
  void game_logic::set_registry(entt::registry *registry){
    assert(registry);
    _Registry = registry;
    HistoryManager->set_registry(registry);
  }

  //==============================================================
  void game_logic::run_simulation(const std::function<void(game_logic *)> &request){
    
    CurrentSimulationData.reset(new simulation_data());
    request(this);

    CurrentSimulationData->EndTiming = std::numeric_limits<timing_t>::max();
    CurrentSimulationData->Finished = false;
    while(!CurrentSimulationData->Finished){
      CurrentSimulationData->run_one_timing();
    }
    HistoryManager->merge_active_branch_to_reality(CurrentSimulationData->EndTiming);
  }

  //==============================================================
  void game_logic::init_attributes(entt::entity entity, const attributes_info_short_changes &changes){
    HistoryManager->set_stable_values(entity, changes);
  }

  //==============================================================
  void game_logic::change_intrinsics(entt::entity entity, const attributes_info_short_changes &changes){
    
    auto snapshot = HistoryManager->get_most_recent_intrinsics(entity);
    auto cumul = cumul_changes_from_short(changes, CurrentSimulationData->ChangesContext.OriginatingEntity);
    

    //Change changers (i.e. pre-change triggers)
    //TODO : Globals
    //TODO : probably provide detailed changes to the trigger functions
    
    //Handles merge conflicts by deferring them to the diff_merger used by history
    logistics::history_storage<changes_category::intrinsics> temp_history;
    temp_history.set_registry(_Registry);
    temp_history.set_branch_name("temp");
    temp_history.init_history_starting_point(entity, snapshot);
    auto next_candidate = snapshot;
    paste_cumulative_changes(cumul, next_candidate);
    auto previous_candidate = next_candidate;
    
    temp_history.commit_changes(entity, cumul, 0, false);

    if(pre_change_triggers_affecting *triggers = _Registry->try_get<pre_change_triggers_affecting>(entity); triggers){
      for(auto trig_ent : triggers->TriggerEntities){
        assert(_Registry->any_of<pre_change_trigger_info>(trig_ent));
        const auto &info = _Registry->get<pre_change_trigger_info>(trig_ent);
        if(info.Filter(this, cumul, entity, info)){
          info.Func(this, cumul, entity, info);

          //this will work for non-incremental parameters, mostly for the case of change cancellation
          next_candidate = snapshot;
          paste_cumulative_changes(cumul, next_candidate);
          auto actual = compute_diff(previous_candidate, next_candidate);

          temp_history.commit_changes(entity, cumul_changes_from_long(actual), 0, false);
          previous_candidate = next_candidate;
        }
      }
    }
    
    auto candidate = temp_history.get_most_recent_snapshot(entity);
    attributes_info_changes actual_changes = compute_diff(snapshot, candidate);

    temp_history.clear_storages();

    if(changes_empty(actual_changes)) {
      return;
    }

    //Question : maybe commit the cumul even if there are no actual changes????
    HistoryManager->commit_changes_for_intrinsics_to_active_branch(entity, cumul, CurrentSimulationData->CurrentTiming);
    CurrentSimulationData->record_intrinsic_attrs_change(entity);
    CurrentSimulationData->enqueue_update(entity, DEFAULT_UPDATE_DELTA, this);
  }

  void game_logic::change_actives(entt::entity entity, const attributes_info_short_changes &changes){
    auto cumul = cumul_changes_from_short(changes, CurrentSimulationData->ChangesContext.OriginatingEntity);
    HistoryManager->commit_local_changes(entity, cumul);
  }

  void game_logic::add_on_status_change_trigger(entt::entity entity, on_status_change_trigger_info &info){
    on_status_change_triggers &triggers = _Registry->get_or_emplace<on_status_change_triggers>(entity);
    triggers.Triggers.push_back(info);
  }

  void game_logic::add_global_on_status_change_trigger(on_status_change_trigger_info &info){
    on_status_change_triggers *triggers = _Registry->ctx().find<on_status_change_triggers>();
    if(!triggers){
      _Registry->ctx().emplace<on_status_change_triggers>();
      triggers = &_Registry->ctx().get<on_status_change_triggers>();
    }
    triggers->Triggers.push_back(info);
  }

  //=========================================================
  entt::entity game_logic::create_status_effect(entt::entity originating_entity, const status_effect_apply_func_t &apply_func){
    auto eff_ent = _Registry->create();
    auto &eff_info = _Registry->emplace<status_effect_info>(eff_ent);
    eff_info.ApplyFunc = apply_func;
    eff_info.OriginatingEntity = originating_entity;
    auto &owned_effs = _Registry->get_or_emplace<status_effects_owned>(originating_entity);
    owned_effs.EffectEntities.push_back(eff_ent);

    return eff_ent;
  }

  //==================================================
  void game_logic::add_status_effect(entt::entity affected_entity, entt::entity eff_entity){
    if(!_Registry->any_of<status_effect_info>(eff_entity)){
      return;
    }
  
    status_effects_affecting curr = HistoryManager->get_most_recent_status_effects(affected_entity);
    curr.EffectEntities.push_back(eff_entity);
    HistoryManager->commit_status_effects_to_active_branch(affected_entity, curr, CurrentSimulationData->CurrentTiming);

    auto &eff_info = _Registry->get<status_effect_info>(eff_entity);
    link(*_Registry, eff_info.OriginatingEntity, affected_entity); 
    
    //Sort Infos according to the current rules about Status Effect Modification priority, then
    CurrentSimulationData->record_status_effect_change(affected_entity);
    CurrentSimulationData->enqueue_update(affected_entity, DEFAULT_UPDATE_DELTA, this); 
  }

  //=====================================================================
  void game_logic::remove_status_effect(entt::entity affected_entity, entt::entity eff_entity){
    if(!_Registry->any_of<status_effect_info>(eff_entity)){
      return;
    }
  
    status_effects_affecting curr = HistoryManager->get_most_recent_status_effects(affected_entity);

    curr.EffectEntities.erase(std::remove_if(curr.EffectEntities.begin(),
                                    curr.EffectEntities.end(),
                                    [=](const entt::entity eff_ent)-> bool 
                                    { return eff_ent == eff_entity; }), 
                    curr.EffectEntities.end());

    HistoryManager->commit_status_effects_to_active_branch(affected_entity, curr, CurrentSimulationData->CurrentTiming);

    auto &eff_info = _Registry->get<status_effect_info>(eff_entity);

    unlink(*_Registry, eff_info.OriginatingEntity, affected_entity);
    
    //Sort Infos according to the current rules about Status Effect Modification priority, then
    
    CurrentSimulationData->record_status_effect_change(affected_entity);
    CurrentSimulationData->enqueue_update(affected_entity,  DEFAULT_UPDATE_DELTA, this);
  }

  //=====================================================================
  entt::entity game_logic::create_pre_change_trigger(const pre_change_trigger_info &info)
  {
    entt::entity trigger_entity = _Registry->create();
    _Registry->emplace<pre_change_trigger_info>(trigger_entity, info);
    return trigger_entity;
  }

  //=====================================================================
  void game_logic::add_pre_change_trigger(entt::entity affected_entity, entt::entity trigger_entity)
  {
    if(!_Registry->any_of<pre_change_trigger_info>(trigger_entity)){
      return;
    }

    //TODO : History management for pre_change_triggers (actually, probably for all of that sort; in history_storage, create an entity list abstraction).
    auto &triggers = _Registry->get_or_emplace<pre_change_triggers_affecting>(affected_entity);
    triggers.TriggerEntities.push_back(trigger_entity);

    auto &trig_info = _Registry->get<pre_change_trigger_info>(trigger_entity);
    link(*_Registry, trig_info.Owner, affected_entity); //Really???? Still logical to do here.
    
  }

  //=====================================================================
  attributes_info_snapshot game_logic::get_active_snapshot(entt::entity entity){
    return HistoryManager->get_active_snapshot(entity);
  }

  //=====================================================================
  parameter_view_t game_logic::get_parameter_view(entt::id_type hash){
    return parameter_view_t{_Registry->storage<parameter>(hash)};
  }

  //=====================================================================
  status_view_t game_logic::get_status_view(entt::id_type hash){
    return status_view_t{_Registry->storage<void>(hash)};
  }

  //=====================================================================
  void game_logic::update_status(entt::entity entity){
    HistoryManager->init_local_changes(entity);

    status_effects_affecting effs = HistoryManager->get_most_recent_status_effects(entity);
    for(const auto &eff_entity : effs.EffectEntities){
      status_effect_info &eff_info = _Registry->get<status_effect_info>(eff_entity);
      set_context_originating_entity(eff_entity);
      eff_info.ApplyFunc(this, entity, eff_info.OriginatingEntity);
    }
    
    //What to do with the originating entity after that????
    set_context_originating_entity(entt::null);

    auto working_copy = HistoryManager->get_active_snapshot(entity);
    attributes_info_snapshot previous_current = HistoryManager->get_most_recent_currents(entity);

    attributes_info_changes actual_changes = compute_diff(previous_current, working_copy);

    HistoryManager->clear_local_changes(entity);
    
    if(changes_empty(actual_changes)){
      return;
    }

    HistoryManager->commit_changes_for_current_to_active_branch(entity, cumul_changes_from_long(actual_changes), CurrentSimulationData->CurrentTiming);
    activate_status_change_triggers(entity, actual_changes);
  }

  //==========================================================
  void game_logic::activate_status_change_triggers(entt::entity entity, const attributes_info_changes &changes){
      

    if(on_status_change_triggers *triggers = _Registry->try_get<on_status_change_triggers>(entity); triggers){
      for(const on_status_change_trigger_info &info : triggers->Triggers){
        if(info.Filter(this, changes, entity, info)){
          CurrentSimulationData->enqueue_trigger(info, entity, changes, this);
          //info.Func(registry, changes, entity, info);
        }
      }
    }

    //For now we can still go fetch global triggers here if they exist
    const on_status_change_triggers *global_triggers = _Registry->ctx().find<on_status_change_triggers>();
    if(global_triggers){
      for(const on_status_change_trigger_info &info : global_triggers->Triggers){
        if(info.Filter(this, changes, entity, info)){
          CurrentSimulationData->enqueue_trigger(info, entity, changes, this);
          //info.Func(registry, changes, entity, info);
        }
      }
    }

    if(_Registry->any_of<combination_info>(entity)){
      auto &info = _Registry->get<combination_info>(entity);
      for(const auto &[kind, entities] : info.CurrentCombinations){
        for(entt::entity target : entities){
          //We are causing an update on another entity here, so add an edge to the graph.
          CurrentSimulationData->enqueue_update(target, 1, this);
        }
      }
    }
  }

  void game_logic::set_context_originating_entity(entt::entity entity){
    CurrentSimulationData->ChangesContext.OriginatingEntity = entity;
  }

  void game_logic::run_one_timing(){
    CurrentSimulationData->run_one_timing();
  }
}
