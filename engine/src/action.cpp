#include "action.h"
#include "game_logic.h"


namespace engine {
  
  void change_edit_history::record_status_edit(entt::id_type hash, Change<status_t> change){
    auto &hist = Statuses.emplace(hash, change_history_t<status_t>()).first->second;
    hist.add_modification_edit(change);
  }

  void change_edit_history::record_param_edit(entt::id_type hash, Change<parameter> change){
    auto &hist = Parameters.emplace(hash, change_history_t<parameter>()).first->second;
    hist.add_modification_edit(change);
  }

  void change_edit_history::record_status_change_suppression(entt::id_type hash, entt::entity committer){
    auto &hist = Statuses.emplace(hash, change_history_t<status_t>()).first->second;
    hist.add_suppression_edit(committer);
  }

  void change_edit_history::record_param_change_suppression(entt::id_type hash, entt::entity committer){
    auto &hist = Parameters.emplace(hash, change_history_t<parameter>()).first->second;
    hist.add_suppression_edit(committer);
  }

  attributes_info_cumulative_changes change_edit_history::create_cumul_changes(game_logic *game) const
  {
    attributes_info_cumulative_changes result;
    for(const auto &[hash, hist] : Statuses){
      std::optional<Change<status_t>> res = hist.merge_result(game->get_registry());
      if(res.has_value()){
        result.StatusesChanges.Changes.emplace(hash, res.value());
      }
    }

    for(const auto &[hash, hist] : Parameters){
      std::optional<Change<parameter>> res = hist.merge_result(game->get_registry());
      if(res.has_value()){
        result.ParamChanges.Changes.emplace(hash, res.value());
      }
    }

    return result;
  }
}
