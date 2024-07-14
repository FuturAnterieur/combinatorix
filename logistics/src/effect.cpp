#include "effect.h"
#include "combine.h"
#include "status.h"
#include "simulation_engine.h"
#include <algorithm>

//=================================================
void update_status_effects(entt::registry &registry, entt::entity entity){
  using namespace logistics;
  //Reset local params and status to their original values
  simulation_engine *sim = registry.ctx().find<simulation_engine>();
  assert(sim);

  attributes_info_snapshot snapshot;
  init_history_for_local_changes(registry, entity);

  attributes_info &attr_info = registry.get<attributes_info>(entity);
  status_effects_affecting effs = get_most_recent_status_effects(registry, entity);
  
  for(const auto &eff_entity : effs.EffectEntities){
    status_effect_info &eff_info = registry.get<status_effect_info>(eff_entity);
    sim->ChangesContext.OriginatingEntity = eff_entity;
    eff_info.ApplyFunc(registry, attr_info, entity, eff_info.OriginatingEntity);
  }
  
  sim->ChangesContext.OriginatingEntity = entity;

  commit_attr_info_to_branch(registry, entity);
}

entt::entity create_status_effect(entt::registry &registry, entt::entity originating_entity, const status_effect_apply_func_t &apply_func){
  auto eff_ent = registry.create();
  auto &eff_info = registry.emplace<status_effect_info>(eff_ent);
  eff_info.ApplyFunc = apply_func;
  eff_info.OriginatingEntity = originating_entity;
  auto &owned_effs = registry.get_or_emplace<status_effects_owned>(originating_entity);
  owned_effs.EffectEntities.push_back(eff_ent);

  return eff_ent;
}

//==================================================
//Update in my thinking : this will be invokable outside of an on_combined_trigger
//Like, directly by the causating entity
void add_status_effect(entt::registry &registry, entt::entity affected_entity, entt::entity eff_entity){
  using namespace logistics;
  if(!registry.any_of<status_effect_info>(eff_entity)){
    return;
  }
  
  status_effects_affecting curr = logistics::get_most_recent_status_effects(registry, affected_entity);
  curr.EffectEntities.push_back(eff_entity);
  commit_status_effects_to_active_branch(registry, affected_entity, curr);

  auto &eff_info = registry.get<status_effect_info>(eff_entity);
  link(registry, eff_info.OriginatingEntity, affected_entity); 
  
  //Sort Infos according to the current rules about Status Effect Modification priority, then
  logistics::simulation_engine *eng = logistics::get_simulation_engine(registry);
  eng->record_status_effect_change(affected_entity);
  
  eng->enqueue_update(affected_entity, eff_info.OriginatingEntity, DEFAULT_TIMING_DELTA); 
}

//==================================================
void remove_status_effect(entt::registry &registry, entt::entity entity, entt::entity the_eff_entity){
  using namespace logistics;
  if(!registry.any_of<status_effect_info>(the_eff_entity)){
    return;
  }
  
  status_effects_affecting curr = logistics::get_most_recent_status_effects(registry, entity);

  curr.EffectEntities.erase(std::remove_if(curr.EffectEntities.begin(),
                                  curr.EffectEntities.end(),
                                  [=](const entt::entity eff_entity)-> bool 
                                  { return eff_entity == the_eff_entity; }), 
                  curr.EffectEntities.end());

  commit_status_effects_to_active_branch(registry, entity, curr);

  auto &eff_info = registry.get<status_effect_info>(the_eff_entity);

  unlink(registry, eff_info.OriginatingEntity, entity);
  
  //Sort Infos according to the current rules about Status Effect Modification priority, then
  logistics::simulation_engine *eng = logistics::get_simulation_engine(registry);
  eng->record_status_effect_change(entity);
  eng->enqueue_update(entity, eng->ChangesContext.OriginatingEntity, DEFAULT_TIMING_DELTA);
}

//==================================================
void use(entt::registry &registry, entt::entity ability, entt::entity target){
  if(!registry.any_of<usable>(ability)){
    return;
  }

  auto view = registry.view<on_use_trigger>();
  view.each([&](auto trigger_owner, on_use_trigger &trig_group){
    for(const on_use_trigger_info &info : trig_group.Triggers){
      info.Func(registry, ability, target, trigger_owner);
    }
  });

  //check for registry.ctx() global triggers???

  usable &used = registry.get<usable>(ability);
  used.UseFunc(registry, ability, target);

}

//====================================================
void add_on_use_trigger(entt::registry &registry, entt::entity owner, const on_use_trigger_func_t &func){
  on_use_trigger &triggers = registry.get_or_emplace<on_use_trigger>(owner);
  triggers.Triggers.push_back({func, owner});
}

//=====================================================
entt::entity add_ability(entt::registry &registry, entt::entity candidate_owner, use_func_t func, const combination_info &info){
  const auto ability = registry.create();
  registry.emplace<usable>(ability, func);
  combination_info final_info = info;
  final_info.AcceptedCombinations.emplace(combination_kind::ability);
  registry.emplace<combination_info>(ability, final_info);

  if(!registry.any_of<combination_info>(candidate_owner)){
    registry.emplace<combination_info>(candidate_owner);
  }

  combination_info &owner_info = registry.get<combination_info>(candidate_owner);
  owner_info.AcceptedCombinations.emplace(combination_kind::ability);

  if(!add_ability(registry, candidate_owner, ability)){
    registry.destroy(ability);
    return entt::null;
  }
  return ability;
}

//===========================================
bool add_ability(entt::registry &registry, entt::entity candidate_owner, entt::entity ability){
  return combine(registry, candidate_owner, ability);
}

//===========================================
std::set<entt::entity> list_abilities(entt::registry &registry, entt::entity owner){
  if(!registry.any_of<combination_info>(owner)){
    return {};
  }

  combination_info &info = registry.get<combination_info>(owner);
  auto it = info.CurrentCombinations.find(combination_kind::ability);
  if(it == info.CurrentCombinations.end()){
    return {};
  }

  return it->second;
}