#pragma once

#include "engine_export.h"
#include <functional>
#include <entt/entity/fwd.hpp>
#include "logistics/include/history_manager.h"
#include "simulation_data.h"
#include "game_communicator.h"
#include "geometry/include/shape.h"

namespace engine {
  struct pre_change_trigger_info;

  using status_view_t = entt::view<entt::get_t<void>>;
  using parameter_view_t = entt::view<entt::get_t<parameter>>;

  class engine_API game_logic {
    private:
      entt::registry *_Registry;

      std::unique_ptr<simulation_data> CurrentSimulationData;
      std::unique_ptr<logistics::history_manager> HistoryManager;
      game_communicator *_Communicator{nullptr};

    public:

      game_logic(entt::registry *registry);
      ~game_logic() = default;

      //only useful for change edit history, the API of which should be changed anyway
      inline entt::registry *get_registry() {
        return _Registry;
      }

      void set_communicator(game_communicator *comm);
      std::string ask_question(size_t chan_index, const std::string &question_data);

      //API
      void run_simulation(const std::function<void(game_logic *)> &request);
      
      //Calculation API
      void init_status(entt::entity entity, entt::id_type hash, smt val);
      void init_parameter(entt::entity entity, entt::id_type hash, parameter_value_t val);
      void init_aabb_collider(entt::entity entity, const geometry::aabb_collider &collider);
      void init_circle_collider(entt::entity entity, const geometry::circle_collider &collider);


      void init_attributes(entt::entity entity, const attributes_info_short_changes &delta);
      void change_intrinsics(entt::entity, const attributes_info_short_changes &changes);
      void change_actives(entt::entity, const attributes_info_short_changes &changes);
      //void request_to_move(entt::entity, );
      void set_originating_entity(entt::entity entity); //To be used e.g. to say when a player is playing. Mostly useful at the start of a simulation, because otherwise, it is managed internally.

      void add_on_status_change_trigger(entt::entity entity, on_status_change_trigger_info &info);
      void add_global_on_status_change_trigger(on_status_change_trigger_info &info); //adds to registry.ctx

      entt::entity create_status_effect(entt::entity originating_entity, const status_effect_apply_func_t &apply_func);
      void add_status_effect(entt::entity affected_entity, entt::entity eff_entity);
      void remove_status_effect(entt::entity affected_entity, entt::entity eff_entity);
      
      entt::entity create_pre_change_trigger(const pre_change_trigger_info &info);
      void add_pre_change_trigger(entt::entity affected_entity, entt::entity trigger_entity);

      attributes_info_snapshot get_active_snapshot(entt::entity entity);
      parameter_view_t get_parameter_view(entt::id_type hash);
      status_view_t get_status_view(entt::id_type hash);


    private:
      friend struct entity_update_executable;
      friend struct status_trigger_executable;

      void set_registry(entt::registry *registry);

      void set_context_originating_entity(entt::entity entity);
      void update_status(entt::entity entity);
      void activate_status_change_triggers(entt::entity entity, const attributes_info_changes &changes);

      void run_one_timing();
  };
}