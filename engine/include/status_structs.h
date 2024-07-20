#pragma once

#include <functional>
#include "logistics/include/attributes_info.h"


namespace engine {
  class game_logic;
  struct on_status_change_trigger_info;
  using status_change_trigger_func_t = std::function<void(game_logic *, const attributes_info_changes &, entt::entity, const on_status_change_trigger_info &info)>;
  using status_change_trigger_filter_t = std::function<bool(game_logic *, const attributes_info_changes &, entt::entity, const on_status_change_trigger_info &info)>;
  struct on_status_change_trigger_info {
    //game_logic, status changes, entity whose status changed, owning entity of the trigger
    status_change_trigger_func_t Func;
    status_change_trigger_filter_t Filter;
    entt::entity TriggerOwner;
    timing_t TimeDelta{DEFAULT_TIMING_DELTA}; //unsigned! Cannot trigger stuff before the thing that triggers it happens!
  };

  struct on_status_change_triggers {
    std::list<on_status_change_trigger_info> Triggers;
  };

  using status_effect_apply_func_t = std::function<void(game_logic *, entt::entity, entt::entity)>;
  struct status_effect_info {
    entt::entity OriginatingEntity;
    status_effect_apply_func_t ApplyFunc;
  };
}
