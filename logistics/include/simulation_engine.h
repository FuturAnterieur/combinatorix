#pragma once

#include <entt/entity/registry.hpp>

#include "status.h"
#include "effect.h"
#include "graph.h"

struct attributes_info_changes;
struct status_effects;

namespace logistics{
  class simulation_engine{
    public:
      simulation_engine() = default;
      ~simulation_engine() = default;

      entt::id_type ActiveBranchHashForStatusChanges;
      graph<entt::entity> DynamicGraph;
      entt::entity StartingNode;
      entt::entity CurrentNode;
  };

  void start_simulating(entt::registry &registry, entt::entity start);
  bool enter_new_entity(entt::registry &registry, entt::entity from, entt::entity to);

  void commit_changes_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const status_effects &info);
  void merge_active_branch_to_reality(entt::registry &registry);

  void apply_changes_to_entity(entt::registry &registry, const attributes_info_changes &changes, entt::entity entity);

  using status_changes_storage_t = entt::constness_as_t<entt::storage_type_t<attributes_info_changes, entt::entity, std::allocator<attributes_info_changes>>, attributes_info_changes>;
  status_changes_storage_t &get_active_branch_status_changes_storage(entt::registry &registry);
}
