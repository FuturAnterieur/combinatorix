#pragma once

#include "logistics/include/attributes_info.h"
#include "engine/include/status_structs.h"

#include <functional>
#include <entt/entity/fwd.hpp>

namespace engine {
  class game_logic;
  struct pre_change_trigger_info;
  //signature : game_logic, proposed status changes, entity whose status changed, trigger's info (containing owning entity)
  using pre_change_trigger_func_t = std::function<void(game_logic *, attributes_info_cumulative_changes &proposed_changes, entt::entity, const pre_change_trigger_info &info)>;
  //signature : game_logic, modifiable status changes, entity whose status changed, trigger's info (containing owning entity)
  using pre_change_trigger_filter_t = std::function<bool(game_logic *, attributes_info_cumulative_changes &proposed_changes, entt::entity, const pre_change_trigger_info &info)>;
  
  struct pre_change_trigger_info {
    pre_change_trigger_func_t Func;
    pre_change_trigger_filter_t Filter;
    entt::entity Owner;
  };

  struct pre_change_triggers_affecting {
    std::list<entt::entity> TriggerEntities;
  };
}
