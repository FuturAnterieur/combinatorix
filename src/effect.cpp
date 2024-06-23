#include "effect.h"

void update_effects(entt::registry &registry){
  auto view = registry.view<effects>();

  view.each([&](auto entity, effects &eff){
    for(const auto &info : eff.Infos){
      info.ApplyFunc(registry, info.OriginatingEntity, entity);
    }
  });
}

void use(entt::registry &registry, entt::entity entity){
  if(!registry.any_of<usable>(entity)){
    return;
  }


  usable &used = registry.get<usable>(entity);
  used.UseFunc(registry, entity, )//how do I get the target???
  //probably keeping it in a temporary component might be an idea. I'll see. Like "select_target"
  //Or keep my own stack of UsageIDs, usable for many purposes
}