#include "engine/include/simulation_data.h"

namespace engine{
  
  void simulation_data::enqueue_update(entt::entity entity_to_update, timing_t timing){
    timing_t absolute_launch_time = timing + CurrentTiming;
    auto &requests = UpdateRequestsPerTiming.emplace(absolute_launch_time, std::set<entt::entity>()).first->second;
    
    auto it = requests.find(entity_to_update);
    if(it != requests.end()) {
      return;
    }
    requests.insert(entity_to_update);
    
    entity_update_executable executable{entity_to_update};
    
    auto &container = ExecutablesPerTimingLevel.emplace(absolute_launch_time, executables_on_same_timing_container{}).first->second;
    container.AbsoluteTiming = absolute_launch_time; //should be no-op if it was found
    container.Executables.emplace_back(executable_common_data{executable_type::update, entity_to_update, executable});
  }

  //===================================
  void simulation_data::record_intrinsic_attrs_change(entt::entity affected_entity){
    Timeline.Events.push_back(timeline_event{event_type::change_intrinsics, ChangesContext.OriginatingEntity, affected_entity, CurrentTiming});
  }
}