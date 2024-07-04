#include "simulation_engine.h"
#include "entt_utils.h"
#include <entt/meta/meta.hpp>

namespace logistics {

  void start_simulating(entt::registry &registry, entt::entity start){
    simulation_engine &eng = registry.ctx().emplace<simulation_engine>();
    eng.ActiveBranchName = "branch 0";
    
    std::string_view struct_attributes_info_changes = entt::type_name<attributes_info_changes>().value();
    std::string full_branch_name = eng.ActiveBranchName + " - " + std::string(struct_attributes_info_changes);

    eng.ActiveBranchHashForStatusChanges = entt::hashed_string::value(full_branch_name.data());
    //eng.StartingNode = start;
    eng.CurrentNode = start;
  }

  //------------------------------------
  void enter_new_entity(entt::registry &registry, entt::entity from, entt::entity to){
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
  void commit_changes_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes){
    //TODO  if many changes go to the same entity on the same branch : function to squash changes together
    auto &status_changes_storage = get_active_branch_status_changes_storage(registry);
    status_changes_storage.emplace(entity, changes);
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
    
    //TODO  if many changes go to the same entity on the same branch : function to squash changes together
    auto &status_changes_storage = registry.storage<attributes_info_changes>(sim->ActiveBranchHashForStatusChanges);
    auto status_changes_view = entt::view<entt::get_t<attributes_info_changes>>{status_changes_storage};

    for(entt::entity entity : status_changes_view){
      apply_changes_to_entity(registry, status_changes_view.get<attributes_info_changes>(entity), entity);
    }

    //TODO : status effects and other stuff

    status_changes_storage.clear();
    registry.ctx().erase<simulation_engine>();
  }

  //================================
  void apply_changes_to_entity(entt::registry &registry, const attributes_info_changes &changes, entt::entity entity){
    assert(registry.all_of<attributes_info>(entity));
    auto &attr_info = registry.get<attributes_info>(entity);

    attributes_info_reference ref{attr_info.CurrentStatusHashes, attr_info.CurrentParamValues};
    paste_attributes_changes(registry, entity, changes, ref, true);
  }
  
  status_changes_storage_t &get_active_branch_status_changes_storage(entt::registry &registry){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    
    return registry.storage<attributes_info_changes>(sim->ActiveBranchHashForStatusChanges);
  }

}