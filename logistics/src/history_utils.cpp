#include "history_utils.h"

#include "entt_utils.h"

namespace logistics {
  void paste_changes_to_official_registry(entt::registry *registry, const attributes_info_short_changes &changes, entt::entity entity){
      for(const auto &[hash, param] : changes.ModifiedParams){
        auto &&storage = registry->storage<parameter>(hash);
        if(param.dt() == data_type::null){ //deletion
          storage.remove(entity);
        } else  { //modification
          utils::emplace_or_replace<parameter>(*registry, entity, hash, param);
        }
      }

      for(const auto &[hash, smt_val] : changes.ModifiedStatuses){
        auto &&storage = registry->storage<void>(hash);
        if(smt_val == smt::removed){
          storage.remove(entity);
        } else {
          if(!storage.contains(entity)) storage.emplace(entity);
        }
      }
    }
}
