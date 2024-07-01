#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"
#include "effect.h"
#include "combine.h"
#include <string_view>

using namespace std::string_view_literals;
using namespace entt::literals;

constexpr const char *k_location_field = "Field";
constexpr const char *k_location_grave = "Grave";
constexpr const char *k_location = "location";
constexpr entt::id_type k_location_hash = "location"_hs;
constexpr entt::id_type k_negated_hash = "negated"_hs;
constexpr entt::id_type k_enchantment_hash = "enchantment"_hs;
constexpr entt::id_type k_creature_hash = "creature"_hs;

bool has_stable_status(entt::registry &registry, entt::entity entity, entt::id_type hash){
  auto &&storage = registry.storage<void>(hash);
  return storage.contains(entity);
}

bool has_stable_status(entt::registry &registry, entt::entity entity, std::string_view status){
  return has_stable_status(registry, entity, entt::hashed_string::value(status.data()));
}

bool changing_location_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.find(k_location_hash);
  return (it != changes.ModifiedParams.end());
}

bool entering_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.find(k_location_hash);
  return (it != changes.ModifiedParams.end() && it->second.second.Value == k_location_field);
}

bool leaving_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.find(k_location_hash);
  return it != changes.ModifiedParams.end() && it->second.first.Value == k_location_field;
}

TEST_CASE("Status effects / simple situation"){
  entt::registry registry;

  entt::entity opalescence = registry.create();
  assign_intrinsic_status(registry, opalescence, k_enchantment_hash, true);
  entt::entity exploration = registry.create();
  assign_intrinsic_status(registry, exploration, k_enchantment_hash, true);
  //entt::entity humility = registry.create();
  //assign_status(registry, humility, "enchantment"_hs);
  
  //when an enchantment is brought to the field, make it become a creature in addition to its other types.
  //Alternately, if it leaves the field, stop modifying it.

  //FUTURE : still have to cover the fact of opalescence entering the field.
  //COULD do it in the same trigger, adding clauses and a registry view applier.
  on_status_change_trigger_info opal_on_other_status_change_info;
  opal_on_other_status_change_info.TriggerOwner = opalescence;
  opal_on_other_status_change_info.Filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, entt::entity owner){
      if(entity == owner){
        return false;
      }
      if(!changing_location_condition(changes)){
        return false;
      }
      return true;
  };

  opal_on_other_status_change_info.Func = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, entt::entity owner){
      

      auto assign_func = [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
        if(get_active_value_for_status(registry, owner, k_negated_hash) || !get_active_value_for_status(registry, target, k_enchantment_hash)){
          return;
        }
        
        assign_active_status(registry, target, k_creature_hash, true); //variant to examine : removing its enchantment status
      };

      if(entering_field_condition(changes)){
        status_effect_info info;
        info.OriginatingEntity = owner;
        info.ApplyFunc = assign_func;
        add_status_effect(registry, entity, info);
      } else if (leaving_field_condition(changes)){
        remove_status_effects_originating_from(registry, entity, owner);
      }
  };

  add_global_on_status_change_trigger(registry, opalescence, opal_on_other_status_change_info);
  
  //trigger the thing!
  add_or_set_intrinsic_parameter(registry, exploration, k_location, data_type::string, k_location_field);

  CHECK(has_stable_status(registry, exploration, k_creature_hash));

  //Chapter 2 : opalescence becomes negated
  entt::entity negater = registry.create();
  status_effect_info neg_info;
  neg_info.OriginatingEntity = negater;
  neg_info.ApplyFunc = 
    [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
      assign_active_status(registry, target, k_negated_hash, true);
    };
  add_status_effect(registry, opalescence, neg_info);
  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 3 : un-negate opalescence
  remove_status_effects_originating_from(registry, opalescence, negater);
  CHECK(has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 4 : remove exploration from the field
  add_or_set_intrinsic_parameter(registry, exploration, k_location, data_type::string, k_location_grave);

  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));

}
