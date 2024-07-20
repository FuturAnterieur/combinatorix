#pragma once

#include "status_structs.h"

namespace engine {
  class game_logic;
  enum class executable_type {
    status_trigger,
    update
  };

  struct status_trigger_executable  {
    game_logic *eng;
    on_status_change_trigger_info Info;
    attributes_info_changes Changes;
    entt::entity TriggeringEntity;
    void operator()();
  };

  struct entity_update_executable {
    game_logic *eng;
    entt::entity EntityToUpdate;
    void operator()();
  };

  struct executable_common_data {
    executable_type ExecType;
    entt::entity UpdatedEntity;
    std::function<void()> Func;
  };
}