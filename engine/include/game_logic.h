#pragma once

#include "engine_export.h"
#include <functional>
#include <entt/entity/fwd.hpp>
#include "logistics/include/history_manager.h"
#include "simulation_data.h"

namespace engine {
  
  using status_view_t = entt::view<entt::get_t<void>>;
  using parameter_view_t = entt::view<entt::get_t<parameter>>;

  class engine_API game_logic {
    private:
      entt::registry *_Registry;

      std::unique_ptr<simulation_data> CurrentSimulationData;
      std::unique_ptr<logistics::history_manager> HistoryManager;

    public:

      game_logic(entt::registry *registry);
      ~game_logic() = default;

      //API
      void run_simulation(const std::function<void(game_logic *)> &request);
      
      //Calculation API
      void init_attributes(entt::entity entity, const attributes_info_short_changes &delta);
      void change_intrinsics(entt::entity, const attributes_info_short_changes &changes);

      void change_actives(entt::entity, const attributes_info_short_changes &changes);
      //void request_to_move(entt::entity, /*move request - to be detailed in geometry*/);

      void add_on_status_change_trigger(entt::entity entity, on_status_change_trigger_info &info);
      void add_global_on_status_change_trigger(on_status_change_trigger_info &info); //adds to registry.ctx

      entt::entity create_status_effect(entt::entity originating_entity, const status_effect_apply_func_t &apply_func);
      void add_status_effect(entt::entity affected_entity, entt::entity eff_entity);
      void remove_status_effect(entt::entity affected_entity, entt::entity eff_entity);
      
      attributes_info_snapshot get_active_snapshot(entt::entity entity);

      parameter_view_t get_parameter_view(entt::id_type hash);
      status_view_t get_status_view(entt::id_type hash);

      //void use(entt::entity ability); //target would be set in the ability entity????
      //void add_on_use_trigger(entt::entity owner, /*const on_use_trigger_func_t &func*/);
      //entt::entity add_ability(entt::entity candidate_owner, use_func_t func, const combination_info &info);

      //bool combine(entt::entity a, entt::entity b, /*combination_kind*/);

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