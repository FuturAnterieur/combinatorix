#pragma once


#include "history_utils.h"
#include "priority.h"
#include "history_storage.h"
#include <string>

namespace logistics {

  //The planned "generic_history<T>" template class (which history_manager will use indirectly) will most probably go into logistics
  //If history_manager has to manage positions too, it will have to go into engine-server.
  class logistics_API  history_manager {
    history_storage<changes_category::current> CurrentAttrHistory;
    history_storage<changes_category::intrinsics> IntrinsicAttrHistory;
    history_storage<changes_category::local> LocalAttrHistory;

    entt::id_type ActiveBranchHashForStatusEffects;
    std::string ActiveBranchName;
    entt::registry *_Registry;

    std::function<void(priority_request &, void *)> PriorityCallback;

    public:
      void set_registry(entt::registry *registry);
      void set_priority_callback(const std::function<void(priority_request &, void *)> &callback);

      void set_stable_values(entt::entity entity, const attributes_info_short_changes &changes);

      void init_local_changes(entt::entity entity);
      void clear_local_changes(entt::entity entity);
      void commit_local_changes(entt::entity entity, const attributes_info_cumulative_changes &changes);

      void commit_changes_for_current_to_active_branch(entt::entity entity, const attributes_info_cumulative_changes &changes, timing_t timing);
      void undo_changes_to_registry();
      void commit_changes_for_intrinsics_to_active_branch(entt::entity entity, const attributes_info_cumulative_changes &changes, timing_t timing);
      
      void commit_status_effects_to_active_branch( entt::entity entity, const sea_state_at_timing &info, timing_t timing);
      void merge_active_branch_to_reality(timing_t upper_bound);

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