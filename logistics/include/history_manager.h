#pragma once


#include "history_utils.h"
#include "priority.h"
#include <string>

namespace logistics {

  class history_manager {
    entt::id_type ActiveBranchHashForCurrentStatusChanges;
    entt::id_type ActiveBranchHashForIntrinsicStatusChanges;
    entt::id_type ActiveBranchHashForLocalStatusChanges; //i.e. during status effect calculatio
    entt::id_type ActiveBranchHashForStatusEffects;
    std::string ActiveBranchName;
    entt::registry *_Registry;

    std::function<void(priority_request &, void *)> PriorityCallback;

    public:
      void set_registry(entt::registry *registry);
      void set_active_branch_name(const std::string &name);
      void set_priority_callback(const std::function<void(priority_request &, void *)> &callback);

      void init_starting_point(status_changes_storage_t &storage, entt::entity entity, changes_category category);
      void commit_changes_for_current_to_active_branch(entt::entity entity, const attributes_info_changes &changes, entt::entity originating_entity, timing_t timing);
      void undo_changes_to_registry();
      void commit_changes_for_intrinsics_to_active_branch(entt::entity entity, const attributes_info_changes &changes, entt::entity originating_entity, timing_t timing);
      
      void commit_status_effects_to_active_branch( entt::entity entity, const sea_state_at_timing &info, timing_t timing);
      void merge_active_branch_to_reality(timing_t upper_bound);

      void apply_history_to_entity(const attributes_info_history &history, entt::entity entity, changes_category category, timing_t upper_bound);

      
      status_changes_storage_t &get_active_branch_current_changes_storage();
      status_changes_storage_t &get_active_branch_intrinsics_changes_storage();
      status_changes_storage_t &get_active_branch_local_changes_storage();
      status_effect_changes_storage_t &get_active_branch_status_effects_changes_storage();

      attributes_info_snapshot get_most_recent_intrinsics(entt::entity entity);
      attributes_info_snapshot get_most_recent_currents(entt::entity entity);
      attributes_info_snapshot get_active_snapshot(entt::entity entity);

      status_effects_affecting get_most_recent_status_effects(entt::entity entity);
  };

}