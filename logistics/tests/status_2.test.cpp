#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"
#include "effect.h"
#include "combine.h"
#include "simulation_engine.h"
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

std::string get_location_value(entt::registry &registry, entt::entity entity){
  auto &&storage = registry.storage<parameter>(k_location_hash);
  std::string ret = "";
  if(storage.contains(entity)){
    return std::get<std::string>(storage.get(entity).value());
  }
  return ret;
}

TEST_CASE("Status effects / simple situation"){
  const size_t num_victims = 10;
  
  entt::registry registry;

  entt::entity torrential_tribute = registry.create();
  init_intrinsic_status(registry, torrential_tribute, k_creature_hash, true);
  init_intrinsic_parameter(registry, torrential_tribute, k_location_hash, k_location_field);
  entt::entity bystander = registry.create();
  init_intrinsic_status(registry, bystander, k_enchantment_hash, true);
  init_intrinsic_parameter(registry, bystander, k_location_hash, k_location_field);

  entt::entity declencheur = registry.create();
  init_intrinsic_status(registry, declencheur, k_creature_hash, true);

  std::vector<entt::entity> victims;
  for(size_t i = 0; i < num_victims; i++){
    auto &victim = victims.emplace_back(registry.create());
    init_intrinsic_status(registry, victim, k_creature_hash, true);
    init_intrinsic_parameter(registry, victim, k_location_hash, k_location_field);
  }

  on_status_change_trigger_info tt_info;
  tt_info.TriggerOwner = torrential_tribute;
  //TODO : for triggers that trigger on MANY entities having certain changes at once : 
  //provide a timing argument to this function so that trigger effects with accumulators can know when to start/stop accumulating.
  auto tt_filter = [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info){
      if(entity == info.TriggerOwner){
        return false;
      }

      return get_active_value_for_status(registry, entity, k_creature_hash) && entering_field_condition(changes);
  };

  auto tt_func = [](entt::registry &registry, const attributes_info_changes &changes, entt::entity entity, const on_status_change_trigger_info &info) {
    auto creature_view = entt::view<entt::get_t<void>>{registry.storage<void>(k_creature_hash)};
    auto location_view = entt::view<entt::get_t<parameter>>{registry.storage<parameter>(k_location_hash)};

    for(entt::entity other_entity : creature_view | location_view){
      if(other_entity == info.TriggerOwner){
        continue;
      }
      auto location = get_active_value_for_parameter(registry, other_entity, k_location_hash);
      if(std::get<std::string>(location.value()) == k_location_field){
        attributes_info_short_changes changes;
        changes.ModifiedParams.emplace(k_location_hash, k_location_grave);
        assign_intrinsic_attributes_changes(registry, other_entity, changes);
      }
    }
  };

  tt_info.Filter = tt_filter;
  tt_info.Func = tt_func;

  add_global_on_status_change_trigger(registry, torrential_tribute, tt_info);

  logistics::run_calculation(registry, [&](){
    attributes_info_short_changes changes;
    changes.ModifiedParams.emplace(k_location_hash, k_location_field);
    assign_intrinsic_attributes_changes(registry, declencheur, changes);
  });

  CHECK(get_location_value(registry, bystander) == k_location_field);
  CHECK(get_location_value(registry, torrential_tribute) == k_location_field);
  CHECK(get_location_value(registry, declencheur) == k_location_grave);
  for(size_t i = 0; i < num_victims; i++){
    CHECK(get_location_value(registry, victims[i]) == k_location_grave);
  }
}