#pragma once

#include "logistics_export.h"
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

struct priority_callback {
  std::function<void(priority_request &, void *)> Func;
  void *UserData;
};

using priority_callback_t = priority_callback;
logistics_API void classic_priority_callback(priority_request &req, void *user_data);

void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2);