#include "engine/include/simulation_data.h"
#include "engine/include/game_logic.h"
#include "logistics/include/floyd.h"

namespace engine{
  
  bool timeline_event::operator==(const timeline_event &rhs) const {
    return Type == rhs.Type && OriginatingEntity == rhs.OriginatingEntity && AffectedEntity == rhs.AffectedEntity;
  }

  bool timeline_event::operator!=(const timeline_event &rhs) const {
    return Type != rhs.Type || OriginatingEntity != rhs.OriginatingEntity || AffectedEntity != rhs.AffectedEntity;
  }

  void simulation_data::enqueue_update(entt::entity entity_to_update, timing_t timing, game_logic *eng){
    timing_t absolute_launch_time = timing + CurrentTiming;
    auto &requests = UpdateRequestsPerTiming.emplace(absolute_launch_time, std::set<entt::entity>()).first->second;
    
    auto it = requests.find(entity_to_update);
    if(it != requests.end()) {
      return;
    }
    requests.insert(entity_to_update);
    
    entity_update_executable executable{eng, entity_to_update};
    
    auto &container = ExecutablesPerTimingLevel.emplace(absolute_launch_time, executables_on_same_timing_container{}).first->second;
    container.AbsoluteTiming = absolute_launch_time; //should be no-op if it was found
    container.Executables.emplace_back(executable_common_data{executable_type::update, entity_to_update, executable});
  }

  //===================================
  void simulation_data::enqueue_trigger(const on_status_change_trigger_info &info, entt::entity entity, const attributes_info_changes &changes, game_logic *eng){
    timing_t absolute_trigger_launch_time = info.TimeDelta + CurrentTiming;
    status_trigger_executable executable{eng, info, changes, entity};

    auto &container = ExecutablesPerTimingLevel.emplace(absolute_trigger_launch_time, executables_on_same_timing_container{}).first->second;
    container.AbsoluteTiming = absolute_trigger_launch_time; //should be no-op if it was found
    container.Executables.emplace_back(executable_common_data{executable_type::status_trigger, info.TriggerOwner, executable});
  }

  //===================================
  void simulation_data::record_intrinsic_attrs_change(entt::entity affected_entity){
    Timeline.Events.push_back(timeline_event{event_type::change_intrinsics, ChangesContext.OriginatingEntity, affected_entity, CurrentTiming});
  }

  //===================================
  void simulation_data::record_status_effect_change(entt::entity affected_entity){
    Timeline.Events.push_back(timeline_event{event_type::change_status_effects, ChangesContext.OriginatingEntity, affected_entity, CurrentTiming});
  }

  //===================================
  void simulation_data::run_one_timing(){
    bool cycle_found = false;

    auto it = ExecutablesPerTimingLevel.find(CurrentTiming);
    if(it == ExecutablesPerTimingLevel.end()){
      CurrentTiming++;
      return;
    }

    while(!it->second.Executables.empty()) {
      auto exec = it->second.Executables.front();
      it->second.Executables.pop_front();
      exec.Func();
      if(exec.ExecType == executable_type::update){
        UpdateRequestsPerTiming.at(CurrentTiming).erase(exec.UpdatedEntity);
        Timeline.Events.push_back(timeline_event{event_type::update, exec.UpdatedEntity, exec.UpdatedEntity, CurrentTiming});
      }

      
      size_t start, end;
      if(floyd::find_cycle(Timeline.Events, 0, start, end)){
        cycle_found = true;
        EndTiming = CurrentTiming;
        break;
        //put that in a callback controlled by game_logic
        /*cycle_found = true;
        std::vector<entt::entity> competing_entities;
        std::vector<timing_t> timings;
        std::vector<priority_t> priorities;

        for(size_t i = start; i < end; i++){
          if(Timeline.Events[i].Type != event_type::update){
            competing_entities.push_back(Timeline.Events[i].OriginatingEntity);
            priorities.push_back(0);
            timings.push_back(Timeline.Events[i].Timing);
          }
        }
        assert(competing_entities.size() == 2);
        priority_request req{{std::make_pair(competing_entities.front(), &priorities.front()), std::make_pair(competing_entities.back(), &priorities.back())}};
        //classic_priority_callback(req, eng);
        if(priorities.front() > priorities.back()){
          EndTiming = timings.front() + 1;
        } else {
          EndTiming = timings.back() + 1;
        }
        break;*/
      }
    }

    ExecutablesPerTimingLevel.erase(it);
    if(cycle_found || ExecutablesPerTimingLevel.empty()){
      Finished = true;
    }
    CurrentTiming++;
  
  }
}