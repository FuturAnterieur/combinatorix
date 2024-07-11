#pragma once 

#include <entt/entity/registry.hpp>
#include "status.h"

enum class executable_type {
  status_trigger,
  update
};

namespace logistics {
  
  class simulation_engine;
  struct status_trigger_executable  {
    on_status_change_trigger_info Info;
    attributes_info_changes Changes;
    entt::entity TriggeringEntity;
    void operator()(entt::registry &registry, simulation_engine *eng);
  };

  struct entity_update_executable {
    entt::entity EntityToUpdate;
    void operator()(entt::registry &registry, simulation_engine *eng);
  };

  struct executable_common_data {
    executable_type ExecType;
    entt::entity UpdatedEntity;
    std::function<void(entt::registry &, simulation_engine *)> Func;
  };

}