#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"
#include "effect.h"
#include "combine.h"
#include <string_view>

using namespace std::string_view_literals;
using namespace entt::literals;

bool has_status(entt::registry &registry, entt::entity entity, entt::id_type hash){
  auto &&storage = registry.storage<void>(hash);
  return storage.contains(entity);
}

bool has_status(entt::registry &registry, entt::entity entity, std::string_view status){
  return has_status(registry, entity, entt::hashed_string::value(status.data()));
}

bool entering_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.find("location"_hs);
  auto added_it = changes.AddedParams.find("location"_hs);
  if((it == changes.ModifiedParams.end() || it->second.second.Value != "Field")
    && (added_it == changes.AddedParams.end() || added_it->second.Value != "Field"))
  {
    return false;
  }
  
  return true;
}

TEST_CASE("Status effects / simple situation"){
  entt::registry registry;

  entt::entity opalescence = registry.create();
  assign_status(registry, opalescence, "enchantment"_hs);
  entt::entity exploration = registry.create();
  assign_status(registry, exploration, "enchantment"_hs);
  //entt::entity humility = registry.create();
  //assign_status(registry, humility, "enchantment"_hs);
  
  //when an enchantment is brought to the field, make it become a creature in addition to its other types.
  

  on_status_change_trigger_info opal_on_other_enter_info;
  opal_on_other_enter_info.TriggerOwner = opalescence;
  opal_on_other_enter_info.Filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, entt::entity owner){
      if(entity == owner || !has_status(registry, entity, "enchantment"_hs)){
        return false;
      }
      if(!entering_field_condition(changes)){
        return false;
      }
      return true;
  };

  opal_on_other_enter_info.Func = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, entt::entity owner){
    
      status_effect_info info;
      info.OriginatingEntity = owner;
      info.ApplyFunc = [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
        if(has_status(registry, owner, "negated"_hs)){
          return;
        }
        link(registry, owner, target); //THIS WILL HAVE TO BE PLACED IN THE CORE CODE, NOT IN CLIENT CODE
        assign_status(registry, target, "creature"_hs, false); //variant to examine : removing its enchantment status
      };
      add_status_effect(registry, entity, info);
  };

  add_global_on_status_change_trigger(registry, opalescence, opal_on_other_enter_info);
  
  //trigger the thing!
  add_or_set_parameter_and_trigger_on_change(registry, exploration, "location", data_type::string, "Field");

  CHECK(has_status(registry, exploration, "creature"_hs));

  //check what happens if opalesence becomes negated!
  entt::entity negater = registry.create();

  status_effect_info neg_info;
  neg_info.OriginatingEntity = negater;
  neg_info.ApplyFunc = 
    [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
      assign_status(registry, target, "negated"_hs, false);
    };


  add_status_effect(registry, opalescence, neg_info);

  CHECK(!has_status(registry, exploration, "creature"_hs));
  CHECK(has_status(registry, exploration, "enchantment"_hs));
}
