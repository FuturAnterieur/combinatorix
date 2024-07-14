#pragma once


#include <entt/entity/fwd.hpp>

typedef int priority_t;

struct priority_info {
  priority_t Value{0};
};

void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2);