#pragma once

#include <entt/entity/registry.hpp>
#include <functional>
#include <list>
#include <set>

struct effect_info {
  entt::entity OriginatingEntity;
  std::function<void(entt::registry&, entt::entity, entt::entity)> ApplyFunc;
};

struct effects { //i.e. generally status effects currently applying to the parent entity
  std::list<effect_info> Infos;
};

void update_effects(entt::registry &registry); //i.e. update (status) effects

//Active effects, Passive effects
//Passive : are called at each update
//Active : are called only on use
//But the signature for the call should be the same so...

//To whom should the effect be added?

//registry, ability, target (can group many targets together in one entity)
using use_func_t = std::function<void(entt::registry&, entt::entity, entt::entity)>;
struct usable {
  use_func_t UseFunc;
};

//How do I tell > Ent B has a trigger that goes off only when A is used?
  //Or better, when a certain type of usable is used?
struct on_use_trigger_info {
  //registry, usable entity source
  std::function<void(entt::registry &, entt::entity)> Func;
  entt::type_info UsableTypeThatTriggersThis; //if I want to handle boolean combinations of types, this will have to be improved
};

struct on_use_trigger {
  std::list<on_use_trigger_info> Triggers;
};

struct can_have_abilities {};

void use(entt::registry &registry, entt::entity ability, entt::entity target);

struct combination_info;
entt::entity add_ability(entt::registry &registry, entt::entity candidate_owner, use_func_t func, const combination_info &info);
bool add_ability(entt::registry &registry, entt::entity candidate_owner, entt::entity ability);

std::set<entt::entity> list_abilities(entt::registry &registry, entt::entity owner);