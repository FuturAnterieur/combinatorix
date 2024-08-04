#pragma once

#include <entt/entity/registry.hpp>
#include "effect.h"
#include "attributes_info.h"

namespace logistics{
  
  enum class changes_request {
      last_committed,
      working_copy
    };


  using status_stable_storage_t = entt::storage_for_t<attributes_info_snapshot>;
  using status_changes_storage_t = entt::storage_for_t<attributes_info_history>;
  using status_effect_changes_storage_t = entt::storage_for_t<status_effects_affecting_history>;

  logistics_API void paste_changes_to_official_registry(entt::registry *registry, const attributes_info_cumulative_changes &changes, entt::entity entity);
}