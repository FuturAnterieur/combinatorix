#include "history_utils.h"

#include "entt_utils.h"

namespace logistics {
  void paste_changes_to_official_registry(entt::registry *registry, const attributes_info_cumulative_changes &changes, entt::entity entity){
      for(const auto &[hash, change] : changes.ParamChanges.Changes){
        assert(change.Diff.Type == param_diff_type::set_value);
        const auto& data = std::get<param_set_value_diff_data>(change.Diff.Data);
        auto &&storage = registry->storage<parameter>(hash);
        if(!data.HasValue){ //deletion
          storage.remove(entity);
        } else  { //modification
          parameter param;
          param = data.Value;
          utils::emplace_or_replace<parameter>(*registry, entity, hash, param);
        }
      }

      for(const auto &[hash, change] : changes.StatusesChanges.Changes){
        auto &&storage = registry->storage<void>(hash);
        if(change.Diff == smt::removed){
          storage.remove(entity);
        } else {
          if(!storage.contains(entity)) storage.emplace(entity);
        }
      }
    }
}
