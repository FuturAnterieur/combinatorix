#include "simulation_engine.h"
#include "entt_utils.h"
#include <entt/meta/meta.hpp>

namespace logistics {

  simulation_engine::simulation_engine(entt::registry *reg) : registry(reg) {
    
  }

  //------------------------------------
  void simulation_engine::enqueue_trigger(const on_status_change_trigger_info &info, entt::entity triggering_entity, const attributes_info_changes &changes){
    timing_t absolute_trigger_launch_time = info.TimeDelta + CurrentTiming;
    status_trigger_executable executable{info, changes, triggering_entity};

    auto &container = ExecutablesPerTimingLevel.emplace(absolute_trigger_launch_time, executables_on_same_timing_container{}).first->second;
    container.AbsoluteTiming = absolute_trigger_launch_time; //should be no-op if it was found
    container.Executables.emplace_back(executable_common_data{executable_type::status_trigger, info.TriggerOwner, executable});

    /*for(int64_t i = 0; i < static_cast<int64_t>(TriggersPerTimingLevel.size()); i++){
      if(TriggersPerTimingLevel.at(i).AbsoluteTiming == absolute_trigger_launch_time){
        TriggersPerTimingLevel.at(i).Triggers.push_back(info);
      } else if (i )

      if(it->AbsoluteTiming == absolute_trigger_launch_time){
        it->Triggers.push_back(info);
        placed = true;
      }
    }*/

  }

  //===================================
  void simulation_engine::enqueue_update(entt::entity entity, timing_t timing){
    if(UpdateRequestsFromCurrentTiming.find(entity) != UpdateRequestsFromCurrentTiming.end()){
      return;
    }
    UpdateRequestsFromCurrentTiming.insert(entity);

    timing_t absolute_launch_time = timing + CurrentTiming;
    entity_update_executable executable{entity};
    
    auto &container = ExecutablesPerTimingLevel.emplace(absolute_launch_time, executables_on_same_timing_container{}).first->second;
    container.AbsoluteTiming = absolute_launch_time; //should be no-op if it was found
    container.Executables.emplace_back(executable_common_data{executable_type::update, entity, executable});
  }

  //===================================
  void simulation_engine::execute_stuff(){
    while(!ExecutablesPerTimingLevel.empty()){
      auto it = ExecutablesPerTimingLevel.begin();
      if(it == ExecutablesPerTimingLevel.end()){
        return;
      }

      CurrentTiming = it->first;
      UpdateRequestsFromCurrentTiming.clear();
      while(!it->second.Executables.empty()) {
        auto exec = it->second.Executables.front();
        it->second.Executables.pop_front();
        exec.Func(*registry);
      }
      

      ExecutablesPerTimingLevel.erase(it);
    }
  }

  //====================================================================================

  void start_simulating(entt::registry &registry, entt::entity start){
    simulation_engine &eng = registry.ctx().emplace<simulation_engine>(&registry);
    eng.ActiveBranchName = "branch 0";
    
    std::string_view struct_attributes_info_history = entt::type_name<attributes_info_history>().value();
    
    std::string current_branch_name = eng.ActiveBranchName + " - current " + std::string(struct_attributes_info_history);
    eng.ActiveBranchHashForCurrentStatusChanges = entt::hashed_string::value(current_branch_name.data());
    std::string intrinsic_branch_name =  eng.ActiveBranchName + " - intrinsic " + std::string(struct_attributes_info_history);
    eng.ActiveBranchHashForIntrinsicStatusChanges = entt::hashed_string::value(intrinsic_branch_name.data());
    std::string local_branch_name = eng.ActiveBranchName + " - local " + std::string(struct_attributes_info_history);
    eng.ActiveBranchHashForLocalStatusChanges = entt::hashed_string::value(local_branch_name.data());

    //eng.StartingNode = start;
    eng.CurrentNode = start;
    eng.CurrentTiming = 0;
  }

  //------------------------------------
  simulation_engine *get_simulation_engine(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    return sim;
  }

  

  //------------------------------------
  void add_edge(entt::registry &registry, entt::entity from, entt::entity to){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);

