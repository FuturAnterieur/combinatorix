#pragma once

#include <entt/entity/registry.hpp>

#include "status.h"
#include "effect.h"

struct attributes_info_changes;
struct status_effects;

namespace logistics{
  class simulation_engine{
    public:
      simulation_engine();
      ~simulation_engine();

      entt::id_type ActiveBranchHash;
  };

  void commit_status_to_active_branch(entt::registry &registry, entt::entity entity,  const attributes_info_changes &changes);
  void commit_status_effects_to_active_branch(entt::registry &registry, entt::entity entity, const status_effects &info);
}
