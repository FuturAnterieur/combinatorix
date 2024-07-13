#pragma once


#include <entt/entity/fwd.hpp>

typedef unsigned int priority_t;


void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2);