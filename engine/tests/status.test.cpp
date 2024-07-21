#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status_structs.h"
#include "game_logic.h"
#include "snapshot_utils.h"

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
  engine::game_logic game(&registry);

  entt::entity opalescence = registry.create();
  game.init_attributes(opalescence, attributes_info_short_changes{{{k_enchantment_hash, smt::added}},{}});
  entt::entity exploration = registry.create();
  game.init_attributes(exploration, attributes_info_short_changes{{{k_enchantment_hash, smt::added}},{}});
  
  auto opal_stat_eff_func = [](engine::game_logic *game, entt::entity target, entt::entity owner){
        auto target_snapshot = game->get_active_snapshot(target);
        auto owner_snapshot = game->get_active_snapshot(owner);
        
        if(get_value_for_status(owner_snapshot, k_negated_hash) || !get_value_for_status(target_snapshot, k_enchantment_hash)){
          return;
        }
        
        game->change_actives(target, attributes_info_short_changes{{{k_creature_hash, smt::added}},{}});
      };

  entt::entity opal_stat_eff = game.create_status_effect(opalescence, opal_stat_eff_func);

  engine::on_status_change_trigger_info opal_on_other_status_change_info;
  opal_on_other_status_change_info.TriggerOwner = opalescence;
  opal_on_other_status_change_info.Filter = 
    [](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &info){
      if(entity == info.TriggerOwner){
        return false;
      }
      if(!changing_location_condition(changes)){
        return false;
      }
      return true;
  };

  opal_on_other_status_change_info.Func = 
    [=](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &trigger_info){
      if(entering_field_condition(changes)){
        game->add_status_effect(entity, opal_stat_eff);
      } else if (leaving_field_condition(changes)){
        game->remove_status_effect(entity, opal_stat_eff);
      }
  };

  game.add_global_on_status_change_trigger(opal_on_other_status_change_info);
  
  //trigger the thing!

  game.run_simulation([&](engine::game_logic *game){
    attributes_info_short_changes exploration_enters_field;
    exploration_enters_field.ModifiedParams.emplace(k_location_hash, parameter(k_location_field));
    game->change_intrinsics(exploration, exploration_enters_field);
  });
  

  CHECK(has_stable_status(registry, exploration, k_creature_hash));

  //Chapter 2 : opalescence becomes negated
  entt::entity negater = registry.create();
   
  auto neg_stat_eff_func = [](engine::game_logic *game, entt::entity target, entt::entity owner){
      attributes_info_short_changes become_negated;
      become_negated.ModifiedStatuses.emplace(k_negated_hash, smt::added);
      game->change_actives(target, become_negated);
    };

  entt::entity neg_stat_eff = game.create_status_effect(negater, neg_stat_eff_func);

  //'use ability' API is not used here, and should be modernized anyway
  /*entt::entity negater_adding_ability = add_ability(registry, negater, 
    [=](entt::registry &registry, entt::entity ability, entt::entity target){
      add_status_effect(registry, target, neg_stat_eff);
    }, combination_info{});*/

  game.run_simulation([&](engine::game_logic *game){
    game->add_status_effect(opalescence, neg_stat_eff);
  });
  
  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 3 : un-negate opalescence
  /*
  entt::entity negater_removal_ability = add_ability(registry, negater,
    [=](entt::registry &registry, entt::entity ability, entt::entity target){
      remove_status_effect(registry, target, neg_stat_eff);
    }, combination_info{});


  
  logistics::run_calculation(registry, [&]() {
    use(registry, negater_removal_ability, opalescence);
  });
  
  CHECK(has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 4 : remove exploration from the field
  logistics::run_calculation( registry, [&]() {
    attributes_info_short_changes exploration_leaves_field;
    exploration_leaves_field.ModifiedParams.emplace(k_location_hash, parameter(k_location_grave));
    assign_intrinsic_attributes_changes(registry, exploration, exploration_leaves_field);
  });
  

  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));*/

}

