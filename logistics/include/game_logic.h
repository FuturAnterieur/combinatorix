#pragma once

#include <functional>
#include <entt/entity/fwd.hpp>
#include "history_manager.h"

struct simulation_data {
  timing_t CurrentTiming;
  std::map<timing_t, executables_on_same_timing_container> ExecutablesPerTimingLevel;
  std::map<timing_t, std::set<entt::entity>> UpdateRequestsPerTiming;
  timeline Timeline;
  changes_context ChangesContext;
};

class game_logic {
  public:
    //API
    void set_registry(entt::registry *registry);

    void run_calculation(const std::function<void(game_logic *)> &request);
    
    //Calculation API
    void init_attributes(entt::entity entity, /*attributes_info_snapshot*/);
    void change_intrinsics(entt::entity, /*attributes_info_short_changes*/);
    void change_actives(entt::entity, /*attributes_info_short_changes*/);
    void request_to_move(entt::entity, /*move request - to be detailed in geometry*/);

    void add_on_status_change_trigger(entt::entity entity, /*info*/);
    void add_global_on_status_change_trigger(/*info*/); //adds to registry.ctx

    entt::entity create_status_effect(entt::entity originating_entity, /*func*/);
    void add_status_effect(entt::entity affected_entity, entt::entity eff_entity);
    void remove_status_effect(entt::entity affected_entity, entt::entity eff_entity);
    status_effects_affecting &get_active_status_effects(entt::entity entity);

    void use(entt::entity ability); //target would be set in the ability entity????
    void add_on_use_trigger(entt::entity owner, /*const on_use_trigger_func_t &func*/);
    entt::entity add_ability(entt::entity candidate_owner, use_func_t func, const combination_info &info)

    bool combine(entt::entity a, entt::entity b, /*combination_kind*/);

    void calculate_priority(entt::registry &registry, entt::entity ent1, entt::entity ent2, priority_t &prio1, priority_t &prio2);

  private:
    entt::registry *_Registry;

    std::unique_ptr<simulation_data> CurrentSimulationData;
    std::unique_ptr<history_manager> HistoryManager;
};