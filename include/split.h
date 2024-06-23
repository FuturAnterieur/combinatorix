#pragma once 
#include <entt/entity/registry.hpp>

#include <utility>
#include <set>
#include <vector>

struct split_rule {
  std::vector<std::set<entt::type_info>> TypeGroups;
};

bool split(entt::registry &registry, entt::entity x, const split_rule &rule, std::vector<entt::entity> &output);