#pragma once

#include <entt/entity/registry.hpp>
#include <functional>
#include <map>
#include <set>

enum class combination_kind {
  equipping,
  substance_of,
};

struct on_combine_trigger {
  std::list<std::function<void(entt::registry &, combination_kind, entt::entity, entt::entity)>> Funcs;
};

struct combination_info{
  std::map<combination_kind, std::set<entt::type_info>> AcceptedCombinations;
  std::map<combination_kind, std::set<entt::entity>> CurrentCombinations;
};

bool combine(entt::registry &registry, entt::entity a, entt::entity b);

template<typename Component>
void emplace_combination_reactive_component(entt::registry &registry, entt::entity e){
  registry.emplace<Component>(e);
  if(registry.any_of<on_combine_trigger>(e)){
    on_combine_trigger &es_funcs = registry.get<on_combine_trigger>(e);
    es_funcs.Funcs.push_back(&Component::on_combined_to);
  }
}
