#include "simulation_engine.h"
#include "entt_utils.h"

namespace logistics {

  void start_simulating(entt::registry &registry){
    simulation_engine &eng = registry.ctx().emplace<simulation_engine>();
    eng.ActiveBranchHashForStatusChanges = entt::hashed_string::value("branch 0 - status_changes");
  }

  //------------------------------------
  void commit_changes_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes){
    simulation_engine *sim = registry.ctx().find<simulation_engine>();
    assert(sim);
    
    //TODO  if many changes go to the same entity on the same branch : function to squash changes together
    auto &status_changes_storage = registry.storage<attributes_info_changes>(sim->ActiveBranchHashForStatusChanges);
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

    for(const auto &[hash, param_pair] : changes.ModifiedParams){
      auto &&storage = registry.storage<parameter>(hash);
      if(param_pair.second.DT == data_type::null){ //deletion
        attr_info.CurrentParamValues.erase(hash);
        bool ret = storage.remove(entity);
        assert(ret);
      } else  { //modification
        attr_info.CurrentParamValues.insert_or_assign(hash, param_pair.second);
        utils::emplace_or_replace<parameter>(registry, entity, hash, param_pair.second);
      }
    }

    for(const auto &[hash, smt_val] : changes.ModifiedStatuses){
      auto &&storage = registry.storage<void>(hash);
      if(smt_val == smt::removed){
        attr_info.CurrentStatusHashes.erase(hash);
        bool ret = storage.remove(entity);
        assert(ret);
      } else {
        attr_info.CurrentStatusHashes.insert(hash);
        storage.emplace(entity);
      }
    }

  }
  
}