#pragma once

#include "logistics/include/attributes_info.h"

#include <functional>
#include <entt/entity/registry.hpp>

namespace engine {
  class game_logic;

  struct action_desc{
    std::function<void(engine::game_logic *, entt::entity)> Func; //entity is the action instance entity
    std::map<std::string, parameter> Inputs;
    std::string Id;
    timing_t TimingDelta;
  };

  struct action_instance{
    attributes_info_snapshot_t<parameter> Parameters;
    entt::entity Description;
  };

  struct action_modifier { //kinda like a status effect but for actions
    std::function<bool(engine::game_logic *, entt::entity, entt::entity)> Filter; //entity1 contains the action description; entity2 is the modifier owner
    std::function<void(engine::game_logic *, entt::entity, entt::entity)> ApplyFunc; //entity1 is the action instance entity; entity2 is the modifier owner
    //I'd like to represent action bans through this structure too
  };

  //game_logic functions
  //entt::entity create_action(id, inputs, func)
  /*entt::entity request_do_action(entt::entity desc_ent, const attributes_info_snapshot_t<parameter> arguments){
    entt::entity instance = registry.create();
    registry.emplace<action_instance>(instance, arguments, desc_ent);
    const auto &desc_data = registry.get<action_desc>(desc_ent);

    for(const auto &modifier : registry.ctx().get<action_modifiers>().at(desc_data.Id)){
      if(modifier.Filter(this, desc_ent)){
        modifier.ApplyFunc(this, instance);
      }
    }

    enqueue_execute_action(instance); // -> will get timing data from instance_data.Description.TimingDelta, in case the modifiers changed the description
  }


  */
}
