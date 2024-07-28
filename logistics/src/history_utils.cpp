#include "history_utils.h"

#include "entt_utils.h"

namespace logistics {
  void paste_changes_to_official_registry(entt::registry *registry, const attributes_info_cumulative_changes &changes, entt::entity entity){
      for(const auto &[hash, change] : changes.ParamChanges){
        auto &&storage = registry->storage<parameter>(hash);
        if(change.Diff.dt() == data_type::null){ //deletion
          storage.remove(entity);
        } else  { //modification
          utils::emplace_or_replace<parameter>(*registry, entity, hash, change.Diff);
        }
      }

      for(const auto &[hash, change] : changes.StatusesChanges){
        auto &&storage = registry->storage<void>(hash);
        if(change.Diff == smt::removed){
          storage.remove(entity);
        } else {
          if(!storage.contains(entity)) storage.emplace(entity);
        }
      }
    }
}
