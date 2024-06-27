#include "effect.h"
#include "combine.h"
#include "status.h"

//=================================================
void update_status_effects(entt::registry &registry, entt::entity entity){
  status_effects* effs = registry.try_get<status_effects>(entity);
  if(!effs){
    return;
  }

  //Reset params and status to their original values
  attributes_info_snapshot snapshot;
  reset_original_status(registry, snapshot, entity);

  attributes_info &attr_info = registry.get<attributes_info>(entity);
  for(const auto &eff_info : effs->Infos){
    eff_info.ApplyFunc(registry, attr_info, eff_info.OriginatingEntity);
  }

  commit_attr_info(registry, attr_info, snapshot, entity);
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
  //Sort Infos according to the current rules about Status Effect Modification priority, then
  update_status_effects(registry, entity);
}

//==================================================
void use(entt::registry &registry, entt::entity ability, entt::entity target){
  if(!registry.any_of<usable>(ability)){
    return;
  }

  auto view = registry.view<on_use_trigger>();
  view.each([&](auto trigger_owner, on_use_trigger &trig_group){
    for(const on_use_trigger_info &info : trig_group.Triggers){
      info.Func(registry, ability, trigger_owner);
    }
  });

  usable &used = registry.get<usable>(ability);
  used.UseFunc(registry, ability, target);
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