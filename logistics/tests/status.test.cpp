#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"
#include "effect.h"
#include "combine.h"
#include "entt_utils.h"
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
  return (it != changes.ModifiedParams.end() && std::get<std::string>(it->second.second.value()) == k_location_field);
}

bool leaving_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.find(k_location_hash);
  return it != changes.ModifiedParams.end() && std::get<std::string>(it->second.first.value()) == k_location_field;
}

TEST_CASE("Status effects / simple situation"){
  entt::registry registry;

  entt::entity opalescence = registry.create();
  init_intrinsic_status(registry, opalescence, k_enchantment_hash, true);
  entt::entity exploration = registry.create();
  init_intrinsic_status(registry, exploration, k_enchantment_hash, true);
  //entt::entity humility = registry.create();
  //assign_status(registry, humility, "enchantment"_hs);
  
  //when an enchantment is brought to the field, make it become a creature in addition to its other types.
  //Alternately, if it leaves the field, stop modifying it.

  //FUTURE : still have to cover the fact of opalescence entering the field.
  //COULD do it in the same trigger, adding clauses and a registry view applier.
  on_status_change_trigger_info opal_on_other_status_change_info;
  opal_on_other_status_change_info.TriggerOwner = opalescence;
  opal_on_other_status_change_info.Filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info){
      if(entity == info.TriggerOwner){
        return false;
      }
      if(!changing_location_condition(changes)){
        return false;
      }
      return true;
  };

  opal_on_other_status_change_info.Func = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &trigger_info){
      

      auto assign_func = [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
        if(get_active_value_for_status(registry, owner, k_negated_hash) || !get_active_value_for_status(registry, target, k_enchantment_hash)){
          return;
        }
        
        assign_active_status(registry, target, k_creature_hash, true); //variant to examine : removing its enchantment status
      };

      if(entering_field_condition(changes)){
        status_effect_info info;
        info.OriginatingEntity = trigger_info.TriggerOwner;
        info.ApplyFunc = assign_func;
        add_status_effect(registry, entity, info);
      } else if (leaving_field_condition(changes)){
        remove_status_effects_originating_from(registry, entity, trigger_info.TriggerOwner);
      }
  };

  add_global_on_status_change_trigger(registry, opalescence, opal_on_other_status_change_info);
  
  //trigger the thing!
  attributes_info_changes exploration_enters_field;
  exploration_enters_field.ModifiedParams.emplace(k_location_hash, std::make_pair(parameter("Hand"), parameter(k_location_field)));
  assign_intrinsic_attributes_changes(registry, exploration, exploration_enters_field);

  CHECK(has_stable_status(registry, exploration, k_creature_hash));

  //Chapter 2 : opalescence becomes negated
  entt::entity negater = registry.create();
  status_effect_info neg_info;
  neg_info.OriginatingEntity = negater;
  neg_info.ApplyFunc = 
    [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
      assign_active_status(registry, target, k_negated_hash, true);
    };


  entt::entity negater_adding_ability = add_ability(registry, negater, 
    [=](entt::registry &registry, entt::entity ability, entt::entity target){
      add_status_effect(registry, target, neg_info);
    }, combination_info{});

  entt::entity negater_removal_ability = add_ability(registry, negater,
    [=](entt::registry &registry, entt::entity ability, entt::entity target){
      remove_status_effects_originating_from(registry, target, negater);
    }, combination_info{});


  
  use(registry, negater_adding_ability, opalescence);

  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 3 : un-negate opalescence
  use(registry, negater_removal_ability, opalescence);
  CHECK(has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 4 : remove exploration from the field
  attributes_info_changes exploration_leaves_field;
  exploration_leaves_field.ModifiedParams.emplace(k_location_hash, std::make_pair(parameter(k_location_field), parameter(k_location_grave)));
  assign_intrinsic_attributes_changes(registry, exploration, exploration_leaves_field);

  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));

}

constexpr entt::id_type k_object_hash = "object"_hs;
constexpr entt::id_type k_illuminated_hash = "illuminated"_hs;
constexpr entt::id_type k_blue_hash = "blue"_hs;
constexpr entt::id_type k_red_hash = "red"_hs;
constexpr entt::id_type k_color_hash = "Color"_hs;

struct multi_target {
  std::vector<entt::entity> Targets;
};

TEST_CASE("Status effects / diamond pattern"){
  entt::registry registry;

  entt::entity sorcerer = registry.create();
  init_intrinsic_status(registry, sorcerer, k_creature_hash, true);
  entt::entity light = registry.create();
  init_intrinsic_status(registry, light, k_object_hash, true);
  entt::entity blue_mirror = registry.create();
  init_intrinsic_status(registry, blue_mirror, k_object_hash, true);
  init_intrinsic_parameter(registry, blue_mirror, "Color", data_type::string, "Blue");
  entt::entity red_mirror = registry.create();
  init_intrinsic_status(registry, red_mirror, k_object_hash, true);
  init_intrinsic_parameter(registry, red_mirror, "Color", data_type::string, "Red");
  
  entt::entity two_mirrors = registry.create();
  registry.emplace<multi_target>(two_mirrors, std::vector<entt::entity>{blue_mirror, red_mirror});

  status_effect_info lighter;
  lighter.OriginatingEntity = sorcerer;
  lighter.ApplyFunc = 
    [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
      assign_active_status(registry, target, k_illuminated_hash, true);
    };


  entt::entity spell = add_ability(registry, sorcerer, 
  [=](entt::registry &registry, entt::entity ability, entt::entity target){
    const multi_target &targets = registry.get<multi_target>(target);
    for(const entt::entity &target : targets.Targets){
      add_status_effect(registry, target, lighter);
    }
  }, combination_info{});

  auto mirror_trigger_filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info){
      auto it = changes.ModifiedStatuses.find(k_illuminated_hash);
      return it != changes.ModifiedStatuses.end() && it->second == smt::added;
  };

  auto light_changer_func = [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
      parameter color = get_active_value_for_parameter(registry, owner, k_color_hash); //utils::get_or_default(registry, entity, k_color_hash, parameter{});
      if(std::get<std::string>(color.value()) == "Red"){
        assign_active_status(registry, target, k_red_hash, true); 
      } else if(std::get<std::string>(color.value()) == "Blue"){
        assign_active_status(registry, target, k_blue_hash, true);
      }
    };
  

  auto mirror_trigger_func = [&](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &trigger_info){
    status_effect_info info;
    info.OriginatingEntity = trigger_info.TriggerOwner;
    info.ApplyFunc = light_changer_func;
    add_status_effect(registry, light, info);
  };

  on_status_change_trigger_info blue_mirror_on_illuminated;
  blue_mirror_on_illuminated.TriggerOwner = blue_mirror;
  on_status_change_trigger_info red_mirror_on_illuminated;
  red_mirror_on_illuminated.TriggerOwner = red_mirror;

  blue_mirror_on_illuminated.Filter = mirror_trigger_filter;
  blue_mirror_on_illuminated.Func = mirror_trigger_func;
  red_mirror_on_illuminated.Filter = mirror_trigger_filter;
  red_mirror_on_illuminated.Func = mirror_trigger_func;

  add_on_status_change_trigger(registry, blue_mirror, blue_mirror_on_illuminated);
  add_on_status_change_trigger(registry, red_mirror, red_mirror_on_illuminated);

  use(registry, spell, two_mirrors);

  CHECK(has_stable_status(registry, light, k_blue_hash));
  CHECK(has_stable_status(registry, light, k_red_hash));
      
}