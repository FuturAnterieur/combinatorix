#include "effect.h"

void update_effects(entt::registry &registry){
  auto view = registry.view<effects>();

  view.each([&](auto entity, effects &eff){
    for(const auto &info : eff.Infos){
      info.ApplyFunc(registry, info.OriginatingEntity, entity);
    }
  });
}

void use(entt::registry &registry, entt::entity source, entt::entity target){
  if(!registry.any_of<usable>(source)){
    return;
  }

  auto view = registry.view<on_use_trigger>();
  view.each([&](auto entity, on_use_trigger &trig_group){
    for(const on_use_trigger_info &info : trig_group.Triggers){
      auto stor_ptr = registry.storage(info.UsableTypeThatTriggersThis.hash());
      if(stor_ptr && stor_ptr->contains(source)){
        info.Func(registry, source);
      }
    }
  });

  usable &used = registry.get<usable>(source);
  used.UseFunc(registry, source, target);
}