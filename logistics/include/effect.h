#pragma once

#include "logistics_export.h"
#include <entt/entity/registry.hpp>
#include <functional>
#include <list>
#include <set>

struct attributes_info;
using status_effect_apply_func_t = std::function<void(entt::registry&, attributes_info&, entt::entity, entt::entity)>;
struct status_effect_info {
  entt::entity OriginatingEntity;
  status_effect_apply_func_t ApplyFunc;
};

struct status_effects_affecting { //i.e. generally status effects currently applying to the parent entity
  std::list<entt::entity> EffectEntities;
  //entt::constness_as_t<entt::storage_type_t<effect_info, entt::entity, std::allocator<effect_info>>, effect_info> InfosOnSteroids;
};

struct status_effects_owned{
  std::list<entt::entity> EffectEntities;
};

logistics_API entt::entity create_status_effect(entt::registry &registry, entt::entity originating_entity, const status_effect_apply_func_t &apply_func);


//Update status effects for a single entity (rerun the calculation in the order the list is sorted)
void update_status_effects(entt::registry &registry, entt::entity entity);
logistics_API void add_status_effect(entt::registry &registry, entt::entity affected_entity, entt::entity eff_entity);
logistics_API void remove_status_effect(entt::registry &registry, entt::entity affected_entity, entt::entity eff_entity);

//I.e. in active branch
status_effects_affecting &get_active_status_effects(entt::registry &registry, entt::entity entity);

//Active effects, Passive effects
//Passive : Status modifying effects -> reran each time they are modified
//Effects that stay on place but do something at regular time intervals / according to certain events -> use a trigger system
//Active : are called only on use


//registry, ability, target (can group many targets together in one entity)
using use_func_t = std::function<void(entt::registry&, entt::entity, entt::entity)>;
struct usable {
  use_func_t UseFunc;
};

//How do I tell > Ent B has a trigger that goes off only when A is used?
//Or better, when a certain type of usable is used?
  
//registry, usable entity source, target of the ability, entity that owns the trigger
using on_use_trigger_func_t = std::function<void(entt::registry &, entt::entity, entt::entity, entt::entity)>;
struct on_use_trigger_info {
  
  on_use_trigger_func_t Func;
  entt::entity TriggerOwner; 
};

struct on_use_trigger {
  std::list<on_use_trigger_info> Triggers;
};

struct can_have_abilities {};

logistics_API void use(entt::registry &registry, entt::entity ability, entt::entity target);
logistics_API void add_on_use_trigger(entt::registry &registry, entt::entity owner, const on_use_trigger_func_t &func);

struct combination_info;
logistics_API entt::entity add_ability(entt::registry &registry, entt::entity candidate_owner, use_func_t func, const combination_info &info);
logistics_API bool add_ability(entt::registry &registry, entt::entity candidate_owner, entt::entity ability);

std::set<entt::entity> list_abilities(entt::registry &registry, entt::entity owner);