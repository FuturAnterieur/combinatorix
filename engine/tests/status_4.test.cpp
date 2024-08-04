#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "doctest.h"
#include "status_structs.h"
#include "game_logic.h"
#include "snapshot_utils.h"
#include "engine/include/action.h"
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
constexpr entt::id_type k_hp_hash = "HP"_hs;

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

std::string get_location_value(entt::registry &registry, entt::entity entity){
  auto &&storage = registry.storage<parameter>(k_location_hash);
  std::string ret = "";
  if(storage.contains(entity)){
    return std::get<std::string>(storage.get(entity).value());
  }
  return ret;
}

float get_hp_value(entt::registry &registry, entt::entity entity){
  auto &&storage = registry.storage<parameter>(k_hp_hash);
  float ret = -1.f;
  if(storage.contains(entity)){
    return std::get<float>(storage.get(entity).value());
  }
  return ret;
}

TEST_CASE("Basic test for parameters with incremental changes applied to them (i.e. HP here)."){
  entt::registry registry;
  engine::game_logic game(&registry);

  auto player = registry.create();
  registry.emplace<priority_info>(player, 5);
  auto subject = registry.create();
  game.init_parameter(subject, k_hp_hash, 10.f);
  
  auto modifier1 = registry.create();
  registry.emplace<priority_info>(modifier1, 10);

  auto modifier2 = registry.create();
  registry.emplace<priority_info>(modifier2, 7);

  auto mod_1_stat_eff_func = [](engine::game_logic *game, entt::entity target, entt::entity owner){
        attributes_info_short_changes changes;
        changes.ModifiedParams.emplace(k_hp_hash, diff_from_op(param_op_type::add, 3.f));
        game->change_actives(target, changes);
      };

  auto mod_2_stat_eff_func = [](engine::game_logic *game, entt::entity target, entt::entity owner){
        attributes_info_short_changes changes;
        changes.ModifiedParams.emplace(k_hp_hash, diff_from_op(param_op_type::mul, 5.f));
        game->change_actives(target, changes);
      };

  auto se1 = game.create_status_effect(modifier1, mod_1_stat_eff_func);
  auto se2 = game.create_status_effect(modifier2, mod_2_stat_eff_func);

  SUBCASE("Case 1 : two modifiers"){
    game.run_simulation([&](engine::game_logic *game){
      
      game->add_status_effect(subject, se1);
      game->add_status_effect(subject, se2);
    });

    float result = get_hp_value(registry, subject);
    CHECK(std::round(result) == 53.0f);
  }

  SUBCASE("Case 2 : two modifiers + one intrinsic change"){
    game.run_simulation([&](engine::game_logic *game){
      attributes_info_short_changes intrinsics;
      intrinsics.ModifiedParams.emplace(k_hp_hash, diff_from_op(param_op_type::mul, 2.f));
      
      game->add_status_effect(subject, se1);
      game->add_status_effect(subject, se2);
      game->change_intrinsics(subject, intrinsics);
    });

    float result = get_hp_value(registry, subject);
    CHECK(std::round(result) == 103.0f);
  }
}