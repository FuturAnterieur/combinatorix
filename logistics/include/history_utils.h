#pragma once

#include <entt/entity/registry.hpp>
#include "effect.h"
#include "attributes_info.h"

namespace logistics{
  
  enum class changes_request {
      last_committed,
      working_copy
    };


  using status_stable_storage_t = entt::constness_as_t<entt::storage_type_t<attributes_info_snapshot, entt::entity, std::allocator<attributes_info_snapshot>>, attributes_info_snapshot>;
  using status_changes_storage_t = entt::constness_as_t<entt::storage_type_t<attributes_info_history, entt::entity, std::allocator<attributes_info_history>>, attributes_info_history>;
  using status_effect_changes_storage_t = entt::constness_as_t<entt::storage_type_t<status_effects_affecting_history, entt::entity, std::allocator<status_effects_affecting_history>>, status_effects_affecting_history>;

}