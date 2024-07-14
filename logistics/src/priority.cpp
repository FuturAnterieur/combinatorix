#include "priority.h"

#include <entt/entity/registry.hpp>
#include "simulation_engine.h"

//will have to add a priority rules system someday!!!

void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2){
  auto &info1 = registry.get_or_emplace<priority_info>(ent1);
  auto &info2 = registry.get_or_emplace<priority_info>(ent2);

  prio1 = info1.Value;
  prio2 = info2.Value;
}