#pragma once

#include <entt/entity/registry.hpp>

#include "logistics_export.h"
#include "status.h"
#include "effect.h"
#include "graph.h"
#include "simulation_executable.h"
#include <deque>

struct attributes_info_changes;
struct status_effects_affecting;

namespace logistics{
  enum class changes_request {
    last_committed,
    working_copy
  };

  struct executables_on_same_timing_container {
    timing_t AbsoluteTiming;
    std::deque<executable_common_data> Executables;
  };

  struct changes_context{
    entt::entity OriginatingEntity;
  };

  enum class event_type {
    change_intrinsics,
    change_status_effects,
    update
  };

  struct timeline_event {
    event_type Type;
    entt::entity OriginatingEntity;
    entt::entity AffectedEntity;
    timing_t Timing;
    bool operator==(const timeline_event &rhs) const;
    bool operator!=(const timeline_event &rhs) const;
  };

  struct timeline {
    std::vector<timeline_event> Events;
  };

  class simulation_engine{
    public:
      simulation_engine(entt::registry *reg);
      ~simulation_engine() = default;

      entt::registry *registry{nullptr};

      entt::id_type ActiveBranchHashForCurrentStatusChanges;
      entt::id_type ActiveBranchHashForIntrinsicStatusChanges;
      entt::id_type ActiveBranchHashForLocalStatusChanges; //i.e. during status effect calculatio
      entt::id_type ActiveBranchHashForStatusEffects;
      std::string ActiveBranchName;

      graph<entt::entity> DynamicGraph;

      timing_t CurrentTiming;
      //Do breadth-first search instead of depth-first and register all triggers at each level; sort them by speed of triggering
      std::map<timing_t, executables_on_same_timing_container> ExecutablesPerTimingLevel;
      std::map<timing_t, std::set<entt::entity>> UpdateRequestsPerTiming;
      timeline Timeline;

      changes_context ChangesContext;

      bool Finished{false};
      timing_t EndTiming;

      //Still breadth-first-searching, when executing a trigger, save all the entities 
      //that will need to be updated (i.e. through update_status_effects) at this speed level.
      //i.e. don't call their update instantaneously, store them instead and then call them one after the other
      //std::map<entity_updates_on_same_timing_container> EntitiesToUpdateFuncsPerTimingLevel; 
      
      void enqueue_trigger(const on_status_change_trigger_info &info, entt::entity triggering_entity, const attributes_info_changes &changes);
      void enqueue_update(entt::entity entity_to_update, entt::entity entity_requesting_update, timing_t timing);
      
      void record_status_effect_change(entt::entity affected_entity);
      void record_intrinsic_attrs_change(entt::entity affected_entity);

      bool timeline_has_cycle(size_t &cycle_start, size_t &cycle_end);

      void run_one_timing();
  };

  using status_changes_storage_t = entt::constness_as_t<entt::storage_type_t<attributes_info_history, entt::entity, std::allocator<attributes_info_history>>, attributes_info_history>;
  using status_effect_changes_storage_t = entt::constness_as_t<entt::storage_type_t<status_effects_affecting_history, entt::entity, std::allocator<status_effects_affecting_history>>, status_effects_affecting_history>;
  void start_simulating(entt::registry &registry);
  simulation_engine *get_simulation_engine(entt::registry &registry);


  //==================================================
  void add_edge(entt::registry &registry, entt::entity from, entt::entity to);
  bool graph_has_cycle(entt::registry &registry);

 
  //==================================================

  void init_starting_point(entt::registry &registry, status_changes_storage_t &storage, entt::entity entity, changes_category category);
  void commit_changes_for_current_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  void undo_changes_to_registry(entt::registry &registry);
  void commit_changes_for_intrinsics_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const sea_state_at_timing &info);
  void merge_active_branch_to_reality(entt::registry &registry, timing_t upper_bound);

  void apply_history_to_entity(entt::registry &registry, const attributes_info_history &history, entt::entity entity, changes_category category, timing_t upper_bound);

  
  status_changes_storage_t &get_active_branch_current_changes_storage(entt::registry &registry);
  status_changes_storage_t &get_active_branch_intrinsics_changes_storage(entt::registry &registry);
  status_changes_storage_t &get_active_branch_local_changes_storage(entt::registry &registry);
  status_effect_changes_storage_t &get_active_branch_status_effects_changes_storage(entt::registry &registry);


  attributes_info_snapshot get_most_recent_intrinsics(entt::registry &registry, entt::entity entity, changes_request req);
  attributes_info_snapshot get_most_recent_currents(entt::registry &registry, entt::entity entity, changes_request req);
  attributes_info_snapshot get_active_snapshot(entt::registry &registry, entt::entity entity);

  status_effects_affecting get_most_recent_status_effects(entt::registry &registry, entt::entity entity);

  logistics_API void run_calculation(entt::registry &registry, const std::function<void()> &command);
}
