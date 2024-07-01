#pragma once

#include <entt/entity/registry.hpp>

#include "status.h"
#include "effect.h"

struct attributes_info_changes;
struct status_effects;

namespace logistics{
  class simulation_engine{
    public:
      simulation_engine() = default;
      ~simulation_engine() = default;

      entt::id_type ActiveBranchHashForStatusChanges;
  };

  void start_simulating(entt::registry &registry);
  

  void commit_changes_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const status_effects &info);
  void merge_active_branch_to_reality(entt::registry &registry);

  void apply_changes_to_entity(entt::registry &registry, const attributes_info_changes &changes, entt::entity entity);

}
