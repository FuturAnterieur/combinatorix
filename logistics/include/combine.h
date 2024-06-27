#pragma once
#include "logistics_export.h"

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
  //FUTURE : 
  //- codify AcceptedCombinations in the registry.ctx()
  //- make it a structure saying : for this combination_kind, these type pairings are accepted
  //  - actually probably a lambda for each combination kind
  //  - entities will only have to register their types for the combine function to analyse them based on the structure in the ctx().
  //  - I could still support the capacity for individual entities to override the comparison lambda for certain combination kinds.
  std::set<combination_kind> AcceptedCombinations;
  std::map<combination_kind, std::set<entt::entity>> CurrentCombinations;
};

logistics_API bool combine(entt::registry &registry, entt::entity a, entt::entity b);

logistics_API void add_combine_trigger(entt::registry &registry, entt::entity e, combine_trigger_t func);
