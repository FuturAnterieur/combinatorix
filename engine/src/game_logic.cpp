#include "game_logic.h"
#include "logistics/include/status.h"

void game_logic::set_registry(entt::registry *registry){
  assert(registry);
  _Registry = registry;
  HistoryManager->set_registry(registry);
}

void game_logic::init_attributes(entt::entity entity, const attributes_info_short_changes &changes){
  HistoryManager->set_stable_values(entity, changes);
}

void game_logic::change_intrinsics(entt::entity entity, const attributes_info_short_changes &changes){
  
  auto snapshot = HistoryManager->get_most_recent_intrinsics(entity);
  auto candidate = snapshot;
  paste_attributes_changes(changes, attributes_info_reference{candidate});
  attributes_info_changes actual_changes = compute_diff(snapshot, candidate);
  
  if(changes_empty(actual_changes)) {
    return;
  }

  HistoryManager->commit_changes_for_intrinsics_to_active_branch(entity, short_changes_from_changes(actual_changes), CurrentSimulationData->ChangesContext.OriginatingEntity, CurrentSimulationData->CurrentTiming);
  CurrentSimulationData->record_intrinsic_attrs_change(entity);
  CurrentSimulationData->enqueue_update(entity, entity, DEFAULT_TIMING_DELTA);
}

void game_logic::change_actives(entt::entity entity, const attributes_info_short_changes &changes){
  HistoryManager->commit_local_changes(entity, changes, CurrentSimulationData->ChangesContext.OriginatingEntity);
}

void game_logic::update_status(entt::entity entity){
   using namespace logistics;
  
  attributes_info &attr_info = _Registry->get<attributes_info>(entity);
  status_effects_affecting effs = HistoryManager->get_most_recent_status_effects(entity);
  
  for(const auto &eff_entity : effs.EffectEntities){
    status_effect_info &eff_info = _Registry->get<status_effect_info>(eff_entity);
    CurrentSimulationData->ChangesContext.OriginatingEntity = eff_entity;
    eff_info.ApplyFunc(*_Registry, attr_info, entity, eff_info.OriginatingEntity); //TODO replace registry argument with a game_logic*
  }
  
  //What to do with the originating entity after that????
  sim->ChangesContext.OriginatingEntity = entt::null;

  auto &working_copy = HistoryManager->get_active_snapshot(entity);
  attributes_info_snapshot previous_current = HistoryManager->get_most_recent_currents(entity);

  attributes_info_changes actual_changes = compute_diff(previous_current, working_copy);

  HistoryManager->reset_local_changes(entity);
  if(changes_empty(actual_changes)){
    return;
  }

  HistoryManager->commit_changes_for_current_to_active_branch(entity, short_changes_from_changes(actual_changes), entt::null, CurrentSimulationData->CurrentTiming);
  activate_status_change_triggers(entity, actual_changes);
}

//==========================================================
void game_logic::activate_status_change_triggers(entt::entity entity, const attributes_info_changes &changes){
    

  if(on_status_change_triggers *triggers = _Registry->try_get<on_status_change_triggers>(entity); triggers){
    for(const on_status_change_trigger_info &info : triggers->Triggers){
      if(info.Filter(registry, changes, entity, info)){
        CurrentSimulationData->enqueue_trigger(info, entity, changes);
        //info.Func(registry, changes, entity, info);
      }
    }
  }

  //For now we can still go fetch global triggers here if they exist
  const on_status_change_triggers *global_triggers = _Registry->ctx().find<on_status_change_triggers>();
  if(global_triggers){
    for(const on_status_change_trigger_info &info : global_triggers->Triggers){
      if(info.Filter(registry, changes, entity, info)){
        CurrentSimulationData->enqueue_trigger(info, entity, changes);
        //info.Func(registry, changes, entity, info);
      }
    }
  }

  if(_Registry->any_of<combination_info>(entity)){
    auto &info = _Registry->get<combination_info>(entity);
    for(const auto &[kind, entities] : info.CurrentCombinations){
      for(entt::entity target : entities){
        //We are causing an update on another entity here, so add an edge to the graph.
        CurrentSimulationData->enqueue_update(target, entity, DEFAULT_TIMING_DELTA);
      }
    }
  }
}
