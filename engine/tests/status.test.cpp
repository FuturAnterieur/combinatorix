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
  auto it = changes.ModifiedParams.Changes.find(k_location_hash);
  return (it != changes.ModifiedParams.Changes.end());
}

bool entering_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.Changes.find(k_location_hash);
  return (it != changes.ModifiedParams.Changes.end() && std::get<std::string>(it->second.Change.second.Value.value()) == k_location_field);
}

bool leaving_field_condition(const attributes_info_changes &changes){
  auto it = changes.ModifiedParams.Changes.find(k_location_hash);
  return it != changes.ModifiedParams.Changes.end() && std::get<std::string>(it->second.Change.first.Value.value()) == k_location_field;
}

TEST_CASE("Status effects / simple situation"){
  entt::registry registry;
  engine::game_logic game(&registry);

  entt::entity opalescence = registry.create();
  game.init_status(opalescence, k_enchantment_hash, smt::added);
  entt::entity exploration = registry.create();
  game.init_status(exploration, k_enchantment_hash, smt::added);
  
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
    exploration_enters_field.ModifiedParams.emplace(k_location_hash, diff_from_set_val(k_location_field));
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
  game.run_simulation([&](engine::game_logic *game) {
    game->remove_status_effect(opalescence, neg_stat_eff);
  });
  
  CHECK(has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));


  // Chapter 4 : remove exploration from the field
  game.run_simulation([&](engine::game_logic *game) {
    attributes_info_short_changes leaves_field;
    leaves_field.ModifiedParams.emplace(k_location_hash, diff_from_set_val(k_location_grave));
    game->change_intrinsics(exploration, leaves_field);
  });
  

  CHECK(!has_stable_status(registry, exploration, k_creature_hash));
  CHECK(has_stable_status(registry, exploration, k_enchantment_hash));

}

constexpr entt::id_type k_object_hash = "object"_hs;
constexpr entt::id_type k_illuminated_hash = "illuminated"_hs;
constexpr entt::id_type k_blue_hash = "blue"_hs;
constexpr entt::id_type k_red_hash = "red"_hs;
constexpr entt::id_type k_color_hash = "Color"_hs;

TEST_CASE("Status effects / diamond pattern"){
  entt::registry registry;
  engine::game_logic game(&registry);

  entt::entity sorcerer = registry.create();
  game.init_status(sorcerer, k_creature_hash, smt::added);
  
  entt::entity light = registry.create();
  game.init_status(light, k_object_hash, smt::added);
  
  entt::entity blue_mirror = registry.create();
  game.init_status(blue_mirror, k_object_hash, smt::added);
  game.init_parameter(blue_mirror, k_color_hash, "Blue");

  entt::entity red_mirror = registry.create();
  game.init_status(red_mirror, k_object_hash, smt::added);
  game.init_parameter(red_mirror, k_color_hash, "Red");
  
  auto illuminate = [](engine::game_logic *game, entt::entity target, entt::entity owner){
      attributes_info_short_changes illum;
      illum.ModifiedStatuses.emplace(k_illuminated_hash, smt::added);
      game->change_actives(target, illum);
    };

  entt::entity illuminate_eff = game.create_status_effect(sorcerer, illuminate);

  auto mirror_trigger_filter = 
    [](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &info){
      auto it = changes.ModifiedStatuses.Changes.find(k_illuminated_hash);
      return it != changes.ModifiedStatuses.Changes.end() && it->second.Change.Diff == smt::added;
  };

  int blue_update_counter{0};
  int red_update_counter{0};
  auto light_changer_func = [&](engine::game_logic *game, entt::entity target, entt::entity owner){
      auto snapshot = game->get_active_snapshot(owner);
      parameter color = get_value_for_parameter(snapshot, k_color_hash);
      
      attributes_info_short_changes color_changer;

      if(std::get<std::string>(color.value()) == "Red"){
        color_changer.ModifiedStatuses.emplace(k_red_hash, smt::added);
        game->change_actives(target, color_changer); 
        red_update_counter++;
      } else if(std::get<std::string>(color.value()) == "Blue"){
        color_changer.ModifiedStatuses.emplace(k_blue_hash, smt::added);
        game->change_actives(target, color_changer); 
        blue_update_counter++;
      }
    };

  entt::entity light_changer_blue = game.create_status_effect( blue_mirror, light_changer_func);
  entt::entity light_changer_red = game.create_status_effect(red_mirror, light_changer_func);
  
  auto mirror_trigger_func = [&](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &trigger_info){
    
    //use combination to get 'light' from 'entity'
    //in this trigger 'entity' is equivalent to trigger_info.Owner
    game->add_status_effect(light, registry.get<status_effects_owned>(entity).EffectEntities.front());
  };

  engine::on_status_change_trigger_info blue_mirror_on_illuminated;
  blue_mirror_on_illuminated.TriggerOwner = blue_mirror;
  engine::on_status_change_trigger_info red_mirror_on_illuminated;
  red_mirror_on_illuminated.TriggerOwner = red_mirror;

  blue_mirror_on_illuminated.Filter = mirror_trigger_filter;
  blue_mirror_on_illuminated.Func = mirror_trigger_func;
  red_mirror_on_illuminated.Filter = mirror_trigger_filter;
  red_mirror_on_illuminated.Func = mirror_trigger_func;

  game.add_on_status_change_trigger(blue_mirror, blue_mirror_on_illuminated);
  game.add_on_status_change_trigger(red_mirror, red_mirror_on_illuminated);

  game.run_simulation([&](engine::game_logic *game){
    game->add_status_effect(red_mirror, illuminate_eff);
    game->add_status_effect(blue_mirror, illuminate_eff);
  });
  
  CHECK(has_stable_status(registry, blue_mirror, k_illuminated_hash));
  CHECK(has_stable_status(registry, red_mirror, k_illuminated_hash));
  CHECK(has_stable_status(registry, light, k_blue_hash));
  CHECK(has_stable_status(registry, light, k_red_hash));
  CHECK(blue_update_counter == 1);
  CHECK(red_update_counter == 1);
}


