#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status_structs.h"
#include "game_logic.h"
#include "snapshot_utils.h"


#include "communication/include/local_communicator.h"
#include "thread_pool/include/thread_pool.h"
#include "geometry/include/collision_tree.h"

using namespace entt::literals;

constexpr entt::id_type k_negated_hash = "negated"_hs;
constexpr entt::id_type k_enchantment_hash = "enchantment"_hs;
constexpr entt::id_type k_creature_hash = "creature"_hs;
constexpr entt::id_type k_color_hash = "color"_hs;

struct client {
  client(int num_threads, int comm_id) : Pool(num_threads), Comm(comm_id) {

  }
  thread_pool Pool;
  local_communicator Comm;
  std::map<std::string, std::string> AnsweringMap;
};

std::string get_string_value(entt::registry &registry, entt::entity entity, entt::id_type hash){
  auto &&storage = registry.storage<parameter>(hash);
  std::string ret = "";
  if(storage.contains(entity)){
    return std::get<std::string>(storage.get(entity).value());
  }
  return ret;
}



TEST_CASE("Exchange between client and server with request to move"){
  entt::registry registry;
  engine::game_logic game(&registry);

  entt::entity opalescence = registry.create();
  game.init_status(opalescence, k_enchantment_hash, smt::added);
  game.init_aabb_collider(opalescence, geometry::aabb_collider{{{-5.f, -5.f}, {5.f, 5.f}}});

  entt::entity exploration = registry.create();
  game.init_status(exploration, k_enchantment_hash, smt::added);
  game.init_circle_collider(exploration, geometry::circle_collider{5.f});

  local_channel_container channels;
  size_t chan_idx = channels.add_channel();

  local_communicator serv_comm(0);
  serv_comm.set_container(&channels);

  game_communicator serv_gam_comm(&serv_comm);

  game.set_communicator(&serv_gam_comm);

  client cli(1, 1);
  cli.Comm.set_container(&channels);
  cli.AnsweringMap["Color"] = "Red";
  
  cli.Pool.launch();

  post(cli.Pool, [&](){
    std::string question;
    cli.Comm.receive_blocking(chan_idx, question);
    CHECK(question == "Color");
    cli.Comm.send(chan_idx, cli.AnsweringMap.at(question));
  });

  game.run_simulation([&](engine::game_logic *game){
    std::string color = game->ask_question(chan_idx, "Color");
    attributes_info_short_changes changes;
    changes.ModifiedParams.emplace(k_color_hash, diff_from_set_val(color));
    game->change_intrinsics(opalescence, changes);
  });

  CHECK(get_string_value(registry, opalescence, k_color_hash) == "Red");
}