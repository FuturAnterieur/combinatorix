#include "priority.h"

#include <entt/entity/registry.hpp>
#include "simulation_engine.h"

//will have to add a priority rules system someday!!!

void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2){
  
}

void classic_priority_callback(priority_request &req, void *user_data)
{
  entt::registry *registry = (entt::registry *)(user_data);

  for(auto &[ent, prio] : req.EntitiesWithResultingPriorityValues){
    auto &info = registry->get_or_emplace<priority_info>(ent);
    *prio = info.Value;
  }
}