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

TEST_CASE("Basic test with change cancellation through pre-change triggers"){
  entt::registry registry;
  engine::game_logic game(&registry);

  auto subject = registry.create();
  game.init_attributes(subject, {{},{{k_location_hash, k_location_field}}});
  auto modifier = registry.create();
  game.init_attributes(modifier, {{},{}});

  registry.emplace<priority_info>(modifier, 10);

  auto filter = [](engine::game_logic *game, const attributes_info_changes &actual_changes, const attributes_info_cumulative_changes &proposed_changes, entt::entity entity, const engine::pre_change_trigger_info &info){
    auto it = proposed_changes.ParamChanges.Changes.find(k_location_hash);
    if(it != proposed_changes.ParamChanges.Changes.end() && std::get<std::string>(it->second.Diff.value()) == k_location_grave){
      return true;
    }
    return false;
  };

  auto func = [](engine::game_logic *game, const attributes_info_changes &actual_changes, const attributes_info_cumulative_changes &proposed_changes, entt::entity entity, const engine::pre_change_trigger_info &info, engine::change_edit_history &hist){
    hist.record_param_change_suppression(k_location_hash, info.Owner);
  };

  auto mod_trigger = game.create_pre_change_trigger(engine::pre_change_trigger_info{func, filter, modifier});
  game.add_pre_change_trigger(subject, mod_trigger);

  game.run_simulation([&](engine::game_logic *game){
    attributes_info_short_changes sc;
    sc.ModifiedParams.emplace(k_location_hash, k_location_grave);
    game->change_intrinsics(subject, sc);
  });

  CHECK(get_location_value(registry, subject) == k_location_field);
}