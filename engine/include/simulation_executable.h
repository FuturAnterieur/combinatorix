#pragma once

#include "status_structs.h"

namespace engine {
  struct status_trigger_executable  {
    on_status_change_trigger_info Info;
    attributes_info_changes Changes;
    entt::entity TriggeringEntity;
    void operator()(game_logic *);
  };

  struct entity_update_executable {
    entt::entity EntityToUpdate;
    void operator()(game_logic *);
  };

  struct executable_common_data {
    executable_type ExecType;
    entt::entity UpdatedEntity;
    std::function<void(game_logic *)> Func;
  };
}