    sim->DynamicGraph.add_edge(from, to);
    sim->CurrentNode = to;
  }

  //-----------------------------------
  bool graph_has_cycle(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);

    return sim->DynamicGraph.find_cycle_simple();
  }

  //------------------------------------
  void commit_changes_for_current_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes){
    //TODO  if many changes go to the same entity on the same branch : function to squash changes together
    auto &status_changes_storage = get_active_branch_current_changes_storage(registry);
    if(!status_changes_storage.contains(entity)){
      attributes_info_snapshot starting_point;
      auto &stable_info = registry.get<attributes_info>(entity);
      starting_point.StatusHashes = stable_info.CurrentStatusHashes;
      starting_point.ParamValues = stable_info.CurrentParamValues;

      status_changes_storage.emplace(entity, starting_point);
    }

    auto &history = status_changes_storage.get(entity);

    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);

    history.add_changes(sim->CurrentTiming, changes);
  }

  //=================================================================
  void commit_changes_for_intrinsics_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes){
    auto &status_changes_storage = get_active_branch_intrinsics_changes_storage(registry);
    if(!status_changes_storage.contains(entity)){
      attributes_info_snapshot starting_point;
      auto &stable_info = registry.get<attributes_info>(entity);
      starting_point.StatusHashes = stable_info.IntrinsicStatusHashes;
      starting_point.ParamValues = stable_info.IntrinsicParamValues;

      status_changes_storage.emplace(entity, starting_point);
    }

    auto &history = status_changes_storage.get(entity);

    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);

    history.add_changes(sim->CurrentTiming, changes);
  }

  //=============================
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const status_effects &info){
    //TODO
    //For now, status effects modifiers are always commited to the reality
    //This should change in the future.
    //But this also entails keeping track of combination_info, triggers, etc... on the side branch. Lots of boilerplate.
  }

  //=============================
  void merge_active_branch_to_reality(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim); 
    
    //TODO : refactor with smth like a branch_changes_storage class
    auto &current_changes_storage = get_active_branch_current_changes_storage(registry);
    auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{current_changes_storage};

    for(entt::entity entity : status_changes_view){
      apply_history_to_entity(registry, status_changes_view.get<attributes_info_history>(entity), entity, changes_category::current);
    }

    auto &intrinsic_changes_storage = get_active_branch_intrinsics_changes_storage(registry);
    auto view = entt::view<entt::get_t<attributes_info_history>>{intrinsic_changes_storage};

    for(entt::entity entity : view){
      apply_history_to_entity(registry, view.get<attributes_info_history>(entity), entity, changes_category::intrinsics);
    }

    //TODO : status effects and other stuff
    current_changes_storage.clear();
    registry.ctx().erase<simulation_engine>();
  }

  //================================
  void apply_history_to_entity(entt::registry &registry, const attributes_info_history &history, entt::entity entity, changes_category category){
    assert(registry.all_of<attributes_info>(entity));
    auto &attr_info = registry.get<attributes_info>(entity);

  
    attributes_info_changes cumulative_changes;
    history.cumulative_changes(cumulative_changes);

    if(category == changes_category::current){
      attributes_info_reference ref{attr_info.CurrentStatusHashes, attr_info.CurrentParamValues};
      paste_attributes_changes(registry, entity, cumulative_changes, ref, true);
    } else if (category == changes_category::intrinsics){
      attributes_info_reference ref{attr_info.IntrinsicStatusHashes, attr_info.IntrinsicParamValues};
      paste_attributes_changes(registry, entity, cumulative_changes, ref, true);
    }
  }
  
  //===============================================================
  status_changes_storage_t &get_active_branch_current_changes_storage(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    
    return registry.storage<attributes_info_history>(sim->ActiveBranchHashForCurrentStatusChanges);
  }

  //=================================================================
  status_changes_storage_t &get_active_branch_intrinsics_changes_storage(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    
    return registry.storage<attributes_info_history>(sim->ActiveBranchHashForIntrinsicStatusChanges);
  }

  //=================================================================
  status_changes_storage_t &get_active_branch_local_changes_storage(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    
    return registry.storage<attributes_info_history>(sim->ActiveBranchHashForLocalStatusChanges);
  }

  //===============================================
  attributes_info_snapshot get_most_recent_intrinsics(entt::registry &registry, entt::entity entity){
    attributes_info_snapshot stable_values;
    auto &info = registry.get<attributes_info>(entity);
    stable_values.StatusHashes = info.IntrinsicStatusHashes;
    stable_values.ParamValues = info.IntrinsicParamValues;
    
    auto &storage = logistics::get_active_branch_intrinsics_changes_storage(registry);
    if(storage.contains(entity)){
      simulation_engine *sim = registry.ctx().find<simulation_engine>();
      assert(sim); 
      
      auto &intrinsic_history = storage.get(entity);
      return intrinsic_history.produce_snapshot(sim->CurrentTiming); 
    }

    return stable_values;
  }

  //===============================================
  attributes_info_snapshot get_most_recent_currents(entt::registry &registry, entt::entity entity){
    attributes_info_snapshot stable_values;
    auto &info = registry.get<attributes_info>(entity);
    stable_values.StatusHashes = info.CurrentStatusHashes;
    stable_values.ParamValues = info.CurrentParamValues;
    
    auto &storage = logistics::get_active_branch_current_changes_storage(registry);
    if(storage.contains(entity)){
      simulation_engine *sim = registry.ctx().find<simulation_engine>();
      assert(sim); 
      
      auto &current_history = storage.get(entity);
      return current_history.produce_snapshot(sim->CurrentTiming); 
    }

    return stable_values;
  }
}