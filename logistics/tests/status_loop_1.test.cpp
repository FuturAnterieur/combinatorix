#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"
#include "effect.h"
#include "combine.h"
#include "priority.h"
#include "entt_utils.h"
#include "simulation_engine.h"
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

bool became_enchant_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedStatuses.find(k_enchantment_hash);
  return (it != changes.ModifiedStatuses.end() && it->second == smt::added);
}

bool became_creature_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedStatuses.find(k_creature_hash);
  return (it != changes.ModifiedStatuses.end() && it->second == smt::added);
}

bool has_stable_status(entt::registry &registry, entt::entity entity, entt::id_type hash){
  auto &&storage = registry.storage<void>(hash);
  return storage.contains(entity);
}

TEST_CASE("Status effects - cyclic case"){
  entt::registry registry;
  
  entt::entity opalescence = registry.create();
  init_intrinsic_status(registry, opalescence, k_enchantment_hash, true);
  registry.emplace<priority_info>(opalescence, 10);
  
  entt::entity humility = registry.create();
  registry.emplace<attributes_info>(humility);
  registry.emplace<priority_info>(humility, 5);
  
  //when a card becomes an enchantment, make it become a creature instead.
  on_status_change_trigger_info opal_on_other_status_change_info;
  opal_on_other_status_change_info.TriggerOwner = opalescence;
  opal_on_other_status_change_info.Filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info){
      if(entity == info.TriggerOwner){
        return false;
      }
      
      return became_enchant_condition(changes);
  };

  #define MAX_LOOPS 10
  int loop_counter = 0;

  auto opal_status_effect_func = [](entt::registry &registry, attributes_info &attrs, entt::entity target, entt::entity owner){
        assign_active_status(registry, target, k_enchantment_hash, false);
        assign_active_status(registry, target, k_creature_hash, true);
      };

  entt::entity opal_status_effect = create_status_effect(registry, opalescence, opal_status_effect_func);

  opal_on_other_status_change_info.Func = 
    [&](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &trigger_info){
      loop_counter++;

      if(loop_counter < MAX_LOOPS) {
        add_status_effect(registry, entity, opal_status_effect);
      }
  };

  add_global_on_status_change_trigger(registry, opalescence, opal_on_other_status_change_info);
  
  on_status_change_trigger_info humility_on_any_become_creature;
  humility_on_any_become_creature.TimeDelta = DEFAULT_TIMING_DELTA;
  humility_on_any_become_creature.TriggerOwner = humility;
  humility_on_any_become_creature.Filter = 
    [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info){
      return became_creature_condition(changes);
  };

  humility_on_any_become_creature.Func = 
    [=](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &trigger_info){
      //here I cheat a little to cause the cycle to happen
      remove_status_effect(registry, entity, opal_status_effect);
  };

  add_global_on_status_change_trigger(registry, humility, humility_on_any_become_creature);

  
  //Uncomment to trigger DA LOOP
  logistics::run_calculation(registry, [&](){
    attributes_info_short_changes humility_becomes_enchantment;
    humility_becomes_enchantment.ModifiedStatuses.emplace(k_enchantment_hash, smt::added);
    assign_intrinsic_attributes_changes(registry, humility, humility_becomes_enchantment);
  });
  
  //Opalescence has the highest priority so it should win here
  CHECK(loop_counter < MAX_LOOPS);
  CHECK(has_stable_status(registry, humility, k_creature_hash));
  CHECK(!has_stable_status(registry, humility, k_enchantment_hash));
  auto &sea = registry.get<status_effects_affecting>(humility);
  REQUIRE(sea.EffectEntities.size() == 1);
  CHECK(sea.EffectEntities.front() == opal_status_effect);
}