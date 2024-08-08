#pragma once

#include "logistics/include/attributes_info.h"
#include "logistics/include/priority.h"
#include "engine/include/status_structs.h"
#include "engine_export.h"

#include <functional>

#include <optional>

namespace engine {
  class game_logic;
  struct pre_change_trigger_info;

  template<typename T>
  struct change_suppression_edit {
    std::function<bool(const Change<T> &change)> Filter;
    entt::entity CommitterId;
  };

  template<typename T>
  struct change_history_t {
    std::vector<Change<T>> ModificationEdits; //Change<T> already has a committer ID
    std::vector<change_suppression_edit<T>> SuppressionEdits;
    void add_suppression_edit(entt::entity committer_id, const std::function<bool(const Change<T> &change)>  &filter){
      SuppressionEdits.push_back(change_suppression_edit{filter, committer_id});
    }
    void add_modification_edit(const Change<T> &change){
      ModificationEdits.push_back(change);
    }
    
    //For now I don't have the notion of incremental changes, so a maximum of one change may be actually applied
    std::optional<Change<T>> merge_result(entt::registry *registry) const{ // argument could be replaced by game_logic
      priority_request req;

      size_t all_edits_size = ModificationEdits.size() + SuppressionEdits.size();
      std::vector<priority_t> all_priorities(all_edits_size, 0);
      size_t counter = 0;
      for(const auto &edit : ModificationEdits){
        req.EntitiesWithResultingPriorityValues.push_back(std::make_pair(edit.CommitterId, &all_priorities[counter]));
        counter++;
      }
      for(const auto &edit : SuppressionEdits){
        req.EntitiesWithResultingPriorityValues.push_back(std::make_pair(edit.CommitterId, &all_priorities[counter]));
        counter++;
      }

      classic_priority_callback(req, registry);

      std::optional<Change<T>> result = std::nullopt;
      
      //filter out modification edits
      //For non-incremental parameters, keep the highest priority only

      //anyway incremental parameters would be calculated differently, because we still want only one change to be outputted for a single timing
      priority_t highest_modif_prio = std::numeric_limits<priority_t>::min();
      for(size_t i = 0; i < ModificationEdits.size(); i++){
        priority_t prio_val = all_priorities[i];
        bool banned = false;
        int suppress_counter = ModificationEdits.size();
        for(const auto &suppress_edit : SuppressionEdits){
          priority_t suppress_prio = all_priorities[suppress_counter];
          bool caught_by_filter = suppress_edit.Filter(ModificationEdits.at(i));
          if(suppress_prio > prio_val && ModificationEdits.at(i).CommitterId != suppress_edit.CommitterId && caught_by_filter){
            banned = true; 
            break;
          }
          suppress_counter++;
        }

        if(!banned && prio_val > highest_modif_prio){
          highest_modif_prio = prio_val;
          result.emplace(ModificationEdits.at(i));
        }
      }

      return result;
    }
  };

  class change_edit_history {
    private:
      std::map<entt::id_type, change_history_t<status_t>> Statuses;
      std::map<entt::id_type, change_history_t<parameter>> Parameters;
    public:
      //TODO offer API functions in game_logic
      void engine_API record_status_edit(entt::id_type hash, Change<status_t> change);
      void engine_API record_param_edit(entt::id_type hash, Change<parameter> change);
      void engine_API record_status_change_suppression(entt::id_type hash, entt::entity committer, const std::function<bool( const Change<status_t> &change)> &filter);
      void engine_API record_param_change_suppression(entt::id_type hash, entt::entity committer, const std::function<bool(const Change<parameter> &change)> &filter);
      attributes_info_cumulative_changes create_cumul_changes(game_logic *game) const;
  };


 
  using pre_change_trigger_filter_t = std::function<bool(game_logic *, const attributes_info_changes &detailed_changes, const attributes_info_cumulative_changes &requested_changes, entt::entity, const pre_change_trigger_info &info)>;
  using pre_change_trigger_func_t = std::function<void(game_logic *, const attributes_info_changes &detailed_changes, const attributes_info_cumulative_changes &requested_changes, entt::entity, const pre_change_trigger_info &info, change_edit_history &io_hist)>;
  
  struct engine_API pre_change_trigger_info {
    pre_change_trigger_func_t Func;
    pre_change_trigger_filter_t Filter;
    entt::entity Owner;
  };

  struct engine_API pre_change_triggers_affecting {
    std::list<entt::entity> TriggerEntities;
  };
}
