#include "effect.h"
#include "combine.h"
#include "status.h"
#include "local_change_tracker.h"
#include "simulation_engine.h"
#include <algorithm>

//=================================================
void update_status_effects(entt::registry &registry, entt::entity entity){
  
  /*if(logistics::graph_has_cycle(registry)){ //cycle detected, do not evaluate further
    return;
  }*/

  status_effects* effs = registry.try_get<status_effects>(entity);
  if(!effs){
    return;
  }

  
  registry.emplace<logistics::local_change_tracker>(entity);
  
  //Reset local params and status to their original values
  attributes_info_snapshot snapshot;
  reset_original_status(registry, snapshot, entity);

  attributes_info &attr_info = registry.get<attributes_info>(entity);
  for(const auto &eff_info : effs->Infos){
    eff_info.ApplyFunc(registry, attr_info, entity, eff_info.OriginatingEntity);
  }

  commit_attr_info_to_branch(registry, attr_info, snapshot, entity);
}

//==================================================
//Update in my thinking : this will be invokable outside of an on_combined_trigger
//Like, directly by the causating entity
void add_status_effect(entt::registry &registry, entt::entity entity, const status_effect_info &info){
  if(!registry.any_of<status_effects>(entity)){
    registry.emplace<status_effects>(entity, std::list<status_effect_info>({info}));
  } else {
    status_effects &effs = registry.get<status_effects>(entity);
    effs.Infos.push_back(info);
  }

  link(registry, info.OriginatingEntity, entity); 
  logistics::enter_new_entity(registry, info.OriginatingEntity, entity);
  
  //Sort Infos according to the current rules about Status Effect Modification priority, then
  update_status_effects(registry, entity);
}

//==================================================
void remove_status_effects_originating_from(entt::registry &registry, entt::entity entity, entt::entity originating_entity){
  if(!registry.any_of<status_effects>(entity)){
    return;
  }

  status_effects &effs = registry.get<status_effects>(entity);
  
  effs.Infos.erase(std::remove_if(effs.Infos.begin(),
                                  effs.Infos.end(),
                                  [=](const status_effect_info &info)-> bool 
                                  { return info.OriginatingEntity == originating_entity; }), 
                  effs.Infos.end());

  unlink(registry, originating_entity, entity);
  logistics::enter_new_entity(registry, originating_entity, entity);
  
  //Sort Infos according to the current rules about Status Effect Modification priority, then
  update_status_effects(registry, entity);
}

//==================================================
void use(entt::registry &registry, entt::entity ability, entt::entity target){
  if(!registry.any_of<usable>(ability)){
    return;
  }

  //TODO : Start simulation here I think (as long as this was called from outside)
  logistics::start_simulating(registry, ability);

  auto view = registry.view<on_use_trigger>();
  view.each([&](auto trigger_owner, on_use_trigger &trig_group){
    for(const on_use_trigger_info &info : trig_group.Triggers){
      info.Func(registry, ability, target, trigger_owner);
    }
  });

  //check for registry.ctx() global triggers???

  usable &used = registry.get<usable>(ability);
  used.UseFunc(registry, ability, target);

  logistics::merge_active_branch_to_reality(registry);
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