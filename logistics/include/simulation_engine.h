#pragma once

#include <entt/entity/registry.hpp>

#include "logistics_export.h"
#include "status.h"
#include "effect.h"
#include "graph.h"
#include <deque>

struct attributes_info_changes;
struct status_effects;


enum class executable_type {
  status_trigger,
  update
};

struct status_trigger_executable  {
  on_status_change_trigger_info Info;
  attributes_info_changes Changes;
  entt::entity TriggeringEntity;
  void operator()(entt::registry &registry) {
    Info.Func(registry, Changes, TriggeringEntity, Info);
  }
};

struct entity_update_executable {
  entt::entity EntityToUpdate;
  void operator()(entt::registry &registry) {
    update_status_effects(registry, EntityToUpdate);
  }
};

struct executable_common_data {
  executable_type ExecType;
  entt::entity UpdatedEntity;
  std::function<void(entt::registry &)> Func;
};

struct executables_on_same_timing_container {
  timing_t AbsoluteTiming;
  std::deque<executable_common_data> Executables;
};


namespace logistics{
  enum class changes_request {
    last_committed,
    working_copy
  };

  class simulation_engine{
    public:
      simulation_engine(entt::registry *reg);
      ~simulation_engine() = default;

      entt::registry *registry{nullptr};

      entt::id_type ActiveBranchHashForCurrentStatusChanges;
      entt::id_type ActiveBranchHashForIntrinsicStatusChanges;
      entt::id_type ActiveBranchHashForLocalStatusChanges; //i.e. during status effect calculation
      std::string ActiveBranchName;

      //graph functionalities are mostly unused right now
      graph<entt::entity> DynamicGraph;

      timing_t CurrentTiming;
      //Do breadth-first search instead of depth-first and register all triggers at each level; sort them by speed of triggering
      std::map<timing_t, executables_on_same_timing_container> ExecutablesPerTimingLevel;
      std::set<entt::entity> UpdateRequestsFromCurrentTiming;

      //Still breadth-first-searching, when executing a trigger, save all the entities 
      //that will need to be updated (i.e. through update_status_effects) at this speed level.
      //i.e. don't call their update instantaneously, store them instead and then call them one after the other
      //std::map<entity_updates_on_same_timing_container> EntitiesToUpdateFuncsPerTimingLevel; 
      
      void enqueue_trigger(const on_status_change_trigger_info &info, entt::entity triggering_entity, const attributes_info_changes &changes);
      void enqueue_update(entt::entity entity_to_update, entt::entity entity_requesting_update, timing_t timing);
      
      //What timing value should be given to enqueue_update? Up to now I tested with 0 and 1 and both seem to work fine. 1 makes more sense to me.


      void execute_stuff();
  };

  void start_simulating(entt::registry &registry);
  simulation_engine *get_simulation_engine(entt::registry &registry);


  //==================================================
  void add_edge(entt::registry &registry, entt::entity from, entt::entity to);
  bool graph_has_cycle(entt::registry &registry);
  //==================================================

  void commit_changes_for_current_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  void sync_current_with_registry_for_views(entt::registry &registry, entt::entity entity);
  void undo_changes_to_registry(entt::registry &registry);
  void commit_changes_for_intrinsics_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  
  
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const status_effects &info);
  void merge_active_branch_to_reality(entt::registry &registry);

  void apply_history_to_entity(entt::registry &registry, const attributes_info_history &history, entt::entity entity, changes_category category);

  using status_changes_storage_t = entt::constness_as_t<entt::storage_type_t<attributes_info_history, entt::entity, std::allocator<attributes_info_history>>, attributes_info_history>;
  status_changes_storage_t &get_active_branch_current_changes_storage(entt::registry &registry);
  status_changes_storage_t &get_active_branch_intrinsics_changes_storage(entt::registry &registry);
  status_changes_storage_t &get_active_branch_local_changes_storage(entt::registry &registry);

  attributes_info_snapshot get_most_recent_intrinsics(entt::registry &registry, entt::entity entity, changes_request req);
  attributes_info_snapshot get_most_recent_currents(entt::registry &registry, entt::entity entity, changes_request req);
  attributes_info_snapshot get_active_snapshot(entt::registry &registry, entt::entity entity);

  logistics_API void run_calculation(entt::registry &registry, const std::function<void()> &command);
}