TEST_CASE("diamond pattern without status effects, triggers only"){
  entt::registry registry;
  engine::game_logic game(&registry);

  entt::entity sorcerer = registry.create();
  game.init_status(sorcerer, k_creature_hash, smt::added);
  
  entt::entity light = registry.create();
  game.init_status(light, k_object_hash, smt::added);
  
  entt::entity blue_mirror = registry.create();
  game.init_status(blue_mirror, k_object_hash, smt::added);
  game.init_parameter(blue_mirror, k_color_hash, "Blue");

  entt::entity red_mirror = registry.create();
  game.init_status(red_mirror, k_object_hash, smt::added);
  game.init_parameter(red_mirror, k_color_hash, "Red");
  
  auto mirror_trigger_filter = 
    [](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &info){
      auto it = changes.ModifiedStatuses.Changes.find(k_illuminated_hash);
      return it != changes.ModifiedStatuses.Changes.end() && it->second.Change.Diff == smt::added;
  };

  int blue_update_counter{0};
  int red_update_counter{0};
  auto mirror_trigger_func = [&](engine::game_logic *game, const attributes_info_changes &changes, entt::entity entity, const engine::on_status_change_trigger_info &trigger_info){
    const auto &owner_snapshot = game->get_active_snapshot(trigger_info.TriggerOwner);
    parameter color = get_value_for_parameter(owner_snapshot, k_color_hash);
    
    if(std::get<std::string>(color.value()) == "Red"){
      attributes_info_short_changes target_changes;
      target_changes.ModifiedStatuses.emplace(k_red_hash, smt::added);
      game->change_intrinsics(light, target_changes);
      red_update_counter++;
    } else if(std::get<std::string>(color.value()) == "Blue"){
      attributes_info_short_changes target_changes;
      target_changes.ModifiedStatuses.emplace(k_blue_hash, smt::added);
      game->change_intrinsics(light, target_changes); 
      blue_update_counter++;
    }
  };

  
  engine::on_status_change_trigger_info blue_mirror_on_illuminated;
  blue_mirror_on_illuminated.TriggerOwner = blue_mirror;
  engine::on_status_change_trigger_info red_mirror_on_illuminated;
  red_mirror_on_illuminated.TriggerOwner = red_mirror;

  blue_mirror_on_illuminated.Filter = mirror_trigger_filter;
  blue_mirror_on_illuminated.Func = mirror_trigger_func;
  red_mirror_on_illuminated.Filter = mirror_trigger_filter;
  red_mirror_on_illuminated.Func = mirror_trigger_func;

  game.add_on_status_change_trigger(blue_mirror, blue_mirror_on_illuminated);
  game.add_on_status_change_trigger(red_mirror, red_mirror_on_illuminated);
  
  game.run_simulation([&](engine::game_logic *game){
    attributes_info_short_changes illum;
    illum.ModifiedStatuses.emplace(k_illuminated_hash, smt::added);
    game->change_intrinsics(blue_mirror, illum);
    game->change_intrinsics(red_mirror, illum);
  });
  
  CHECK(has_stable_status(registry, blue_mirror, k_illuminated_hash));
  CHECK(has_stable_status(registry, red_mirror, k_illuminated_hash));
  CHECK(has_stable_status(registry, light, k_blue_hash));
  CHECK(has_stable_status(registry, light, k_red_hash));
  CHECK(blue_update_counter == 1);
  CHECK(red_update_counter == 1);
}