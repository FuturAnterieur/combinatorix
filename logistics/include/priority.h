#pragma once


#include <entt/entity/fwd.hpp>
#include <vector>
#include <functional>

typedef int priority_t;

struct priority_info {
  priority_t Value{0};
};

struct priority_request {
  std::vector<std::pair<entt::entity, priority_t*>> EntitiesWithResultingPriorityValues;
};

using priority_callback_t = std::function<void(priority_request &, void *)>;
void classic_priority_callback(priority_request &req, void *user_data);

void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2);