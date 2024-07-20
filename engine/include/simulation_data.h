#pragma once

#include "engine/include/simulation_executable.h"
#include <entt/entity/fwd.hpp>
#include <deque>

namespace engine {
  
  struct executables_on_same_timing_container {
    timing_t AbsoluteTiming;
    std::deque<executable_common_data> Executables;
  };

  struct changes_context{
    entt::entity OriginatingEntity;
  };

  enum class event_type {
    change_intrinsics,
    change_status_effects,
    update
  };

  struct timeline_event {
    event_type Type;
    entt::entity OriginatingEntity;
    entt::entity AffectedEntity;
    timing_t Timing;
    bool operator==(const timeline_event &rhs) const;
    bool operator!=(const timeline_event &rhs) const;
  };

  struct timeline {
    std::vector<timeline_event> Events;
  };

  struct simulation_data {
    timing_t CurrentTiming;
    std::map<timing_t, executables_on_same_timing_container> ExecutablesPerTimingLevel;
    std::map<timing_t, std::set<entt::entity>> UpdateRequestsPerTiming;
    timeline Timeline;
    changes_context ChangesContext;
    void enqueue_update(entt::entity entity, timing_t timing);
    void record_intrinsic_attrs_change(entt::entity affected_entity);
  };
}