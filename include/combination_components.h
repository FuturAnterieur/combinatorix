#pragma once

#include <entt/entity/registry.hpp>
#include <functional>
#include <map>
#include <set>

enum class combination_kind {
  equipping,
  substance_of,
  attached, //produced items could also be simply 'equipped' to a factory
  ability
};

using combine_trigger_t = std::function<void(entt::registry &, combination_kind, entt::entity, entt::entity)>;
struct on_combine_trigger {
  std::list<combine_trigger_t> Funcs;
};

struct combination_info{

  std::set<combination_kind> AcceptedCombinations;
  std::map<combination_kind, std::set<entt::entity>> CurrentCombinations;
};

bool combine(entt::registry &registry, entt::entity a, entt::entity b);

void add_combine_trigger(entt::registry &registry, entt::entity e, combine_trigger_t func);
