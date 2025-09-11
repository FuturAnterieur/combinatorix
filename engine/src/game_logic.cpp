#include "game_logic.h"
#include "logistics/include/combine.h"
#include "status_structs.h"
#include "action.h"
#include "move_request_history.h"
#include "logistics/include/history_storage.h"
#include "geometry/include/collision_processor.h"
#include "geometry/include/collision_tree.h"

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

  void game_logic::set_communicator(game_communicator *comm)
  {
    _Communicator = comm;
  }

  std::string game_logic::ask_question(size_t chan_index, const std::string &question_data)
  {
    if(!_Communicator){
      return "";
    }

    return _Communicator->ask_question(question_data, chan_index);
  }

  //==============================================================
  void game_logic::run_simulation(const std::function<void(game_logic *)> &request){
    
    CurrentSimulationData.reset(new simulation_data());
    request(this);

    CurrentSimulationData->EndTiming = std::numeric_limits<timing_t>::max();
    CurrentSimulationData->Finished = false;
    while(!CurrentSimulationData->Finished){
      CurrentSimulationData->run_one_timing();
      // of course at each timing we process moves
      // timing has priority over priority_info
      _Registry->sort<move_request_history>([&](entt::entity lhs, entt::entity rhs) {
        int lhs_prio = 1;
        int rhs_prio = 0;
        priority_request req{{{lhs, &lhs_prio}, {rhs, &rhs_prio}}};
        classic_priority_callback(req, _Registry);
        return lhs_prio > rhs_prio;
      });
  
      geometry::collision_processor processor(_Registry);
  
      auto mrh_view = _Registry->view<move_request_history>();
      for (entt::entity entity : mrh_view) {
        auto &history = mrh_view.get<move_request_history>(entity);
        //do_move will limit the movement of illegal moves
        for (const auto &delta : history.PendingRequests) {
          processor.do_move(geometry::move_request{entity, delta});
        }
      }
      _Registry->clear<move_request_history>();
    }

    HistoryManager->merge_active_branch_to_reality(CurrentSimulationData->EndTiming);

  }

  //==============================================================
  void game_logic::init_status(entt::entity entity, entt::id_type hash, smt val)
  {
    attributes_info_short_changes temp;
    temp.ModifiedStatuses.emplace(hash, val);
    init_attributes(entity, temp);
  }

  void game_logic::init_parameter(entt::entity entity, entt::id_type hash, parameter_value_t val)
  {
    attributes_info_short_changes temp;
    temp.ModifiedParams.emplace(hash, diff_from_set_val(val));
    init_attributes(entity, temp);
  }

  //==============================================================
  void game_logic::init_aabb_collider(entt::entity entity, const geometry::aabb_collider &collider)
  {
    if (!_Registry->any_of<geometry::position>(entity)) {
      return;
    }

    const auto &position = _Registry->get<geometry::position>(entity);
    // Important : check for collision first

    geometry::aabb absolute_aabb = collider.RelativeAABB;
    absolute_aabb.move_of(position.Value);
    geometry::collision_processor processor(_Registry);
    if (processor.aabb_collision_query(absolute_aabb)) {
      return;
    }

    _Registry->emplace<geometry::aabb_collider>(entity, collider);
    geometry::create_proxy(*_Registry, absolute_aabb, entity);
  }

  //==============================================================
  void game_logic::init_circle_collider(entt::entity entity, const geometry::circle_collider &collider)
  {
    if (!_Registry->any_of<geometry::position>(entity)) {
      return;
    }

    const auto &position = _Registry->get<geometry::position>(entity);
    // Important : check for collision first
    geometry::collision_processor processor(_Registry);
    if (processor.circle_collision_query(position.Value, collider.Radius)) {
      return;
    }

    _Registry->emplace<geometry::circle_collider>(entity, collider);
    geometry::create_proxy(*_Registry, geometry::aabb_from_circle(position.Value, collider.Radius), entity);
  }

  //==============================================================
  void game_logic::enqueue_move_request(entt::entity entity, const glm::vec2 &delta)
  {
    geometry::collision_processor processor(_Registry);
    if (!processor.is_move_allowed(geometry::move_request{entity, delta})) {
      return;
    }

    auto &history = _Registry->get_or_emplace<move_request_history>(entity);
    history.PendingRequests.push_back(delta);
  
  }

  //==============================================================
  void game_logic::init_attributes(entt::entity entity, const attributes_info_short_changes &changes){
    HistoryManager->set_stable_values(entity, changes);
  }

  //==============================================================
  void game_logic::change_intrinsics(entt::entity entity, const attributes_info_short_changes &changes){
    
    auto snapshot = HistoryManager->get_most_recent_intrinsics(entity);
    auto proposed = cumul_changes_from_short(changes, CurrentSimulationData->ChangesContext.OriginatingEntity);
    attributes_info_snapshot candidate = snapshot;
    paste_cumulative_changes(proposed, candidate);
    attributes_info_changes detailed_changes = compute_diff(snapshot, candidate);

    //Change changers (i.e. pre-change triggers)
    //TODO : Globals
    
    change_edit_history hist;
    for(const auto &[hash, change] : proposed.StatusesChanges.Changes){
      hist.record_status_edit(hash, change);
    }

    for(const auto &[hash, change] : proposed.ParamChanges.Changes){
      hist.record_param_edit(hash, change);
    }
    
    if(pre_change_triggers_affecting *triggers = _Registry->try_get<pre_change_triggers_affecting>(entity); triggers){
      for(auto trig_ent : triggers->TriggerEntities){
        assert(_Registry->any_of<pre_change_trigger_info>(trig_ent));
        const auto &info = _Registry->get<pre_change_trigger_info>(trig_ent);
        if(info.Filter(this, detailed_changes, proposed, entity, info)){
          info.Func(this, detailed_changes, proposed, entity, info, hist);
        }
      }
    }
    
    attributes_info_cumulative_changes modified_changes = hist.create_cumul_changes(this);
    attributes_info_snapshot candidate2 = snapshot;
    paste_cumulative_changes(modified_changes, candidate2);
    
    attributes_info_changes actual_changes = compute_diff(snapshot, candidate2);

    if(changes_empty(actual_changes)) {
      return;
    }

    //Question : maybe commit the cumul even if there are no actual changes????
    HistoryManager->commit_changes_for_intrinsics_to_active_branch(entity, modified_changes, CurrentSimulationData->CurrentTiming);
    CurrentSimulationData->record_intrinsic_attrs_change(entity);
    CurrentSimulationData->enqueue_update(entity, DEFAULT_UPDATE_DELTA, this);
  }

  //=========================================================================
  void game_logic::change_actives(entt::entity entity, const attributes_info_short_changes &changes){
    auto cumul = cumul_changes_from_short(changes, CurrentSimulationData->ChangesContext.OriginatingEntity);
    HistoryManager->add_local_changes(entity, cumul);
  }

  void game_logic::set_originating_entity(entt::entity entity){
    CurrentSimulationData->ChangesContext.OriginatingEntity = entity;
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
    return HistoryManager->get_most_recent_currents(entity);
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

    HistoryManager->commit_local_changes(entity);
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
